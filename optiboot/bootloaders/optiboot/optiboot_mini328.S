/*
 * Optiboot Mini STK500v1 512B production-layout candidate (500B code + 12B data), 29 October 2026.
 * ATmega328P / 16MHz / UART0 115200 8N1. BOOTSZ = 256 words = 512B (EXPERIMENTAL).
 * FLASH boot base 0x7E00, version at 0x7FFE, signature and "AVR STK" data
 * at 0x7FF4..0x7FFD. Experimental: no silicon test and no real avrdude run.
 *
 * Fixes vs the 512B Optiboot Mini: checks BOTH length bytes for
 * STK_READ_PAGE/STK_PROG_PAGE, supports valid read length 0x0100=256,
 * refuses unknown memory types with INSYNC+FAILED instead of touching FLASH,
 * implements STK_GET_SIGN_ON with AVR061's "AVR STK" reply.
 *
 * Uses Y as stable USART0 register base, LDD/STD instead of most LDS/STS.
 * Shares two-byte receive and common error responses across handlers.
 * Keeps avrdude -c arduino format, read/write FLASH, 1..16B EEPROM writes,
 * EEPROM read, read-only UNIVERSAL fuse/lock commands, WDT and SPM interlock.
 * Preserves Optiboot's ACK without true Chip Erase for legacy avrdude.
 * Rejects overflow in LOAD_ADDRESS, restores r1 and UBRR0H; validates FLASH read ends.
 * r1 is cleared at boot entry. Intended for 512-byte BOOTSZ fuses (pending hardware verification).
 * Full AVR061 is NOT implemented. Experimental 512-byte boot-section image, requires hardware testing.
 *
 * Based on Optiboot, Copyright (c) Peter Knight, Bill Westfield et al.
 * GPL-2.0-or-later; see the repository LICENSE. Hardware testing and ISP recovery required.
 */

.section .text
.global _start
_start:
    ; No app-callable SPM entry; the extra RET stub is removed to make room for safety checks.

/* Register allocation:
 * r1  = cleared on entry; later used as HIGH data byte during Flash fill
 * r2  = copy of MCUSR for application startup
 * r29 = zero from boot entry; must remain zero (Y = 0x00C0 after UART setup)
 * r16 = UART byte/input/output value
 * r17 = watchdog configuration value
 * r18 = USART status/temp
 * r19 = shared discard count / EEPROM receive count / SPMEN preload (1)
 * r20 = GET_PARAMETER selector, READ_PAGE memory type, ISP opcode1
 * r21 = UNIVERSAL ISP opcode2 (bit 3 mapped into Z via T flag)
 * r22:r23 = current BYTE address (Flash or EEPROM)
 * r25 = page byte length for READ_PAGE, word count for Flash PROG_PAGE
 * X = EEPROM READ pointer; EEPROM WRITE payload is held on AVR stack
 * Y = fixed memory-mapped UART register base 0x00C0 (LDD/STD)
 * Z = flash address, page-fill address, or EEPROM last-byte address
 */
boot_start:
    cli
    clr r1                 ; AVR zero register restored for application
    clr r29                ; zero register available from very first instruction
    in   r16, 0x34         ; MCUSR
    mov  r2, r16
    out  0x34, r29         ; clear reset reason (r29=0)
    wdr
    clr  r17
    rcall wdt_config        ; disable watchdog on application start
    tst  r16              ; cleared MCUSR = explicit software bootloader entry
    breq start_uart
    andi r16, 0x0A         ; WDRF | EXTRF
    cpi  r16, 0x02         ; enter only on external reset without WDRF
    breq start_uart
    rjmp boot_end          ; 16K-word PC wraps from end of Flash to 0

start_uart:
    ; Y was restored to 0x00C0 by wdt_config; no second initialization needed.
    ldi  r16, 0x02         ; U2X0 (double-speed UART0)
    std Y+0,r16         ; UCSR0A
    std Y+1,r18          ; RXEN0 | TXEN0: r18=0x18 left by initial wdt_config
    ldi  r16, 0x06         ; 8N1
    std Y+2,r16         ; UCSR0C
    ldi  r16, 16           ; UBRR0 = 16, 117647 actual baud @16MHz (+2.12%)
    std Y+4,r16         ; UBRR0L
    std Y+5,r29         ; UBRR0H = 0, even on software-entry
    ldi  r17, 0x0E         ; WDT 1s: WDE|WDP2|WDP1
    rcall wdt_config

/* EEPE can remain busy after an external reset while an application
 * EEPROM write is finishing.  Datasheet 26.8.8 prohibits SPMCSR writes
 * (including fuse reads) until EEPE clears.  Poll at every new packet.
 * Internal EEPROM writes already finish before STK_OK, so this replaces
 * the duplicate wait previously inside eeprom_read without size growth.
 */
next_command:
.Lwait_eepe_before_command:  sbic 0x1F, 1           ; wait EEPE before any SPM/BLBSET operation
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
    breq command_read
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
    ldi r30,0xF4         ; absolute 0x7FF4, 3 signature bytes
    rjmp static_bytes
command_signon:
    ldi r25,7
    ldi r30,0xF7         ; absolute 0x7FF7, "AVR STK"
static_bytes:
    clr r20              ; select LPM branch in every iteration
    ldi r31,0x7F
    rcall verify_eop
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
    brcs protocol_error   ; reject word address >= 0x8000, avoid 16-bit wrap
    rjmp command_default

/* The high byte matters: READ_PAGE allows up to 256 bytes (0x0100).
 * 0x0000 retains legacy Optiboot's 256-byte interpretation.
 */
universal_handler:
    rcall read_length      ; first two opcode bytes
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
    out  0x37,r16
    lpm  r16,Z
universal_response:
    rcall tx_char
    rjmp command_ok


command_read:
    rcall read_length
    subi r24,1           ; high length 0 accepts 0..255, 1 accepts only low=0
    brcs .Lread_length_valid              ; original high byte 0
    or   r24,r25         ; (high-1)|low must be zero for 0x0100
    brne protocol_error
.Lread_length_valid:  rcall rx_char
    mov r20,r16
    rcall verify_eop
    ldi r18,0x80         ; Flash upper limit = 0x8000
    cpi r20,0x46         ; Flash
    breq read_check_common
    ldi r18,4            ; EEPROM upper limit = 0x0400
    cpi r20,0x45         ; EEPROM
    brne read_bad_memtype
read_check_common:
    rcall memory_range_check
    movw r30,r22
read_loop:
    sbrc r20,0           ; E odd; F even
    rjmp read_eeprom_byte
    lpm r16,Z+
    rjmp read_send
read_eeprom_byte:
    out 0x21,r30
    out 0x22,r31
    sbi 0x1F,0
    in r16,0x20
    adiw r30,1
read_send:
    rcall tx_char
    dec r25
    brne read_loop
    rjmp command_ok
verify_eop:
    rcall rx_char
    cpi  r16, 0x20         ; CRC_EOP = ' '
    brne protocol_error
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
    adc  r31, r29         ; YH is a permanent zero register after USART init
    brcs protocol_error  ; reject 0xFFFF -> 0x0000 wrapping
    cp   r31, r18
    brsh protocol_error
    ret

command_write:
    rcall read_length
    tst r24                  ; reject nonzero high length on write
    brne protocol_error
    tst r25                  ; reject zero length for both EEPROM and Flash
    breq protocol_error
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
    lsr  r25               ; C=odd length; input was checked nonzero
    brcs protocol_error
    cpi  r25, 65           ; ATmega328P hardware page holds 64 words
    brsh protocol_error
    ; Datasheet 26.8.3: Page Write requires PCWORD=0 in Z.
    ; Reject a non-page-aligned start, preventing SPM address misuse
    ; and a payload spanning two different 128-byte flash pages.
    mov  r16, r22
    andi r16, 0x7F        ; SPM page size: 64 words = 128 bytes
    brne protocol_error
    movw r30, r22
    ldi  r19, 0x01         ; preloaded SPMEN across whole page (saves cycles)

/* Fill the ATmega328P's hardware temporary page buffer BEFORE flash erase.
 * Atmel datasheet 30.8 explicitly allows SPM fill -> page erase -> write.
 * A bad CRC_EOP cannot destroy old Flash: no page erase has happened.
 * Unlike the SRAM version this uses no 128-byte software buffer.
 */
hardware_fill_loop:
    rcall rx_char
    mov  r0, r16
    rcall rx_char
    mov  r1, r16
    out  0x37, r19         ; SPMEN page buffer fill
    spm
    adiw r30, 2
    dec  r25
    brne hardware_fill_loop
    ; r1 now contains the last Flash word high byte. No later code
    ; uses r1 as its zero register; r29 (YH) remains zero permanently.
    rcall verify_eop      ; STK_INSYNC must precede actual erase
    cpi  r23, 0x7E        ; refuse modifying 512B boot partition
    brsh protocol_error

    movw r30, r22
    ldi  r16, 0x03        ; erase, hardware temp page buffer retained!
    rcall spm_execute_wait
    ldi  r16, 0x05        ; write temporary page buffer to Flash
    rcall spm_execute_wait
    ldi  r16, 0x11        ; RWWSRE|SPMEN
    rcall spm_execute_wait
    rjmp command_ok

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
    cpi r25,17
    brsh protocol_error
    ldi  r18, 4
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
.Leeprom_commit_next:  out  0x1F,r29        ; EECR=0: YH=0 after USART init
    out  0x21,r30        ; EEARL
    out  0x22,r31        ; EEARH
    pop  r16
    out  0x20,r16        ; EEDR
    sbi  0x1F,2          ; EEMPE, 4 CPU cycles to trigger EEPE
    sbi  0x1F,1          ; EEPE, start 3.4ms atomic erase/write
    sbiw r30,1
    dec  r25             ; Z flag is retained by SBIC/RJMP
.Leeprom_wait_write_complete:  sbic 0x1F,1          ; every write complete before next/ACK
    rjmp .Leeprom_wait_write_complete
    brne .Leeprom_commit_next
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
    ldd  r18, Y+0        ; UCSR0A
    sbrs r18, 7           ; RXC0
    rjmp rx_wait
    sbrs r18, 4           ; FE0=1: do not keep WDT alive on framing error
    wdr                    ; good frames only
    ldd  r16, Y+6        ; UDR0
    ret

tx_char:
tx_wait:
    ldd  r18, Y+0        ; UCSR0A
    sbrs r18, 5           ; UDRE0
    rjmp tx_wait
    std  Y+6, r16        ; UDR0
    ret

wdt_16ms:
    ldi  r17, 0x08
wdt_config:
    ; Temporarily repoint Y from USART0 (0xC0) to WDTCSR (0x60).
    ; STD is one word, unlike two-word STS, with the same 2-cycle execution.
    ; Both WDT writes remain adjacent, within the 4-cycle unlock window.
    ; Restore Y before returning, including the initial call before UART setup.
    ldi  r28, 0x60
    ldi  r18, 0x18          ; WDCE | WDE
    std  Y+0, r18          ; WDTCSR timed unlock
    std  Y+0, r17          ; WDTCSR new configuration
    ldi  r28, 0xC0         ; restore USART0 base
    ret

spm_execute_wait:
    out  0x37, r16
    spm
wait_spm:
    in   r18, 0x37        ; SPMCSR
    sbrc r18, 0
    rjmp wait_spm
    ret

/* EEPROM address is 0..1023.  r25=0 means 256 bytes for READ_PAGE,
 * just as in the historical Optiboot read loop.  Test the inclusive
 * end address = start + (length - 1) to avoid wraparound/wrong writes.
 * r18=limit (4 for EEPROM, 128 for Flash); side effects: r16, r30:r31 and flags; r22:r23/r25 preserved.
 */


/* EXPERIMENT 512-byte BOOT: BOOTSZ=256 words, 0x7E00..0x7FFF.
 * Signature and AVR061 GET_SIGN_ON data at absolute 0x7FF4..0x7FFD.
 * Device fuses must be correct; test through ISP before relying on it.
 */
.global _code_end
_code_end:
.fill (0x01F4 - (. - _start)),1,0xFF
.byte 0x1E,0x95,0x0F
.ascii "AVR STK"
.byte 3, 8
boot_end:
