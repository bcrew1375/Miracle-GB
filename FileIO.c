#include <stdio.h>
#include <windows.h>

// Handle to track open file.
FILE *handle;

// Define the maximum size in bytes the loaded file can be.
unsigned int maxFileSize = 0x1000000;

char *bootRomFile = "bootrom.bin";

// Variable to hold number of bytes read from file.
unsigned long int bytesRead;


unsigned int bootRomPresent;

//----------------------------------------//
// Function name: LoadRomFile             //
// Purpose: This function will load a ROM //
// binary into the ROM buffer.            //
// Variables: filename is a string holding//
// the ROM file to be opened.             //
//----------------------------------------//
/*unsigned char* LoadRomFile(char filename[], )
{
	FILE *romHandle;
	unsigned char *fileBuffer;
	unsigned long int fileSize;

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
/*	if((romHandle = fopen(filename, "rb")) == NULL)
		return -1;
	
	// Make sure the file doesn't exceed maximum allowed size
	fileSize = filelength(romHandle);
	if (fileSize > maxFileSize)
		return -2;
	fileBuffer = malloc(fileSize);
	if (!fileBuffer)
		return -3;
	rewind(romHandle);

	//----------------------------------------//
	// Read up to the max size of a GB ROM.   //
	//----------------------------------------//
	fread(fileBuffer, 1, 0x200000, romHandle);
		return -4;

	//----------------------------------------//
	// Close the file.                        //
	//----------------------------------------//
	fclose(romHandle);

	//----------------------------------------//
	// Return with no errors.                 //
	//----------------------------------------//
	return &fileBuffer;
}*/

int OpenLogFile()
{
	handle = fopen("log.txt", "w");
	if (!handle)
		return 1;
	else
		return 0;
}

void WriteToLog(const char *writeData)
{
	fprintf(handle, writeData, NULL);
}