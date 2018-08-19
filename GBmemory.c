/*#include <memory.h>

#include "Common.h"
#include "define.h"
#include "GBmemory.h"
//#include "gbsound/audio.h"

extern unsigned char *romBuffer;
extern unsigned char *memory;
extern unsigned char screenData[160 * 144];
//extern struct AUDIO aud;

extern void CheckLYC();

//----------------------------------------//
// Data for Memory Bank Controllers.      //
//----------------------------------------//



void WriteMemory(unsigned short int address, unsigned char data);
void ClearGBMemory();

//----------------------------------------//
// This function will handle any writes   //
// to the GB memory.                      //
//----------------------------------------//


//----------------------------------------//
// This function zeroes all the GB memory.//
//----------------------------------------//
/*
*/