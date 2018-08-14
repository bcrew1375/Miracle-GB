#include <stdio.h>
#include <windows.h>

// File handle to track open file.
FILE *fileHandle;

char *bootRomFile = "bootrom.bin";

// Variable to hold number of bytes read from file.
unsigned long int bytesRead;

// External variable used to store the ROM binary.
extern unsigned char *bootBuffer;
extern unsigned char *romBuffer;

unsigned int bootRomPresent;

//----------------------------------------//
// Function name: LoadRomFile             //
// Purpose: This function will load a ROM //
// binary into the ROM buffer.            //
// Variables: filename is a string holding//
// the ROM file to be opened.             //
//----------------------------------------//
int LoadRomFile(char filename[])
{
	//----------------------------------------//
	// If bootRom is present, load it into    //
	// memory.                                //
	//----------------------------------------//
	if((fileHandle = fopen(bootRomFile, "rb")) == NULL)
	{
		bootRomFile = "bootrom/bootrom.bin";	// If not found in program folder, check "bootrom" subfolder.
		
		if((fileHandle = fopen(bootRomFile, "rb")) == NULL)
			bootRomPresent = 0;
	}
	else
	{
		bytesRead = fread(bootBuffer, 1, 0x100, fileHandle);
		bootRomPresent = 1;
	}
	//----------------------------------------//
	// Try to open the file in Read-Only      //
	// binary mode, if it fails, return with  //
	// an error.                              //
	//----------------------------------------//
	if((fileHandle = fopen(filename, "rb")) == NULL)
		return -1;

	//----------------------------------------//
	// Read up to the max size of a GB ROM.   //
	//----------------------------------------//
	bytesRead = fread(romBuffer, 1, 0x200000, fileHandle);

	//----------------------------------------//
	// Close the file.                        //
	//----------------------------------------//
	fclose(fileHandle);

	//----------------------------------------//
	// See if there were any problems reading //
	// the file and make sure the file size   //
	// is not 0.                              //
	//----------------------------------------//
	if ((!bytesRead) || (bytesRead == 0))
		return -2;

	//----------------------------------------//
	// Return with no errors.                 //
	//----------------------------------------//
	return 0;
}
