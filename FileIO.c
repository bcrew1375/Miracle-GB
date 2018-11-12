#include <stdio.h>
#include <windows.h>

// Handle to track open file.
FILE *handle;
FILE *romHandle;

char *bootRomFile = "bootrom.bin";

// Variable to hold number of bytes read from file.
unsigned long int bytesRead;

// External variable used to store the ROM binary.
unsigned char romBuffer[0x1000000];

unsigned int bootRomPresent;

//----------------------------------------//
// Function name: LoadRomFile             //
// Purpose: This function will load a ROM //
// binary into the ROM buffer.            //
// Variables: filename is a string holding//
// the ROM file to be opened.             //
//----------------------------------------//
unsigned char * LoadRomFile(char filename[], unsigned long int *bytesRead)
{
	//----------------------------------------//
	// If bootRom is present, load it into    //
	// memory.                                //
	//----------------------------------------//
	/*if((romHandle = fopen(bootRomFile, "rb")) == NULL)
	{
		bootRomFile = "bootrom/bootrom.bin";	// If not found in program folder, check "bootrom" subfolder.
		
		if((romHandle = fopen(bootRomFile, "rb")) == NULL)
			bootRomPresent = 0;
	}
	else
	{
		bytesRead = fread(bootBuffer, 1, 0x100, romHandle);
		bootRomPresent = 1;
	}*/
	//----------------------------------------//
	// Try to open the file in Read-Only      //
	// binary mode, if it fails, return with  //
	// an error.                              //
	//----------------------------------------//
	if((romHandle = fopen(filename, "rb")) == NULL)
		return 0;

	//----------------------------------------//
	// Read up to the max size of a GB ROM.   //
	//----------------------------------------//
	*bytesRead = fread(romBuffer, 1, 0x1000000, romHandle);

	//----------------------------------------//
	// Close the file.                        //
	//----------------------------------------//
	fclose(romHandle);

	//----------------------------------------//
	// See if there were any problems reading //
	// the file and make sure the file size   //
	// is not 0.                              //
	//----------------------------------------//
	if ((!bytesRead) || (bytesRead == 0))
		return 0;

	//----------------------------------------//
	// Return with no errors.                 //
	//----------------------------------------//
	return romBuffer;
}

int OpenLogFile()
{
	handle = fopen("log.txt", "w");
	if (!handle)
		return 1;
	else
		return 0;
}

void WriteToLog(unsigned char *writeData)
{
	fprintf(handle, writeData);
}