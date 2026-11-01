/*
 * Optiboot Mini STK500v1 multi-device experimental port, October 2026.
 * Default ATmega328P / 16MHz / UART0 115200 8N1; target via -mmcu and device includes.
 * Boot origin/footer are calculated from FLASH_SIZE and BOOT_BYTES.
 * Experimental: see README_MINI_MULTI.md for configuration-specific hardware coverage.
 *
 * Fixes vs the 512B Optiboot Mini: checks BOTH length bytes for
 * STK_READ_PAGE/STK_PROG_PAGE, supports valid read length 0x0100=256,
 * refuses unknown memory types with INSYNC+FAILED instead of touching FLASH,
 * implements STK_GET_SIGN_ON with AVR061's "AVR STK" reply.
 *
 * Uses Y as stable USART0 base for modern parts; legacy parts use IN/OUT.
 * For 128/256 KiB Flash: r3 holds upper byte-address bits, RAMPZ drives
 * ELPM/SPM, and full-bank overflow and boot-sector writes are rejected.
 * 128/256 KiB targets use 1024-byte boot sections (not limited to 512 B).
 * Shares two-byte receive and common error responses across handlers.
 * Keeps avrdude -c arduino format, read/write FLASH, 1..16B EEPROM writes,
 * EEPROM read, read-only UNIVERSAL fuse/lock commands, WDT and SPM interlock.
 * Preserves Optiboot's ACK without true Chip Erase for legacy avrdude.
 * Rejects overflow in LOAD_ADDRESS, restores r1 and UBRR0H; validates FLASH read ends.
 * r1 is cleared at boot entry. Match BOOTSZ to the emitted build manifest.
 * Full AVR061 is NOT implemented. Every new MCU port requires hardware testing.
 *
 * Based on Optiboot, Copyright (c) Peter Knight, Bill Westfield et al.
 * GPL-2.0-or-later; see the repository LICENSE. Hardware testing and ISP recovery required.
 */

#include "devices/select.inc"
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#ifndef BAUD_RATE
#define BAUD_RATE 115200UL
#endif
#include "uart/select.inc"
#ifndef MINI_U2X
#define MINI_U2X 1
#endif
#ifndef MINI_UBRR
#define MINI_UBRR (((F_CPU + 4UL * BAUD_RATE) / (8UL * BAUD_RATE)) - 1)
#endif
#if MINI_UBRR < 0 || MINI_UBRR > 4095
#error Unsupported UART divider
#endif
#if FLASH_SIZE > 262144
#error This implementation addresses at most 256 KiB of Flash
#endif
#if MINI_FLASH_BANKED && ((FLASH_SIZE != 131072) && (FLASH_SIZE != 262144))
#error RAMPZ addressing is supported for 128 or 256 KiB devices
#endif
#if MINI_FLASH_256K && FLASH_SIZE != 262144
#error The STK_UNIVERSAL extended address port requires 256 KiB
#endif
/* AVRDUDE releases before its 2022 STK500v1 fix can use BYTE addresses
 * for EEPROM on ATmega64A/ATmega128A, unlike the normal WORD convention.
 * This build-time variant is explicit because the two host conventions
 * cannot be reliably inferred from the 16-bit LOAD_ADDRESS packet.
 */
#ifndef MINI_OLD_AVRDUDE_EEPROM
#define MINI_OLD_AVRDUDE_EEPROM 0
#endif

.section .text
.global _start
_start:
    ; No app-callable SPM entry; the extra RET stub is removed to make room for safety checks.

/* Register allocation:
 * r1  = cleared on entry; later used as HIGH data byte during Flash fill
 * r2  = copy of MCUSR for application startup
 * r3  = Flash 64-KiB bank index (RAMPZ), when MINI_FLASH_BANKED
 * r4  = high word-address bit for 256-KiB STK500v1 LOAD_EXT_ADDR
 * r29 = zero from boot entry; must remain zero (Y=0x00C0 on modern USART)
 * r16 = UART byte/input/output value
 * r17 = watchdog configuration value
 * r18 = USART status/temp
 * r19 = shared discard count / EEPROM receive count / SPMEN preload (1)
 * r20 = GET_PARAMETER selector, READ_PAGE memory type, ISP opcode1
 * r21 = UNIVERSAL ISP opcode2 (bit 3 mapped into Z via T flag)
 * r22:r23 = low 16 bits of the current BYTE address (Flash or EEPROM)
 * r25 = page byte length for READ_PAGE, word count for Flash PROG_PAGE
 * X = EEPROM READ pointer; EEPROM WRITE payload is held on AVR stack
 * Y = UART base on modern parts, zero-high register on legacy devices
 * Z = flash address, page-fill address, or EEPROM last-byte address
 */
boot_start:
    cli
#if MINI_LEGACY_STACK
    ; Older AVR devices do not initialize SP automatically on reset.
    ldi r16, (SRAM_END & 0xFF)
    out 0x3D, r16
    ldi r16, ((SRAM_END >> 8) & 0xFF)
    out 0x3E, r16
#endif
    clr r1                 ; AVR zero register restored for application
    clr r29                ; zero register available from very first instruction
#if MINI_FLASH_BANKED
    clr r3                 ; current Flash byte-address bank index (RAMPZ)
    out RAMPZ_IO,r3
#endif
#if MINI_FLASH_256K
    clr r4                 ; extended word address (from STK_UNIVERSAL 4D)
#endif
    in   r16, MCUSR_IO     ; MCUSR varies on ATtiny1634
    mov  r2, r16
    out  MCUSR_IO, r29     ; clear reset reason (r29=0)
    wdr
    clr  r17
    rcall wdt_config        ; disable watchdog on application start
    tst  r16              ; cleared MCUSR = explicit software bootloader entry
    breq start_uart
    andi r16, 0x0A         ; WDRF | EXTRF
    cpi  r16, 0x02         ; enter only on external reset without WDRF
    breq start_uart
#if MINI_VIRTUAL_BOOT
    ; No BOOTRST fuse: reset-vector has been patched by tiny/patch_vectors.py.
    ; The reserved EEPROM-ready vector holds a relocated RJMP into the app.
    ; Jump through the end of FLASH so the short RJMP wraps to the vector.
    rjmp boot_end + (MINI_APP_VECTOR_WORD * 2)
#elif MINI_FLASH_BANKED || (BOOT_BYTES > 4096)
    jmp 0                  ; direct reset vector (also needed for 8KiB boot on 64KiB AVR)
#else
    rjmp boot_end          ; PC wraps from the end of Flash to address zero
#endif

start_uart:
    MINI_UART_INIT
    ldi  r17, 0x0E         ; WDT 1s: WDE|WDP2|WDP1
    rcall wdt_config

/* EEPE can remain busy after an external reset while an application
 * EEPROM write is finishing.  Datasheet 26.8.8 prohibits SPMCSR writes
 * (including fuse reads) until EEPE clears.  Poll at every new packet.
 * Internal EEPROM writes already finish before STK_OK, so this replaces
 * the duplicate wait previously inside eeprom_read without size growth.
 */
next_command:
.Lwait_eepe_before_command:  sbic EEPROM_EECR_IO, 1           ; wait EEPE before any SPM/BLBSET operation
    rjmp .Lwait_eepe_before_command
    rcall rx_char
    cpi  r16, 0x31         ; AVR061 GET_SIGN_ON
    breq command_signon
  cpi  r16, 0x41         ; GET_PARAMETER
    breq command_parameter
    ldi  r19,2            ; candidate count for SET_PARAMETER
    cpi  r16, 0x40
    breq command_discard
    ldi  r19,20           ; candidate count for SET_DEVICE
    cpi  r16, 0x42
    breq command_discard
    ldi  r19,5            ; candidate count for SET_DEVICE_EXT
    cpi  r16, 0x45
    breq command_discard
    cpi  r16, 0x55         ; LOAD_ADDRESS
    breq command_address
.Ldispatch_universal:  cpi  r16, 0x56         ; UNIVERSAL
    breq universal_handler
  cpi  r16, 0x64         ; PROG_PAGE
    brne .Ldispatch_read_page
    rjmp command_write
.Ldispatch_read_page:  cpi  r16, 0x74         ; READ_PAGE
#if MINI_FLASH_256K
    brne .Ldispatch_read_signature
    rjmp command_read
#else
    breq command_read
#endif
.Ldispatch_read_signature:  cpi  r16, 0x75         ; READ_SIGN
    breq command_signature
.Ldispatch_leave_program_mode:  cpi  r16, 0x51         ; LEAVE_PROGMODE
    brne command_default
    rcall wdt_16ms        ; 16ms WDT with WDE
command_default:
    rcall verify_eop       ; SYNC, ENTER, LEAVE and other no-payload cmds
command_ok:
    ldi  r16, 0x10         ; STK_OK
command_finish:
    rcall tx_char
    rjmp next_command


command_signature:
    ldi r25,3
    ldi r30,((FLASH_SIZE - 12) & 0xFF)         ; absolute 0x7FF4, 3 signature bytes
    rjmp static_bytes
command_signon:
    ldi r25,7
    ldi r30,((FLASH_SIZE - 9) & 0xFF)         ; absolute 0x7FF7, "AVR STK"
static_bytes:
    clr r20              ; select LPM branch in every iteration
    ldi r31,(((FLASH_SIZE - 1) >> 8) & 0xFF)
    rcall verify_eop
#if MINI_FLASH_BANKED
    ldi r16,((FLASH_SIZE - 1) >> 16)  ; footer bank = 1 for 128K, 3 for 256K
    out RAMPZ_IO,r16
#endif
    rjmp read_loop

/* UNIVERSAL: read-only ISP emulation for ATmega328P.
 * 50 00 xx yy low fuse; 50 08 xx yy ext fuse
 * 58 00 xx yy lock bits; 58 08 xx yy high fuse
 * Third/fourth ISP bytes are ignored for these read commands.
 * Strictly recognises only these four opcode pairs; other commands
 * (including 0x30 signature requests and 0xAC ISP writes) return 0.
 * Signature is already supplied via the standard STK_READ_SIGN (0x75).
 */
command_parameter:
    rcall rx_char
    mov  r20, r16
    rcall verify_eop
    ldi  r16, 3            ; non-version parameter
    cpi  r20, 0x81         ; software major version
    brne .Lparameter_version_ready
    ldi  r16, 8
.Lparameter_version_ready:  rjmp universal_response  ; common payload byte then STK_OK

/* Shared discard for SET_PARAMETER (2), SET_DEVICE (20), SET_DEVICE_EXT (5). */
command_discard:
    rcall discard_and_verify
    rjmp command_ok


command_address:
    rcall read_length     ; receive low/high word bytes into r24:r25
    movw r22, r24         ; low/high bytes into address register pair
    lsl  r22               ; word -> byte address
    rol  r23
#if MINI_FLASH_BANKED
    clr  r3
    adc  r3, r29          ; word bit 15 becomes byte-address bit 16
#if MINI_FLASH_256K
    bst  r4,0             ; 4D 00 <bank> 00 -> 17th word-address bit
    bld  r3,1             ; 18th byte-address bit / RAMPZ bit 1
#endif
#elif MINI_SPM_EXT
    brcc .Lme64_error_skip_0
    rjmp protocol_error
.Lme64_error_skip_0:
#else
    brcs protocol_error   ; reject word address >= 0x8000, avoid 16-bit wrap
#endif
    rjmp command_default

/* The high byte matters: READ_PAGE allows up to 256 bytes (0x0100).
 * 0x0000 retains legacy Optiboot's 256-byte interpretation.
 */
universal_handler:
    rcall read_length      ; first two opcode bytes
#if MINI_FLASH_256K
    ; AVRdude STK500v1 extension: 56 4D 00 <word_addr[23:16]> 00 20
    ; 256 KiB has only one extended word-address bit.  Reject other values
    ; rather than allowing wrap-around into another writable page.
    cpi r24,0x4D
    brne .Lordinary_universal
    tst r25
    brne .Lordinary_universal
    rcall rx_char
    cpi r16,2
    brlo .Lvalid_extended_word_address
    rjmp protocol_error
.Lvalid_extended_word_address:
    mov r4,r16
    rcall rx_char          ; trailing ISP byte must be zero
    tst r16
    breq .Lvalid_extended_opcode_tail
    rjmp protocol_error
.Lvalid_extended_opcode_tail:
    rcall verify_eop
    clr r16
    rjmp universal_response
.Lordinary_universal:
#endif
    ldi r19,2
    rcall discard_and_verify
    clr r16                   ; unsupported ISP command -> 0
    bst  r24,3
    andi r24,0xF7
    cpi  r24,0x50
    brne universal_response
    clr  r30
    bld  r30,0
    bst  r25,3
    andi r25,0xF7
    brne universal_response
    bld  r30,1
    clr  r31
    ldi  r16,0x09         ; BLBSET|SPMEN, followed immediately by LPM
    #if MINI_SPM_EXT
    sts  SPMCSR_MEM,r16
#else
    out  SPMCSR_IO,r16
#endif
    lpm  r16,Z
universal_response:
    rcall tx_char
    rjmp command_ok


command_read:
    rcall read_length
    subi r24,1           ; high length 0 accepts 0..255, 1 accepts only low=0
    brcs .Lread_length_valid              ; original high byte 0
    or   r24,r25         ; (high-1)|low must be zero for 0x0100
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    breq .Lme64_error_skip_1
    rjmp protocol_error
.Lme64_error_skip_1:
#else
    brne protocol_error
#endif
.Lread_length_valid:  rcall rx_char
    mov r20,r16
    rcall verify_eop
#if FLASH_SIZE >= 65536
    clr r18              ; sentinel: complete 64/128 KiB Flash range
#else
    ldi r18,(FLASH_SIZE >> 8)
#endif
    cpi r20,0x46         ; Flash
    breq read_check_common
    ldi r18,(EEPROM_SIZE >> 8)
    cpi r20,0x45         ; EEPROM
#if MINI_FLASH_128K
    breq .Lread_memtype_good
    rjmp read_bad_memtype
.Lread_memtype_good:
#else
    brne read_bad_memtype
#endif
#if MINI_OLD_AVRDUDE_EEPROM
    movw r26,r22           ; preserve standard word-derived address
    lsr  r23               ; old host supplied EEPROM BYTE address
    ror  r22               ; undo LOAD_ADDRESS's x2 conversion
#endif
read_check_common:
    rcall memory_range_check
#if MINI_FLASH_BANKED
    out RAMPZ_IO,r3      ; ELPM Z+ increments RAMPZ across 64-KiB boundaries
#endif
    movw r30,r22
read_loop:
    sbrc r20,0           ; E odd; F even
    rjmp read_eeprom_byte
#if MINI_FLASH_BANKED
    elpm r16,Z+
#else
    lpm r16,Z+
#endif
    rjmp read_send
read_eeprom_byte:
    out EEPROM_EEARL_IO,r30
    out EEPROM_EEARH_IO,r31
    sbi EEPROM_EECR_IO,0
    in r16,EEPROM_EEDR_IO
    adiw r30,1
read_send:
    rcall tx_char
    dec r25
    brne read_loop
#if MINI_OLD_AVRDUDE_EEPROM
    sbrc r20,0            ; restore Flash/word-derived internal address only for E
    movw r22,r26
#endif
    rjmp command_ok
verify_eop:
    rcall rx_char
    cpi  r16, 0x20         ; CRC_EOP = ' '
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    breq .Lme64_error_skip_2
    rjmp protocol_error
.Lme64_error_skip_2:
#else
    brne protocol_error
#endif
    ldi  r16, 0x14         ; STK_INSYNC
    rjmp tx_char           ; tail call: tx_char returns to caller of verify_eop

/* Reply with static program-memory signature / sign-on string.
 * Both are at the end of the boot section, immediately before version.
 * Reuses READ_PAGE's LPM/putch loop, with EEPROM selector cleared.
 */
protocol_error:
    rcall wdt_16ms         ; 16ms watchdog
error_loop:
    rjmp error_loop



memory_range_check:
    movw r30, r22
    mov  r16, r25
    dec  r16
    add  r30, r16
    adc  r31, r29         ; YH is zero in every supported register profile
#if MINI_FLASH_BANKED
    ; r18=0: banked Flash; r18=EEPROM_SIZE>>8: EEPROM.
    ; Addition wrapping at 0xFFFF is valid ONLY for bank-0 Flash reads.
    brcs .Lrange_carry
    tst r18
    breq .Lrange_pass       ; no 16-bit carry -> valid in either Flash bank
    tst r3
    brne .Lrange_reject     ; EEPROM is always in bank zero
    cp r31,r18
    brlo .Lrange_pass
.Lrange_reject:
    rjmp protocol_error
.Lrange_carry:
    tst r18
    brne .Lrange_reject     ; EEPROM 16-bit wrap is always invalid
#if MINI_FLASH_256K
    mov r16,r3
    cpi r16,3
    breq .Lrange_reject     ; bank 3 wraps beyond 0x3FFFF
#else
    tst r3
    brne .Lrange_reject    ; bank 1 wraps beyond 0x1FFFF
#endif
.Lrange_pass:
    ret
#elif MINI_SPM_EXT
    brcc .Lme64_error_skip_3
    rjmp protocol_error
.Lme64_error_skip_3:
#else
    brcs protocol_error
#endif
#if FLASH_SIZE == 65536
    tst r18              ; zero sentinel skips upper bound check for full 64 KiB Flash
    breq .Lrange_ok
#endif
    cp   r31, r18
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    brlo .Lme64_error_skip_4
    rjmp protocol_error
.Lme64_error_skip_4:
#else
    brsh protocol_error
#endif
#if FLASH_SIZE == 65536
.Lrange_ok:
#endif
    ret

command_write:
    rcall read_length
#if FLASH_PAGE_BYTES == 256
    ; Accept 1..255 bytes or exactly 0x0100 (a whole 256-byte page).
    tst r24
    breq .Lwrite_high_zero
    cpi r24,1
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    breq .Lme64_error_skip_5
    rjmp protocol_error
.Lme64_error_skip_5:
#else
    brne protocol_error
#endif
    tst r25
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    breq .Lme64_error_skip_6
    rjmp protocol_error
.Lme64_error_skip_6:
#else
    brne protocol_error
#endif
    rjmp .Lwrite_length_ok
.Lwrite_high_zero:
    tst r25
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    brne .Lme64_error_skip_7
    rjmp protocol_error
.Lme64_error_skip_7:
#else
    breq protocol_error
#endif
.Lwrite_length_ok:
#else
    tst r24
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    breq .Lme64_error_skip_8
    rjmp protocol_error
.Lme64_error_skip_8:
#else
    brne protocol_error
#endif
    tst r25
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    brne .Lme64_error_skip_9
    rjmp protocol_error
.Lme64_error_skip_9:
#else
    breq protocol_error
#endif
#endif
    rcall rx_char          ; memory type, accept all except 'E'
    cpi  r16, 0x45        ; EEPROM write: stage on stack, validate EOP
    breq eeprom_write
    cpi r16,0x46           ; accept only F otherwise
    breq flash_write
    ; Invalid memtype: drain this payload, acknowledge framing, return FAILED.
    ; This cannot modify flash/EEPROM and does not de-sync a proper host.
    mov r19,r25
    rcall discard_and_verify
read_bad_memtype:
    ldi r16,0x11         ; INSYNC was already sent, report STK_FAILED
    rjmp command_finish
flash_write:
#if MINI_UART_STYLE == 4
    ; ATtiny1634 erases an ENTIRE 4-page group (4 x 32 = 128 bytes).
    ; Never erase on each 32-byte write: doing so destroys the other 3 pages.
    ; Require one 128-byte STK_PROG_PAGE block, stage it in SRAM stack,
    ; verify CRC_EOP, erase once, then fill/write four 32-byte pages.
    ; Stock AVRDUDE typically sends 32-byte pages; a special sender is needed.
    tst r24                   ; require exactly 0x0080 bytes
    breq .Ltiny_high_ok
    rjmp protocol_error
.Ltiny_high_ok:
    cpi r25,128
    breq .Ltiny_length_ok
    rjmp protocol_error
.Ltiny_length_ok:
    mov r16,r22
    andi r16,127             ; 128-byte group aligned
    breq .Ltiny_align_ok
    rjmp protocol_error
.Ltiny_align_ok:
    cpi r23,(BOOT_START >> 8)
    brlo .Ltiny_boot_safe
    rjmp protocol_error
.Ltiny_boot_safe:
    ldi r19,128
.Ltiny_receive_group:
    rcall rx_char
    push r16                 ; 128 bytes on normal SRAM stack
    dec r19
    brne .Ltiny_receive_group
    rcall verify_eop          ; malformed packet never erases Flash
    movw r30,r22
    ldi r16,3                ; erase all four pages once
    rcall spm_execute_wait
    movw r30,r22
    ldi r16,128
    add r30,r16              ; Z = start+128, fill backwards
    adc r31,r29
    ldi r20,4                ; pages remaining
.Ltiny_next_page:
    ldi r25,16               ; 16 words per physical page
.Ltiny_next_word:
    sbiw r30,2
    pop r1                   ; reverse staging: high byte first
    pop r0
    ldi r16,1
    out SPMCSR_IO,r16
    spm
    dec r25
    brne .Ltiny_next_word
    ldi r16,5
    rcall spm_execute_wait   ; writes this 32-byte page, clears temp buffer
    dec r20
    brne .Ltiny_next_page
    rjmp command_ok
#else
#if FLASH_PAGE_BYTES == 256
    tst r24
    breq .Lflash_short_page
    ldi r25, 128           ; full 0x0100-byte page
    rjmp .Lflash_length_ready
.Lflash_short_page:
#endif
    lsr r25                ; convert even bytes to words
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    brcc .Lme64_error_skip_10
    rjmp protocol_error
.Lme64_error_skip_10:
#else
    brcs protocol_error
#endif
    cpi r25, ((FLASH_PAGE_BYTES / 2) + 1)
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    brlo .Lme64_error_skip_11
    rjmp protocol_error
.Lme64_error_skip_11:
#else
    brsh protocol_error
#endif
#if FLASH_PAGE_BYTES == 256
.Lflash_length_ready:
#endif
    ; Datasheet 26.8.3: Page Write requires PCWORD=0 in Z.
    ; Reject a non-page-aligned start, preventing SPM address misuse
    ; and a payload spanning two different physical flash pages.
    mov  r16, r22
    andi r16, (FLASH_PAGE_BYTES - 1)  ; page-aligned address
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    breq .Lme64_error_skip_12
    rjmp protocol_error
.Lme64_error_skip_12:
#else
    brne protocol_error
#endif
    movw r30, r22
#if MINI_FLASH_BANKED
    out RAMPZ_IO,r3      ; required for SPM temporary-buffer fill / erase / write
#endif
    ldi  r19, 0x01         ; preloaded SPMEN across whole page (saves cycles)

/* Fill the target AVR's hardware temporary page buffer BEFORE flash erase.
 * Atmel datasheet 30.8 explicitly allows SPM fill -> page erase -> write.
 * A bad CRC_EOP cannot destroy old Flash: no page erase has happened.
 * Unlike the SRAM version this uses no 128-byte software buffer.
 */
hardware_fill_loop:
    rcall rx_char
    mov  r0, r16
    rcall rx_char
    mov  r1, r16
    #if MINI_SPM_EXT
    sts SPMCSR_MEM,r19
#else
    out SPMCSR_IO,r19
#endif
    spm
    adiw r30, 2
    dec  r25
    brne hardware_fill_loop
    ; r1 now contains the last Flash word high byte. No later code
    ; uses r1 as its zero register; r29 (YH) remains zero permanently.
    rcall verify_eop      ; STK_INSYNC must precede actual erase
#if MINI_FLASH_BANKED
    ; Reject only the last physical Flash bank above BOOT_START.
    ; Other banks and addresses below BOOT_START are writable.
#if MINI_FLASH_256K
    mov r16,r3
    cpi r16,3
    brne .Lboot_target_safe
#else
    tst r3
    breq .Lboot_target_safe
#endif
    cpi r23, ((BOOT_START >> 8) & 0xFF)
    brlo .Lboot_target_safe
    rjmp protocol_error
.Lboot_target_safe:
#else
    cpi  r23, (BOOT_START >> 8) ; reject addresses within boot section
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    brlo .Lme64_error_skip_13
    rjmp protocol_error
.Lme64_error_skip_13:
#else
    brsh protocol_error
#endif
#endif

    movw r30, r22
    ldi  r16, 0x03        ; erase, hardware temp page buffer retained!
    rcall spm_execute_wait
    ldi  r16, 0x05        ; write temporary page buffer to Flash
    rcall spm_execute_wait
    ldi  r16, 0x11        ; RWWSRE|SPMEN
    rcall spm_execute_wait
    rjmp command_ok
#endif /* MINI_UART_STYLE == 4 */

/* EEPROM write: push 1..16 received bytes onto normal AVR hardware stack.
 * avrdude -c arduino sends EEPROM data in 16-byte blocks for ATmega328P.
 * CRC_EOP is validated BEFORE any EEPROM writes. Then POP bytes in LIFO
 * order and program addresses from the block's end down to its start.
 * Incomplete or incorrect packets do not modify EEPROM; sudden power loss
 * during commit can still leave a partially written EEPROM block.
 * No dedicated 16-byte array in SRAM; peak stack usage increases by 16B.
 * Valid EEPROM address range 0x000..0x3FF (inclusive) is enforced.
 */
eeprom_write:
#if FLASH_PAGE_BYTES == 256
    tst r24               ; EEPROM blocks never use the 256-byte form
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    breq .Lme64_error_skip_14
    rjmp protocol_error
.Lme64_error_skip_14:
#else
    brne protocol_error
#endif
#endif
    cpi r25,17
#if MINI_SPM_EXT || FLASH_PAGE_BYTES == 256 || MINI_UART_STYLE == 4
    brlo .Lme64_error_skip_15
    rjmp protocol_error
.Lme64_error_skip_15:
#else
    brsh protocol_error
#endif
#if MINI_OLD_AVRDUDE_EEPROM
    movw r26,r22          ; save word-derived address for subsequent commands
    lsr  r23              ; reconstruct host's EEPROM byte address
    ror  r22
#endif
    ldi  r18, (EEPROM_SIZE >> 8)
    rcall memory_range_check
    mov  r19, r25         ; keep commit count; use r19 for input loop
.Leeprom_receive_next:  rcall rx_char
    push r16              ; stage byte in stack, no dedicated 16-byte SRAM array
    dec  r19
    brne .Leeprom_receive_next
    rcall verify_eop      ; validate packet before modifying any EEPROM
/* Z = last EEPROM byte, retained from eeprom_range_check.
 * Pop bytes in reverse order, write from end of block backwards.
 * RCALL/RET in receiver leave the staged bytes untouched.
 */
    rcall wait_spm        ; ensure Flash self-programming has finished
    ; At each command entry we already wait until EEPE clears. There is
    ; no other EEPROM writer (interrupts disabled), so the first write
    ; starts with EEPE clear. Wait AFTER each write, including the last.
.Leeprom_commit_next:  out EEPROM_EECR_IO,r29        ; EECR=0: YH=0 after USART init
    out EEPROM_EEARL_IO,r30        ; EEARL
    out EEPROM_EEARH_IO,r31        ; EEARH
    pop  r16
    out EEPROM_EEDR_IO,r16        ; EEDR
    sbi EEPROM_EECR_IO,2          ; EEMPE, 4 CPU cycles to trigger EEPE
    sbi EEPROM_EECR_IO,1          ; EEPE, start 3.4ms atomic erase/write
    sbiw r30,1
    dec  r25             ; Z flag is retained by SBIC/RJMP
.Leeprom_wait_write_complete:  sbic EEPROM_EECR_IO,1          ; every write complete before next/ACK
    rjmp .Leeprom_wait_write_complete
    brne .Leeprom_commit_next
#if MINI_OLD_AVRDUDE_EEPROM
    movw r22,r26          ; restore standard internal address for next command
#endif
    rjmp command_ok


/* Shared discard and EOP verification: SET, invalid memory and UNIVERSAL. */
discard_and_verify:
.Ldiscard_next_byte:  rcall rx_char
    dec r19
    brne .Ldiscard_next_byte
    rjmp verify_eop

read_length:
    rcall rx_char          ; high length byte
    mov  r24, r16
    rcall rx_char          ; low length byte
    mov  r25, r16
    ret

rx_char:
rx_wait:
    MINI_UART_RX_POLL
    MINI_UART_RECEIVE
    ret

tx_char:
tx_wait:
    MINI_UART_TX_POLL
    MINI_UART_TRANSMIT
    ret

wdt_16ms:
    ldi  r17, 0x08
wdt_config:
    ; Temporarily repoint Y from USART0 (0xC0) to WDTCSR (0x60).
    ; STD is one word, unlike two-word STS, with the same 2-cycle execution.
    ; Both WDT writes remain adjacent, within the 4-cycle unlock window.
    ; Restore Y before returning, including the initial call before UART setup.
#if MINI_UART_STYLE == 0
    ldi  r28, 0x60
    ldi  r18, 0x18
    std  Y+0, r18
    std  Y+0, r17
    ldi  r28, 0xC0
#elif MINI_VIRTUAL_BOOT && MINI_UART_STYLE != 5
    ; ATtiny841/1634 use protected I/O write via CCP (0xD8), NOT WDCE.
    ; tiny841 CCP is extended-memory 0x71, tiny1634 CCP is I/O 0x2F.
    ; The following OUT to WDTCSR occurs within 4 instruction cycles.
    ldi r18,0xD8
#if MINI_UART_STYLE == 3
    sts 0x71,r18
#else
    out 0x2F,r18
#endif
    out WDT_IO,r17
    ldi r18,0x18             ; needed for RXEN|TXEN by MINI_UART_INIT
#else
    ldi r18,0x18
    out WDT_IO,r18
    out WDT_IO,r17
#endif
    ret

spm_execute_wait:
#if MINI_SPM_EXT
    sts SPMCSR_MEM,r16
#else
    out SPMCSR_IO,r16
#endif
    spm
wait_spm:
#if MINI_SPM_EXT
    lds r18, SPMCSR_MEM
#else
    in r18, SPMCSR_IO
#endif
    sbrc r18, 0
    rjmp wait_spm
    ret

/* EEPROM address is 0..1023.  r25=0 means 256 bytes for READ_PAGE,
 * just as in the historical Optiboot read loop.  Test the inclusive
 * end address = start + (length - 1) to avoid wraparound/wrong writes.
 * r18=top address high-byte limit (zero sentinel for 64 KiB); clobbers r16, Z, flags.
 */


#if MINI_UART_STYLE == 5
    MINI_UART_SOFT_ROUTINES
#endif

/* The boot section, signature and AVR061 GET_SIGN_ON footer are device-relative.
 * BOOTSZ and BOOTRST fuses must match the emitted image.
 */
.global _code_end
_code_end:
.fill ((BOOT_BYTES - 12) - (. - _start)),1,0xFF
.byte SIG0,SIG1,SIG2
.ascii "AVR STK"
.byte 3, 8
boot_end:
