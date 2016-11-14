#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Coverage options
#define DO_STATEMENT_SINGLE 0x01
#define DO_STATEMENT_COUNT  0x02
#define DO_BRANCH           0x04
#define DO_MCDC             0x08
#define DO_CONDITION        0x10

char _vamp_filename[256];
unsigned char _vamp_inName = 1;    // Flag waiting for filename to save data to
unsigned char _vamp_inOptions = 0; // Flag waiting for coverage options byte
unsigned char _vamp_inCount = 0;   // Flag getting byte count
unsigned short _vamp_count = 0;    // Number of bytes of data to load
unsigned char _vamp_options = 0;   // Coverage options
unsigned char _vamp_chkOpt = 1;    // Walk through options to find next
unsigned char _vamp_done = 0;      // Done with history

// Determine next coverage option to load
void _vamp_nextOption(void)
{
  while ((_vamp_chkOpt <= DO_CONDITION) && ((_vamp_options & _vamp_chkOpt) == 0))
    _vamp_chkOpt <<= 1;
  if (_vamp_chkOpt <= DO_CONDITION)
    _vamp_inCount = 2;  // Get 2 bytes of count
  else
    _vamp_done = 1;
}

void _vamp_send(unsigned char data)
{
  char *ext;
#ifdef UNBUFFERED_IO
  static int fd;
#else
  static FILE *fd;
#endif

  if (_vamp_inName)
  {
    // Build filename
    _vamp_filename[_vamp_count++] = data;

    if (data == 0)
    {
       // Filename received
       _vamp_inName = 0;
       _vamp_inOptions = 1;

       // Find extension
       ext = strrchr(_vamp_filename, '.');

       // Remove if found
       if (ext)
         *ext = '\0';

       // Append new extension
       strcat(_vamp_filename, ".hist");

       // Open output file
#ifdef UNBUFFERED_IO
       fd = creat(_vamp_filename, 0666);
       if (fd == -1)
#else
       fd = fopen(_vamp_filename, "wb");
       if (fd == NULL)
#endif
       {
         perror("Could not create file");
         exit(0);
       }
    }
  }
  else
  {
    // Add to output file
#ifdef UNBUFFERED_IO
    write(fd, (const void *) &data, 1);
#else
    fwrite((const void *) &data, 1, 1, fd);
#endif

    if (_vamp_inOptions)
    {
      _vamp_options = data;
      _vamp_inOptions = 0;
      _vamp_count = 0;
      _vamp_nextOption();
    }
    else
    if (_vamp_inCount)
    {
      _vamp_count = (_vamp_count << 8) | data;
      if (--_vamp_inCount == 0)
      {
        // Got both bytes of count - now get data
        if (_vamp_chkOpt == DO_STATEMENT_COUNT)
        {
          // Adjust count to get 4 bytes of statement count
          _vamp_count *= 4;
        }
        else
        if (_vamp_chkOpt == DO_MCDC)
        {
          // Adjust count to get 2 bytes of stack overflow
          _vamp_count += 2;
        }
      }
    }
    else
    {
      --_vamp_count;
    }

    if ((_vamp_inCount == 0) && (_vamp_count == 0))
    {
      // Done processing data, switch to next state
      _vamp_chkOpt <<= 1;
      _vamp_nextOption();

      if (_vamp_done)
      {
        // All data loaded, close file and wait for next to process
#ifdef UNBUFFERED_IO
        close(fd);
#else
        fclose(fd);
#endif
        _vamp_inName = 1;
        _vamp_chkOpt = 1;
        _vamp_done = 0;
      }
    }
  }
}

