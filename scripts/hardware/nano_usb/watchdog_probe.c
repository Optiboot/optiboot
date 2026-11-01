/* GPL-2.0-or-later. Observe boot handoff without disabling the watchdog. */
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <util/delay.h>

volatile uint8_t boot_reason __attribute__((section(".noinit")));
volatile uint8_t initial_r1 __attribute__((section(".noinit")));
volatile uint8_t initial_mcusr __attribute__((section(".noinit")));

void capture_boot_state(void) __attribute__((used, naked, section(".init1")));
void capture_boot_state(void) {
    /* Run before the C runtime clears r1. No stack or calls are used here. */
    __asm__ __volatile__(
        "sts boot_reason,r2\n\t"
        "sts initial_r1,r1\n\t"
        "in r24,%0\n\t"
        "sts initial_mcusr,r24\n\t"
        : : "I" (_SFR_IO_ADDR(MCUSR)) : "r24");
}

static void put_char(char value) {
    while (!(UCSR0A & _BV(UDRE0))) {}
    UDR0 = value;
}
static void put_text(const char *text) {
    while (*text) put_char(*text++);
}
static void hex8(uint8_t value) {
    const char digits[] = "0123456789ABCDEF";
    put_char(digits[value >> 4]);
    put_char(digits[value & 15]);
}
static void ready(void) {
    put_text("APP_READY reason="); hex8(boot_reason);
    put_text(" r1="); hex8(initial_r1);
    put_text(" mcusr="); hex8(initial_mcusr);
    put_text(" wdt="); hex8(WDTCSR);
    put_text(" entry=");
    for (uint8_t index = 0; index < 8; ++index)
        hex8(eeprom_read_byte((const uint8_t *)(uintptr_t)(1016 + index)));
    put_text(" ticks=");
    uint16_t ticks = eeprom_read_word((const uint16_t *)(uintptr_t)1014);
    hex8((uint8_t)(ticks >> 8)); hex8((uint8_t)ticks);
    put_text("\r\n");
}
int main(void) {
    /* Record the FIRST application entry after the host arms the test.
     * Only that invocation advances the 500ms uptime counter. A later reset
     * caused by opening the serial driver preserves the earlier evidence.
     * The suite backs up and restores the complete EEPROM in finally. */
    uint8_t saved[4];
    eeprom_read_block(saved, (const void *)(uintptr_t)1016, sizeof saved);
    uint8_t first_entry = saved[0] != 'S' || saved[1] != 'T' ||
                          saved[2] != 'A' || saved[3] != 'R';
    if (first_entry) {
        uint8_t record[8] = {'S', 'T', 'A', 'R', boot_reason,
                             initial_r1, initial_mcusr, WDTCSR};
        eeprom_update_word((uint16_t *)(uintptr_t)1014, 0);
        eeprom_update_block(record, (void *)(uintptr_t)1016, sizeof record);
    }
    UCSR0A = _BV(U2X0);
    UBRR0H = 0;
    UBRR0L = 16;
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0B = _BV(TXEN0) | _BV(RXEN0);
    ready();
    uint16_t milliseconds = 0;
    for (;;) {
        if (first_entry) {
            _delay_ms(1);
            if (++milliseconds == 500) {
                milliseconds = 0;
                uint16_t ticks = eeprom_read_word((const uint16_t *)(uintptr_t)1014);
                eeprom_update_word((uint16_t *)(uintptr_t)1014, ticks + 1);
            }
        }
        if (!(UCSR0A & _BV(RXC0))) continue;
        uint8_t command = UDR0;
        if (command == 'C') {
            uint8_t empty[10] = {0};
            first_entry = 0;
            eeprom_update_block(empty, (void *)(uintptr_t)1014, sizeof empty);
            put_text("COLD_ARMED\r\n");
            ready();
        }
        if (command == 'I') ready();
        if (command == 'P') put_text("PONG\r\n");
        if (command == 'W') {
            put_text("WDT_ARMED\r\n");
            _delay_ms(3);
            wdt_enable(WDTO_15MS);
            for (;;) {}
        }
    }
}
