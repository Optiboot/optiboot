/*
   clicpp
   Implement a command-line parser.
   Written 2014 by Bill Westfield (WestfW)
   refactored 2020
   Released to the public domain.
*/

#include "Arduino.h"
#include "cli.h"
#include <avr/pgmspace.h>

char linebuffer[CLI_LINELEN];
static byte inptr, cliptr, termchar;


void cliReset ()
{
  /*
     Reset the line buffer
  */
  memset(linebuffer, 0, sizeof(linebuffer));
  inptr = 0;
  cliptr = 0;
  termchar = 0;
}

/*
   cliGetline
   Read a line of text from Serial into the internal line buffer.
   With echoing and editing!
   Non-blocking.  Returns 0 until end-of-line seen.
*/
uint8_t cliGetline (void)
{
  int c;

  c = Serial.read();
  switch (c) {
    case 127:
    case CTRL('H'):
      /*
         Destructive backspace: remove last character
      */
      if (inptr > 0) {
        Serial.print("\010 \010");
        linebuffer[--inptr] = 0;
      }
      break;
    case CTRL('R'):
      /*
         Ctrl-R retypes the line
      */
      Serial.print("\r\n");
      Serial.print(linebuffer);
      break;
    case CTRL('U'):
      /*
         Ctrl-U deletes the entire line and starts over.
      */
      Serial.println("XXX");
      cliReset();
      break;
    case CTRL('J'):
    case CTRL('M'):
      Serial.println();     /* Echo newline too. */
      return inptr;
    case -1:
      /*
         No character present; don't do anything.
      */
      return 0;
    default:
      /*
         Otherwise, echo the character and put it into the buffer
      */
      linebuffer[inptr++] = c;
      Serial.write(c);
  }
  return 0;
}

/*
    cliGetlineWait
    like cliGetline, bu block until a complete line is read
*/
uint8_t cliGetlineWait (void)
{
  uint8_t status;

  do {
    status = cliGetline();
  } while (status == 0);
  return status;
}

bool cliIsWhitespace (char c)
{
  return (c == ' ' || c == CTRL('I'));
}


const char cliDelimiters[] PROGMEM = "\r\n ,;:=\t";
bool cliDelim(char c)
{
  if (c == 0 || strchr_P(cliDelimiters, c))
    return true;
  return false;
}

/*
   cliNumber
   Advance the token and parse a number.  Accept decimal, hex, octal.
*/
int cliNumber()
{
  char *p = cliToken();
  if (p) {
    return strtol(p, 0, 0);
  }
  return -1;
}

/*
   cliTermChar
   return the termination character of the last token
*/
uint8_t cliTermChar()
{
  return termchar;
}

/*
   cliCharacter
*/

/*
   cliToken
   A token is a set of non-delimiter characters ending at a delimiter.
   As a line is parsed from the internal buffer, cliptr is advanced, and
   the delimiters of parsed tokens are replaced with nulls.
   Note that a line always ends with the newline character AND a null.
*/
char * cliToken(void)
{
  uint8_t i;

  while (cliIsWhitespace(linebuffer[cliptr])) { /* skip leading whitespace */
    cliptr++;
  }
  if (linebuffer[cliptr] == 0) {  // reached the end of the line?
    return NULL;
  }
  i = cliptr;  // save start position of token
  while ((!cliDelim(linebuffer[cliptr])) && (cliptr < CLI_LINELEN)) {
    cliptr++; // advance pointer till we hit a delimiter.
  }
  termchar = linebuffer[cliptr];
  linebuffer[cliptr++] = 0;  // replace the delimiter with null
  return &linebuffer[i];  // convert position to pointer for retval
}

/*
   Match the next token with a list of keywords.
   The list of keywords is in PROGMEM, separated by spaces.
   returns either the position of the found keyword (0..n),
   CLI_NOMATCH, CLI_AMB, or CLI_EOL at the end of line
*/

// extern char buffer[];
int8_t cliKeywordPM (const char *keys)
{
  char *p = cliToken();
  char *thisKey = (char *)keys;
  int8_t i = 0, match, first=CLI_NOMATCH;
  if (!p)
    return CLI_EOL;

  while (pgm_read_byte(thisKey)) {
    match = tokcasecmp(p, thisKey);
#if 0
    char buffer[128];
    sprintf(buffer, "key='%S', p='%s', match = %d, i=%d\n", thisKey, p, match, i);
    Serial.print(buffer);
#endif
    if (match == CMP_MATCH) {
      return i;
    }
    while (pgm_read_byte(thisKey) > ' ') {
      thisKey++;  // advance to next keyword
    }
    thisKey++;  // skip delimiter
    if (match == CMP_PARTIALMATCH) {
      // There was a partial match; check for another...
      if (first != CLI_NOMATCH) {   // already another match?
        return (CLI_AMB);
      } else {
        first = i;
        continue;
      }
      return i;  // match
    }
    i++; // next keyword
  }
  return first;
}

/*
   tokcasecmp
   tokcasecmp is like strcasecmp_P, except that the strings are terminated
   by any char < 32.  Return value is 0 for match, or pointer to the delimiter
   for non-match (to expedite comparing against strings of targets.)
*/

uint8_t tokcasecmp(const char *tok, const char *target)
{
  char tokc, keyc;
  const char *t = target;

  do {
    tokc = toupper(*tok++);
    keyc = toupper(pgm_read_byte(t++));
    // tok++; t++;
    if (tokc == 0) {
      // End of token; see if end of keyword as well
      if (keyc <= ' ') {
        return CMP_MATCH;  // both ended - exact match
      }
      return CMP_PARTIALMATCH; // keyword is longer - partial
    }
    // Not end of token
    if (keyc <= ' ') {
      return CMP_NONMATCH;  // key ended before tok - non match
    }
  } while (tokc == keyc);
  return CMP_NONMATCH;  // neither string ended, but non-match
}
