/*
 * simpleParser
 * Implement a command-line parser.
 * Written 2014 by Bill Westfield (WestfW)
 * refactored 2020
 * Released to the public domain.
 */

#include "Arduino.h"
#include "simpleParser.h"
#include <avr/pgmspace.h>


/*
 *  Reset the line buffer
 */
void parserCore::reset ()
{
  memset(buffer, 0, lineLen);
  inptr = 0;
  parsePtr = 0;
  termchar = 0;
}

/*
 * getLine
 * Read a line of text from Serial into the internal line buffer.
 * With echoing and editing!
 * Non-blocking.  Returns 0 until end-of-line seen.
 */
uint8_t parserCore::getLine ()
{
  int c;

  c = S->read();
  switch (c) {
    case 127:
    case CTRL('H'):
      /*
         Destructive backspace: remove last character
      */
      if (inptr > 0) {
        S->print("\010 \010");
        buffer[--inptr] = 0;
      }
      break;
    case CTRL('R'):
      /*
         Ctrl-R retypes the line
      */
      S->print("\r\n");
      S->print(buffer);
      break;
    case CTRL('U'):
      /*
         Ctrl-U deletes the entire line and starts over.
      */
      S->println("XXX");
      reset();
      break;
    case CTRL('J'):
    case CTRL('M'):
      buffer[inptr++] = '\n';
      S->println();     /* Echo newline too. */
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
      buffer[inptr++] = c;
      S->write(c);
  }
  return 0;
}

/*
 *  getLineWait
 *  like getLine, but block until a complete line is read
 */

uint8_t parserCore::getLineWait (void)
{
  uint8_t status;

  do {
    status = getLine();
  } while (status == 0);
  return status;
}


bool parserCore::IsWhitespace (char c)
{
  return (c == ' ' || c == CTRL('I'));
}



bool parserCore::delim(char c)
{
  static const char Delimiters[] PROGMEM = "\r\n ,;:=\t";
  if (c == 0 || strchr_P(Delimiters, c))
    return true;
  return false;
}

/*
 * Number
 *  Advance the token and parse a number.  Accept decimal, hex, octal.
 */

int parserCore::number()
{
  char *p = token();
  if (p) {
    return strtol(p, 0, 0);
  }
  return -1;
}

/*
 * eol
 * return true if we're at the end of the line.
 */
boolean parserCore::eol ()
{
  while (IsWhitespace(buffer[parsePtr])) { /* skip leading whitespace */
    parsePtr++;
  }
  return buffer[parsePtr] == '\n' || buffer[parsePtr] == 0;
}

/*
 *  cliTermChar
 * return the termination character of the last token
 */
uint8_t parserCore::termChar ()
{
  return termchar;
}

/*
 *  cliCharacter
 */

/*
 * token
 * A token is a set of non-delimiter characters ending at a delimiter.
 * As a line is parsed from the internal buffer, parsePtr is advanced, and
 *  the delimiters of parsed tokens are replaced with nulls.
 * Note that a line always ends with the newline character AND a null.
 */

char *parserCore::token ()
{
  uint8_t i;

  if (eol()) {  // reached the end of the line?
    return NULL;
  }
  i = parsePtr;  // save start position of token
  while ((!delim(buffer[parsePtr])) && (parsePtr < lineLen)) {
    parsePtr++; // advance pointer till we hit a delimiter.
  }
  termchar = buffer[parsePtr];
  buffer[parsePtr++] = 0;  // replace the delimiter with null
  return &buffer[i];  // convert position to pointer for retval
}

/*
 * Match the next token with a list of keywords.
 * The list of keywords is in PROGMEM, separated by spaces.
 * returns either the position of the found keyword (0..n),
 * PARSER_NOMATCH, PARSER_AMB, or PARSER_EOL at the end of line
 */
int8_t parserCore::keyword (const char *keys)
{
  char *p = token();
  char *thisKey = (char *)keys;
  int8_t i = 0, match, first=PARSER_NOMATCH;
  if (!p)
    return PARSER_EOL;

  while (pgm_read_byte(thisKey)) {
    match = tokcasecmp(p, thisKey);
#if defined(DEBUG) && DEBUG
    extern char spbuffer[];
    sprintf(spbuffer, "key='%s', p='%s', match = %d\n", thisKey, p, match);
    S->print(spbuffer);
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
      if (first != PARSER_NOMATCH) {   // already another match?
        return (PARSER_AMB);
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
 * tokcasecmp
 * tokcasecmp is like strcasecmp_P, except that the strings are terminated
 * by any char < 32.  Return value is 0 for match, or pointer to the delimiter
 * for non-match (to expedite comparing against strings of targets.)
 */
uint8_t parserCore::tokcasecmp(const char *tok, const char *target)
{
  char tokc, keyc;
  const char *t = (char *)target;

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
