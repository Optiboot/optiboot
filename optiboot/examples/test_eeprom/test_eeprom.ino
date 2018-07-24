/*
   EEPROM test program with more features.
*/

#include <EEPROM.h>

void setup() {
  // initialize the LED pin as an output.
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  Serial.println("EEPROM test program.\n"
                 "Enter one of (C)lear, (E)rase, (I)incrementing, (U)p&Down, (0)check,\n"
                 "(D)ump, (F)check, (T)inccheck");

}

void hex2(uint8_t n) {
  if (n <= 15)
    Serial.write('0');
  Serial.print(n, HEX);
  Serial.write(' ');
}

void print16(uint8_t *buf) {
  for (int i = 0; i < 16; i++) {
    hex2(buf[i]);
  }
  Serial.print("    ");
  for (int i = 0; i < 16; i++) {
    uint8_t c = buf[i];
    if (c < 32 || c > 126)
      Serial.write('.');
    else
      Serial.write(c);
  }
  Serial.println();
}

void dump16(uint16_t addr) {
  struct {
    uint8_t buf[16];
  } s;

  EEPROM.get(addr, s);
  print16(s.buf);
}


int cmd = -1;
void loop() {
  uint16_t i;
  do {
    cmd = Serial.read();
  } while (cmd < 0);

  switch (cmd & ~('a' - 'A')) {
    case 'C':
      for (i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
      }
      break;
    case 'D':
      Serial.println("EEPROM Dump\n");
      for (i = 0; i < EEPROM.length(); i += 16) {
        Serial.print(i, HEX);
        Serial.print(": ");
        dump16(i);
      }
      Serial.println();
      break;
    case 'E':
      for (i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0xFF);
      }
      break;
    case 'I':
      for (i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, i & 0xFF);
      }
      break;
    case 'U':
      for (i = 0 ; i < EEPROM.length() ; i++) {
        if ((i & 0x100) == 0) {
        EEPROM.write(i, i & 0xFF);
        } else {
          EEPROM.write(i, 255 - (i & 0xFF));
        }
      }
      break;
    case '?':
      Serial.println("EEPROM test program.\n"
                     "Enter one of (C)lear, (E)rase, (I)incrementing, (0)check,\n"
                     "(D)ump, (F)check, (T)inccheck");
    // Fall through
    case '\n':
    case '\r':
      break;
    default:
      Serial.print("unrecognized command ");
      Serial.println(cmd);
      break;
      // turn the LED on when we're done
      digitalWrite(13, HIGH);
  }
}


