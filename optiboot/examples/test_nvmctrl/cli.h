/* 
 * Written 2014 by Bill Westfield (WestfW)
 * Refactored 2020
 * Released to the public domain.
 */
#include "Arduino.h"

/*
 * API
 */
uint8_t cliGetline(void);
uint8_t cliGetlineWait(void);
void cliReset(void);
char *cliToken(void);
int cliNumber();
int8_t cliKeyword(const char *keys);
int8_t cliKeywordPM (const char *keys);

/*
 * Constants
 */
#define CLI_LINELEN 80
#define CLI_NOMATCH -1
#define CLI_EOL -2
#define CLI_AMB -3

#define CMP_PARTIALMATCH 2
#define CMP_NONMATCH 1
#define CMP_MATCH 0

/*
 * Internals
 */
char getcwait(void);
bool substrdcmp(char *cmd,  char *line);
bool cliIsWhitespace(char c);
bool cliDelim(char c);
// bool clirEOL();
uint8_t tokcasecmp(const char *tok, const char * target);

#define CTRL(x) (x-64)
