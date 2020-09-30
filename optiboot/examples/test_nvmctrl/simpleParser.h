/* 
 * Written 2014 by Bill Westfield (WestfW)
 * Refactored 2020
 * Released to the public domain.
 */
#include "Arduino.h"


class parserCore {
private:
  char *buffer;
  byte lineLen;
  Stream *S;

  byte inptr;            /* read character into here */
  byte parsePtr;
  byte termchar;
  // Internal functions.
  bool IsWhitespace(char c);
  bool delim(char c);
  char *token(void);
//  int8_t KeywordPM (const char *keys);
  uint8_t tokcasecmp(const char *tok, const char * target);
    
public:
  parserCore(char *buf, byte buflen, Stream &io) {
    buffer = buf;
    lineLen = buflen;
    S = &io;
  }
  uint8_t getLine(void);     /* Non-blocking read line w/editing*/
  uint8_t getLineWait(void); /* wait for a full line of input */
  void reset(void);          /* reset the parser */
  int number();              /* parse a number */
  boolean eol();             /* check for EOL */
  uint8_t termChar();        /* return the terminating char of last token */
  int8_t keyword(const char *keys);  /* keyword with partial matching */
//  int8_t keywordExact(const char *keys);   /* keyword exact match */
};

/*
 * Constants
 */
#define PARSER_NOMATCH -1
#define PARSER_EOL -2
#define PARSER_AMB -3

#define CMP_PARTIALMATCH 2
#define CMP_NONMATCH 1
#define CMP_MATCH 0

#define CTRL(x) (x-64)

template <size_t maxline=80>
class simpleParser: public parserCore {
private:
  char buf[maxline];
public:
  simpleParser(Stream &io) : parserCore(buf, sizeof(buf), io) {}
};
