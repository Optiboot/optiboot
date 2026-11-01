/*
 * Read-only ATmega328P/328PB boot-access diagnostics. GPL-2.0-or-later.
 * Report actual fuse/lock values with LPM, not bootloader UNIVERSAL replies.
 * No SPM instruction, Flash write, EEPROM write or lock-bit programming.
 * UART: 115200 baud, 8N1; repeat the report every two seconds.
 */
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <util/delay.h>

#if !defined(__AVR_ATmega328P__) && !defined(__AVR_ATmega328PB__)
#error This diagnostic is restricted to ATmega328P/328PB.
#endif
#if F_CPU != 16000000UL
#error This diagnostic requires an actual 16 MHz CPU clock.
#endif

#ifdef DIAGNOSTICS_STANDALONE
void disable_watchdog_early(void) __attribute__((naked, used, section(".init3")));
void disable_watchdog_early(void) {
    MCUSR = 0;
    wdt_disable();
}
#endif

static void put_char(char value) {
    while (!(UCSR0A & _BV(UDRE0))) {}
    UDR0 = value;
}

static void put_text(PGM_P text) {
    char value;
    while ((value = (char)pgm_read_byte(text++))) put_char(value);
}

static void hex8(uint8_t value) {
    const char digits[] = "0123456789ABCDEF";
    put_char(digits[value >> 4]);
    put_char(digits[value & 15]);
}

static void hex16(uint16_t value) {
    hex8((uint8_t)(value >> 8));
    hex8((uint8_t)value);
}

static void byte_field(PGM_P name, uint8_t value) {
    put_text(name);
    hex8(value);
    put_text(PSTR("\r\n"));
}

static void word_field(PGM_P name, uint16_t value) {
    put_text(name);
    hex16(value);
    put_text(PSTR("\r\n"));
}

static void permission(PGM_P name, uint8_t allowed) {
    put_text(name);
    put_text(allowed ? PSTR("ALLOWED_BY_LOCK_BITS\r\n") : PSTR("BLOCKED_BY_LOCK_BITS\r\n"));
}

static uint16_t boot_crc16(uint16_t first) {
    uint16_t crc = 0xffff;
    for (uint16_t address = first; address <= FLASHEND; ++address) {
        crc ^= (uint16_t)pgm_read_byte_near(address) << 8;
        for (uint8_t bit = 0; bit < 8; ++bit)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

void setup(void) {
    MCUSR = 0;
    wdt_disable();
    UCSR0A = _BV(U2X0);
    UBRR0H = 0;
    UBRR0L = (uint8_t)((F_CPU + 4UL * 115200UL) / (8UL * 115200UL) - 1);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0B = _BV(TXEN0);
}

void loop(void) {
    uint8_t signature[3];
    uint8_t low, high, extended, lock;
    uint8_t stable = 1;
    uint8_t saved_status = SREG;
    cli();
    eeprom_busy_wait();
    boot_spm_busy_wait();
    signature[0] = boot_signature_byte_get(0);
    signature[1] = boot_signature_byte_get(2);
    signature[2] = boot_signature_byte_get(4);
    low = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    high = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    extended = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    lock = boot_lock_fuse_bits_get(GET_LOCK_BITS);
    for (uint8_t sample = 0; sample < 8; ++sample) {
        if (boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS) != low ||
            boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS) != high ||
            boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS) != extended ||
            boot_lock_fuse_bits_get(GET_LOCK_BITS) != lock) stable = 0;
    }
    SREG = saved_status;

    put_text(PSTR("BOOT_ACCESS_DIAGNOSTICS_V1\r\nsignature="));
    for (uint8_t index = 0; index < 3; ++index) hex8(signature[index]);
    put_text(PSTR("\r\n"));
    byte_field(PSTR("lfuse="), low);
    byte_field(PSTR("hfuse="), high);
    byte_field(PSTR("efuse="), extended);
    byte_field(PSTR("lock_raw="), lock);
    byte_field(PSTR("lock_implemented="), lock & 0x3f);
    put_text(stable ? PSTR("repeated_reads=STABLE\r\n") : PSTR("repeated_reads=UNSTABLE\r\n"));

    if (signature[0] != 0x1e || signature[1] != 0x95 ||
        (signature[2] != 0x0f && signature[2] != 0x16) || !stable) {
        put_text(PSTR("interpretation=REFUSED_SIGNATURE_OR_UNSTABLE_READS\r\nEND_BOOT_ACCESS_DIAGNOSTICS\r\n"));
        _delay_ms(2000);
        return;
    }
    put_text(signature[2] == 0x16 ? PSTR("mcu=ATMEGA328PB\r\n") : PSTR("mcu=ATMEGA328P\r\n"));

    uint16_t boot_bytes = (uint16_t)(4096U >> ((high >> 1) & 3));
    uint16_t boot_start = (uint16_t)((uint32_t)FLASHEND + 1 - boot_bytes);
    word_field(PSTR("boot_size_hex="), boot_bytes);
    word_field(PSTR("boot_start_hex="), boot_start);
    put_text((high & _BV(0)) ? PSTR("reset_target=APPLICATION\r\n") : PSTR("reset_target=BOOT\r\n"));
    permission(PSTR("app_lpm_boot="), lock & _BV(BLB12));
    permission(PSTR("boot_spm_boot="), lock & _BV(BLB11));
    permission(PSTR("boot_lpm_app="), lock & _BV(BLB02));
    permission(PSTR("boot_spm_app="), lock & _BV(BLB01));
    word_field(PSTR("app_read_boot_crc16="), boot_crc16(boot_start));
    put_text(PSTR("app_spm_any_flash=BLOCKED_BY_CPU_LOCATION\r\n"
                  "lock_and_fuse_read=DIRECT_LPM\r\n"
                  "write_permissions=DECODED_NOT_DESTRUCTIVELY_PROBED\r\n"
                  "lock_clear=EXTERNAL_CHIP_ERASE_REQUIRED\r\n"
                  "software_update_requires=EXISTING_BOOT_SPM_CODE_AND_PERMISSIONS\r\n"
                  "flash_eeprom_lock_writes=NONE\r\nEND_BOOT_ACCESS_DIAGNOSTICS\r\n"));
    _delay_ms(2000);
}

#ifdef DIAGNOSTICS_STANDALONE
int main(void) {
    setup();
    for (;;) loop();
}
#endif
