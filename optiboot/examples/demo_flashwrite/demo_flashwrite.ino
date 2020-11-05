/*------------- Optiboot flasher example for the MiniCore ------------------|
 |                                                                          |
 | Created May 2016 by MCUdude, https://github.com/MCUdude                  |
 | Based on the work done by Marek Wodzinski, https://github.com/majekw     |
 | Released to public domain                                                |
 |                                                                          |
 | This is example how to use optiboot.h together with Optiboot             |
 | bootloader to write to FLASH memory by application code.                 |
 |                                                                          |
 | IMPORTANT THINGS:                                                        |
 | - All flash content gets erased after each upload cycle                  |
 | - Buffer must be page aligned (see declaration of flash_buffer)          |
 | - Interrupts must be disabled during SPM                                 |
 | - Writing to EEPROM destroys temporary buffer                            |
 | - You can write only once into one location of temporary buffer          |
 | - Only safe and always working sequence is erase-fill-write              |
 | - If you want to do fill-erase-write, you must put code in NRWW          |
 |   and pass data!=0 for erase. It's not easy, but possible.               |
 |                                                                          |
 | WRITE SEQUENCE - OPTION 1 (used in this example)                         |
 | 1. Erase page by optiboot_page_erase                                     |
 | 2. Write contents of page into temporary buffer by optiboot_page_fill    |
 | 3. Write temporary buffer to FLASH by optiboot_page_write                |
 |                                                                          |
 | WRITE SEQUENCE - OPTION 2 (works only for code in NRWW)                  |
 | 1. Write contents of page into temporary buffer by optiboot_page_fill    |
 | 2. Erase page by optiboot_page_erase (set data to NOT zero)              |
 | 3. Write temporary buffer to FLASH by optiboot_page_write                |
 |-------------------------------------------------------------------------*/

// optiboot.h contains the functions that lets you read to
// and write from the flash memory
#include "optiboot.h"
#include "simpleParser.h"


// Define the number of pages you want to write to here (limited by flash size)
// these are flash pages "internal" to the sketch.
#define NUMBER_OF_PAGES 8

// Define your termination and blank character here
const char terminationChar = '@';


uint8_t charBuffer;
int8_t menuOption;
int16_t pageNumber;
char returnToMenu;

// The temporary data (data that's read or is about to get written) is stored here
uint8_t ramBuffer[SPM_PAGESIZE + 1];

// This array allocates the space you'll be able to write to
const uint8_t flashSpace[SPM_PAGESIZE * NUMBER_OF_PAGES] __attribute__ (( aligned(SPM_PAGESIZE) )) PROGMEM = {
  "This some default content stored on page one"
};



void setup()
{
  // Initialize serial
  Serial.begin(115200);
  while (!Serial)
    ; // possibly wait for USB Serial to enumerate.
}

simpleParser<80> ttycli(Serial);

void loop()
{
  byte clicmd;
  // Print main menu
  Serial.println();
  Serial.print(F("|------------------------------------------------|\n"
                 "| Welcome to the Optiboot flash writer example!  |\n"
#ifdef USE_NVMCTRL
                 "| Running on mega0/xTiny with NVMCTRL support.   |\n"
#else
                 "| Running on traditional AVR with SPM support.   |\n"
#endif
                 "| Each flash page is "));
  Serial.print(SPM_PAGESIZE);
  Serial.print  (                      F(" bytes long.             |\n"));
  Serial.print(F("| There are "));
  Serial.print(NUMBER_OF_PAGES);
  Serial.print  (F(           " pages that can be read/written to. |\n"));
  Serial.print(F("| Total assigned flash space: "));
  Serial.print(NUMBER_OF_PAGES * SPM_PAGESIZE);
  Serial.print(F(" bytes.        |\n"
                 "| Change the NUMBER_OF_PAGES constant to         |\n"
                 "| increase or decrease this number.              |\n"
                 "|                                                |\n"
                 "| What do you want to do?                        |\n"
                 "| Read - read current flash content              |\n"
                 "| Write - Write to flash memory                  |\n"
                 "|------------------------------------------------|\n\n"));


  uint16_t counter = 0;

  static const char PROGMEM cmdStrings[] = "read write ?";
  enum { CMD_READ = 0, CMD_WRITE, CMD_HELP };
  Serial.print("Cmd: ");
  ttycli.reset();
  ttycli.getLineWait();
  clicmd = ttycli.keyword(cmdStrings); // look for a command.

  switch (clicmd) {
    case CMD_READ:
      // Read flash option selected
      if (ttycli.eol()) {
        Serial.print(F("What page number do you want to read? Page: "));
        ttycli.reset();
        ttycli.getLineWait();
      }
      pageNumber = ttycli.number();

      if (pageNumber < 0 || pageNumber > NUMBER_OF_PAGES) {
        Serial.print(F("\nPlease enter a valid page between 1 and "));
        Serial.print(NUMBER_OF_PAGES);
        Serial.println(F(".\nThe number of pages can be extended by changing NUMBER_OF_PAGES constant"));
        return;  // restart parsing
      }
#if 0
      if (pageNumber > _etext && (pageNumber & (SPM_PAGESIZE-1)) == 0) {
        
      if (pageNumber > 0)
        Serial.println(pageNumber);
#endif

      // READ SELECTED PAGE AND STORE THE CONTENT IN THE ramBuffer ARRAY
      // flash_buffer is where the data is stored (contains the memory addresses)
      // ramBuffer is where the data gets stored after reading from flash
      // pageNumber is the page the data is read from
      // blankChar is the character that gets printed/stored if there are unused space (default '.')
      // use optiboot_readPage(flashSpace, ramBuffer, pageNumber) if you don't want blank chars

      if (pageNumber == 0) // Read all pages
      {
        Serial.println(F("\nAll flash content:"));
        for (uint16_t page = 1; page < NUMBER_OF_PAGES + 1; page++)
        {
          Serial.print(F("Page "));
          Serial.print(page);
          Serial.println(F(": "));
          optiboot_readPage(flashSpace, ramBuffer, page);
          for (int i = 0; i < SPM_PAGESIZE; i += 16) {
            DumpHex(&ramBuffer[i], 16);
            Serial.println();
          }
        }
      }
      else // Read selected page
      {
        // Print page content
        Serial.print(F("\nContent of page "));
        Serial.print(pageNumber);
        Serial.println(F(":"));
        optiboot_readPage(flashSpace, ramBuffer, pageNumber);
        for (int i = 0; i < SPM_PAGESIZE; i += 16) {
          DumpHex(&ramBuffer[i], 16);
          Serial.println();
        }
      }
      break;

    case CMD_WRITE:
      // Write flash option selected
      if (ttycli.eol()) {
        Serial.print(F("What page number do you want to Write? Page: "));
        ttycli.reset();
        ttycli.getLineWait();
      }
      pageNumber = ttycli.number();

      if (pageNumber < 1 || pageNumber > NUMBER_OF_PAGES) {
        Serial.print(F("\nPlease enter a valid page between 1 and "));
        Serial.print(NUMBER_OF_PAGES);
        Serial.println(F(".\nThe number of pages can be extended by changing NUMBER_OF_PAGES constant"));
        return;  // restart parsing
      }

      // Print prompt to enter some new characters to write to flash
      Serial.print(F("Please type the characters you want to store (max "));
      Serial.print(SPM_PAGESIZE);
      Serial.println(F(" characters)"));
      Serial.print(F("End with a line containing only '"));
      Serial.write(terminationChar);
      Serial.println(F("' character:"));

      // Get all characters from the serial monitor and store it to the ramBuffer
      memset(ramBuffer, 0, sizeof(ramBuffer));
      counter = 0;
      do {
        ttycli.reset();
        ttycli.getLineWait();
        if (ttycli.nextChar() == terminationChar) {
          break;
        }
        char *p = ttycli.restOfLine();
        byte c;
        do {
          c = *p++;
          if (c == 0)
            break;
          ramBuffer[counter++] = c;
          if (counter >= SPM_PAGESIZE) {
            Serial.println(F("Page is full"));
            break;
          }
        } while (1);
      } while (counter < SPM_PAGESIZE);
      Serial.println(F("\n\nAll chars received \nWriting to flash..."));

      // WRITE RECEIVED DATA TO THE CURRENT FLASH PAGE
      // flash_buffer is where the data is stored (contains the memory addresses)
      // ramBuffer contains the data that's going to be stored in the flash
      // pageNumber is the page the data is written to
      optiboot_writePage(flashSpace, ramBuffer, pageNumber);

      Serial.println(F("Writing finished.\nYou can now reset or power cycle the board and check for new contents!"));
      confirm();
      break;

    case CMD_HELP:
      return;
  }
} // End of loop

void confirm()
{
  while (true) {
    Serial.println("\n--type newline to continue--");
    ttycli.reset();
    ttycli.getLineWait();
    if (ttycli.eol()) {
      return;
    }
  }
}
