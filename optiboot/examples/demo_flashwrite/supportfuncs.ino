// Dump a byte as two hex characters.
void hexout(uint8_t b)
{
  uint8_t high, low;

  high = b >> 4;
  low = b & 0xF;
  if (high > 9) {
    high += ('A'-10) - '0';
  }
  Serial.write(high + '0');
  if (low > 9) {
    low += ('A'-10) - '0';
  }
  Serial.write(low + '0');
}

void DumpHex(uint8_t *p, uint8_t len) {
  for (int i = 0; i < len; i++) {
    Serial.write(' ');
    hexout(p[i]);
  }
  Serial.print("   ");
  for (int i = 0; i < len; i++) {
    if (p[i] < ' ') {
      Serial.write('.');
    } else {
      Serial.write(p[i]);
    }
  }
}
