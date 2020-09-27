#include "optiboot_x.h"
#include "cli.h"

static int debug = 0;

#define debugtxt(a) if (debug) Serial.println(a)
#define debugnum(a) do {if (debug) { Serial.print(#a " = "); Serial.println(a); }} while (0)

#undef SERIAL

#ifdef SERIAL_PORT_USBVIRTUAL
#define SERIAL SERIAL_PORT_USBVIRTUAL
#else
#define SERIAL Serial
#endif

// Configure the baud rate:

#define BAUDRATE	19200

void setup() {
  SERIAL.begin(BAUDRATE);
  while (!SERIAL)
    ;
  SERIAL.flush();
  delay(500);
  SERIAL.println(F("Flash Self-Programming test tool\n"));
}

static const char PROGMEM cmd_strings[] =
  "dump_a "     "dospm_c_a_v... "   "nvmctrla_v " "set_a_v... "   "spm "   "statusnv " "datanv " "addrnv "
  "help "    "? ";
enum {
  CMD_DUMP = 0,    CMD_DOSPM,        CMD_NVMCTRL, CMD_SET,        CMD_SPM,  CMD_STAT, CMD_DATA,  CMD_ADDR,
  CMD_HELP, CMD_HELP2  // make sure this matches the string
};

void loop() {
  int8_t clicmd;
  int16_t addr, val, nvcmd, i;
  Serial.print("Cmd: ");
  cliReset();
  cliGetlineWait();
  clicmd = cliKeywordPM(cmd_strings); // look for a command.
  debugnum(clicmd);
  switch (clicmd) {
    case CMD_DUMP:
      addr = cliNumber();
      for (i = 0; i < 8; i++) {
        SERIAL.print(addr, HEX); SERIAL.print(": ");
        DumpHex((uint8_t *)addr, 32);
        addr += 32;
        SERIAL.println();
      }
      SERIAL.println();
      break;
    case CMD_DOSPM:
      debugtxt("cmd_dospm");
      nvcmd = cliNumber();
      addr = cliNumber();
      while (1) {
        val = cliNumber();
        if (val < 0)
          break;
#if 0
        SERIAL.print(F("dospm(0x")); SERIAL.print(addr, HEX);
        SERIAL.print(F(", ")); SERIAL.print(nvcmd);
        SERIAL.print(F(", ")); SERIAL.print(val);
        SERIAL.println(F(")"));
#endif
        do_spm(addr++, nvcmd, val);
      }
      printNVMStat();
      break;
    case CMD_NVMCTRL:
      debugtxt("cmd_nvmctrl");
      nvcmd = cliNumber();
      _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, nvcmd);
      printNVMStat();
      break;
    case CMD_SET:
      debugtxt("cmd_set");
      addr = cliNumber();
      while (1) {
        val = cliNumber();
        if (val < 0)
          break;
        *(uint8_t *)addr++ = val;
      }
      printNVMStat();
      break;
    case CMD_SPM:
      debugtxt("cmd_spm");
      break;
    case CMD_STAT:
      debugtxt("cmd_stat");
      printNVMStat();
      break;
    case CMD_DATA:
      debugtxt("cmd_data");
      SERIAL.print("NVMCTRL.DATA = 0x");
      SERIAL.print(NVMCTRL.DATA, HEX);
      SERIAL.println("\n");
      break;
    case CMD_ADDR:
      debugtxt("cmd_addr");
      SERIAL.print("NVMCTRL.ADDR = 0x");
      SERIAL.print(NVMCTRL.ADDR, HEX);
      SERIAL.println("\n");
      break;
    case CMD_HELP2:
    case CMD_HELP:
      SERIAL.println(F("Flash Self-Programming test tool\n"));
      SERIAL.println(F("Enter a command.  One of:"));
      SERIAL.println(reinterpret_cast<const __FlashStringHelper *>(cmd_strings));
      SERIAL.println();
      break;
    default:
      break;
  }
}

// Dump a byte as two hex characters.
void hexout(uint8_t b)
{
  uint8_t high, low;

  high = b >> 4;
  low = b & 0xF;
  if (high > 9) {
    high += 'A' - ('9' + 1);
  }
  SERIAL.write(high + '0');
  if (low > 9) {
    low += 'A' - ('9' + 1);
  }
  SERIAL.write(low + '0');
}

void DumpHex(uint8_t *p, uint8_t len) {
  while (len--) {
    SERIAL.write(' ');
    hexout(*p++);
  }
}

void printNVMStat() {
  SERIAL.print("NVMCTRL.STATUS = ");
  SERIAL.print(NVMCTRL.STATUS, HEX);
  SERIAL.println("\n");
}
