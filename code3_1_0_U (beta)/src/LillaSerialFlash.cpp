/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "LillaSerialFlash.h"

void LillaSerialFlashFile::fast_open(int id_file)
{
  // se usato per Packet: id_file = id_packet + RAW_FILES

  this->address = FlashFileRegisterParser::address(id_file);
  this->length = FlashFileRegisterParser::length(id_file);
  this->offset = 0;
  this->dirindex = FlashFileRegisterParser::dirindex(id_file);
}

void LillaSerialFlashFile::packet_fast_open(int id_packet)
{
  this->address = FlashFileRegisterParser::address(id_packet + RAW_FILES);
  this->length = FlashFileRegisterParser::length(id_packet + RAW_FILES);
  this->offset = 0;
  this->dirindex = FlashFileRegisterParser::dirindex(id_packet + RAW_FILES);
}

void FlashFileRegisterParser::Read_all_file_data(void)
{
  elapsedMicros T;
  int tempo;
  SerialFlashFile rawfile;

  int index = 0;

  Serial.println("*** Lilla_SerialFlash - rilevamento di tutti i file audio presenti ***");
  for (auto i = 0; i < (RAW_FILES + PACKETS); ++i)
  {
    T = 0;
    if (i < RAW_FILES) // n.raw, n.rec (.liv sono su PSRAM)
    {
      rawfile = SerialFlash.open(name_file[i]);
    }
    else //  P(acket)n.raw
    {
      rawfile = SerialFlash.open(name_packet[i - RAW_FILES]);
    }

    // T e' il tempo impiegato per da SerialFlash per accedere al file ed e' in gran parte dovuto al parsing
    // del registro file nella Flash.
    // Utilizzando FlashFileRegisterParser::fast_open T si annulla perche' l'indirizzo del (primo byte del) file
    // sulla Flash e' annotato in address_array[].
    tempo = T;

    address_array[i] = rawfile.getFlashAddress();
    length_array[i] = rawfile.size();

    if (length_array[i] > 0)
    {
      dirindex_array[i] = index;
      ++index;
    }

    if (false)
    {
      if (i < RAW_FILES)
      {
        Serial.print(name_file[i]);
      }
      else
      {
        Serial.print(name_packet[i - RAW_FILES]);
      }

      Serial.print(" SerialFlash.open() waste time:");
      Serial.print(tempo);
      Serial.print("us  dimension:");
      Serial.print(length_array[i]);
      Serial.print("  index:");
      Serial.print(dirindex_array[i]);
      Serial.print(" address_array:");
      Serial.println(address_array[i]);
    }

    rawfile.close();
  }
}

uint32_t FlashFileRegisterParser::address_array[RAW_FILES + PACKETS] = {0};
uint32_t FlashFileRegisterParser::length_array[RAW_FILES + PACKETS] = {0};
uint16_t FlashFileRegisterParser::dirindex_array[RAW_FILES + PACKETS] = {0};

uint32_t FlashFileRegisterParser::address(int id_file)
{
  return address_array[id_file];
}

uint32_t FlashFileRegisterParser::length(int id_file)
{
  return length_array[id_file];
}

uint16_t FlashFileRegisterParser::dirindex(int id_file)
{
  return dirindex_array[id_file];
}

/*
** On-chip SerialFlash file allocation data structures:

  uint32_t signature = 0xFA96554C;
  uint16_t maxfiles
  uint16_t stringssize  // div by 4
  uint16_t hashes[maxfiles]
  struct {
    uint32_t file_begin
    uint32_t file_length
    uint16_t string_index  // div4
  } fileinfo[maxfiles]
  char strings[stringssize]

PSRAM memory bytes:

0 1 2 3 4 = signature
          5 6 = number of files (max: 2^16 = 65.536)
              7 8 = size of strings section divided by 4 (max size: 4*2^16 = 256KB)

** signature
A 32 bit signature is stored at the beginning of the flash memory.
If 0xFFFFFFFF is seen, the entire chip should be assumed blank.
If any value other than 0xFA96554C is found, a different data format
is stored.  This could should refuse to access the flash.

** Number of files stored, size of the string section
The next 4 bytes store number of files (first 2 bytes) and size of the strings
section (last 2 bytes) divided by 4, which allow the position of every other item to be found.
The string section size is given the 16 bit integer multiplied by 4;
maximum size for string data is 4x2^16 = 256KB.

** Hash
An array of 16 bit filename hashes allows for quick linear search
for potentially matching filenames.  A hash value of 0xFFFF indicates
no file is allocated for the remainder of the array.

hashes space = 2*(max number of files) = 128KB

** "fileinfo": localizzazione del file
Following the hashes, and array of 10 byte structs give the location
and length of the file's actual data, and the offset of its filename
in the strings section.

location&offset space = 10*(max number of files) = 640KB

Strings are null terminated.  The remainder of the chip is file data.
*/