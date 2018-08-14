#include <memory.h>

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
unsigned char ramBank[0x20000];
unsigned char ramBankEnable = 0;
unsigned char romBankRegisterLow = 0;
unsigned char romBankRegisterHigh = 0;
unsigned char currentRamBank = 0;

unsigned short int romBankRegister;

void WriteMemory(unsigned short int address, unsigned char data);
void ClearGBMemory();

//----------------------------------------//
// This function will handle any writes   //
// to the GB memory.                      //
//----------------------------------------//
void WriteMemory(unsigned short int address, unsigned char data)
{
	//if (OAMTransferEnabled == 0)
	{
	if ((address >= 0x0000) && (address <= 0x1FFF))
	{
		// Writing a value of 0xA enables the RAM bank.
		// This value should only be evaluated, not written.
		if (data & 0x0A)
			ramBankEnable = 1;
		// Anything else disables the RAM bank.
		else
			ramBankEnable = 0;
	}

	// If using MBC, switch ROM bank.
	else if ((address >= 0x2000) && (address <= 0x3FFF))
	{
		// Again, the data should only be evaluated.
		if (MBCType > 0)
		{	
			// If using MBC3, the whole ROM bank number is written here.
			if (MBCType == 3)
			{
				romBankRegister = (data & 127);
			}
			// If using MBC5, the lower 8-bits are written to 0x2000-0x2FFF.
			else if ((MBCType == 5) && (address >= 0x2000) && (address <= 0x2FFF))
			{
				romBankRegisterLow = (data & 255);
			}			
			// If using MBC5, the upper 1-bit is written to 0x3000-0x3FFF.
			else if ((MBCType == 5) && (address >= 0x3000) && (address <= 0x3FFF))
			{
				romBankRegisterHigh = (data & 1);
			}
			else
			{	
				// Combine the written data with the register.
				romBankRegisterLow = (data & 31);
			}
			
			if (MBCType == 1)
			{
				romBankRegister = (romBankRegisterHigh << 5) + romBankRegisterLow;

				if (romBankRegister == 0)
					romBankRegister++;
			}
			else if (MBCType == 2)
			{
				romBankRegister = romBankRegisterLow;
			}
			else if (MBCType == 5)
			{
				romBankRegister = (romBankRegisterHigh << 8) + romBankRegisterLow;
			}
			
			// Switch 16K ROM bank into 0x4000.
			memcpy(&memory[0x4000], &romBuffer[romBankRegister << 14], 0x4000);
		}
	}

	else if ((address >= 0x4000) && (address <= 0x5FFF))
	{
		// If the ROM uses MBC.
		if (MBCType > 0)
		{
			// If model is set to 16 Mbit/8K mode.
			if (((MBCType == 1) && (MBC1Model == 0)))
			{	
				// Combine the written data with the register.
				romBankRegisterHigh = (data & 0x03);
								
				romBankRegister = (romBankRegisterHigh << 5) + romBankRegisterLow;
				
				// With MBC1, Banks 0x20, 0x40, and 0x60 will access 0x21, 0x41, and 0x61 respectively.
				if ((romBankRegister & 0x1F) == 0)
					romBankRegister++;

				// Switch in the current ROM bank.
				memcpy(&memory[0x4000], &romBuffer[romBankRegister << 14], 0x4000);

				if (currentRamBank != 0)
				{				
					// Swap out the old RAM bank.
					memcpy(&ramBank[currentRamBank << 13], &memory[0xA000], 0x2000);
					
					// Swap in RAM bank 0x00.
					memcpy(&memory[0xA000], &ramBank[0x0000], 0x2000);
				}
			}

			// If model is set to 4 Mbit/32K mode.
			if (((MBCType == 1) && (MBC1Model == 1)) || (MBCType > 1))
			{
				if ((MBCType != 3) || ((MBCType == 3) && (data <= 0x03)))
				{
					// Swap out the old RAM bank.
					memcpy(&ramBank[currentRamBank << 13], &memory[0xA000], 0x2000);

					// Swap in the new RAM bank.
					memcpy(&memory[0xA000], &ramBank[data << 13], 0x2000);

					currentRamBank = data;
				}
			}
		}
	}

	//----------------------------------------//
	// If using MBC1, select the memory model //
	// to use.  (0 = 2048K ROM/8K RAM,        //
	// 1 = 512K ROM/32K RAM).                 //
	//----------------------------------------//
	else if ((address >= 0x6000) && (address <= 0x7FFF))
	{
		if (MBCType == 1)
		{
			MBC1Model = data & 1;
		}
	}

	else if ((address >= 0x8000) && (address <= 0x9FFF))
	{
		//----------------------------------------//
		// Probably don't need to check the mode  //
		// flags since games shouldn't try writing//
		// here in the first place.               //
		//----------------------------------------//
		
		if (modeFlag != 3)
		{
			memory[address] = data;
		}
	}

	else if ((address >= 0xA000) && (address <= 0xBFFF))
	{
		if (ramBankEnable == 1)
		{
			memory[address] = data;
		}
	}

	else if ((address >= 0xC000) && (address <= 0xDDFF))
	{
		memory[address] = data;
		memory[address + 0x2000] = data;
	}

	else if ((address >= 0xE000) && (address <= 0xFDFF))
	{
		memory[address] = data;
		memory[address - 0x2000] = data;
	}

	else if ((address >= 0xFE00) && (address <= 0xFE9F))
	{
		//----------------------------------------//
		// Probably don't need to check the mode  //
		// flags since games shouldn't try writing//
		// here in the first place.               //
		//----------------------------------------//

		if ((modeFlag != 2) && (modeFlag != 3))
		{
			memory[address] = data;
		}
	}

	// This area is restricted.
	else if ((address >= 0xFEA0) && (address <= 0xFEFF))
	{
	}

	// Take care of the joystick register.
	else if (address == 0xFF00)
	{
		// Make sure that only bits 4 and 5 are changed.
		data &= 48;

		memory[0xFF00] &= 207;
		memory[0xFF00] |= data;
	}

	else if (address == 0xFF02)
	{
		// Don't write to bit 7 since no transfer will be made.
		data &= 129;

		//memory[0xFF02] &= 126;
		memory[0xFF02] |= data;

		// Since there is not another Gameboy connected, set the received data to 0xFF.
		
		if ((memory[0xFF02] & BIT_0) && (memory[0xFF02] & BIT_7))
		{
			memory[0xFF01] = 0xFF;
			IOregister_IF |= BIT_3;

			// Turn off bit 7.
			memory[0xFF02] &= 127;
		}
	}

	else if (address == 0xFF03)
	{
	}

	// If the Divider register is written to, reset it.
	else if (address == 0xFF04)
	{
		memory[0xFF04] = 0;

		dividerCounter = DIVIDER_INTERVAL;
	}

	else if (address == 0xFF07)
	{
		memory[0xFF07] &= 248;
		memory[0xFF07] |= data;

		//----------------------------------------//
		// Make sure the timer is enabled.        //
		//----------------------------------------//
		if (IOregister_TAC & 4)
		{
			//----------------------------------------//
			// Set the timer interval.  I think this  //
			// area is unnecessarily bulky.           //
			//----------------------------------------//
			switch (IOregister_TAC & 3)
			{
			case 0x0:
				{
					timerInterval = 256; // 4096;
				}
			break;
			case 0x1:
				{
					timerInterval = 4; // 262144;
				}
			break;
			case 0x2:
				{
					timerInterval = 16; // 65536
				}
			break;
			case 0x3:
				{
					timerInterval = 64; // 16384
				}
			break;
			}
			
			timerCounter = timerInterval;
		}
	}

	else if ((address >= 0xFF10) && (address <= 0xFF26))
	{
		memory[address] = data;
		/*if (address == 0xFF10)
		{
			memory[0xFF10] = data & 127;
		}
		else if (address == 0xFF11)
		{
			memory[0xFF11] = data;
		}
		else if (address == 0xFF12)
		{
			memory[0xFF12] = data;
		}
		else if (address == 0xFF13)
		{
			memory[0xFF13] = data;
		}
		else if (address == 0xFF14)
		{
			memory[0xFF14] = data;
		}
		else if (address == 0xFF15)
		{
		}
		else if (address == 0xFF16)
		{
			memory[0xFF16] = data;
		}
		else if (address == 0xFF17)
		{
			memory[0xFF17] = data;
		}
		else if (address == 0xFF18)
		{
			memory[0xFF18] = data;
		}
		else if (address == 0xFF19)
		{
			memory[0xFF19] = data;
		}
		else if (address == 0xFF1A)
		{
			memory[0xFF1A] = data & 128;
		}
		else if (address == 0xFF1B)
		{
			memory[0xFF1B] = data;
		}
		else if (address == 0xFF1C)
		{
			memory[0xFF1C] = data &= 96;
		}
		else if (address == 0xFF1D)
		{
			memory[0xFF1D] = data;
		}
		else if (address == 0xFF1E)
		{
			memory[0xFF1E] = data;
		}
		else if (address == 0xFF20)
		{
			memory[0xFF20] = data & 63;
		}
		else if (address == 0xFF21)
		{
			memory[0xFF21] = data;
		}
		else if (address == 0xFF22)
		{
			memory[0xFF22] = data;
		}
		else if (address == 0xFF23)
		{
			memory[0xFF23] = data & 192;
		}
		else if (address == 0xFF24)
		{
			memory[0xFF24] = data;
		}
		else if (address == 0xFF25)
		{
			memory[0xFF25] = data;
		}
		else if (address == 0xFF26)
		{
			memory[0xFF26] = data & 143;
		}*/

		//aud.WriteRegister(address, data);
	}

	// If there is a write to the LCD Control Register...
	else if (address == 0xFF40)
	{
		memory[0xFF40] = data;

		if (!(memory[0xFF40] & BIT_7))
		{
			IOregister_LY = 0;
			LYCounter = VERTICAL_RETRACE_INTERVAL;
			modeFlag = 2;
			retraceCounter = 20;
			tilePosYWin = 0;
			WYTemp = IOregister_WY;
			//memory[0xFF41]
			CheckLYC();

			// If the display is disabled, clear the screen.
			//memcpy(&oldScreenData, &screenData, 0x5A00);
			memset(&screenData, 0, 0x5A00);
		}
		//UpdateScreen();
	}

	// If the STAT register is written to.
	else if (address == 0xFF41)
	{
		// Make sure only allowed bits are written.
		data &= 120;

		memory[0xFF41] |= data;

		// Emulate STAT write during HBLANK or VBLANK bug.
		if ((modeFlag == 0) || (modeFlag == 1))
		{
			memory[0xFF0F] |= BIT_1;
		}
	}

	// If the LY register is written to, reset it.
	else if (address == 0xFF44)
	{
		LYCounter = VERTICAL_RETRACE_INTERVAL;

		IOregister_LY = 0;

		CheckLYC();
	}

	// If the LY Compare register is written to, see if it is equal to LY.
	else if (address == 0xFF45)
	{
		memory[0xFF45] = data;

		CheckLYC();
	}

	else if (address == 0xFF46)
	{
		memcpy(&memory[0xFE00], &memory[(regs.A << 8)], 0xA0);

		// OAM transfer lasts for approx 168 cycles.
		//OAMCounter = 168;

		//OAMTransferEnabled = 1;
	}

	/*else if (address == 0xFF47)
	{
	}

	else if (address == 0xFF48)
	{
	}

	else if (address == 0xFF49)
	{
	}*/

	// This area is restricted.  (0xFF50 may be used for turning off boot rom.)
	else if ((address >= 0xFF4C) && (address <= 0xFF7F))
	{
		if (address == 0xFF50)
		{
			if (data == 1)
			{
				// Overwrite the bootrom in the ROM buffer.
				memcpy(&memory[0x0000], &romBuffer[0x0000], 0x100);
			}
		}
	}
	else
	{
		memory[address] = data;
	}
	}
	/*else if ((address >= 0xFF80) && (address <= 0xFFFE))
	{
		memory[address] = data;
	}*/
}

//----------------------------------------//
// This function zeroes all the GB memory.//
//----------------------------------------//
void ClearGBMemory()
{
	memset(memory, 0xFF, 0x10000);
}
