#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <sdl.h>
#include <windows.h>

#include "define.h"

struct Emulation
{
	struct MemoryMap
	{
		unsigned char romBank0[0x4000];		// Primary rom bank from 0x0000-0x3FFF.
		unsigned char romBank1[0x4000];		// Secondary rom bank from 0x4000-0x7FFF (Switchable on MBC carts).
		unsigned char videoRam[0x2000];		// The Gameboy's 8 KB video RAM from 0x8000-0x9FFF.
		unsigned char ramBank[0x2000];		// An 8 KB switchable RAM bank that is supported by some cartridges from 0xA000-0xBFFF.
		unsigned char internRam[0x2000];	// The Gameboy's 8 KB internal RAM from 0xC000-0xDFFF, this is also echoed from 0xE000-FDFF.
		unsigned char sprite[0xA0];			// Sprite Attribute Memory (OAM) from 0xFE00-0xFE9F.
											// The address space from FEA0-FEFF is unusable.
		unsigned char ioRegs[0x4C];			// I/O registers from 0xFF00-0xFF4B.  FF03, FF08-FF0E, FF15, FF1F, and FF27-FF2F are unused.
											// The address space from FF4C-FF7F is unusable.
		unsigned char highRam[0x7F];		// The Gameboy's high RAM from 0xFF80-FFFE.
		unsigned char intrpFlags;			// The interrupt enable(IE) flags at 0xFFFF.
	} memory;

	struct Cartridge
	{
		unsigned char *dataBuffer;  // Buffer that will hold the data from the cartridge.
		unsigned char romBankRegister;  // This will store the currently selected MBC ROM bank.
		unsigned int mbcType;
		unsigned int numberOfRomBanks;  // Needed to calculate some MBC bank-switching behavior.
		unsigned int systemType;
	} cart;

	struct CPU
	{
		union Registers
		{
			struct { unsigned short int AF, BC, DE, HL, SP, PC; };
			struct { unsigned char F, A, C, B, E, D, L, H, SPL, SPH, PCL, PCH; };
		} regs;
	} cpu;

	struct State
	{
		boolean eiDelay;  // EI instruction enables interrupts one instruction after its execution.
		boolean intrpEnable;  // Master interrupt enable switch.
		boolean halted;
		boolean haltInstructionRepeat;
		boolean running;
		boolean stopped;
	} state;

	struct Cycles
	{
		unsigned short int internalCounterCycles;  // Cycle counter for DIV and TIMA register.
		unsigned short int previousInternalCounterCycles;  // Cycle counter for DIV and TIMA register.
		unsigned short int timaIncCounterCycles;  // A cycle counter for incrementing the TIMA register.
		unsigned int statCycles;  // Cycle counter for STAT register.
		unsigned char opCycles[0x100];  // Store the number of clock cycles for every instruction
		unsigned char opCBCycles[0x100];  // Store cycles for CB bit instructions
	} cycles;

	struct IO
	{
		struct Display
		{
			boolean lcdDelay;  // The LCD will display nothing during the first frame it is enabled.
			unsigned char tileBuffer[384][8][8];  // A buffer that will hold the tile table converted to 8-bits.
			unsigned char spriteBuffer[40][40];  // A buffer that will hold sorted sprite information.
		} display;
	} io;
} emu;

// Standard Gameboy opcode clock cycles.
unsigned char GB_CycleTable[0x100] = {
			//0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
	/*0x00*/    4,  12,   8,   8,   4,   4,   8,   4,  20,   8,   8,   8,   4,   4,   8,   4,
	/*0x10*/    0,  12,   8,   8,   4,   4,   8,   4,  12,   8,   8,   8,   4,   4,   8,   4,
	/*0x20*/    8,  12,   8,   8,   4,   4,   8,   4,   8,   8,   8,   8,   4,   4,   8,   4,
	/*0x30*/    8,  12,   8,   8,  12,  12,  12,   4,   8,   8,   8,   8,   4,   4,   8,   4,
	/*0x40*/    4,   4,   4,   4,   4,   4,   8,   4,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0x50*/    4,   4,   4,   4,   4,   4,   8,   4,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0x60*/    4,   4,   4,   4,   4,   4,   8,   4,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0x70*/    8,   8,   8,   8,   8,   8,   0,   8,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0x80*/    4,   4,   4,   4,   4,   4,   8,   4,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0x90*/    4,   4,   4,   4,   4,   4,   8,   4,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0xA0*/    4,   4,   4,   4,   4,   4,   8,   4,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0xB0*/    4,   4,   4,   4,   4,   4,   8,   4,   4,   4,   4,   4,   4,   4,   8,   4,
	/*0xC0*/    8,  12,  12,  16,  12,  16,   8,  16,   8,  16,  12,   0,  12,  24,   8,  16,
	/*0xD0*/    8,  12,  12,   0,  12,  16,   8,  16,   8,  16,  12,   0,  12,   0,   8,  16,
	/*0xE0*/   12,  12,   8,   0,   0,  16,   8,  16,  16,   4,  16,   0,   0,   0,   8,  16,
	/*0xF0*/   12,  12,   8,   4,   0,  16,   8,  16,  12,   8,  16,   4,   0,   0,   8,  16
};

// Gameboy bit operation clock cycles
unsigned char GB_CBCycleTable[0x100] = {
			//0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
	/*0x00*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0x10*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0x20*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0x30*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0x40*/    8,   8,   8,   8,   8,   8,  12,   8,   8,   8,   8,   8,   8,   8,  12,   8,
	/*0x50*/    8,   8,   8,   8,   8,   8,  12,   8,   8,   8,   8,   8,   8,   8,  12,   8,
	/*0x60*/    8,   8,   8,   8,   8,   8,  12,   8,   8,   8,   8,   8,   8,   8,  12,   8,
	/*0x70*/    8,   8,   8,   8,   8,   8,  12,   8,   8,   8,   8,   8,   8,   8,  12,   8,
	/*0x80*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0x90*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0xA0*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0xB0*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0xC0*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0xD0*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0xE0*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8,
	/*0xF0*/    8,   8,   8,   8,   8,   8,  16,   8,   8,   8,   8,   8,   8,   8,  16,   8
};

//Blank 8-bit table for possible future use
//unsigned char GB_BitCycleTable[0x100] = {//0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
//	/*0x00*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x10*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x20*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x30*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x40*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x50*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x60*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x70*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x80*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0x90*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0xA0*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0xB0*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0xC0*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0xD0*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0xE0*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,
//	/*0xF0*/     ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,    ,

unsigned char opcodeDescription[256][32];

// An array that holds the pixel data that will actually be drawn to the screen.
unsigned char screenData[0x5A00];

//----------------------------------------//
// Gameboy status arrays.                 //
//----------------------------------------//
unsigned char joyState[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

//----------------------------------------//
// Miscellaneous variables                //
//----------------------------------------//
unsigned char logText[1024];

unsigned long long int FPS = 0;
unsigned int SpeedKey = 0;
unsigned int LCDCInterruptExecuted = 0;
unsigned int previousPCvalue = 0;

unsigned char WinYPrevious = 0;

SDL_TimerID FPSTimerID;

//----------------------------------------//
// Miscellaneous arrays                   //
//----------------------------------------//
unsigned char errorText[256];

//----------------------------------------//
// Define all functions used for the CPU  //
// instructions.                          //
//----------------------------------------//
void z80_ADC_A_immediate();
void z80_ADC_A_reg8(unsigned char *reg);
void z80_ADD_A_immediate();
void z80_ADD_A_reg8(unsigned char *reg);
void z80_ADD_HL_reg16(unsigned short int *reg);
void z80_ADD_SP_immediate();
void z80_AND_immediate();
void z80_AND_reg8(unsigned char *reg);
void z80_BIT_bit_reg8(unsigned char bit, unsigned char *reg);
int z80_CALL_condition_immediate(unsigned char condition);
void z80_CALL_immediate();
void z80_CCF();
void z80_CP_immediate();
void z80_CP_reg8(unsigned char *reg);
void z80_CPL();
void z80_DAA();
void z80_DEC_location_HL();
void z80_DEC_reg16(unsigned short int *reg);
void z80_DEC_reg8(unsigned char *reg);
void z80_DI();
void z80_EI();
void z80_HALT();
void z80_INC_location_HL();
void z80_INC_reg16(unsigned short int *reg);
void z80_INC_reg8(unsigned char *reg);
int z80_JP_condition_immediate(unsigned char condition);
void z80_JP_immediate();
int z80_JR_condition_offset(unsigned char condition);
void z80_JR_offset();
void z80_LD_A_location_immediate();
void z80_LD_A_location_reg16(unsigned short int *reg);
void z80_LD_A_0xFF00_C();
void z80_LD_A_0xFF00_immediate();
void z80_LD_HL_SP_immediate();
void z80_LD_location_HL_immediate();
void z80_LD_location_HL_reg8(unsigned char *reg);
void z80_LD_location_immediate_A();
void z80_LD_location_SP();
void z80_LD_reg16_value(unsigned char *hiReg, unsigned char *loReg);
void z80_LD_location_reg16_A(unsigned short int *reg);
void z80_LD_reg8_value(unsigned char *reg);
void z80_LD_reg8_location_HL(unsigned char *reg);
void z80_LD_reg8_reg8(unsigned char *reg1, unsigned char *reg2);
void z80_LD_SP_HL();
void z80_LD_0xFF00_C_A();
void z80_LD_0xFF00_immediate_A();
void z80_LDD_A_HL();
void z80_LDD_HL_A();
void z80_LDI_A_HL();
void z80_LDI_HL_A();
//void z80_NOP();
void z80_OR_immediate();
void z80_OR_reg8(unsigned char *reg);
void z80_POP_reg16(unsigned char *hiReg, unsigned char *loReg);
void z80_PUSH_reg16(unsigned char *hiReg, unsigned char *loReg);
void z80_RES_bit_reg8(unsigned char bit, unsigned char *reg);
void z80_RET();
int z80_RET_condition(unsigned char condition);
void z80_RL_reg8(unsigned char *reg);
void z80_RLA();
void z80_RLC_reg8(unsigned char *reg);
void z80_RLCA();
void z80_RR_reg8(unsigned char *reg);
void z80_RRA();
void z80_RRC_reg8(unsigned char *reg);
void z80_RRCA();
void z80_RST(unsigned char address);
void z80_SBC_A_immediate();
void z80_SBC_A_reg8(unsigned char *reg);
void z80_SCF();
void z80_SET_bit_reg8(unsigned char bit, unsigned char *reg);
void z80_SLA_reg8(unsigned char *reg);
void z80_SRA_reg8(unsigned char *reg);
void z80_SRL_reg8(unsigned char *reg);
void z80_STOP();
void z80_SUB_immediate();
void z80_SUB_reg8(unsigned char *reg);
void z80_SWAP_reg8(unsigned char *reg);
void z80_XOR_immediate();
void z80_XOR_reg8(unsigned char *reg);

//-----------------------------------------//
// Define all other functions.             //
//-----------------------------------------//
void CheckLYC();
void OpcodeError(unsigned char *errorText);
void DrawScanline();
int EmulationInitialize(unsigned char *fileBuffer, unsigned long int fileSize);
int HandleInterrupts();
void RunEmulation();
void UpdateIORegisters();
void UpdateJoypadIORegister();
void WriteMemory(unsigned short int address, unsigned char data);
void FillTileBuffer(unsigned short int tileNumber);

extern void UpdateFPS();
extern void LimitFPS();
extern void HandleSDLEvents();
extern void SetupSDLTimers();
extern void OpcodeError();
extern int OpenLogFile();
extern void WriteToLog(unsigned char *writeData);

// This will check if the read address is readable and return the data from the proper location.
unsigned char ReadMemory(unsigned short int address)
{
	if (address <= 0x3FFF)
		return emu.memory.romBank0[address];
	else if ((address >= 0x4000) && (address <= 0x7FFF))
		return emu.memory.romBank1[address - 0x4000];
	else if ((address >= 0x8000) && (address <= 0x9FFF))
		return emu.memory.videoRam[address - 0x8000];
	else if ((address >= 0xA000) && (address <= 0xBFFF))
		return emu.memory.ramBank[address - 0xA000];
	else if ((address >= 0xC000) && (address <= 0xDFFF))
		return emu.memory.internRam[address - 0xC000];
	else if ((address >= 0xE000) && (address <= 0xFDFF))  // Echo Ram.  Returns the same value as the C000-DDFF space.
		return emu.memory.internRam[address - 0xE000];
	else if ((address >= 0xFE00) && (address <= 0xFE9F))  // Read from the sprite OAM RAM.
		return emu.memory.sprite[address - 0xFE00];
	//else if ((address >= 0xFEA0) && (address <= 0xFEFF))  // Restricted memory space.
	//	return 0xFF;
	else if ((address >= 0xFF00) && (address <= 0xFF4B))  // Read from an I/O register.
		return emu.memory.ioRegs[address - 0xFF00];
	else if ((address >= 0xFF4C) && (address <= 0xFF7F))  // Restricted memory space.
	{
		// Some of these addresses return specific values.  This shouldn't matter as a typical ROM won't try
		// to read or write these.  This is mainly included for authenticity.
		if (address == 0xFF4F)
			return 0xFF;
		else if (address == 0xFF68)
			return 0xFF;
		else if (address == 0xFF6A)
			return 0xFF;
		else if (address == 0xFF72)
			return 0x40;
		else
			return 0x00;
	}
	else if ((address >= 0xFF80) && (address <= 0xFFFE))
		return emu.memory.highRam[address - 0xFF80];
	else if (address == 0xFFFF)
		return emu.memory.intrpFlags;
	else
		return 0xFF;
}

// This will check whether a write to memory is valid and if any special location is written
void WriteMemory(unsigned short int address, unsigned char data)
{
	// Make sure the instruction isn't trying to write to the ROM bank areas.
	if (address <= 0x7FFF)
	{
		// See if this is a special memory write.
		if ((address >= 0x2000) && (address <= 0x3FFF))
			if ((emu.cart.mbcType >= 1) && (emu.cart.mbcType <= 3))
			{
				// The low 5 bits of the 7-bit ROM bank are selected here.
				// 0 or ROM banks that are a multiple of 0x20 refer to the next ROM bank (e.g. 0x20 = 0x21, 0x40 = 0x41).
				// The actual ROM bank selected is the modulus of written data by the number of ROM banks.
				if ((data % 0x20) == 0)
					data = 1;
				data %= emu.cart.numberOfRomBanks;
				// This will combine the lower 5 bits of the written data with the ROM bank register.
				emu.cart.romBankRegister = (emu.cart.romBankRegister & 0xE0) | (data & 0x1F);
				memcpy(&emu.memory.romBank1[0], &emu.cart.dataBuffer[emu.cart.romBankRegister * 0x4000], 0x4000);
			}
		else if ((address >= 0x4000) && (address <= 0x5FFF))
			if ((emu.cart.mbcType >= 1) && (emu.cart.mbcType <= 3))
			{
				// 0 or ROM banks that are a multiple of 0x20 refer to the next ROM bank (0x20 = 0x21, 0x40 = 0x41).
				// The high 2 bits of the 7-bit ROM bank are selected here.
				emu.cart.romBankRegister = (emu.cart.romBankRegister & 0x1F) | (data & 0x60);
				memcpy(&emu.memory.romBank1[0], &emu.cart.dataBuffer[emu.cart.romBankRegister * 0x4000], 0x4000);
			}
	}
	
	else if ((address >= 0x8000) && (address <= 0x9FFF))
	{
		emu.memory.videoRam[address - 0x8000] = data;
		// If the write is to the tile data area and a complete tile(16 bytes) has been written,
		// also write it to an 8-bit tile buffer.  This will reduce the work needed when drawing the screen.
		if ((address <= 0x97FF) && (((address - 0x8000) & 0xF) == 0xF))
			FillTileBuffer((address - 0x8000) / 0x10);
	}
	else if ((address >= 0xA000) && (address <= 0xBFFF))
		emu.memory.ramBank[address - 0xA000] = data;
	else if ((address >= 0xC000) && (address <= 0xDFFF))
		emu.memory.internRam[address - 0xC000] = data;
	else if ((address >= 0xE000) && (address <= 0xFDFF))  // Echo Ram.  Writes the value to the C000-DDFF space.
		emu.memory.internRam[address - 0xE000] = data;
	else if ((address >= 0xFE00) && (address <= 0xFE9F))  // Write to the sprite OAM RAM.
		emu.memory.sprite[address - 0xFE00] = data;
	else if ((address >= 0xFEA0) && (address <= 0xFEFF))  // Restricted memory space.
		return;
	else if ((address >= 0xFF00) && (address <= 0xFF4B))  // Write to an I/O register.
	{
		if (address == 0xFF00)
			IOregister_P1 = 0xCF + (data & 0x30);  // Only bits 4 and 5 of the P1 register are writable.

		else if (address == 0xFF01)
			IOregister_SB = data;

		else if (address == 0xFF02)
		{
			// If a serial transfer was attempted, set the received data to 0xFF since no second Gameboy is present.
			// The interrupt is only triggered if the system has set itself as the master Gameboy.
			if ((data & BIT_7) && (data & BIT_0))
				IOregister_IF |= BIT_3;
			IOregister_SB = 0xFF;
		}

		// Reset the internal counter if DIV is written to.
		else if (address == 0xFF04)
		{
			emu.cycles.internalCounterCycles = 0;
			emu.cycles.previousInternalCounterCycles = 0;
			emu.cycles.timaIncCounterCycles = 0;
		}

		else if (address == 0xFF05)
			IOregister_TIMA = data;

		else if (address == 0xFF06)
			IOregister_TMA = data;

		else if (address == 0xFF07)
		{
			IOregister_TAC = 0xF8 + (data & 0x07);  // Only the low 3 bits of TAC can be written.

			emu.cycles.timaIncCounterCycles = 0;
			//emu.cycles.previousInternalCounterCycles = 0;
		}
		else if (address == 0xFF0F)
			IOregister_IF = 0xE0 + (data & 0x1F);  // Only the low 5 bits of IF can be written.

		else if (address == 0xFF10)
			IOregister_NR10 = 0x80 + (data & 0x7F);  // The 7th bit always returns 1.

		else if (address == 0xFF40)
		{
			IOregister_LCDC = data;

			if (!(IOregister_LCDC & BIT_7))
			{
				// If the LCD is turned off, STAT mode, LY, and triggered display interrupts are all reset, but the LY/LYC compare bit and enabled STAT interrupt are retained.
				IOregister_STAT &= (BIT_0_OFF & BIT_1_OFF);
				IOregister_LY = 0;
				//				IOregister_IF &= (BIT_0_OFF & BIT_1_OFF);
				emu.cycles.statCycles = 0;
				emu.io.display.lcdDelay = 1;
			}
		}

		else if (address == 0xFF41)
			IOregister_STAT = (BIT_7 | (data & 0x78)) | IOregister_STAT & (BIT_1 | BIT_0);  // Make sure the mode flag is not affected and the 7th bit always returns 1.

		else if (address == 0xFF42)
			IOregister_SCY = data;

		else if (address == 0xFF43)
			IOregister_SCX = data;

		else if (address == 0xFF44)
			IOregister_LY = data;

		else if (address == 0xFF45)
			IOregister_LYC = data;

		else if (address == 0xFF46)
		{
			IOregister_DMA = data;
			for (int i = 0; i < 0xA0; i++)
				emu.memory.sprite[i] = ReadMemory((IOregister_DMA << 8) + i);  // If data is written to the OAM register, begin an OAM transfer.
		}

		else if (address == 0xFF47)
			IOregister_BGP = data;

		else if (address == 0xFF48)
			IOregister_OBP0 = data;

		else if (address == 0xFF49)
			IOregister_OBP1 = data;

		else if (address == 0xFF4A)
			IOregister_WY = data;

		else if (address == 0xFF4B)
			IOregister_WX = data;
		else
			emu.memory.ioRegs[address - 0xFF00] = data;
	}
	//else if ((address >= 0xFF4C) && (address <= 0xFF7F))  // Restricted memory space.
	//	return;
	else if ((address >= 0xFF80) && (address <= 0xFFFE))
		emu.memory.highRam[address - 0xFF80] = data;

	else if (address == 0xFFFF)
		IOregister_IE = data;
		//IOregister_IE = 0xE0 | (data & 0x1F);  // Only the low 5-bits of IE can be written.
}

//----------------------------------------//
// This instruction will add a given value//
// plus the carry flag to register A.     //
//----------------------------------------//
void z80_ADC_A_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);
	
	// If there is a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((emu.cpu.regs.A & 0xF) + (value & 0xF) + FLAG_C) > 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((emu.cpu.regs.A + value + FLAG_C) > 0xFF)
	{
		emu.cpu.regs.A += value + FLAG_C;
		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		emu.cpu.regs.A += value + FLAG_C;
		emu.cpu.regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag N is reset.
	emu.cpu.regs.F &= FLAG_N_OFF;
	
	// Increment Program Counter to skip read value.
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will add an 8-bit     //
// register plus the carry flag to        //
// register A.                            //
//----------------------------------------//
void z80_ADC_A_reg8(unsigned char *reg)
{
	// If there will be a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((emu.cpu.regs.A & 0xF) + (*reg & 0xF) + FLAG_C) > 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((emu.cpu.regs.A + *reg + FLAG_C) > 0xFF)
	{
		emu.cpu.regs.A += *reg + FLAG_C;
		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		emu.cpu.regs.A += *reg + FLAG_C;
		emu.cpu.regs.F &= FLAG_C_OFF;
	}
	
	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag N is reset.
	emu.cpu.regs.F &= FLAG_N_OFF;
}

//----------------------------------------//
// This instruction will add a given value//
// to register A.                         //
//----------------------------------------//
void z80_ADD_A_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);
	
	// If there will be a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((emu.cpu.regs.A & 0xF) + (value & 0xF)) > 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((emu.cpu.regs.A + value) > 0xFF)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;
	
	// Add *reg to register A.
	emu.cpu.regs.A += value;

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag N is reset.
	emu.cpu.regs.F &= FLAG_N_OFF;

	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will add an 8-bit     //
// register's value to register A.        //
//----------------------------------------//
void z80_ADD_A_reg8(unsigned char *reg)
{
	// If there will be a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((emu.cpu.regs.A & 0xF) + (*reg & 0xF)) > 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((emu.cpu.regs.A + *reg) > 0xFF)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;
	
	// Add *reg to register A.
	emu.cpu.regs.A += *reg;

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag N is reset.
	emu.cpu.regs.F &= FLAG_N_OFF;
}

//----------------------------------------//
// This instruction will add the value of //
// a 16-bit register to register HL.      //
//----------------------------------------//
void z80_ADD_HL_reg16(unsigned short int *reg)
{
	// If there will be a carry from bit 11, set flag H.
	if (((emu.cpu.regs.HL & 0xFFF) + (*reg & 0xFFF)) > 0xFFF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 15, set flag C.
	if ((emu.cpu.regs.HL + *reg) > 0xFFFF)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;
	
	// Add *reg to HL.
	emu.cpu.regs.HL += *reg;

	// Flag N is reset.
	emu.cpu.regs.F &= FLAG_N_OFF;
}

//----------------------------------------//
// This instruction will add a given,     //
// signed 8-bit value to the              //
// Stack pointer.                         //
//----------------------------------------//
void z80_ADD_SP_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);

	// The flags are determined with an unsigned value.
	
	// Turn on flag H if there is a carry from bit 3.
	if (((emu.cpu.regs.SP & 0xF) + (value & 0xF)) > 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Turn on flag C if the result is above 0xFFFF, or below 0x0000.
	if (((emu.cpu.regs.SP & 0xFF) + value) > 0xFF)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	emu.cpu.regs.SP += (signed char)value;

	// Flag Z and N are reset.
	emu.cpu.regs.F &= (FLAG_Z_OFF & FLAG_N_OFF);

	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will logical AND a    //
// given value with register A.           //
//----------------------------------------//
void z80_AND_immediate()
{
	// Logically AND register A with an immediate value.
	emu.cpu.regs.A &= ReadMemory(emu.cpu.regs.PC);

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag_H is set.
	emu.cpu.regs.F |= FLAG_H_ON;

	// Flags N and C are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_C_OFF);

	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will logical AND a    //
// given value with an 8-bit register.    //
//----------------------------------------//
void z80_AND_reg8(unsigned char *reg)
{
	// Logically AND register A with *reg.
	emu.cpu.regs.A &= *reg;

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag_H is set.
	emu.cpu.regs.F |= FLAG_H_ON;

	// Flags N and C are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_C_OFF);
}

//----------------------------------------//
// This instruction will test a bit of an //
// 8-bit register.                        //
//----------------------------------------//
void z80_BIT_bit_reg8(unsigned char bit, unsigned char *reg)
{
	// If result is 0, set flag Z, otherwise reset it.
	if ((*reg & bit) == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag N is reset.
	emu.cpu.regs.F &= FLAG_N_OFF;

	// Flag H is set.
	emu.cpu.regs.F |= FLAG_H_ON;
}

//----------------------------------------//
// This instruction will push the current //
// value of the Program Counter onto the  //
// stack and jump to a new address, but   //
// only if the given condition is met.    //
//----------------------------------------//
int z80_CALL_condition_immediate(unsigned char condition)
{
	unsigned short int callAddress;
	unsigned int conditionTrue;

	callAddress = (ReadMemory(emu.cpu.regs.PC) + (ReadMemory(emu.cpu.regs.PC + 1) << 8));
	conditionTrue = 0;

	switch (condition)
	{
	case 0x01:
	{
		if (FLAG_Z == 0)
			conditionTrue = 1;
	}
	break;
	case 0x02:
	{
		if (FLAG_Z == 1)
			conditionTrue = 1;
	}
	break;
	case 0x03:
	{
		if (FLAG_C == 0)
			conditionTrue = 1;
	}
	break;
	case 0x04:
	{
		if (FLAG_C == 1)
			conditionTrue = 1;
	}
	break;
	}

	emu.cpu.regs.PC += 2;

	if (conditionTrue == 1)
	{
		// Push address of next instruction onto the stack.
		emu.cpu.regs.SP--;
		WriteMemory(emu.cpu.regs.SP, emu.cpu.regs.PCH);
		emu.cpu.regs.SP--;
		WriteMemory(emu.cpu.regs.SP, emu.cpu.regs.PCL);

		emu.cpu.regs.PC = callAddress;
		return 12;  // return additional cycles if jump was executed.
	}
	else
		return 0;
}

//----------------------------------------//
// This instruction will push the current //
// value of the Program Counter onto the  //
// stack and jump to a new address.       //
//----------------------------------------//
void z80_CALL_immediate()
{
	unsigned short int callAddress;

	callAddress = ReadMemory(emu.cpu.regs.PC) + (ReadMemory(emu.cpu.regs.PC + 1) << 8);

	emu.cpu.regs.PC += 2;

	// Push address of next instruction onto the stack.
	emu.cpu.regs.SP--;
	WriteMemory(emu.cpu.regs.SP, emu.cpu.regs.PCH);
	emu.cpu.regs.SP--;
	WriteMemory(emu.cpu.regs.SP, emu.cpu.regs.PCL);

	// Load new address into Program Counter.
	emu.cpu.regs.PC = callAddress;
}

//----------------------------------------//
// This instruction will complement(flip) //
// the carry flag.                        //
//----------------------------------------//
void z80_CCF()
{
	// Complement flag C.
	emu.cpu.regs.F ^= FLAG_C_ON;

	// Flags N and H are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction will set flags as if  //
// a given value with subtracted from     //
// register A.                            //
//----------------------------------------//
void z80_CP_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);

	if (emu.cpu.regs.A == value)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	if ((emu.cpu.regs.A & 0xF) < (value & 0xF))
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	if (emu.cpu.regs.A < value)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	emu.cpu.regs.F |= FLAG_N_ON;

	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will set flags as if  //
// a register was subtracted from         //
// register A.                            //
//----------------------------------------//
void z80_CP_reg8(unsigned char *reg)
{
	if (emu.cpu.regs.A == *reg)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	if ((emu.cpu.regs.A & 0xF) < (*reg & 0xF))
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	if (emu.cpu.regs.A < *reg)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	emu.cpu.regs.F |= FLAG_N_ON;
}

//----------------------------------------//
// This instruction will flip all of      //
// register A's bits.                     //
//----------------------------------------//
void z80_CPL()
{
	// Exclusive OR register A with 0xFF, this will flip the bits.
	emu.cpu.regs.A ^= 0xFF;

	// Flags N and H are set.
	emu.cpu.regs.F |= (FLAG_N_ON | FLAG_H_ON);
}

//----------------------------------------//
// This instruction will convert register //
// A to its packed-BCD representation.    //
//----------------------------------------//
void z80_DAA()
{
	unsigned short int result;

	result = emu.cpu.regs.A;

	// Check if flag N is on indicating the last operation was a subtraction.
	if (FLAG_N == 1)
	{
		if (FLAG_H == 1)
			result = (result - 0x06) & 0xFF;
		if (FLAG_C == 1)
			result -= 0x60;
	}
	// Otherwise, convert for an addition.
	else
	{
		if (((result & 0xF) > 0x09) || (FLAG_H == 1))
			result += 0x06;
		if ((result > 0x9F) || (FLAG_C == 1))
			result += 0x60;
	}

	// Set the carry flag if the BCD value of the result is greater than 99.
	if ((result & 0x100) == 0x100)
		emu.cpu.regs.F |= FLAG_C_ON;
//	else
//		emu.cpu.regs.F &= FLAG_C_OFF;

	emu.cpu.regs.A = (unsigned char)(result & 0xFF);

	// If the result was 0, turn the Z flag on.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flag H is turned off.
	emu.cpu.regs.F &= FLAG_H_OFF;
}

//----------------------------------------//
// This instruction will decrease the     //
// value at memory location (HL) by 1.    //
//----------------------------------------//
void z80_DEC_location_HL()
{
	WriteMemory(emu.cpu.regs.HL, ReadMemory(emu.cpu.regs.HL) - 1);

	// Turn on flag N since operation is a subtraction.
	emu.cpu.regs.F |= FLAG_N_ON;

	if (ReadMemory(emu.cpu.regs.HL) == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	if ((ReadMemory(emu.cpu.regs.HL) & 0xF) == 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;
}

//----------------------------------------//
// This instruction decreases a given     //
// 16-bit register's value by 1.          //
//----------------------------------------//
void z80_DEC_reg16(unsigned short int *reg)
{
	*reg -= 1;
}

//----------------------------------------//
// This instruction decreases a given     //
// 8-bit register's value by 1.           //
//----------------------------------------//
void z80_DEC_reg8(unsigned char *reg)
{
	// If the lo nibble of the register is 0, then there will be a borrow from bit 4.
	if ((*reg & 0xF) == 0)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Decrement *reg
	*reg -= 1;

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;
	
	// Turn on flag N.
	emu.cpu.regs.F |= FLAG_N_ON;
}

//----------------------------------------//
// This instruction turns off the         //
// Interrupt Master Enable flag.          //
//----------------------------------------//
void z80_DI()
{
	IntrpMasterEnable = 0;
}

//----------------------------------------//
// This instruction turns on the          //
// Interrupt Master Enable flag.          //
//----------------------------------------//
void z80_EI()
{
	IntrpMasterEnable = 1;
}

//----------------------------------------//
// This instruction will halt the GB CPU  //
// until an interrupt occurs.             //
//----------------------------------------//
void z80_HALT()
{
	// Make sure interrupts are enabled before allowing the CPU to halt.
	if (IntrpMasterEnable == 1)
		emu.state.halted = 1;
	else
	{
		if ((IOregister_IE & IOregister_IF & 0x1F) != 0)
			emu.state.haltInstructionRepeat = 1;
		else
			emu.state.halted = 1;
	}
}

//----------------------------------------//
// This instruction will increase the     //
// value at memory location (HL) by 1.    //
//----------------------------------------//
void z80_INC_location_HL()
{
	// See if there will be a carry from bit 3, if there was, set flag H.
	if ((ReadMemory(emu.cpu.regs.HL) & 0xF) == 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Add 1 to the value at the address in register HL.
	WriteMemory(emu.cpu.regs.HL, ReadMemory(emu.cpu.regs.HL) + 1);

	// If result is 0, set flag Z, otherwise reset it.
	if (ReadMemory(emu.cpu.regs.HL) == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;
	
	// Turn off flag N.
	emu.cpu.regs.F &= FLAG_N_OFF;
}

//----------------------------------------//
// This instruction increases a given     //
// 16-bit register's value by 1.          //
//----------------------------------------//
void z80_INC_reg16(unsigned short int *reg)
{
	*reg += 1;
}

//----------------------------------------//
// This instruction increases a given     //
// 8-bit register's value by 1.           //
//----------------------------------------//
void z80_INC_reg8(unsigned char *reg)
{
	// Turn the H flag on if incrementing the register carries from bit 3.
	if ((*reg & 0xF) == 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Add 1 to *reg.
	*reg += 1;

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;
	
	// Turn off flag N.  Flag C is not affected.
	emu.cpu.regs.F &= FLAG_N_OFF;
}

void z80_JP_location_HL()
{
	emu.cpu.regs.PC = emu.cpu.regs.HL;
	//emu.cpu.regs.PCL = ReadMemory(emu.cpu.regs.HL);
	//emu.cpu.regs.PCH = ReadMemory(emu.cpu.regs.HL + 1);
}

//----------------------------------------//
// This instruction will relocate the     //
// Program Counter(PC) to a given         //
// immediate address.                     //
//----------------------------------------//
void z80_JP_immediate()
{
	unsigned short int readAddress;

	readAddress = emu.cpu.regs.PC;

	emu.cpu.regs.PCL = ReadMemory(readAddress);
	readAddress++;
	emu.cpu.regs.PCH = ReadMemory(readAddress);
}

//----------------------------------------//
// This instruction will relocate the     //
// Program Counter(PC) to a given address,//
// but only if the given condition is met.//
//----------------------------------------//
int z80_JP_condition_immediate(unsigned char condition)
{
	unsigned short int jumpAddress;
	boolean conditionTrue;

	jumpAddress = (ReadMemory(emu.cpu.regs.PC) + ((ReadMemory(emu.cpu.regs.PC + 1) << 8)));
	conditionTrue = 0;

	switch (condition)
	{
	// If the Z flag is off, jump to the address.
	case 0x01:
	{
		if (FLAG_Z == 0)
			conditionTrue = 1;
	}
	break;
	// If the Z flag is on, jump to the address.
	case 0x02:
	{
		if (FLAG_Z == 1)
			conditionTrue = 1;
	}
	break;
	// If the C flag is off, jump to the address.
	case 0x03:
	{
		if (FLAG_C == 0)
			conditionTrue = 1;
	}
	break;
	// If the C flag is on, jump to the address.
	case 0x04:
	{
		if (FLAG_C == 1)
			conditionTrue = 1;
	}
	break;
	}

	if (conditionTrue == 1)
	{
		emu.cpu.regs.PC = jumpAddress;
		return 4;  // Add four additional cycles if jump succeeds.
	}
	else
	{
		emu.cpu.regs.PC += 2;
		return 0;
	}
}


//----------------------------------------//
// This instruction will add a signed     //
// 8-bit offset to the Program Counter    //
// only if the given condition is met.    //
//----------------------------------------//
int z80_JR_condition_offset(unsigned char condition)
{
	boolean conditionTrue;

	conditionTrue = 0;

	// condition decides the jump condition to look for.
	switch(condition)
	{
	case 0x01:			//JR NZ, offset
	{
		if (FLAG_Z == 0)
			conditionTrue = 1;
	}
	break;
	case 0x02:			//JR Z, offset
	{
		if (FLAG_Z == 1)
			conditionTrue = 1;
	}
	break;
	case 0x03:			//JR NC, offset
	{
		if (FLAG_C == 0)
			conditionTrue = 1;
	}
	break;
	case 0x04:			//JR C, offset
	{
		if (FLAG_C == 1)
			conditionTrue = 1;
	}
	break;
	}

	if (conditionTrue)
	{
		// Relative jump within 128 bytes.
		emu.cpu.regs.PC += (signed char)ReadMemory(emu.cpu.regs.PC);
		emu.cpu.regs.PC++;
		return 4;
	}
	else
		emu.cpu.regs.PC++;
	
	return 0;
}

//----------------------------------------//
// This instruction will add a signed     //
// 8-bit offset to the Program Counter.   //
//----------------------------------------//
void z80_JR_offset()
{
	// Relative jump within 128 bytes.
	emu.cpu.regs.PC += (signed char)ReadMemory(emu.cpu.regs.PC);
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will load the value at//
// a given memory location into A.        //
//----------------------------------------//
void z80_LD_A_location_immediate()
{
	unsigned short int readAddress;
	readAddress = ReadMemory(emu.cpu.regs.PC) + (ReadMemory(emu.cpu.regs.PC + 1) << 8);

	emu.cpu.regs.A = ReadMemory(readAddress);
	emu.cpu.regs.PC += 2;
}

//----------------------------------------//
// This instruction loads the value at    //
// memory location 0xFF00 plus register C //
// into register A.                       //
//----------------------------------------//
void z80_LD_A_0xFF00_C()
{
	// Load the value at 0xFF00 + register C into register A.
	emu.cpu.regs.A = ReadMemory(0xFF00 + emu.cpu.regs.C);
}

//----------------------------------------//
// This instruction will load the value   //
// at memory location 0xFF00 plus an 8-bit//
// value into register A.                 //
//----------------------------------------//
void z80_LD_A_0xFF00_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);

	// Load the value at 0xFF00 + value into register A.
	emu.cpu.regs.A = ReadMemory(0xFF00 + value);

	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will load the value in//
// memory at (reg16) into register A.     //
//----------------------------------------//
void z80_LD_A_location_reg16(unsigned short int *reg)
{
	// Load A with the value at the address in *reg.
	emu.cpu.regs.A = ReadMemory(*reg);
}

//----------------------------------------//
// Load the Stack Pointer plus a given,   //
// signed 8-bit value into memory         //
// location (HL).                         //
//----------------------------------------//
void z80_LD_HL_SP_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);

	// The flags are determined with an unsigned value.

	// Flag H is set if there is a carry from bit 3.
	if (((emu.cpu.regs.SP & 0xF) + (value & 0xF)) > 0xF)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;
	
	// Flag C is set if there is a carry from bit 7.
	if ((emu.cpu.regs.SP & 0xFF) + value > 0xFF)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	emu.cpu.regs.HL = emu.cpu.regs.SP + (signed char)value;

	// Reset flags Z and N.
	emu.cpu.regs.F &= (FLAG_Z_OFF & FLAG_N_OFF);

	// Increment Program Counter.
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will load a given     //
// value into memory location (HL).       //
//----------------------------------------//
void z80_LD_location_HL_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);

	// Load the address in HL with the immediate value
	WriteMemory(emu.cpu.regs.HL, value);

	// Increment the Program Counter to skip over the 8-bit value.
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will load the value of//
// an 8-bit register into memory location //
// (HL).                                  //
//----------------------------------------//
void z80_LD_location_HL_reg8(unsigned char *reg)
{
	// Load *reg into the address in register HL
	WriteMemory(emu.cpu.regs.HL, *reg);
}

//----------------------------------------//
// This instruction will load the value of//
// register A into memory at a given      //
// location.                              //
//----------------------------------------//
void z80_LD_location_immediate_A()
{
	unsigned short int writeAddress;
	writeAddress = ReadMemory(emu.cpu.regs.PC) + (ReadMemory(emu.cpu.regs.PC + 1) << 8);

	// Load register A into the location.
	WriteMemory(writeAddress, emu.cpu.regs.A);

	// Skip over the 16-bit address.
	emu.cpu.regs.PC += 2;
}

//----------------------------------------//
// This will load the Stack Pointer into a//
// given memory location.                 //
//----------------------------------------//
void z80_LD_location_SP()
{
	unsigned short int writeAddress;
	writeAddress = ReadMemory(emu.cpu.regs.PC) + (ReadMemory(emu.cpu.regs.PC + 1) << 8);

	WriteMemory(writeAddress, emu.cpu.regs.SPL);
	writeAddress++;
	WriteMemory(writeAddress, emu.cpu.regs.SPH);

	// Skip over the 16-bit address.
	emu.cpu.regs.PC += 2;
}

//----------------------------------------//
// This instruction will load a 16-bit    //
// register with a given value.           //
//----------------------------------------//
void z80_LD_reg16_value(unsigned char *hiReg, unsigned char *loReg)
{
	// Load the 16-bit value into the registers.
	*loReg = ReadMemory(emu.cpu.regs.PC);
	emu.cpu.regs.PC++;
	*hiReg = ReadMemory(emu.cpu.regs.PC);
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will load the address //
// at a 16-bit pointer register with the  //
// value in register A.                   //
//----------------------------------------//
void z80_LD_location_reg16_A(unsigned short int *reg)
{
	WriteMemory(*reg, emu.cpu.regs.A);
}

//----------------------------------------//
// This instruction will load an 8-bit    //
// register with a given value.           //
//----------------------------------------//
void z80_LD_reg8_value(unsigned char *reg)
{
	// Load *reg with the 8-bit value immediately after it.
	*reg = ReadMemory(emu.cpu.regs.PC);
	
	// Increment the program counter to skip the value.
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will load an 8-bit    //
// register with the value at memory      //
// location (HL).                         //
//----------------------------------------//
void z80_LD_reg8_location_HL(unsigned char *reg)
{
	// Load the value at the address in HL into *reg.
	*reg = ReadMemory(emu.cpu.regs.HL);
}
 
//----------------------------------------//
// This instruction will load an 8-bit    //
// register with the value of another     //
// 8-bit register.                        //
//----------------------------------------//
void z80_LD_reg8_reg8(unsigned char *reg1, unsigned char *reg2)
{
	// Load the value in *reg2 into *reg1.
	*reg1 = *reg2;
}

//----------------------------------------//
// This instruction will load the Stack   //
// Pointer with the value of register HL. //
//----------------------------------------//
void z80_LD_SP_HL()
{
	// Load register HL into Stack Pointer.
	emu.cpu.regs.SP = emu.cpu.regs.HL;
}

//----------------------------------------//
// This instruction loads register A into //
// memory location 0xFF00 plus register C.//
//----------------------------------------//
void z80_LD_0xFF00_C_A()
{
	WriteMemory(0xFF00 + emu.cpu.regs.C, emu.cpu.regs.A);
}

//----------------------------------------//
// This instruction will load the value   //
// of register A into memory at the       //
// location of 0xFF00 plus an 8-bit value.//
//----------------------------------------//
void z80_LD_0xFF00_immediate_A()
{
	// Write the value of register A into memory.
	WriteMemory(0xFF00 + ReadMemory(emu.cpu.regs.PC), emu.cpu.regs.A);

	// Increment Program Counter
	emu.cpu.regs.PC++;
}

// Load A with data at location HL and decrement HL.
void z80_LDD_A_HL()
{
	emu.cpu.regs.A = ReadMemory(emu.cpu.regs.HL);
	emu.cpu.regs.HL--;
}

// Load location HL with data from register A and decrement HL.
void z80_LDD_HL_A()
{
	WriteMemory(emu.cpu.regs.HL, emu.cpu.regs.A);
	emu.cpu.regs.HL--;
}

// Load A with data at location HL and increment HL.
void z80_LDI_A_HL()
{
	emu.cpu.regs.A = ReadMemory(emu.cpu.regs.HL);
	emu.cpu.regs.HL++;
}

// Load location HL with data from register A and increment HL.
void z80_LDI_HL_A()
{
	WriteMemory(emu.cpu.regs.HL, emu.cpu.regs.A);
	emu.cpu.regs.HL++;
}

//----------------------------------------//
// This instruction will inclusive OR a   //
// given value with register A.           //
//----------------------------------------//
void z80_OR_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);
	
	emu.cpu.regs.A |= value;

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// The other three flags are reset.
	emu.cpu.regs.F &= (FLAG_H_OFF & FLAG_N_OFF & FLAG_C_OFF);

	// Increment Program Counter.
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will logical OR a     //
// 8-bit register and register A.         //
//----------------------------------------//
void z80_OR_reg8(unsigned char *reg)
{
	// Logically Inclusive OR register A and *reg.
	emu.cpu.regs.A |= *reg;

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// The other three flags are reset.
	emu.cpu.regs.F &= (FLAG_H_OFF & FLAG_N_OFF & FLAG_C_OFF);
}

//----------------------------------------//
// This instruction will pop a value off  //
// the stack into a 16-bit register.      //
//----------------------------------------//
void z80_POP_reg16(unsigned char *hiReg, unsigned char *loReg)
{
	// Pop two bytes from the stack into the low and high bytes of a 16-bit register.
	// If the register being popped is the AF register, make sure only the high nibble is filled.
	if (loReg == &emu.cpu.regs.F)
		*loReg = ReadMemory(emu.cpu.regs.SP) & 0xF0;
	else
		*loReg = ReadMemory(emu.cpu.regs.SP);
	emu.cpu.regs.SP++;
	*hiReg = ReadMemory(emu.cpu.regs.SP);
	emu.cpu.regs.SP++;
}

//----------------------------------------//
// This instruction will push the value of//
// a 16-bit register onto the stack.      //
//----------------------------------------//
void z80_PUSH_reg16(unsigned char *hiReg, unsigned char *loReg)
{
	// Push the 16-bit register's two bytes onto the stack.
	emu.cpu.regs.SP--;
	WriteMemory(emu.cpu.regs.SP, *hiReg);
	emu.cpu.regs.SP--;
	WriteMemory(emu.cpu.regs.SP, *loReg);
}

//----------------------------------------//
// This instruction will reset(turn off) a//
// given bit of an 8-bit register.        //
//----------------------------------------//
void z80_RES_bit_reg8(unsigned char bit, unsigned char *reg)
{
	*reg &= (0xFF - bit);
}

//----------------------------------------//
// This instruction will pop two bytes off//
// the stack and jump to that address.    //
//----------------------------------------//
void z80_RET()
{
	// Set Program Counter to the first 16-bit address in the stack.
	emu.cpu.regs.PCL = ReadMemory(emu.cpu.regs.SP);
	emu.cpu.regs.SP++;
	emu.cpu.regs.PCH = ReadMemory(emu.cpu.regs.SP);  // Combine the next 8-bits with the first ones
	emu.cpu.regs.SP++;
}

//----------------------------------------//
// This instruction will pop two bytes off//
// the stack and jump to that address, but//
// only if the given condition is met.    //
//----------------------------------------//
int z80_RET_condition(unsigned char condition)
{
	unsigned char conditionTrue;
	conditionTrue = 0;

	switch (condition)
	{
	case 0x01:
	{
		if (FLAG_Z == 0)
			conditionTrue = 1;
	}
	break;
	case 0x02:
	{
		if (FLAG_Z == 1)
			conditionTrue = 1;
	}
	break;
	case 0x03:
	{
		if (FLAG_C == 0)
			conditionTrue = 1;
	}
	break;
	case 0x04:
	{
		if (FLAG_C == 1)
			conditionTrue = 1;
	}
	break;
	}

	if (conditionTrue == 1)
	{
		// Set Program Counter to the first 16-bit address in the stack.
		emu.cpu.regs.PCL = ReadMemory(emu.cpu.regs.SP);
		emu.cpu.regs.SP++;
		emu.cpu.regs.PCH = ReadMemory(emu.cpu.regs.SP);  // Combine the next 8-bits with the first ones
		emu.cpu.regs.SP++;

		return 12;
	}
	return 0;
}

//----------------------------------------//
// This instruction rotates an 8-bit      //
// register left by 1 bit.                //
//----------------------------------------//
void z80_RL_reg8(unsigned char *reg)
{
	if (*reg & BIT_7)
	{
		if (FLAG_C == 1)
			*reg = (*reg << 1) | BIT_0;
		else
			*reg <<= 1;
		
		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C == 1)
			*reg = (*reg << 1) | BIT_0;
		else
			*reg <<= 1;
		
		emu.cpu.regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flags H and N are reset
	emu.cpu.regs.F &= (FLAG_H_OFF & FLAG_N_OFF);
}

//----------------------------------------//
// This instruction rotates register A    //
// left by 1 bit.                         //
//----------------------------------------//
void z80_RLA()
{
	if (emu.cpu.regs.A & BIT_7)
	{
		if (FLAG_C == 1)
			emu.cpu.regs.A = (emu.cpu.regs.A << 1) | BIT_0;
		else
			emu.cpu.regs.A <<= 1;

		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C == 1)
			emu.cpu.regs.A = (emu.cpu.regs.A << 1) | BIT_0;
		else
			emu.cpu.regs.A <<= 1;

		emu.cpu.regs.F &= FLAG_C_OFF;
	}

	// The other three flags are reset.  Only the carry flag
	// remains unchanged.
	emu.cpu.regs.F &= (FLAG_Z_OFF & FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction will rotate an 8-bit  //
// register left through the carry flag.  //
//----------------------------------------//
void z80_RLC_reg8(unsigned char *reg)
{
	// See if the old bit 7 data is on or off and set flag C accordingly.
	if (*reg & BIT_7)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	// Shift *reg 1 bit to the left.  Old bit 7 goes to bit 0.
	*reg <<= 1;
	
	// Add flag C to *reg to make up for the shifted out bit.
	if (FLAG_C == 1)
		*reg |= BIT_0;

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;
	
	// Flags N and H are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction rotates register A    //
// by 1 bit left through the carry flag.  //
//----------------------------------------//
void z80_RLCA()
{
	if (emu.cpu.regs.A & BIT_7)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	// Shift register A 1 bit to the left.  Old bit 7 goes to bit 0.
	emu.cpu.regs.A <<= 1;
	
	// Add flag C to register A to make up for the shifted out bit.
	if (FLAG_C == 1)
		emu.cpu.regs.A |= BIT_0;

	// The other three flags are reset.  Only the carry flag
	// remains unchanged.
	emu.cpu.regs.F &= (FLAG_Z_OFF & FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction rotates an 8-bit      //
// register right by 1 bit.                //
//----------------------------------------//
void z80_RR_reg8(unsigned char *reg)
{
	if (*reg & BIT_0)
	{
		if (FLAG_C == 1)
			*reg = (*reg >> 1) | BIT_7;
		else
			*reg >>= 1;
		
		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C == 1)
			*reg = (*reg >> 1) | BIT_7;
		else
			*reg >>= 1;
		
		emu.cpu.regs.F &= FLAG_C_OFF;
	}
	
	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flags N and H are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction rotates register A    //
// to the right 1 bit.                    //
//----------------------------------------//
void z80_RRA()
{
	if (emu.cpu.regs.A & BIT_0)
	{
		if (FLAG_C == 1)
			emu.cpu.regs.A = (emu.cpu.regs.A >> 1) | BIT_7;
		else
			emu.cpu.regs.A >>= 1;

		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C == 1)
			emu.cpu.regs.A = (emu.cpu.regs.A >> 1) | BIT_7;
		else
			emu.cpu.regs.A >>= 1;

		emu.cpu.regs.F &= FLAG_C_OFF;
	}

	// The other three flags are reset.
	emu.cpu.regs.F &= (FLAG_Z_OFF & FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction will rotate an 8-bit  //
// register right through the carry flag. //
//----------------------------------------//
void z80_RRC_reg8(unsigned char *reg)
{
	// See if the old bit 0 data was on or off and set flag C accordingly.
	if (*reg & BIT_0)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	// Shift *reg 1 bit to the right.  Old bit 0 goes to bit 7.
	*reg >>= 1;

	// If flag C is on, add 0x80 to *reg to make up for the
	// lost bit.
	if (FLAG_C)
		*reg |= 0x80;

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flags N and H are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction rotates register A    //
// right through the carry flag.          //
//----------------------------------------//
void z80_RRCA()
{
	// See if the old bit 0 data was on or off and set flag C accordingly.
	if (emu.cpu.regs.A & BIT_0)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	// Shift register A 1 bit to the right.  Old bit 0 goes to bit 7.
	emu.cpu.regs.A >>= 1;

	// If flag C is on, add 0x80 to register A to make up for the
	// lost bit.
	if (FLAG_C)
		emu.cpu.regs.A |= BIT_7;

	// The other three flags are reset.
	emu.cpu.regs.F &= (FLAG_Z_OFF & FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction will push the current //
// Program Counter onto the stack and     //
// jumps to a new address(This is used    //
// mainly by interrupts).                //
//----------------------------------------//
void z80_RST(unsigned char address)
{
	// Push the Program Counter into the stack in little-endian
	// (low byte first) order.
	emu.cpu.regs.SP--;
	WriteMemory(emu.cpu.regs.SP, emu.cpu.regs.PCH);
	emu.cpu.regs.SP--;
	WriteMemory(emu.cpu.regs.SP, emu.cpu.regs.PCL);

	// Set Program Counter to address.
	emu.cpu.regs.PC = address;
}

//----------------------------------------//
// This instruction will subtract a given //
// value and the carry flag from          //
// register A.                            //
//----------------------------------------//
void z80_SBC_A_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);

	// Check for a borrow from bit 4.
	if ((emu.cpu.regs.A & 0xF) < (value & 0xF) + FLAG_C)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((emu.cpu.regs.A - (value + FLAG_C)) < 0)
	{
		emu.cpu.regs.A -= (value + FLAG_C);
		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		emu.cpu.regs.A -= (value + FLAG_C);
		emu.cpu.regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Set N since the operation was a subtraction.
	emu.cpu.regs.F |= FLAG_N_ON;

	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will subtract an 8-bit//
// register and the carry flag from       //
// register A.                            //
//----------------------------------------//
void z80_SBC_A_reg8(unsigned char *reg)
{
	// Check for a borrow from bit 4.
	if ((emu.cpu.regs.A & 0xF) < (*reg & 0xF) + FLAG_C)
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((emu.cpu.regs.A - (*reg + FLAG_C)) < 0)
	{
		emu.cpu.regs.A -= (*reg + FLAG_C);
		emu.cpu.regs.F |= FLAG_C_ON;
	}
	else
	{
		emu.cpu.regs.A -= (*reg + FLAG_C);
		emu.cpu.regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Set N since the operation was a subtraction.
	emu.cpu.regs.F |= FLAG_N_ON;
}

//----------------------------------------//
// This instruction will set the carry    //
// flag.                                  //
//----------------------------------------//
void z80_SCF()
{
	// Turn off flag N and H.  Z is not changed.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);

	// Set flag C.
	emu.cpu.regs.F |= FLAG_C_ON;
}

//----------------------------------------//
// This instruction will set(turn on) a   //
// given bit of an 8-bit register.        //
//----------------------------------------//
void z80_SET_bit_reg8(unsigned char bit, unsigned char *reg)
{
	*reg |= bit;
}

//----------------------------------------//
// This instruction shifts bit 7 of an    //
// 8-bit register left into the carry flag//
// A 0 is shifted into bit 0.             //
//----------------------------------------//
void z80_SLA_reg8(unsigned char *reg)
{
	// Put *reg's old bit 7 data in Carry flag.
	if (*reg & BIT_7)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	// Shift *reg left once.  0 is shifted in from right.
	*reg <<= 1;

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flags N and H are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction shifts bit 0 of an    //
// 8-bit register right into the carry    //
// flag.  Bit 7 doesn't change.           //
//----------------------------------------//
void z80_SRA_reg8(unsigned char *reg)
{
	// Put *reg's old bit 0 data in Carry flag.
	if (*reg & BIT_0)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	// Shift *reg right once.  0 is shifted in from left.  If bit 7
	// is set, make sure it stays set.
	if (*reg & BIT_7)
		*reg = (*reg >> 1) + BIT_7;
	else
		*reg >>= 1;

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flags N and H are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This instruction shifts bit 0 of an    //
// 8-bit register right into the carry    //
// flag.  A 0 is shifted into bit 7.      //
//----------------------------------------//
void z80_SRL_reg8(unsigned char *reg)
{
	// Put *reg's old bit 0 data in Carry flag.
	if (*reg & BIT_0)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	// Shift *reg right once.  0 is shifted in from right.
	*reg >>= 1;

	// If result is 0, set flag Z, otherwise reset it.
	if (*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Flags N and H are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF);
}

//----------------------------------------//
// This halts the GB CPU until a button is//
// pressed.                               //
//----------------------------------------//
void z80_STOP()
{
	// Don't allow STOP if interrupts are disabled.
	//if (emu.state.intrpEnable == 1)
		emu.state.stopped = 1;
	// Skip over the extra 0x00
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will subtract a given //
// value from register A.                 //
//----------------------------------------//
void z80_SUB_immediate()
{
	unsigned char value;
	value = ReadMemory(emu.cpu.regs.PC);

	// Check for a borrow from bit 4.
	if ((emu.cpu.regs.A & 0xF) < (value & 0xF))
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((emu.cpu.regs.A - value) < 0)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	emu.cpu.regs.A -= value;

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Set N since the operation was a subtraction.
	emu.cpu.regs.F |= FLAG_N_ON;

	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This instruction will subtract an 8-bit//
// register's value from register A.      //
//----------------------------------------//
void z80_SUB_reg8(unsigned char *reg)
{
	// Check for a borrow from bit 4.
	if ((emu.cpu.regs.A & 0xF) < (*reg & 0xF))
		emu.cpu.regs.F |= FLAG_H_ON;
	else
		emu.cpu.regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((emu.cpu.regs.A - *reg) < 0)
		emu.cpu.regs.F |= FLAG_C_ON;
	else
		emu.cpu.regs.F &= FLAG_C_OFF;

	emu.cpu.regs.A -= *reg;

	// If result is 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Set N since the operation was a subtraction.
	emu.cpu.regs.F |= FLAG_N_ON;
}

//----------------------------------------//
// This instruction swaps the higher and  //
// lower 4-bits of an 8-bit register.     //
//----------------------------------------//
void z80_SWAP_reg8(unsigned char *reg)
{
	// Swap the upper and lower nibbles of *reg.
	*reg = (((*reg & 0xF0) >> 4) | ((*reg & 0x0F) << 4));

	// If the result was 0, set flag Z, otherwise reset it.
	if(*reg == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Other flags are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF & FLAG_C_OFF);
}

//----------------------------------------//
// This will exclusive OR a given value   //
// and register A.                        //
//----------------------------------------//
void z80_XOR_immediate()
{
	// Logically Exclusive OR register A and an immediate value.
	emu.cpu.regs.A ^= ReadMemory(emu.cpu.regs.PC);

	// If the result was 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Other flags are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF & FLAG_C_OFF);
	
	// Increment Program Counter.
	emu.cpu.regs.PC++;
}

//----------------------------------------//
// This will exclusive OR an 8-bit        //
// register and register A.               //
//----------------------------------------//
void z80_XOR_reg8(unsigned char *reg)
{
	// Logically Exclusive OR register A and *reg.
	emu.cpu.regs.A ^= *reg;
	
	// If the result was 0, set flag Z, otherwise reset it.
	if (emu.cpu.regs.A == 0)
		emu.cpu.regs.F |= FLAG_Z_ON;
	else
		emu.cpu.regs.F &= FLAG_Z_OFF;

	// Other flags are reset.
	emu.cpu.regs.F &= (FLAG_N_OFF & FLAG_H_OFF & FLAG_C_OFF);
}

//----------------------------------------//
// This function will initialize the GB   //
// to its startup state.
//----------------------------------------//
int EmulationInitialize(unsigned char *fileBuffer, unsigned long int fileSize)
{
	emu.cart.dataBuffer = malloc(fileSize);
	if (!emu.cart.dataBuffer)
		return -1;
	memcpy(&emu.cart.dataBuffer[0x0000000], &fileBuffer[0x0000000], fileSize);  // Copy ROM file data to cartridge buffer 
	
	if (emu.cart.dataBuffer[0x0143] == 0x80)
		emu.cart.systemType = SYSTEM_CGB;
	if (emu.cart.dataBuffer[0x0146] == 0x03)
		emu.cart.systemType = SYSTEM_SGB;
	if (emu.cart.dataBuffer[0x0146] == 0x00)
		emu.cart.systemType = SYSTEM_GB;

	if ((emu.cart.systemType != SYSTEM_GB) && (emu.cart.systemType != SYSTEM_SGB)) // Only original Gameboy supported for now.
		return -1;

	emu.cart.mbcType = emu.cart.dataBuffer[0x147];

	emu.cart.numberOfRomBanks = fileSize / 0x4000;  // Calculate the number of 16 KB ROM banks the cartridge has.
	//----------------------------------------//
	// Set the CPU and IO register startup    //
	// values.                                //
	//----------------------------------------//
//	if (emu.cart.systemType == SYSTEM_GB)
//	{
		IOregister_NR52 = 0xF1;
		emu.cpu.regs.AF = 0x01B0;
//	}
//	else
//		emu.cpu.regs.AF = 0x00B0;
	emu.cpu.regs.BC = 0x0013;
	emu.cpu.regs.DE = 0x00D8;
	emu.cpu.regs.HL = 0x014D;
	emu.cpu.regs.SP = 0xFFFE;
	emu.cpu.regs.PC = 0x0100;

	// Set memory spaces as value 0xFF by default.
	memset(&emu.memory.videoRam, 0xFF, 0x2000);
	memset(&emu.memory.internRam, 0xFF, 0x2000);
	memset(&emu.memory.sprite, 0xFF, 0xA0);
	memset(&emu.memory.ioRegs, 0xFF, 0x4C);
	memset(&emu.memory.highRam, 0xFF, 0x7F);

	// Initial values of I/O registers.  These are important for some ROMS to work correctly.
	IOregister_P1 =		0xCF;
	IOregister_SB =		0x00;
	IOregister_SC =		0x7E;
	IOregister_DIV =	0xAB;
	IOregister_TIMA =	0x00;
	IOregister_TMA =	0x00;
	IOregister_TAC =	0x00;
	IOregister_IF =		0xE1;
	IOregister_NR10 =	0x80;
	IOregister_NR11 =	0xBF;
	IOregister_NR12 =	0xF3;
	IOregister_NR14 =	0xBF;
	IOregister_NR21 =	0xF3;
	IOregister_NR12 =	0xF3;
	IOregister_NR22 =	0x00;
	IOregister_NR24 =	0xBF;
	IOregister_NR30 =	0x7F;
	IOregister_NR31 =	0xFF;
	IOregister_NR32 =	0x9F;
	IOregister_NR33 =	0xBF;
	IOregister_NR41 =	0xFF;
	IOregister_NR42 =	0x00;
	IOregister_NR43 =	0x00;
	IOregister_NR44 =	0xBF;
	IOregister_NR50 =	0x77;
	IOregister_NR51 =	0xF3;
	IOregister_LCDC =	0x91;
	IOregister_STAT =	0x81;
	IOregister_SCY =	0x00;
	IOregister_SCX =	0x00;
	IOregister_LY =		0x90;
	IOregister_LYC =	0x00;
	IOregister_DMA =	0x00;
	IOregister_BGP =	0xFC;
	IOregister_OBP0 =	0xFF;
	IOregister_OBP1 =	0xFF;
	IOregister_WY =		0x00;
	IOregister_WX =		0x07;
	IOregister_IE =		0x00;
	
	IntrpMasterEnable = 1;

	// Assume standard Gameboy opcode cycles for now
	memcpy(&emu.cycles.opCycles[0], &GB_CycleTable[0], 0x100);
	memcpy(&emu.cycles.opCBCycles[0], &GB_CBCycleTable[0], 0x100);

	//----------------------------------------//
	// Load the base ROM (32K).               //
	//----------------------------------------//
	memcpy(&emu.memory.romBank0[0x0000], &emu.cart.dataBuffer[0x0000], 0x4000);
	memcpy(&emu.memory.romBank1[0x0000], &emu.cart.dataBuffer[0x4000], 0x4000);

	emu.cycles.statCycles = 0;
	emu.cycles.internalCounterCycles = 0xABCC;

	emu.state.halted = 0;
	emu.state.haltInstructionRepeat = 0;
	emu.state.stopped = 0;

	emu.io.display.lcdDelay = 0;

	sprintf(opcodeDescription[0x00], "NOP");
	sprintf(opcodeDescription[0x01], "LD BC, $####");
	sprintf(opcodeDescription[0x02], "LD (BC), A");
	sprintf(opcodeDescription[0x03], "INC BC");
	sprintf(opcodeDescription[0x04], "INC B");
	sprintf(opcodeDescription[0x05], "DEC B");
	sprintf(opcodeDescription[0x06], "LD B, $##");
	sprintf(opcodeDescription[0x07], "RLCA");
	sprintf(opcodeDescription[0x08], "LD ($####), SP");
	sprintf(opcodeDescription[0x09], "ADD HL, BC");
	sprintf(opcodeDescription[0x0A], "LD A, (BC)");
	sprintf(opcodeDescription[0x0B], "DEC BC");
	sprintf(opcodeDescription[0x0C], "INC C");
	sprintf(opcodeDescription[0x0D], "DEC C");
	sprintf(opcodeDescription[0x0E], "LD C, $##");
	sprintf(opcodeDescription[0x0F], "RRCA");
	sprintf(opcodeDescription[0x10], "STOP");
	sprintf(opcodeDescription[0x11], "LD DE, $####");
	sprintf(opcodeDescription[0x12], "LD (DE), A");
	sprintf(opcodeDescription[0x13], "INC DE");
	sprintf(opcodeDescription[0x14], "INC D");
	sprintf(opcodeDescription[0x15], "DEC D");
	sprintf(opcodeDescription[0x16], "LD D, $##");
	sprintf(opcodeDescription[0x17], "RLA");
	sprintf(opcodeDescription[0x18], "JR $****");
	sprintf(opcodeDescription[0x19], "ADD HL, DE");
	sprintf(opcodeDescription[0x1A], "LD A, (DE)");
	sprintf(opcodeDescription[0x1B], "DEC DE");
	sprintf(opcodeDescription[0x1C], "INC E");
	sprintf(opcodeDescription[0x1D], "DEC E");
	sprintf(opcodeDescription[0x1E], "LD E, $##");
	sprintf(opcodeDescription[0x1F], "RRA");
	sprintf(opcodeDescription[0x20], "JR NZ, $****");
	sprintf(opcodeDescription[0x21], "LD HL, $####");
	sprintf(opcodeDescription[0x22], "LDI (HL), A");
	sprintf(opcodeDescription[0x23], "INC HL");
	sprintf(opcodeDescription[0x24], "INC H");
	sprintf(opcodeDescription[0x25], "DEC H");
	sprintf(opcodeDescription[0x26], "LD H, $##");
	sprintf(opcodeDescription[0x27], "DAA");
	sprintf(opcodeDescription[0x28], "JR Z, $****");
	sprintf(opcodeDescription[0x29], "ADD HL, HL");
	sprintf(opcodeDescription[0x2A], "LDI A, (HL)");
	sprintf(opcodeDescription[0x2B], "DEC HL");
	sprintf(opcodeDescription[0x2C], "INC L");
	sprintf(opcodeDescription[0x2D], "DEC L");
	sprintf(opcodeDescription[0x2E], "LD L, $##");
	sprintf(opcodeDescription[0x2F], "CPL");
	sprintf(opcodeDescription[0x30], "JR NC, $****");
	sprintf(opcodeDescription[0x31], "LD SP, $####");
	sprintf(opcodeDescription[0x32], "LDD (HL), A");
	sprintf(opcodeDescription[0x33], "INC SP");
	sprintf(opcodeDescription[0x34], "INC (HL)");
	sprintf(opcodeDescription[0x35], "DEC (HL)");
	sprintf(opcodeDescription[0x36], "LD (HL)");
	sprintf(opcodeDescription[0x37], "SCF");
	sprintf(opcodeDescription[0x38], "JR C, $****");
	sprintf(opcodeDescription[0x39], "ADD HL, SP");
	sprintf(opcodeDescription[0x3A], "LDD A, (HL)");
	sprintf(opcodeDescription[0x3B], "DEC SP");
	sprintf(opcodeDescription[0x3C], "INC A");
	sprintf(opcodeDescription[0x3D], "DEC A");
	sprintf(opcodeDescription[0x3E], "LD A, $##");
	sprintf(opcodeDescription[0x3F], "CCF");
	sprintf(opcodeDescription[0x40], "LD B, B");
	sprintf(opcodeDescription[0x41], "LD B, C");
	sprintf(opcodeDescription[0x42], "LD B, D");
	sprintf(opcodeDescription[0x43], "LD B, E");
	sprintf(opcodeDescription[0x44], "LD B, H");
	sprintf(opcodeDescription[0x45], "LD B, L");
	sprintf(opcodeDescription[0x46], "LD B, (HL)");
	sprintf(opcodeDescription[0x47], "LD B, A");
	sprintf(opcodeDescription[0x48], "LD C, B");
	sprintf(opcodeDescription[0x49], "LD C, C");
	sprintf(opcodeDescription[0x4A], "LD C, D");
	sprintf(opcodeDescription[0x4B], "LD C, E");
	sprintf(opcodeDescription[0x4C], "LD C, H");
	sprintf(opcodeDescription[0x4D], "LD C, L");
	sprintf(opcodeDescription[0x4E], "LD C, (HL)");
	sprintf(opcodeDescription[0x4F], "LD C, A");
	sprintf(opcodeDescription[0x50], "LD D, B");
	sprintf(opcodeDescription[0x51], "LD D, C");
	sprintf(opcodeDescription[0x52], "LD D, D");
	sprintf(opcodeDescription[0x53], "LD D, E");
	sprintf(opcodeDescription[0x54], "LD D, H");
	sprintf(opcodeDescription[0x55], "LD D, L");
	sprintf(opcodeDescription[0x56], "LD D, (HL)");
	sprintf(opcodeDescription[0x57], "LD D, A");
	sprintf(opcodeDescription[0x58], "LD E, B");
	sprintf(opcodeDescription[0x59], "LD E, C");
	sprintf(opcodeDescription[0x5A], "LD E, D");
	sprintf(opcodeDescription[0x5B], "LD E, E");
	sprintf(opcodeDescription[0x5C], "LD E, H");
	sprintf(opcodeDescription[0x5D], "LD E, L");
	sprintf(opcodeDescription[0x5E], "LD E, (HL)");
	sprintf(opcodeDescription[0x5F], "LD E, A");
	sprintf(opcodeDescription[0x60], "LD H, B");
	sprintf(opcodeDescription[0x61], "LD H, C");
	sprintf(opcodeDescription[0x62], "LD H, D");
	sprintf(opcodeDescription[0x63], "LD H, E");
	sprintf(opcodeDescription[0x64], "LD H, H");
	sprintf(opcodeDescription[0x65], "LD H, L");
	sprintf(opcodeDescription[0x66], "LD H, (HL)");
	sprintf(opcodeDescription[0x67], "LD H, A");
	sprintf(opcodeDescription[0x68], "LD L, B");
	sprintf(opcodeDescription[0x69], "LD L, C");
	sprintf(opcodeDescription[0x6A], "LD L, D");
	sprintf(opcodeDescription[0x6B], "LD L, E");
	sprintf(opcodeDescription[0x6C], "LD L, H");
	sprintf(opcodeDescription[0x6D], "LD L, L");
	sprintf(opcodeDescription[0x6E], "LD L, (HL)");
	sprintf(opcodeDescription[0x6F], "LD L, A");
	sprintf(opcodeDescription[0x70], "LD (HL), B");
	sprintf(opcodeDescription[0x71], "LD (HL), C");
	sprintf(opcodeDescription[0x72], "LD (HL), D");
	sprintf(opcodeDescription[0x73], "LD (HL), E");
	sprintf(opcodeDescription[0x74], "LD (HL), H");
	sprintf(opcodeDescription[0x75], "LD (HL), L");
	sprintf(opcodeDescription[0x76], "HALT");
	sprintf(opcodeDescription[0x77], "LD (HL), A");
	sprintf(opcodeDescription[0x78], "LD A, B");
	sprintf(opcodeDescription[0x79], "LD A, C");
	sprintf(opcodeDescription[0x7A], "LD A, D");
	sprintf(opcodeDescription[0x7B], "LD A, E");
	sprintf(opcodeDescription[0x7C], "LD A, H");
	sprintf(opcodeDescription[0x7D], "LD A, L");
	sprintf(opcodeDescription[0x7E], "LD A, (HL)");
	sprintf(opcodeDescription[0x7F], "LD A, A");
	sprintf(opcodeDescription[0x80], "ADD A, B");
	sprintf(opcodeDescription[0x81], "ADD A, C");
	sprintf(opcodeDescription[0x82], "ADD A, D");
	sprintf(opcodeDescription[0x83], "ADD A, E");
	sprintf(opcodeDescription[0x84], "ADD A, H");
	sprintf(opcodeDescription[0x85], "ADD A, L");
	sprintf(opcodeDescription[0x86], "ADD A, (HL)");
	sprintf(opcodeDescription[0x87], "ADD A, A");
	sprintf(opcodeDescription[0x88], "ADC A, B");
	sprintf(opcodeDescription[0x89], "ADC A, C");
	sprintf(opcodeDescription[0x8A], "ADC A, D");
	sprintf(opcodeDescription[0x8B], "ADC A, E");
	sprintf(opcodeDescription[0x8C], "ADC A, H");
	sprintf(opcodeDescription[0x8D], "ADC A, L");
	sprintf(opcodeDescription[0x8E], "ADC A, (HL)");
	sprintf(opcodeDescription[0x8F], "ADC A, A");
	sprintf(opcodeDescription[0x90], "SUB B");
	sprintf(opcodeDescription[0x91], "SUB C");
	sprintf(opcodeDescription[0x92], "SUB D");
	sprintf(opcodeDescription[0x93], "SUB E");
	sprintf(opcodeDescription[0x94], "SUB H");
	sprintf(opcodeDescription[0x95], "SUB L");
	sprintf(opcodeDescription[0x96], "SUB (HL)");
	sprintf(opcodeDescription[0x97], "SUB A");
	sprintf(opcodeDescription[0x98], "SBC A, B");
	sprintf(opcodeDescription[0x99], "SBC A, C");
	sprintf(opcodeDescription[0x9A], "SBC A, D");
	sprintf(opcodeDescription[0x9B], "SBC A, E");
	sprintf(opcodeDescription[0x9C], "SBC A, H");
	sprintf(opcodeDescription[0x9D], "SBC A, L");
	sprintf(opcodeDescription[0x9E], "SBC A, (HL)");
	sprintf(opcodeDescription[0x9F], "SBC A, A");
	sprintf(opcodeDescription[0xA0], "AND B");
	sprintf(opcodeDescription[0xA1], "AND C");
	sprintf(opcodeDescription[0xA2], "AND D");
	sprintf(opcodeDescription[0xA3], "AND E");
	sprintf(opcodeDescription[0xA4], "AND H");
	sprintf(opcodeDescription[0xA5], "AND L");
	sprintf(opcodeDescription[0xA6], "AND (HL)");
	sprintf(opcodeDescription[0xA7], "AND A");
	sprintf(opcodeDescription[0xA8], "XOR B");
	sprintf(opcodeDescription[0xA9], "XOR C");
	sprintf(opcodeDescription[0xAA], "XOR D");
	sprintf(opcodeDescription[0xAB], "XOR E");
	sprintf(opcodeDescription[0xAC], "XOR H");
	sprintf(opcodeDescription[0xAD], "XOR L");
	sprintf(opcodeDescription[0xAE], "XOR (HL)");
	sprintf(opcodeDescription[0xAF], "XOR A");
	sprintf(opcodeDescription[0xB0], "OR B");
	sprintf(opcodeDescription[0xB1], "OR C");
	sprintf(opcodeDescription[0xB2], "OR D");
	sprintf(opcodeDescription[0xB3], "OR E");
	sprintf(opcodeDescription[0xB4], "OR H");
	sprintf(opcodeDescription[0xB5], "OR L");
	sprintf(opcodeDescription[0xB6], "OR (HL)");
	sprintf(opcodeDescription[0xB7], "OR A");
	sprintf(opcodeDescription[0xB8], "CP B");
	sprintf(opcodeDescription[0xB9], "CP C");
	sprintf(opcodeDescription[0xBA], "CP D");
	sprintf(opcodeDescription[0xBB], "CP E");
	sprintf(opcodeDescription[0xBC], "CP H");
	sprintf(opcodeDescription[0xBD], "CP L");
	sprintf(opcodeDescription[0xBE], "CP (HL)");
	sprintf(opcodeDescription[0xBF], "CP A");
	sprintf(opcodeDescription[0xC0], "RET NZ");
	sprintf(opcodeDescription[0xC1], "POP BC");
	sprintf(opcodeDescription[0xC2], "JP NZ, $####");
	sprintf(opcodeDescription[0xC3], "JP $####");
	sprintf(opcodeDescription[0xC4], "CALL NZ, $####");
	sprintf(opcodeDescription[0xC5], "PUSH BC");
	sprintf(opcodeDescription[0xC6], "ADD A, $####");
	sprintf(opcodeDescription[0xC7], "RST 00H");
	sprintf(opcodeDescription[0xC8], "RET Z");
	sprintf(opcodeDescription[0xC9], "RET");
	sprintf(opcodeDescription[0xCA], "JP Z, $####");
	//	sprintf(opcodeDescription[0xCB],"");
	sprintf(opcodeDescription[0xCC], "CALL Z, $####");
	sprintf(opcodeDescription[0xCD], "CALL $####");
	sprintf(opcodeDescription[0xCE], "ADC A, $##");
	sprintf(opcodeDescription[0xCF], "RST 08H");
	sprintf(opcodeDescription[0xD0], "RET NC");
	sprintf(opcodeDescription[0xD1], "POP DE");
	sprintf(opcodeDescription[0xD2], "JP NC, $####");
	sprintf(opcodeDescription[0xD3], "Undefined");
	sprintf(opcodeDescription[0xD4], "CALL NC, $####");
	sprintf(opcodeDescription[0xD5], "PUSH DE");
	sprintf(opcodeDescription[0xD6], "SUB $##");
	sprintf(opcodeDescription[0xD7], "RST 10H");
	sprintf(opcodeDescription[0xD8], "RET C");
	sprintf(opcodeDescription[0xD9], "RETI");
	sprintf(opcodeDescription[0xDA], "JP C, $####");
	sprintf(opcodeDescription[0xDB], "Undefined");
	sprintf(opcodeDescription[0xDC], "CALL C, $####");
	sprintf(opcodeDescription[0xDD], "Undefined");
	sprintf(opcodeDescription[0xDE], "SBC A, $##");
	sprintf(opcodeDescription[0xDF], "RST 18H");
	sprintf(opcodeDescription[0xE0], "LDH ($FF00+##), A");
	sprintf(opcodeDescription[0xE1], "POP HL");
	sprintf(opcodeDescription[0xE2], "LDH ($FF##+C), A");
	sprintf(opcodeDescription[0xE3], "Undefined");
	sprintf(opcodeDescription[0xE4], "Undefined");
	sprintf(opcodeDescription[0xE5], "PUSH HL");
	sprintf(opcodeDescription[0xE6], "AND $##");
	sprintf(opcodeDescription[0xE7], "RST 20H");
	sprintf(opcodeDescription[0xE8], "ADD SP, ***");
	sprintf(opcodeDescription[0xE9], "JP (HL)");
	sprintf(opcodeDescription[0xEA], "LD ($####), A");
	sprintf(opcodeDescription[0xEB], "Undefined");
	sprintf(opcodeDescription[0xEC], "Undefined");
	sprintf(opcodeDescription[0xED], "Undefined");
	sprintf(opcodeDescription[0xEE], "XOR $##");
	sprintf(opcodeDescription[0xEF], "RST 28H");
	sprintf(opcodeDescription[0xF0], "LD A, ($FF00+##)");
	sprintf(opcodeDescription[0xF1], "POP AF");
	sprintf(opcodeDescription[0xF2], "LD A, (C)");
	sprintf(opcodeDescription[0xF3], "DI");
	sprintf(opcodeDescription[0xF4], "Undefined");
	sprintf(opcodeDescription[0xF5], "PUSH AF");
	sprintf(opcodeDescription[0xF6], "OR $##");
	sprintf(opcodeDescription[0xF7], "RST 30H");
	sprintf(opcodeDescription[0xF8], "LD HL, SP+$##");
	sprintf(opcodeDescription[0xF9], "LD SP, HL");
	sprintf(opcodeDescription[0xFA], "LD A, ($####)");
	sprintf(opcodeDescription[0xFB], "EI");
	sprintf(opcodeDescription[0xFC], "Undefined");
	sprintf(opcodeDescription[0xFD], "Undefined");
	sprintf(opcodeDescription[0xFE], "CP $##");
	sprintf(opcodeDescription[0xFF], "RST 38H");
	return 1;	
}

//----------------------------------------//
// This function takes care of the main GB//
// CPU processes.                         //
//----------------------------------------//
void RunEmulation()
{
	unsigned int systemRunning = 1;
	unsigned int conditionalCycles = 0;
	unsigned int cyclesRan = 0;

	unsigned char previousOpcode = 0;
	unsigned char previousOpcode2 = 0;
	unsigned char previousOpcode3 = 0;
	unsigned char opcode = 0;
	unsigned char cbOpcode = 0;
	unsigned char hlMemVal = 0;

	OpenLogFile();
	// Add a FPS timer.

	// Record previous PC value For logging.
	unsigned int previousPCvalue = 0;
	unsigned int previousPCvalue2 = 0;
	unsigned int previousPCvalue3 = 0;
	unsigned int previousPCvalue4 = 0;
	unsigned int previousPCvalue5 = 0;
	unsigned int previousPCvalue6 = 0;
	unsigned short int stackValue = 0;

	while (systemRunning)
	{
		/*** NOTE: Need to account for HALT instruction repeat bug on classic Gameboy. ***/
		// Fetch an opcode from memory.
		previousOpcode3 = previousOpcode2;
		previousOpcode2 = previousOpcode;
		previousOpcode = opcode;
		previousPCvalue6 = previousPCvalue5;
		previousPCvalue5 = previousPCvalue4;
		previousPCvalue4 = previousPCvalue3;
		previousPCvalue3 = previousPCvalue2;
		previousPCvalue2 = previousPCvalue;
		previousPCvalue = emu.cpu.regs.PC;

		cyclesRan += HandleInterrupts();

		if (emu.state.eiDelay == 1)
		{
			emu.state.eiDelay = 0;
			IntrpMasterEnable = 1;
		}

		//			sprintf(logText, "%d", IOregister_STAT & 3);
		//			WriteToLog(logText);

					// Check if the opcode is a corrupted stop.
		if ((opcode == 0x10) && ReadMemory(emu.cpu.regs.PC) != 0x00)
		{
			sprintf(logText, "Corrupted stop at 0x%04x.  Previous PC values are 0x%04x : 0x%04x : 0x%04x : 0x%04x : 0x%04x : 0x%04x.  Previous opcodes are 0x%02x : 0x%02x : 0x%02x", emu.cpu.regs.PC, previousPCvalue, previousPCvalue2, previousPCvalue3, previousPCvalue4, previousPCvalue5, previousPCvalue6, previousOpcode, previousOpcode2, previousOpcode3);
			MessageBox(NULL, logText, "Error!", MB_OK);
		}

		// Do some boundary checking.
		if (((emu.cpu.regs.PC >= 0x8000) && (emu.cpu.regs.PC <= 0x9FFF)) || ((emu.cpu.regs.PC >= 0xFF4C) && (emu.cpu.regs.PC <= 0xFF7F)) || ((emu.cpu.regs.PC >= 0xFEA0) && (emu.cpu.regs.PC <= 0xFEFF)))
		{
			sprintf(logText, "PC set to illegal memory space at 0x%04x.  Previous PC values are 0x%04x : 0x%04x : 0x%04x : 0x%04x : 0x%04x : 0x%04x.  Previous opcodes are 0x%02x : 0x%02x : 0x%02x", emu.cpu.regs.PC, previousPCvalue, previousPCvalue2, previousPCvalue3, previousPCvalue4, previousPCvalue5, previousPCvalue6, previousOpcode, previousOpcode2, previousOpcode3);
			MessageBox(NULL, logText, "Error!", MB_OK);
		}
		/*			if (((emu.cpu.regs.SP >= 0x0000) && (emu.cpu.regs.SP <= 0x9FFF)) || ((emu.cpu.regs.SP >= 0xFF4C) && (emu.cpu.regs.SP <= 0xFF7F)) || ((emu.cpu.regs.SP >= 0xFEA0) && (emu.cpu.regs.SP <= 0xFEFF)))
					{
						sprintf(logText, "SP set to illegal memory space at 0x%04x.  Previous PC value was 0x%04x.  Previous opcode was 0x%02x.  Previous stack value: 0x%04x", emu.cpu.regs.SP, previousPCvalue, previousOpcode, stackValue);
						MessageBox(NULL, logText, "Error!", MB_OK);
					}*/
					// Get the value from memory at the HL pointer in case an operation needs it.
		hlMemVal = ReadMemory(emu.cpu.regs.HL);

		// Don't actually execute an opcode if the system is halted or stopped.
		if ((emu.state.halted == 0) && (emu.state.stopped == 0))
		{
//			stackValue = ReadMemory(emu.cpu.regs.SP) + (ReadMemory(emu.cpu.regs.SP + 1) << 8);
//			sprintf(logText, "Regs.PC = 0x%04x  AF = 0x%04x  BC = 0x%04x  DE = 0x%04x HL = 0x%04x  SP = 0x%04x  IE = 0x%02x  IF = 0x%02x  IME = 0x%01x  LY = 0x%02x  STAT = 0x%02x  CS = 0x%04x  %s\r\n", emu.cpu.regs.PC, emu.cpu.regs.AF, emu.cpu.regs.BC, emu.cpu.regs.DE, emu.cpu.regs.HL, emu.cpu.regs.SP, IOregister_IE, IOregister_IF, emu.state.intrpEnable, IOregister_LY, IOregister_STAT, stackValue, opcodeDescription[opcode]);
//			WriteToLog(logText);

			opcode = ReadMemory(emu.cpu.regs.PC);

			if (emu.state.haltInstructionRepeat == 1) // If the halt bug occured, don't increment PC for one instruction.
				emu.state.haltInstructionRepeat = 0;
			else
				emu.cpu.regs.PC++;

			// Use the opcode to call the appropriate function.
			switch (opcode)
			{
			case 0x00: /*z80_NOP();*/ break;
			case 0x01: z80_LD_reg16_value(&emu.cpu.regs.B, &emu.cpu.regs.C); break;
			case 0x02: z80_LD_location_reg16_A(&emu.cpu.regs.BC); break;
			case 0x03: z80_INC_reg16(&emu.cpu.regs.BC); break;
			case 0x04: z80_INC_reg8(&emu.cpu.regs.B); break;
			case 0x05: z80_DEC_reg8(&emu.cpu.regs.B); break;
			case 0x06: z80_LD_reg8_value(&emu.cpu.regs.B); break;
			case 0x07: z80_RLCA(); break;
			case 0x08: z80_LD_location_SP(); break;
			case 0x09: z80_ADD_HL_reg16(&emu.cpu.regs.BC); break;
			case 0x0A: z80_LD_A_location_reg16(&emu.cpu.regs.BC); break;
			case 0x0B: z80_DEC_reg16(&emu.cpu.regs.BC); break;
			case 0x0C: z80_INC_reg8(&emu.cpu.regs.C); break;
			case 0x0D: z80_DEC_reg8(&emu.cpu.regs.C); break;
			case 0x0E: z80_LD_reg8_value(&emu.cpu.regs.C); break;
			case 0x0F: z80_RRCA(); break;
			case 0x10: z80_STOP(); break;
			case 0x11: z80_LD_reg16_value(&emu.cpu.regs.D, &emu.cpu.regs.E); break;
			case 0x12: z80_LD_location_reg16_A(&emu.cpu.regs.DE); break;
			case 0x13: z80_INC_reg16(&emu.cpu.regs.DE); break;
			case 0x14: z80_INC_reg8(&emu.cpu.regs.D); break;
			case 0x15: z80_DEC_reg8(&emu.cpu.regs.D); break;
			case 0x16: z80_LD_reg8_value(&emu.cpu.regs.D); break;
			case 0x17: z80_RLA(); break;
			case 0x18: z80_JR_offset(); break;
			case 0x19: z80_ADD_HL_reg16(&emu.cpu.regs.DE); break;
			case 0x1A: z80_LD_A_location_reg16(&emu.cpu.regs.DE); break;
			case 0x1B: z80_DEC_reg16(&emu.cpu.regs.DE); break;
			case 0x1C: z80_INC_reg8(&emu.cpu.regs.E); break;
			case 0x1D: z80_DEC_reg8(&emu.cpu.regs.E); break;
			case 0x1E: z80_LD_reg8_value(&emu.cpu.regs.E); break;
			case 0x1F: z80_RRA(); break;
			case 0x20: conditionalCycles = z80_JR_condition_offset(0x01); break;
			case 0x21: z80_LD_reg16_value(&emu.cpu.regs.H, &emu.cpu.regs.L); break;
			case 0x22: z80_LDI_HL_A(); break;
			case 0x23: z80_INC_reg16(&emu.cpu.regs.HL); break;
			case 0x24: z80_INC_reg8(&emu.cpu.regs.H); break;
			case 0x25: z80_DEC_reg8(&emu.cpu.regs.H); break;
			case 0x26: z80_LD_reg8_value(&emu.cpu.regs.H); break;
			case 0x27: z80_DAA(); break;
			case 0x28: conditionalCycles = z80_JR_condition_offset(0x02); break;
			case 0x29: z80_ADD_HL_reg16(&emu.cpu.regs.HL); break;
			case 0x2A: z80_LDI_A_HL(); break;
			case 0x2B: z80_DEC_reg16(&emu.cpu.regs.HL); break;
			case 0x2C: z80_INC_reg8(&emu.cpu.regs.L); break;
			case 0x2D: z80_DEC_reg8(&emu.cpu.regs.L); break;
			case 0x2E: z80_LD_reg8_value(&emu.cpu.regs.L); break;
			case 0x2F: z80_CPL(); break;
			case 0x30: conditionalCycles = z80_JR_condition_offset(0x03); break;
			case 0x31: z80_LD_reg16_value(&emu.cpu.regs.SPH, &emu.cpu.regs.SPL); break;
			case 0x32: z80_LDD_HL_A(); break;
			case 0x33: z80_INC_reg16(&emu.cpu.regs.SP); break;
			case 0x34: z80_INC_location_HL(); break;
			case 0x35: z80_DEC_location_HL(); break;
			case 0x36: z80_LD_location_HL_immediate(); break;
			case 0x37: z80_SCF(); break;
			case 0x38: conditionalCycles = z80_JR_condition_offset(0x04); break;
			case 0x39: z80_ADD_HL_reg16(&emu.cpu.regs.SP); break;
			case 0x3A: z80_LDD_A_HL(); break;
			case 0x3B: z80_DEC_reg16(&emu.cpu.regs.SP); break;
			case 0x3C: z80_INC_reg8(&emu.cpu.regs.A); break;
			case 0x3D: z80_DEC_reg8(&emu.cpu.regs.A); break;
			case 0x3E: z80_LD_reg8_value(&emu.cpu.regs.A); break;
			case 0x3F: z80_CCF(); break;
			case 0x40: z80_LD_reg8_reg8(&emu.cpu.regs.B, &emu.cpu.regs.B); break;
			case 0x41: z80_LD_reg8_reg8(&emu.cpu.regs.B, &emu.cpu.regs.C); break;
			case 0x42: z80_LD_reg8_reg8(&emu.cpu.regs.B, &emu.cpu.regs.D); break;
			case 0x43: z80_LD_reg8_reg8(&emu.cpu.regs.B, &emu.cpu.regs.E); break;
			case 0x44: z80_LD_reg8_reg8(&emu.cpu.regs.B, &emu.cpu.regs.H); break;
			case 0x45: z80_LD_reg8_reg8(&emu.cpu.regs.B, &emu.cpu.regs.L); break;
			case 0x46: z80_LD_reg8_location_HL(&emu.cpu.regs.B); break;
			case 0x47: z80_LD_reg8_reg8(&emu.cpu.regs.B, &emu.cpu.regs.A); break;
			case 0x48: z80_LD_reg8_reg8(&emu.cpu.regs.C, &emu.cpu.regs.B); break;
			case 0x49: z80_LD_reg8_reg8(&emu.cpu.regs.C, &emu.cpu.regs.C); break;
			case 0x4A: z80_LD_reg8_reg8(&emu.cpu.regs.C, &emu.cpu.regs.D); break;
			case 0x4B: z80_LD_reg8_reg8(&emu.cpu.regs.C, &emu.cpu.regs.E); break;
			case 0x4C: z80_LD_reg8_reg8(&emu.cpu.regs.C, &emu.cpu.regs.H); break;
			case 0x4D: z80_LD_reg8_reg8(&emu.cpu.regs.C, &emu.cpu.regs.L); break;
			case 0x4E: z80_LD_reg8_location_HL(&emu.cpu.regs.C); break;
			case 0x4F: z80_LD_reg8_reg8(&emu.cpu.regs.C, &emu.cpu.regs.A); break;
			case 0x50: z80_LD_reg8_reg8(&emu.cpu.regs.D, &emu.cpu.regs.B); break;
			case 0x51: z80_LD_reg8_reg8(&emu.cpu.regs.D, &emu.cpu.regs.C); break;
			case 0x52: z80_LD_reg8_reg8(&emu.cpu.regs.D, &emu.cpu.regs.D); break;
			case 0x53: z80_LD_reg8_reg8(&emu.cpu.regs.D, &emu.cpu.regs.E); break;
			case 0x54: z80_LD_reg8_reg8(&emu.cpu.regs.D, &emu.cpu.regs.H); break;
			case 0x55: z80_LD_reg8_reg8(&emu.cpu.regs.D, &emu.cpu.regs.L); break;
			case 0x56: z80_LD_reg8_location_HL(&emu.cpu.regs.D); break;
			case 0x57: z80_LD_reg8_reg8(&emu.cpu.regs.D, &emu.cpu.regs.A); break;
			case 0x58: z80_LD_reg8_reg8(&emu.cpu.regs.E, &emu.cpu.regs.B); break;
			case 0x59: z80_LD_reg8_reg8(&emu.cpu.regs.E, &emu.cpu.regs.C); break;
			case 0x5A: z80_LD_reg8_reg8(&emu.cpu.regs.E, &emu.cpu.regs.D); break;
			case 0x5B: z80_LD_reg8_reg8(&emu.cpu.regs.E, &emu.cpu.regs.E); break;
			case 0x5C: z80_LD_reg8_reg8(&emu.cpu.regs.E, &emu.cpu.regs.H); break;
			case 0x5D: z80_LD_reg8_reg8(&emu.cpu.regs.E, &emu.cpu.regs.L); break;
			case 0x5E: z80_LD_reg8_location_HL(&emu.cpu.regs.E); break;
			case 0x5F: z80_LD_reg8_reg8(&emu.cpu.regs.E, &emu.cpu.regs.A); break;
			case 0x60: z80_LD_reg8_reg8(&emu.cpu.regs.H, &emu.cpu.regs.B); break;
			case 0x61: z80_LD_reg8_reg8(&emu.cpu.regs.H, &emu.cpu.regs.C); break;
			case 0x62: z80_LD_reg8_reg8(&emu.cpu.regs.H, &emu.cpu.regs.D); break;
			case 0x63: z80_LD_reg8_reg8(&emu.cpu.regs.H, &emu.cpu.regs.E); break;
			case 0x64: z80_LD_reg8_reg8(&emu.cpu.regs.H, &emu.cpu.regs.H); break;
			case 0x65: z80_LD_reg8_reg8(&emu.cpu.regs.H, &emu.cpu.regs.L); break;
			case 0x66: z80_LD_reg8_location_HL(&emu.cpu.regs.H); break;
			case 0x67: z80_LD_reg8_reg8(&emu.cpu.regs.H, &emu.cpu.regs.A); break;
			case 0x68: z80_LD_reg8_reg8(&emu.cpu.regs.L, &emu.cpu.regs.B); break;
			case 0x69: z80_LD_reg8_reg8(&emu.cpu.regs.L, &emu.cpu.regs.C); break;
			case 0x6A: z80_LD_reg8_reg8(&emu.cpu.regs.L, &emu.cpu.regs.D); break;
			case 0x6B: z80_LD_reg8_reg8(&emu.cpu.regs.L, &emu.cpu.regs.E); break;
			case 0x6C: z80_LD_reg8_reg8(&emu.cpu.regs.L, &emu.cpu.regs.H); break;
			case 0x6D: z80_LD_reg8_reg8(&emu.cpu.regs.L, &emu.cpu.regs.L); break;
			case 0x6E: z80_LD_reg8_location_HL(&emu.cpu.regs.L); break;
			case 0x6F: z80_LD_reg8_reg8(&emu.cpu.regs.L, &emu.cpu.regs.A); break;
			case 0x70: z80_LD_location_HL_reg8(&emu.cpu.regs.B); break;
			case 0x71: z80_LD_location_HL_reg8(&emu.cpu.regs.C); break;
			case 0x72: z80_LD_location_HL_reg8(&emu.cpu.regs.D); break;
			case 0x73: z80_LD_location_HL_reg8(&emu.cpu.regs.E); break;
			case 0x74: z80_LD_location_HL_reg8(&emu.cpu.regs.H); break;
			case 0x75: z80_LD_location_HL_reg8(&emu.cpu.regs.L); break;
			case 0x76: z80_HALT(); break;
			case 0x77: z80_LD_location_HL_reg8(&emu.cpu.regs.A); break;
			case 0x78: z80_LD_reg8_reg8(&emu.cpu.regs.A, &emu.cpu.regs.B); break;
			case 0x79: z80_LD_reg8_reg8(&emu.cpu.regs.A, &emu.cpu.regs.C); break;
			case 0x7A: z80_LD_reg8_reg8(&emu.cpu.regs.A, &emu.cpu.regs.D); break;
			case 0x7B: z80_LD_reg8_reg8(&emu.cpu.regs.A, &emu.cpu.regs.E); break;
			case 0x7C: z80_LD_reg8_reg8(&emu.cpu.regs.A, &emu.cpu.regs.H); break;
			case 0x7D: z80_LD_reg8_reg8(&emu.cpu.regs.A, &emu.cpu.regs.L); break;
			case 0x7E: z80_LD_reg8_location_HL(&emu.cpu.regs.A); break;
			case 0x7F: z80_LD_reg8_reg8(&emu.cpu.regs.A, &emu.cpu.regs.A); break;
			case 0x80: z80_ADD_A_reg8(&emu.cpu.regs.B); break;
			case 0x81: z80_ADD_A_reg8(&emu.cpu.regs.C); break;
			case 0x82: z80_ADD_A_reg8(&emu.cpu.regs.D); break;
			case 0x83: z80_ADD_A_reg8(&emu.cpu.regs.E); break;
			case 0x84: z80_ADD_A_reg8(&emu.cpu.regs.H); break;
			case 0x85: z80_ADD_A_reg8(&emu.cpu.regs.L); break;
			case 0x86: z80_ADD_A_reg8(&hlMemVal); break;
			case 0x87: z80_ADD_A_reg8(&emu.cpu.regs.A); break;
			case 0x88: z80_ADC_A_reg8(&emu.cpu.regs.B); break;
			case 0x89: z80_ADC_A_reg8(&emu.cpu.regs.C); break;
			case 0x8A: z80_ADC_A_reg8(&emu.cpu.regs.D); break;
			case 0x8B: z80_ADC_A_reg8(&emu.cpu.regs.E); break;
			case 0x8C: z80_ADC_A_reg8(&emu.cpu.regs.H); break;
			case 0x8D: z80_ADC_A_reg8(&emu.cpu.regs.L); break;
			case 0x8E: z80_ADC_A_reg8(&hlMemVal); break;
			case 0x8F: z80_ADC_A_reg8(&emu.cpu.regs.A); break;
			case 0x90: z80_SUB_reg8(&emu.cpu.regs.B); break;
			case 0x91: z80_SUB_reg8(&emu.cpu.regs.C); break;
			case 0x92: z80_SUB_reg8(&emu.cpu.regs.D); break;
			case 0x93: z80_SUB_reg8(&emu.cpu.regs.E); break;
			case 0x94: z80_SUB_reg8(&emu.cpu.regs.H); break;
			case 0x95: z80_SUB_reg8(&emu.cpu.regs.L); break;
			case 0x96: z80_SUB_reg8(&hlMemVal); break;
			case 0x97: z80_SUB_reg8(&emu.cpu.regs.A); break;
			case 0x98: z80_SBC_A_reg8(&emu.cpu.regs.B); break;
			case 0x99: z80_SBC_A_reg8(&emu.cpu.regs.C); break;
			case 0x9A: z80_SBC_A_reg8(&emu.cpu.regs.D); break;
			case 0x9B: z80_SBC_A_reg8(&emu.cpu.regs.E); break;
			case 0x9C: z80_SBC_A_reg8(&emu.cpu.regs.H); break;
			case 0x9D: z80_SBC_A_reg8(&emu.cpu.regs.L); break;
			case 0x9E: z80_SBC_A_reg8(&hlMemVal); break;
			case 0x9F: z80_SBC_A_reg8(&emu.cpu.regs.A); break;
			case 0xA0: z80_AND_reg8(&emu.cpu.regs.B); break;
			case 0xA1: z80_AND_reg8(&emu.cpu.regs.C); break;
			case 0xA2: z80_AND_reg8(&emu.cpu.regs.D); break;
			case 0xA3: z80_AND_reg8(&emu.cpu.regs.E); break;
			case 0xA4: z80_AND_reg8(&emu.cpu.regs.H); break;
			case 0xA5: z80_AND_reg8(&emu.cpu.regs.L); break;
			case 0xA6: z80_AND_reg8(&hlMemVal); break;
			case 0xA7: z80_AND_reg8(&emu.cpu.regs.A); break;
			case 0xA8: z80_XOR_reg8(&emu.cpu.regs.B); break;
			case 0xA9: z80_XOR_reg8(&emu.cpu.regs.C); break;
			case 0xAA: z80_XOR_reg8(&emu.cpu.regs.D); break;
			case 0xAB: z80_XOR_reg8(&emu.cpu.regs.E); break;
			case 0xAC: z80_XOR_reg8(&emu.cpu.regs.H); break;
			case 0xAD: z80_XOR_reg8(&emu.cpu.regs.L); break;
			case 0xAE: z80_XOR_reg8(&hlMemVal); break;
			case 0xAF: z80_XOR_reg8(&emu.cpu.regs.A); break;
			case 0xB0: z80_OR_reg8(&emu.cpu.regs.B); break;
			case 0xB1: z80_OR_reg8(&emu.cpu.regs.C); break;
			case 0xB2: z80_OR_reg8(&emu.cpu.regs.D); break;
			case 0xB3: z80_OR_reg8(&emu.cpu.regs.E); break;
			case 0xB4: z80_OR_reg8(&emu.cpu.regs.H); break;
			case 0xB5: z80_OR_reg8(&emu.cpu.regs.L); break;
			case 0xB6: z80_OR_reg8(&hlMemVal); break;
			case 0xB7: z80_OR_reg8(&emu.cpu.regs.A); break;
			case 0xB8: z80_CP_reg8(&emu.cpu.regs.B); break;
			case 0xB9: z80_CP_reg8(&emu.cpu.regs.C); break;
			case 0xBA: z80_CP_reg8(&emu.cpu.regs.D); break;
			case 0xBB: z80_CP_reg8(&emu.cpu.regs.E); break;
			case 0xBC: z80_CP_reg8(&emu.cpu.regs.H); break;
			case 0xBD: z80_CP_reg8(&emu.cpu.regs.L); break;
			case 0xBE: z80_CP_reg8(&hlMemVal); break;
			case 0xBF: z80_CP_reg8(&emu.cpu.regs.A); break;
			case 0xC0: conditionalCycles = z80_RET_condition(0x01); break;
			case 0xC1: z80_POP_reg16(&emu.cpu.regs.B, &emu.cpu.regs.C); break;
			case 0xC2: conditionalCycles = z80_JP_condition_immediate(0x01); break;
			case 0xC3: z80_JP_immediate(); break;
			case 0xC4: conditionalCycles = z80_CALL_condition_immediate(0x01); break;
			case 0xC5: z80_PUSH_reg16(&emu.cpu.regs.B, &emu.cpu.regs.C); break;
			case 0xC6: z80_ADD_A_immediate(); break;
			case 0xC7: z80_RST(0x00); break;
			case 0xC8: conditionalCycles = z80_RET_condition(0x02); break;
			case 0xC9: z80_RET(); break;
			case 0xCA: conditionalCycles = z80_JP_condition_immediate(0x02); break;
				// Cover the special 0xCB opcodes.
			case 0xCB:
			{
				cbOpcode = ReadMemory(emu.cpu.regs.PC);
				emu.cpu.regs.PC++;
				switch (cbOpcode)
				{
				case 0x00: z80_RLC_reg8(&emu.cpu.regs.B); break;
				case 0x01: z80_RLC_reg8(&emu.cpu.regs.C); break;
				case 0x02: z80_RLC_reg8(&emu.cpu.regs.D); break;
				case 0x03: z80_RLC_reg8(&emu.cpu.regs.E); break;
				case 0x04: z80_RLC_reg8(&emu.cpu.regs.H); break;
				case 0x05: z80_RLC_reg8(&emu.cpu.regs.L); break;
				case 0x06: z80_RLC_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x07: z80_RLC_reg8(&emu.cpu.regs.A); break;
				case 0x08: z80_RRC_reg8(&emu.cpu.regs.B); break;
				case 0x09: z80_RRC_reg8(&emu.cpu.regs.C); break;
				case 0x0A: z80_RRC_reg8(&emu.cpu.regs.D); break;
				case 0x0B: z80_RRC_reg8(&emu.cpu.regs.E); break;
				case 0x0C: z80_RRC_reg8(&emu.cpu.regs.H); break;
				case 0x0D: z80_RRC_reg8(&emu.cpu.regs.L); break;
				case 0x0E: z80_RRC_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x0F: z80_RRC_reg8(&emu.cpu.regs.A); break;
				case 0x10: z80_RL_reg8(&emu.cpu.regs.B); break;
				case 0x11: z80_RL_reg8(&emu.cpu.regs.C); break;
				case 0x12: z80_RL_reg8(&emu.cpu.regs.D); break;
				case 0x13: z80_RL_reg8(&emu.cpu.regs.E); break;
				case 0x14: z80_RL_reg8(&emu.cpu.regs.H); break;
				case 0x15: z80_RL_reg8(&emu.cpu.regs.L); break;
				case 0x16: z80_RL_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x17: z80_RL_reg8(&emu.cpu.regs.A); break;
				case 0x18: z80_RR_reg8(&emu.cpu.regs.B); break;
				case 0x19: z80_RR_reg8(&emu.cpu.regs.C); break;
				case 0x1A: z80_RR_reg8(&emu.cpu.regs.D); break;
				case 0x1B: z80_RR_reg8(&emu.cpu.regs.E); break;
				case 0x1C: z80_RR_reg8(&emu.cpu.regs.H); break;
				case 0x1D: z80_RR_reg8(&emu.cpu.regs.L); break;
				case 0x1E: z80_RR_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x1F: z80_RR_reg8(&emu.cpu.regs.A); break;
				case 0x20: z80_SLA_reg8(&emu.cpu.regs.B); break;
				case 0x21: z80_SLA_reg8(&emu.cpu.regs.C); break;
				case 0x22: z80_SLA_reg8(&emu.cpu.regs.D); break;
				case 0x23: z80_SLA_reg8(&emu.cpu.regs.E); break;
				case 0x24: z80_SLA_reg8(&emu.cpu.regs.H); break;
				case 0x25: z80_SLA_reg8(&emu.cpu.regs.L); break;
				case 0x26: z80_SLA_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x27: z80_SLA_reg8(&emu.cpu.regs.A); break;
				case 0x28: z80_SRA_reg8(&emu.cpu.regs.B); break;
				case 0x29: z80_SRA_reg8(&emu.cpu.regs.C); break;
				case 0x2A: z80_SRA_reg8(&emu.cpu.regs.D); break;
				case 0x2B: z80_SRA_reg8(&emu.cpu.regs.E); break;
				case 0x2C: z80_SRA_reg8(&emu.cpu.regs.H); break;
				case 0x2D: z80_SRA_reg8(&emu.cpu.regs.L); break;
				case 0x2E: z80_SRA_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x2F: z80_SRA_reg8(&emu.cpu.regs.A); break;
				case 0x30: z80_SWAP_reg8(&emu.cpu.regs.B); break;
				case 0x31: z80_SWAP_reg8(&emu.cpu.regs.C); break;
				case 0x32: z80_SWAP_reg8(&emu.cpu.regs.D); break;
				case 0x33: z80_SWAP_reg8(&emu.cpu.regs.E); break;
				case 0x34: z80_SWAP_reg8(&emu.cpu.regs.H); break;
				case 0x35: z80_SWAP_reg8(&emu.cpu.regs.L); break;
				case 0x36: z80_SWAP_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x37: z80_SWAP_reg8(&emu.cpu.regs.A); break;
				case 0x38: z80_SRL_reg8(&emu.cpu.regs.B); break;
				case 0x39: z80_SRL_reg8(&emu.cpu.regs.C); break;
				case 0x3A: z80_SRL_reg8(&emu.cpu.regs.D); break;
				case 0x3B: z80_SRL_reg8(&emu.cpu.regs.E); break;
				case 0x3C: z80_SRL_reg8(&emu.cpu.regs.H); break;
				case 0x3D: z80_SRL_reg8(&emu.cpu.regs.L); break;
				case 0x3E: z80_SRL_reg8(&hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x3F: z80_SRL_reg8(&emu.cpu.regs.A); break;
				case 0x40: z80_BIT_bit_reg8(BIT_0, &emu.cpu.regs.B); break;
				case 0x41: z80_BIT_bit_reg8(BIT_0, &emu.cpu.regs.C); break;
				case 0x42: z80_BIT_bit_reg8(BIT_0, &emu.cpu.regs.D); break;
				case 0x43: z80_BIT_bit_reg8(BIT_0, &emu.cpu.regs.E); break;
				case 0x44: z80_BIT_bit_reg8(BIT_0, &emu.cpu.regs.H); break;
				case 0x45: z80_BIT_bit_reg8(BIT_0, &emu.cpu.regs.L); break;
				case 0x46: z80_BIT_bit_reg8(BIT_0, &hlMemVal); break;
				case 0x47: z80_BIT_bit_reg8(BIT_0, &emu.cpu.regs.A); break;
				case 0x48: z80_BIT_bit_reg8(BIT_1, &emu.cpu.regs.B); break;
				case 0x49: z80_BIT_bit_reg8(BIT_1, &emu.cpu.regs.C); break;
				case 0x4A: z80_BIT_bit_reg8(BIT_1, &emu.cpu.regs.D); break;
				case 0x4B: z80_BIT_bit_reg8(BIT_1, &emu.cpu.regs.E); break;
				case 0x4C: z80_BIT_bit_reg8(BIT_1, &emu.cpu.regs.H); break;
				case 0x4D: z80_BIT_bit_reg8(BIT_1, &emu.cpu.regs.L); break;
				case 0x4E: z80_BIT_bit_reg8(BIT_1, &hlMemVal); break;
				case 0x4F: z80_BIT_bit_reg8(BIT_1, &emu.cpu.regs.A); break;
				case 0x50: z80_BIT_bit_reg8(BIT_2, &emu.cpu.regs.B); break;
				case 0x51: z80_BIT_bit_reg8(BIT_2, &emu.cpu.regs.C); break;
				case 0x52: z80_BIT_bit_reg8(BIT_2, &emu.cpu.regs.D); break;
				case 0x53: z80_BIT_bit_reg8(BIT_2, &emu.cpu.regs.E); break;
				case 0x54: z80_BIT_bit_reg8(BIT_2, &emu.cpu.regs.H); break;
				case 0x55: z80_BIT_bit_reg8(BIT_2, &emu.cpu.regs.L); break;
				case 0x56: z80_BIT_bit_reg8(BIT_2, &hlMemVal); break;
				case 0x57: z80_BIT_bit_reg8(BIT_2, &emu.cpu.regs.A); break;
				case 0x58: z80_BIT_bit_reg8(BIT_3, &emu.cpu.regs.B); break;
				case 0x59: z80_BIT_bit_reg8(BIT_3, &emu.cpu.regs.C); break;
				case 0x5A: z80_BIT_bit_reg8(BIT_3, &emu.cpu.regs.D); break;
				case 0x5B: z80_BIT_bit_reg8(BIT_3, &emu.cpu.regs.E); break;
				case 0x5C: z80_BIT_bit_reg8(BIT_3, &emu.cpu.regs.H); break;
				case 0x5D: z80_BIT_bit_reg8(BIT_3, &emu.cpu.regs.L); break;
				case 0x5E: z80_BIT_bit_reg8(BIT_3, &hlMemVal); break;
				case 0x5F: z80_BIT_bit_reg8(BIT_3, &emu.cpu.regs.A); break;
				case 0x60: z80_BIT_bit_reg8(BIT_4, &emu.cpu.regs.B); break;
				case 0x61: z80_BIT_bit_reg8(BIT_4, &emu.cpu.regs.C); break;
				case 0x62: z80_BIT_bit_reg8(BIT_4, &emu.cpu.regs.D); break;
				case 0x63: z80_BIT_bit_reg8(BIT_4, &emu.cpu.regs.E); break;
				case 0x64: z80_BIT_bit_reg8(BIT_4, &emu.cpu.regs.H); break;
				case 0x65: z80_BIT_bit_reg8(BIT_4, &emu.cpu.regs.L); break;
				case 0x66: z80_BIT_bit_reg8(BIT_4, &hlMemVal); break;
				case 0x67: z80_BIT_bit_reg8(BIT_4, &emu.cpu.regs.A); break;
				case 0x68: z80_BIT_bit_reg8(BIT_5, &emu.cpu.regs.B); break;
				case 0x69: z80_BIT_bit_reg8(BIT_5, &emu.cpu.regs.C); break;
				case 0x6A: z80_BIT_bit_reg8(BIT_5, &emu.cpu.regs.D); break;
				case 0x6B: z80_BIT_bit_reg8(BIT_5, &emu.cpu.regs.E); break;
				case 0x6C: z80_BIT_bit_reg8(BIT_5, &emu.cpu.regs.H); break;
				case 0x6D: z80_BIT_bit_reg8(BIT_5, &emu.cpu.regs.L); break;
				case 0x6E: z80_BIT_bit_reg8(BIT_5, &hlMemVal); break;
				case 0x6F: z80_BIT_bit_reg8(BIT_5, &emu.cpu.regs.A); break;
				case 0x70: z80_BIT_bit_reg8(BIT_6, &emu.cpu.regs.B); break;
				case 0x71: z80_BIT_bit_reg8(BIT_6, &emu.cpu.regs.C); break;
				case 0x72: z80_BIT_bit_reg8(BIT_6, &emu.cpu.regs.D); break;
				case 0x73: z80_BIT_bit_reg8(BIT_6, &emu.cpu.regs.E); break;
				case 0x74: z80_BIT_bit_reg8(BIT_6, &emu.cpu.regs.H); break;
				case 0x75: z80_BIT_bit_reg8(BIT_6, &emu.cpu.regs.L); break;
				case 0x76: z80_BIT_bit_reg8(BIT_6, &hlMemVal); break;
				case 0x77: z80_BIT_bit_reg8(BIT_6, &emu.cpu.regs.A); break;
				case 0x78: z80_BIT_bit_reg8(BIT_7, &emu.cpu.regs.B); break;
				case 0x79: z80_BIT_bit_reg8(BIT_7, &emu.cpu.regs.C); break;
				case 0x7A: z80_BIT_bit_reg8(BIT_7, &emu.cpu.regs.D); break;
				case 0x7B: z80_BIT_bit_reg8(BIT_7, &emu.cpu.regs.E); break;
				case 0x7C: z80_BIT_bit_reg8(BIT_7, &emu.cpu.regs.H); break;
				case 0x7D: z80_BIT_bit_reg8(BIT_7, &emu.cpu.regs.L); break;
				case 0x7E: z80_BIT_bit_reg8(BIT_7, &hlMemVal); break;
				case 0x7F: z80_BIT_bit_reg8(BIT_7, &emu.cpu.regs.A); break;
				case 0x80: z80_RES_bit_reg8(BIT_0, &emu.cpu.regs.B); break;
				case 0x81: z80_RES_bit_reg8(BIT_0, &emu.cpu.regs.C); break;
				case 0x82: z80_RES_bit_reg8(BIT_0, &emu.cpu.regs.D); break;
				case 0x83: z80_RES_bit_reg8(BIT_0, &emu.cpu.regs.E); break;
				case 0x84: z80_RES_bit_reg8(BIT_0, &emu.cpu.regs.H); break;
				case 0x85: z80_RES_bit_reg8(BIT_0, &emu.cpu.regs.L); break;
				case 0x86: z80_RES_bit_reg8(BIT_0, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x87: z80_RES_bit_reg8(BIT_0, &emu.cpu.regs.A); break;
				case 0x88: z80_RES_bit_reg8(BIT_1, &emu.cpu.regs.B); break;
				case 0x89: z80_RES_bit_reg8(BIT_1, &emu.cpu.regs.C); break;
				case 0x8A: z80_RES_bit_reg8(BIT_1, &emu.cpu.regs.D); break;
				case 0x8B: z80_RES_bit_reg8(BIT_1, &emu.cpu.regs.E); break;
				case 0x8C: z80_RES_bit_reg8(BIT_1, &emu.cpu.regs.H); break;
				case 0x8D: z80_RES_bit_reg8(BIT_1, &emu.cpu.regs.L); break;
				case 0x8E: z80_RES_bit_reg8(BIT_1, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x8F: z80_RES_bit_reg8(BIT_1, &emu.cpu.regs.A); break;
				case 0x90: z80_RES_bit_reg8(BIT_2, &emu.cpu.regs.B); break;
				case 0x91: z80_RES_bit_reg8(BIT_2, &emu.cpu.regs.C); break;
				case 0x92: z80_RES_bit_reg8(BIT_2, &emu.cpu.regs.D); break;
				case 0x93: z80_RES_bit_reg8(BIT_2, &emu.cpu.regs.E); break;
				case 0x94: z80_RES_bit_reg8(BIT_2, &emu.cpu.regs.H); break;
				case 0x95: z80_RES_bit_reg8(BIT_2, &emu.cpu.regs.L); break;
				case 0x96: z80_RES_bit_reg8(BIT_2, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x97: z80_RES_bit_reg8(BIT_2, &emu.cpu.regs.A); break;
				case 0x98: z80_RES_bit_reg8(BIT_3, &emu.cpu.regs.B); break;
				case 0x99: z80_RES_bit_reg8(BIT_3, &emu.cpu.regs.C); break;
				case 0x9A: z80_RES_bit_reg8(BIT_3, &emu.cpu.regs.D); break;
				case 0x9B: z80_RES_bit_reg8(BIT_3, &emu.cpu.regs.E); break;
				case 0x9C: z80_RES_bit_reg8(BIT_3, &emu.cpu.regs.H); break;
				case 0x9D: z80_RES_bit_reg8(BIT_3, &emu.cpu.regs.L); break;
				case 0x9E: z80_RES_bit_reg8(BIT_3, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0x9F: z80_RES_bit_reg8(BIT_3, &emu.cpu.regs.A); break;
				case 0xA0: z80_RES_bit_reg8(BIT_4, &emu.cpu.regs.B); break;
				case 0xA1: z80_RES_bit_reg8(BIT_4, &emu.cpu.regs.C); break;
				case 0xA2: z80_RES_bit_reg8(BIT_4, &emu.cpu.regs.D); break;
				case 0xA3: z80_RES_bit_reg8(BIT_4, &emu.cpu.regs.E); break;
				case 0xA4: z80_RES_bit_reg8(BIT_4, &emu.cpu.regs.H); break;
				case 0xA5: z80_RES_bit_reg8(BIT_4, &emu.cpu.regs.L); break;
				case 0xA6: z80_RES_bit_reg8(BIT_4, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xA7: z80_RES_bit_reg8(BIT_4, &emu.cpu.regs.A); break;
				case 0xA8: z80_RES_bit_reg8(BIT_5, &emu.cpu.regs.B); break;
				case 0xA9: z80_RES_bit_reg8(BIT_5, &emu.cpu.regs.C); break;
				case 0xAA: z80_RES_bit_reg8(BIT_5, &emu.cpu.regs.D); break;
				case 0xAB: z80_RES_bit_reg8(BIT_5, &emu.cpu.regs.E); break;
				case 0xAC: z80_RES_bit_reg8(BIT_5, &emu.cpu.regs.H); break;
				case 0xAD: z80_RES_bit_reg8(BIT_5, &emu.cpu.regs.L); break;
				case 0xAE: z80_RES_bit_reg8(BIT_5, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xAF: z80_RES_bit_reg8(BIT_5, &emu.cpu.regs.A); break;
				case 0xB0: z80_RES_bit_reg8(BIT_6, &emu.cpu.regs.B); break;
				case 0xB1: z80_RES_bit_reg8(BIT_6, &emu.cpu.regs.C); break;
				case 0xB2: z80_RES_bit_reg8(BIT_6, &emu.cpu.regs.D); break;
				case 0xB3: z80_RES_bit_reg8(BIT_6, &emu.cpu.regs.E); break;
				case 0xB4: z80_RES_bit_reg8(BIT_6, &emu.cpu.regs.H); break;
				case 0xB5: z80_RES_bit_reg8(BIT_6, &emu.cpu.regs.L); break;
				case 0xB6: z80_RES_bit_reg8(BIT_6, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xB7: z80_RES_bit_reg8(BIT_6, &emu.cpu.regs.A); break;
				case 0xB8: z80_RES_bit_reg8(BIT_7, &emu.cpu.regs.B); break;
				case 0xB9: z80_RES_bit_reg8(BIT_7, &emu.cpu.regs.C); break;
				case 0xBA: z80_RES_bit_reg8(BIT_7, &emu.cpu.regs.D); break;
				case 0xBB: z80_RES_bit_reg8(BIT_7, &emu.cpu.regs.E); break;
				case 0xBC: z80_RES_bit_reg8(BIT_7, &emu.cpu.regs.H); break;
				case 0xBD: z80_RES_bit_reg8(BIT_7, &emu.cpu.regs.L); break;
				case 0xBE: z80_RES_bit_reg8(BIT_7, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xBF: z80_RES_bit_reg8(BIT_7, &emu.cpu.regs.A); break;
				case 0xC0: z80_SET_bit_reg8(BIT_0, &emu.cpu.regs.B); break;
				case 0xC1: z80_SET_bit_reg8(BIT_0, &emu.cpu.regs.C); break;
				case 0xC2: z80_SET_bit_reg8(BIT_0, &emu.cpu.regs.D); break;
				case 0xC3: z80_SET_bit_reg8(BIT_0, &emu.cpu.regs.E); break;
				case 0xC4: z80_SET_bit_reg8(BIT_0, &emu.cpu.regs.H); break;
				case 0xC5: z80_SET_bit_reg8(BIT_0, &emu.cpu.regs.L); break;
				case 0xC6: z80_SET_bit_reg8(BIT_0, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xC7: z80_SET_bit_reg8(BIT_0, &emu.cpu.regs.A); break;
				case 0xC8: z80_SET_bit_reg8(BIT_1, &emu.cpu.regs.B); break;
				case 0xC9: z80_SET_bit_reg8(BIT_1, &emu.cpu.regs.C); break;
				case 0xCA: z80_SET_bit_reg8(BIT_1, &emu.cpu.regs.D); break;
				case 0xCB: z80_SET_bit_reg8(BIT_1, &emu.cpu.regs.E); break;
				case 0xCC: z80_SET_bit_reg8(BIT_1, &emu.cpu.regs.H); break;
				case 0xCD: z80_SET_bit_reg8(BIT_1, &emu.cpu.regs.L); break;
				case 0xCE: z80_SET_bit_reg8(BIT_1, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xCF: z80_SET_bit_reg8(BIT_1, &emu.cpu.regs.A); break;
				case 0xD0: z80_SET_bit_reg8(BIT_2, &emu.cpu.regs.B); break;
				case 0xD1: z80_SET_bit_reg8(BIT_2, &emu.cpu.regs.C); break;
				case 0xD2: z80_SET_bit_reg8(BIT_2, &emu.cpu.regs.D); break;
				case 0xD3: z80_SET_bit_reg8(BIT_2, &emu.cpu.regs.E); break;
				case 0xD4: z80_SET_bit_reg8(BIT_2, &emu.cpu.regs.H); break;
				case 0xD5: z80_SET_bit_reg8(BIT_2, &emu.cpu.regs.L); break;
				case 0xD6: z80_SET_bit_reg8(BIT_2, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xD7: z80_SET_bit_reg8(BIT_2, &emu.cpu.regs.A); break;
				case 0xD8: z80_SET_bit_reg8(BIT_3, &emu.cpu.regs.B); break;
				case 0xD9: z80_SET_bit_reg8(BIT_3, &emu.cpu.regs.C); break;
				case 0xDA: z80_SET_bit_reg8(BIT_3, &emu.cpu.regs.D); break;
				case 0xDB: z80_SET_bit_reg8(BIT_3, &emu.cpu.regs.E); break;
				case 0xDC: z80_SET_bit_reg8(BIT_3, &emu.cpu.regs.H); break;
				case 0xDD: z80_SET_bit_reg8(BIT_3, &emu.cpu.regs.L); break;
				case 0xDE: z80_SET_bit_reg8(BIT_3, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xDF: z80_SET_bit_reg8(BIT_3, &emu.cpu.regs.A); break;
				case 0xE0: z80_SET_bit_reg8(BIT_4, &emu.cpu.regs.B); break;
				case 0xE1: z80_SET_bit_reg8(BIT_4, &emu.cpu.regs.C); break;
				case 0xE2: z80_SET_bit_reg8(BIT_4, &emu.cpu.regs.D); break;
				case 0xE3: z80_SET_bit_reg8(BIT_4, &emu.cpu.regs.E); break;
				case 0xE4: z80_SET_bit_reg8(BIT_4, &emu.cpu.regs.H); break;
				case 0xE5: z80_SET_bit_reg8(BIT_4, &emu.cpu.regs.L); break;
				case 0xE6: z80_SET_bit_reg8(BIT_4, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xE7: z80_SET_bit_reg8(BIT_4, &emu.cpu.regs.A); break;
				case 0xE8: z80_SET_bit_reg8(BIT_5, &emu.cpu.regs.B); break;
				case 0xE9: z80_SET_bit_reg8(BIT_5, &emu.cpu.regs.C); break;
				case 0xEA: z80_SET_bit_reg8(BIT_5, &emu.cpu.regs.D); break;
				case 0xEB: z80_SET_bit_reg8(BIT_5, &emu.cpu.regs.E); break;
				case 0xEC: z80_SET_bit_reg8(BIT_5, &emu.cpu.regs.H); break;
				case 0xED: z80_SET_bit_reg8(BIT_5, &emu.cpu.regs.L); break;
				case 0xEE: z80_SET_bit_reg8(BIT_5, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xEF: z80_SET_bit_reg8(BIT_5, &emu.cpu.regs.A); break;
				case 0xF0: z80_SET_bit_reg8(BIT_6, &emu.cpu.regs.B); break;
				case 0xF1: z80_SET_bit_reg8(BIT_6, &emu.cpu.regs.C); break;
				case 0xF2: z80_SET_bit_reg8(BIT_6, &emu.cpu.regs.D); break;
				case 0xF3: z80_SET_bit_reg8(BIT_6, &emu.cpu.regs.E); break;
				case 0xF4: z80_SET_bit_reg8(BIT_6, &emu.cpu.regs.H); break;
				case 0xF5: z80_SET_bit_reg8(BIT_6, &emu.cpu.regs.L); break;
				case 0xF6: z80_SET_bit_reg8(BIT_6, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xF7: z80_SET_bit_reg8(BIT_6, &emu.cpu.regs.A); break;
				case 0xF8: z80_SET_bit_reg8(BIT_7, &emu.cpu.regs.B); break;
				case 0xF9: z80_SET_bit_reg8(BIT_7, &emu.cpu.regs.C); break;
				case 0xFA: z80_SET_bit_reg8(BIT_7, &emu.cpu.regs.D); break;
				case 0xFB: z80_SET_bit_reg8(BIT_7, &emu.cpu.regs.E); break;
				case 0xFC: z80_SET_bit_reg8(BIT_7, &emu.cpu.regs.H); break;
				case 0xFD: z80_SET_bit_reg8(BIT_7, &emu.cpu.regs.L); break;
				case 0xFE: z80_SET_bit_reg8(BIT_7, &hlMemVal); WriteMemory(emu.cpu.regs.HL, hlMemVal); break;
				case 0xFF: z80_SET_bit_reg8(BIT_7, &emu.cpu.regs.A); break;
				default:
				{
					sprintf(&errorText, "Error in 0xCB instruction at %x!  Opcode number %x is invalid.", emu.cpu.regs.PC - 1, cbOpcode);
					OpcodeError(&errorText);
				}
				break;
				}
			}
			break;
			case 0xCC: conditionalCycles = z80_CALL_condition_immediate(0x02); break;
			case 0xCD: z80_CALL_immediate(); break;
			case 0xCE: z80_ADC_A_immediate(); break;
			case 0xCF: z80_RST(0x08); break;
			case 0xD0: conditionalCycles = z80_RET_condition(0x03); break;
			case 0xD1: z80_POP_reg16(&emu.cpu.regs.D, &emu.cpu.regs.E); break;
			case 0xD2: conditionalCycles = z80_JP_condition_immediate(0x03); break;
				//case 0xD3:  Maybe doesn't exist?
			case 0xD4: conditionalCycles = z80_CALL_condition_immediate(0x03); break;
			case 0xD5: z80_PUSH_reg16(&emu.cpu.regs.D, &emu.cpu.regs.E); break;
			case 0xD6: z80_SUB_immediate(); break;
			case 0xD7: z80_RST(0x10); break;
			case 0xD8: conditionalCycles = z80_RET_condition(0x04); break;
			case 0xD9: z80_RET(); IntrpMasterEnable = 1; break;
			case 0xDA: conditionalCycles = z80_JP_condition_immediate(0x04); break;
				//case 0xDB:  Maybe doesn't exist?
			case 0xDC: conditionalCycles = z80_CALL_condition_immediate(0x04); break;
				//case 0xDD:  Maybe doesn't exist?
			case 0xDE: z80_SBC_A_immediate(); break;
			case 0xDF: z80_RST(0x18); break;
			case 0xE0: z80_LD_0xFF00_immediate_A(); break;
			case 0xE1: z80_POP_reg16(&emu.cpu.regs.H, &emu.cpu.regs.L); break;
			case 0xE2: z80_LD_0xFF00_C_A(); break;
				//case 0xE3:  Maybe doesn't exist?
				//case 0xE4:  Maybe doesn't exist?
			case 0xE5: z80_PUSH_reg16(&emu.cpu.regs.H, &emu.cpu.regs.L); break;
			case 0xE6: z80_AND_immediate(); break;
			case 0xE7: z80_RST(0x20); break;
			case 0xE8: z80_ADD_SP_immediate(); break;
			case 0xE9: z80_JP_location_HL(); break;
			case 0xEA: z80_LD_location_immediate_A(); break;
				//case 0xEB:  Maybe doesn't exist?
				//case 0xEC:  Maybe doesn't exist?
				//case 0xED:  Maybe doesn't exist?
			case 0xEE: z80_XOR_immediate(); break;
			case 0xEF: z80_RST(0x28); break;
			case 0xF0: z80_LD_A_0xFF00_immediate(); break;
			case 0xF1: z80_POP_reg16(&emu.cpu.regs.A, &emu.cpu.regs.F); break;
			case 0xF2: z80_LD_A_0xFF00_C(); break;
			case 0xF3: /*z80_DI()*/ IntrpMasterEnable = 0; break;  // z80_DI instruction.  Disable all interrupts.
				//case 0xF4:  Maybe doesn't exist?
			case 0xF5: z80_PUSH_reg16(&emu.cpu.regs.A, &emu.cpu.regs.F); break;
			case 0xF6: z80_OR_immediate(); break;
			case 0xF7: z80_RST(0x30); break;
			case 0xF8: z80_LD_HL_SP_immediate(); break;
			case 0xF9: z80_LD_SP_HL(); break;
			case 0xFA: z80_LD_A_location_immediate(); break;
			case 0xFB: /*z80_EI();*/ emu.state.eiDelay = 1; break;
				//case 0xFC:  Maybe doesn't exist?
				//case 0xFD:  Maybe doesn't exist?
			case 0xFE: z80_CP_immediate(); break;
			case 0xFF: z80_RST(0x38); break;
			default:
			{
				sprintf(&errorText, "Error in instruction at 0x%x!  Opcode 0x%x is invalid.", emu.cpu.regs.PC - 1, previousOpcode);
				OpcodeError(&errorText);
			}
			break;
			}
		}

		// If the opcode was 0xCB, add cycles from the bit operation cycle table.
		if (opcode == 0xCB)
			cyclesRan += emu.cycles.opCBCycles[cbOpcode];
		else
			cyclesRan += emu.cycles.opCycles[opcode] + conditionalCycles;

		// If the last instruction ran was a HALT or a STOP, run cycles until the system resumes.
		if ((opcode == 0x76) || (opcode == 0x10))
			cyclesRan += 4;

		if (IOregister_LCDC & BIT_7)
			emu.cycles.statCycles += cyclesRan;

		emu.cycles.internalCounterCycles += cyclesRan;

		UpdateIORegisters();

		conditionalCycles = 0;  // Reset the conditional cycles added if an instruction's condition was true.
		cyclesRan = 0;  // Reset the total cycles ran.

		HandleSDLEvents();
	}
}

//----------------------------------------//
// This function will check if an         //
// interrupt has occured, and take the    //
// appropriate action.                    //
//----------------------------------------//
int HandleInterrupts()
{
	// V-Blank interrupt.
	if ((IOregister_IE & BIT_0) && (IOregister_IF & BIT_0))
	{
		if (IntrpMasterEnable == 1)
		{
			// This will account for the behavior where the IE flag is overwritten if the stack overflows on it
			// with the high byte push.
			if (emu.cpu.regs.SP == 0x0000)
			{
				emu.cpu.regs.PC = 0x00;
			}
			else
			{
				IntrpMasterEnable = 0;
				z80_PUSH_reg16(&emu.cpu.regs.PCH, &emu.cpu.regs.PCL);
				emu.cpu.regs.PC = 0x40;
				//IOregister_IF &= (BIT_0_OFF & BIT_1_OFF & BIT_2_OFF & BIT_3_OFF & BIT_4_OFF);
				IOregister_IF &= BIT_0_OFF;
				// If an interrupt was called with halt enabled, 24 clocks are spent.  Otherwise, it's 20.
				if (emu.state.halted == 1)
				{
					emu.state.halted = 0;
					return 24;
				}
				return 20;
			}
		}
		else
		{
			if (emu.state.halted == 1)
			{
				emu.state.halted = 0;
				return 4;
			}
		}
	}
	// STAT mode interrupt.
	if ((IOregister_IE & BIT_1) && (IOregister_IF & BIT_1))
	{
		if (IntrpMasterEnable == 1)
		{
			// This will account for the behavior where the IE flag is overwritten if the stack overflows on it
			// with the high byte push.
			if (emu.cpu.regs.SP == 0x0000)
			{
				emu.cpu.regs.PC = 0x00;
			}
			else
			{
				IntrpMasterEnable = 0;
				z80_PUSH_reg16(&emu.cpu.regs.PCH, &emu.cpu.regs.PCL);
				emu.cpu.regs.PC = 0x48;
				//IOregister_IF &= (BIT_0_OFF & BIT_1_OFF & BIT_2_OFF & BIT_3_OFF & BIT_4_OFF);
				IOregister_IF &= BIT_1_OFF;
				if (emu.state.halted == 1)
				{
					emu.state.halted = 0;
					return 24;
				}
				return 20;
			}
		}
		else
		{
			if (emu.state.halted == 1)
			{
				emu.state.halted = 0;
				return 4;
			}
		}
	}
	// Timer interrupt
	if ((IOregister_IE & BIT_2) && (IOregister_IF & BIT_2))
	{
		if (IntrpMasterEnable == 1)
		{
			// This will account for the behavior where the IE flag is overwritten if the stack overflows on it
			// with the high byte push.
			if (emu.cpu.regs.SP == 0x0000)
			{
				emu.cpu.regs.PC = 0x00;
			}
			else
			{
				IntrpMasterEnable = 0;
				z80_PUSH_reg16(&emu.cpu.regs.PCH, &emu.cpu.regs.PCL);
				emu.cpu.regs.PC = 0x50;
				//IOregister_IF &= (BIT_0_OFF & BIT_1_OFF & BIT_2_OFF & BIT_3_OFF & BIT_4_OFF);
				IOregister_IF &= BIT_2_OFF;
				if (emu.state.halted == 1)
				{
					emu.state.halted = 0;
					return 24;
				}
				return 20;
			}
		}
		else
		{
			if (emu.state.halted == 1)
			{
				emu.state.halted = 0;
				return 4;
			}
		}
	}
	// Serial interrupt.
	if ((IOregister_IE & BIT_3) && (IOregister_IF & BIT_3))
	{
		if (IntrpMasterEnable == 1)
		{
			// This will account for the behavior where the IE flag is overwritten if the stack overflows on it
			// with the high byte push.
			if (emu.cpu.regs.SP == 0x0000)
			{
				emu.cpu.regs.PC = 0x00;
			}
			else
			{
				IntrpMasterEnable = 0;
				z80_PUSH_reg16(&emu.cpu.regs.PCH, &emu.cpu.regs.PCL);
				emu.cpu.regs.PC = 0x58;
				//IOregister_IF &= (BIT_0_OFF & BIT_1_OFF & BIT_2_OFF & BIT_3_OFF & BIT_4_OFF);
				IOregister_IF &= BIT_3_OFF;
				if (emu.state.halted == 1)
				{
					emu.state.halted = 0;
					return 24;
				}
				return 20;
			}
		}
		else
		{
			if (emu.state.halted == 1)
			{
				emu.state.halted = 0;
				return 4;
			}
		}
	}
	// Joypad interrupt.
	if ((IOregister_IE & BIT_4) && (IOregister_IF & BIT_4))
	{
		if (IntrpMasterEnable == 1)
		{
			emu.state.stopped = 0;
			// This will account for the behavior where the IE flag is overwritten if the stack overflows on it
			// with the high byte push.
			if (emu.cpu.regs.SP == 0x0000)
			{
				emu.cpu.regs.PC = 0x00;
			}
			else
			{
				IntrpMasterEnable = 0;
				z80_PUSH_reg16(&emu.cpu.regs.PCH, &emu.cpu.regs.PCL);
				emu.cpu.regs.PC = 0x60;
				//IOregister_IF &= (BIT_0_OFF & BIT_1_OFF & BIT_2_OFF & BIT_3_OFF & BIT_4_OFF);
				IOregister_IF &= BIT_4_OFF;
				if (emu.state.halted == 1)
				{
					emu.state.halted = 0;
					return 24;
				}
				return 20;
			}
		}
		else
		{
			if (emu.state.halted == 1)
			{
				emu.state.halted = 0;
				return 4;
			}
		}
	}
	return 0;
}

void UpdateJoypadIORegister()
{
	// If P14 was set low, the D-Pad is selected.
	if ((IOregister_P1 & BIT_4) == 0)
	{
		// If Down was pressed.
		if (joyState[0] == 1)
		{
			IOregister_P1 &= BIT_3_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_3;

		// If Up was pressed.
		if (joyState[1] == 1)
		{
			IOregister_P1 &= BIT_2_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_2;

		// If Left was pressed.
		if (joyState[2] == 1)
		{
			IOregister_P1 &= BIT_1_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_1;

		// If Right was pressed.
		if (joyState[3] == 1)
		{
			IOregister_P1 &= BIT_0_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_0;
	}

	// If P15 was set low, the A, B, Start, and Select buttons are selected.
	else if ((IOregister_P1 & BIT_5) == 0)
	{
		// If Start was pressed.
		if (joyState[4] == 1)
		{
			IOregister_P1 &= BIT_3_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_3;

		// If Select was pressed.
		if (joyState[5] == 1)
		{
			IOregister_P1 &= BIT_2_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_2;

		// If B was pressed.
		if (joyState[6] == 1)
		{
			IOregister_P1 &= BIT_1_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_1;

		// If A was pressed.
		if (joyState[7] == 1)
		{
			IOregister_P1 &= BIT_0_OFF;
			IOregister_IF |= BIT_4;
		}
		else
			IOregister_P1 |= BIT_0;
	}

	// If a button was pressed, generate an interrupt.  This occurs regardless of selected pins.
/*	for (int i = 0; i < 8; i++)
	{
		if (joyState[i] == 1)
		{
			IOregister_IF |= BIT_4;
			break;
		}
	}*/

	// Reset all fired joystick buttons.
	memset(&joyState[0], 0, 8);
}

//----------------------------------------//
// This compares the LY register with the //
// LYC register.                          //
//----------------------------------------//
void CheckLYC()
{
	//----------------------------------------//
	// If the LY Compare at 0xFF45 is equal to//
	// the LY register at 0xFF44, turn on the //
	// coincident flag in the STAT register.  //
	// Otherwise, turn it off.                //
	//----------------------------------------//
	if (IOregister_LY == IOregister_LYC)
	{
		IOregister_STAT |= BIT_2;
		
		if (IOregister_STAT & BIT_6)
			IOregister_IF |= BIT_1;
	}
	else
		IOregister_STAT &= BIT_2_OFF;
}

void UpdateSTATRegister()
{
	static long int totalCycles = 0;
	// Check if it's the V-Blank period.
	if ((IOregister_STAT & (BIT_0 | BIT_1)) == 1)
	{
		if (emu.cycles.statCycles >= GB_CyclesSTAT)
		{
			// See if V-Blank is over.
			if (IOregister_LY == 153)
			{
				IOregister_LY = 0;

				emu.cycles.statCycles -= (GB_CyclesSTAT - 4);
			}
			else if (IOregister_LY == 0)
			{
				IOregister_STAT &= BIT_0_OFF;
				emu.cycles.statCycles -= GB_CyclesSTAT;
				WinYPrevious = IOregister_WY;
			}
			else
			{
				IOregister_LY++;
				// LY register value 153 lasts less time than other values.
				if (IOregister_LY == 153)
				{
					emu.cycles.statCycles -= 4;
				}
				else
				{
					emu.cycles.statCycles -= GB_CyclesSTAT;
				}
			}
			CheckLYC();
		}
	}
	else
	{
		// Check if OAM is being accessed and mode flag 2 should be set.  Mode 2 lasts about 80 clock cycles.
		if ((emu.cycles.statCycles >= 0) && (emu.cycles.statCycles <= 79) && ((IOregister_STAT & (BIT_0 | BIT_1)) != 2))
		{
			IOregister_STAT &= BIT_0_OFF;
			IOregister_STAT |= BIT_1;

			if (IOregister_STAT & BIT_5)
				IOregister_IF |= BIT_1;
		}
		// Check if OAM and video RAM is being accessed and mode flag 3 should be set.  This mode lasts about 172 clock cycles.
		else if ((emu.cycles.statCycles >= 80) && (emu.cycles.statCycles <= 251) && ((IOregister_STAT & (BIT_0 | BIT_1)) != 3))
			IOregister_STAT |= (BIT_0 | BIT_1);
		// Check if it's the H-Blank period and mode flag 0 should be set.  This mode lasts about 204 clock cycles.  STAT register has already been reset for this.
		else if ((emu.cycles.statCycles >= 252) && (emu.cycles.statCycles <= 455) && ((IOregister_STAT & (BIT_0 | BIT_1)) != 0))
		{
			if (emu.io.display.lcdDelay == 0)
				DrawScanline();
	
			IOregister_STAT &= (BIT_0_OFF & BIT_1_OFF);

			// If the STAT H-Blank interrupt is selected, set the LCDC interrupt flag.
			if (IOregister_STAT & BIT_3)
				IOregister_IF |= BIT_1;
		}
		else if (emu.cycles.statCycles >= GB_CyclesSTAT)
		{
			emu.cycles.statCycles -= GB_CyclesSTAT;

			// See if it's the V-Blank period and time to set the mode 1 flag.
			if (IOregister_LY < 144)
			{
				IOregister_LY++;  // Set IOregister_LY to the next scanline.
			}
			if (IOregister_LY == 144)
			{
				emu.io.display.lcdDelay = 0;
				// The first 4 cycles of line 144 are spent in mode 0 on a classic Gameboy.
				// Some ROMs are dependent on this behavior.
				if (emu.cycles.statCycles >= 4)
				{
					UpdateScreen();
					FPS++;
					LimitFPS();

					IOregister_STAT &= BIT_1_OFF;
					IOregister_STAT |= BIT_0;

					// If the STAT V-Blank interrupt is selected, set the LCDC V-Blank interrupt flag.
					if (IOregister_STAT & BIT_4)
						IOregister_IF |= BIT_1;
					// Set the V-Blank interrupt flag.
					IOregister_IF |= BIT_0;
				}
				else
					emu.cycles.statCycles += GB_CyclesSTAT; // Set the mode flag to 1 after delaying at least 4 cycles.
			}
			
			CheckLYC();
		}
	}
}

//----------------------------------------//
// This function will update the I/O      //
// registers.                             //
//----------------------------------------//
void UpdateIORegisters()
{
	boolean timaOverflow = 0;
	/*** NOTE: All cycle counters currently assume classic Gameboy ***/

	UpdateJoypadIORegister();

	// Update the DIV counter(16384 times a second for GB and 16779 for SGB).
	// DIV is equal to the upper 8 bits of the internal counter.
	IOregister_DIV = (unsigned char)((emu.cycles.internalCounterCycles & 0xFF00) >> 8);

	//----------------------------------------//
	// The TIMA register generates an         //
	// interrupt when it exceeds 255.         //
	//----------------------------------------//

	// See if the timer is enabled.
	if (IOregister_TAC & BIT_2)
	{
		emu.cycles.timaIncCounterCycles += (emu.cycles.internalCounterCycles - emu.cycles.previousInternalCounterCycles);
		switch (IOregister_TAC & (BIT_0 | BIT_1))  // If timer is enabled, determine which timer frequency to use.
		{
		case 0:
			while (emu.cycles.timaIncCounterCycles >= GB_CyclesTIMA_4096)
			{
				IOregister_TIMA++;  // If bit 9 of the internal counter has overflowed since the last check, then increment TIMA.
				if (IOregister_TIMA == 0)
					timaOverflow = 1;
				emu.cycles.timaIncCounterCycles -= GB_CyclesTIMA_4096;
			}
		break;
		case 1:
			while (emu.cycles.timaIncCounterCycles >= GB_CyclesTIMA_262144)
			{
				IOregister_TIMA++;  // If bit 9 of the internal counter has overflowed since the last check, then increment TIMA.
				if (IOregister_TIMA == 0)
					timaOverflow = 1;
				emu.cycles.timaIncCounterCycles -= GB_CyclesTIMA_262144;
			}
		break;
		case 2:
			while (emu.cycles.timaIncCounterCycles >= GB_CyclesTIMA_65536)
			{
				IOregister_TIMA++;  // If bit 9 of the internal counter has overflowed since the last check, then increment TIMA.
				if (IOregister_TIMA == 0)
					timaOverflow = 1;
				emu.cycles.timaIncCounterCycles -= GB_CyclesTIMA_65536;
			}
		break;
		case 3:
			while (emu.cycles.timaIncCounterCycles >= GB_CyclesTIMA_16384)
			{
				IOregister_TIMA++;  // If bit 9 of the internal counter has overflowed since the last check, then increment TIMA.
				if (IOregister_TIMA == 0)
					timaOverflow = 1;
				emu.cycles.timaIncCounterCycles -= GB_CyclesTIMA_16384;
			}
		break;
		}

		// Determine if TIMA has overflowed and that it has not already overflowed.
		if (timaOverflow == 1)
		{
			IOregister_IF |= BIT_2;
			//----------------------------------------//
			// This loads TIMA with TMA.              //
			// Not quite sure if this is correct yet. //
			//----------------------------------------//
			IOregister_TIMA = IOregister_TMA;

			timaOverflow = 0;
		}
	}

	emu.cycles.previousInternalCounterCycles = emu.cycles.internalCounterCycles;

	if (IOregister_LCDC & BIT_7)
		UpdateSTATRegister();	// Handle the STAT and LY registers.
}

//---------------------------------------------------------------------------------------//
// This will fill an 8-bit converted tile buffer with tiles from 0x8000 to 0x97FF as     //
// they are written to video RAM.                                                        //
//---------------------------------------------------------------------------------------//
void FillTileBuffer(unsigned short int tileNumber)
{
	unsigned char tileLineData1, tileLineData2, tileX, tileY, color;

	for (tileY = 0; tileY < 8; tileY++)
	{
		tileLineData1 = emu.memory.videoRam[(tileNumber * 16) + (tileY * 2)];
		tileLineData2 = emu.memory.videoRam[(tileNumber * 16) + (tileY * 2) + 1];

		for (tileX = 0; tileX < 8; tileX++)
		{
			color = tileLineData1 & (128 >> tileX) ? 1 : 0;
			color += tileLineData2 & (128 >> tileX) ? 2 : 0;

			emu.io.display.tileBuffer[tileNumber][tileX][tileY] = color;
		}
	}
}

//---------------------------------------------------------------------------------//
// This will convert and sort sprite information to assist in sprite priority.     //
// The sprites will be sorted according to their x coordinates.                    //
//---------------------------------------------------------------------------------//
void SortSprites()
{
	unsigned int sprites;
	unsigned int x;

	for (sprites = 0; sprites < 40; sprites++)
	{
//		emu.io.display.spriteBuffer[sprites][]
		for (x = 0; x < 160; x++)
		{
//			if (emu.memory.sprite[sprites * 4] == x)
		}
	}
}

void DrawScanline()
{
	//----------------------------------------//
	// Display variables.                     //
	//----------------------------------------//
	static unsigned char spriteHeight = 0, spritePalette = 0, windowScanline;
	static unsigned short int plotX = 0, plotY = 0;

	unsigned char bgBuffer[160];
	unsigned char backgroundX = 0, backgroundY = 0, windowX = 0, windowY = 0;
	unsigned char tileX = 0, tileY = 0, color = 0, sprites = 0;
	unsigned short int bgMapData = 0, windowMapData = 0, tileData = 0, tileNumber = 0;

	signed short int spriteX = 0, spriteY = 0, scanlineX;

	// All addresses are relative to memory space 0x8000.

	//----------------------------------------//
	// This decides where to get background   //
	// and window tile patterns from.         //
	//----------------------------------------//
	if (IOregister_LCDC & BIT_4)
		tileData = 0x0000;
	else
		tileData = 0x0800;

	//----------------------------------------//
	// This decides where to get background   //
	// tile numbers to be displayed.          //
	//----------------------------------------//
	if (IOregister_LCDC & BIT_3)
		bgMapData = 0x1C00;
	else
		bgMapData = 0x1800;

	//----------------------------------------//
	// This decides where to get window       //
	// tile numbers to be displayed.          //
	//----------------------------------------//
	if (IOregister_LCDC & BIT_6)
		windowMapData = 0x1C00;
	else
		windowMapData = 0x1800;

	// The window's scanline is tracked separately to account for the window tile map location not increasing
	// when the window is disabled.
	if (IOregister_LY == 0)
	{
		memset(&screenData, 0, 0x5A00);
		windowScanline = 0;
	}

	// Draw a single scanline.
	for (scanlineX = 0; scanlineX < 160; scanlineX++)
	{
		// If the background and window are enabled.
		if (IOregister_LCDC & BIT_0)
		{
			// Add and divide on separate lines to make sure the 8-bit values wrap properly.
			// This is necessary for background scrolling to work properly.
			backgroundX = (IOregister_SCX + scanlineX);
			backgroundX /= 8;
			backgroundY = (IOregister_SCY + IOregister_LY);
			backgroundY /= 8;
			tileX = (IOregister_SCX + scanlineX) & 7;
			tileY = (IOregister_SCY + IOregister_LY) & 7;

			if (tileData == 0x0000)
				tileNumber = emu.memory.videoRam[bgMapData + (backgroundY * 32) + backgroundX];
			else
				tileNumber = 256 + (signed char)emu.memory.videoRam[bgMapData + (backgroundY * 32) + backgroundX];

			color = emu.io.display.tileBuffer[tileNumber][tileX][tileY];
			// Record the color number to be used for the sprite priority.
			bgBuffer[scanlineX] = color;
			
			if (color == 3)
				screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_7) >> 6) + ((IOregister_BGP & BIT_6) >> 6);
			if (color == 2)
				screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_5) >> 4) + ((IOregister_BGP & BIT_4) >> 4);
			if (color == 1)
				screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_3) >> 2) + ((IOregister_BGP & BIT_2) >> 2);
			if (color == 0)
				screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_1)) + ((IOregister_BGP & BIT_0));

			// If the window layer is enabled.
			if (IOregister_LCDC & BIT_5)
			{
				if ((scanlineX >= (IOregister_WX - 7)) && (IOregister_LY >= IOregister_WY))
				{
					windowX = (scanlineX - (IOregister_WX - 7)) / 8;
					windowY = windowScanline / 8;
					tileX = (scanlineX - (IOregister_WX - 7)) & 7;
					tileY = windowScanline & 7;

					if (tileData == 0x0000)
						tileNumber = emu.memory.videoRam[windowMapData + (windowY * 32) + windowX];
					else
						tileNumber = 256 + (signed char)emu.memory.videoRam[windowMapData + (windowY * 32) + windowX];

					color = emu.io.display.tileBuffer[tileNumber][tileX][tileY];
					// Record the color number to be used for the sprite priority.
					bgBuffer[scanlineX] = color;

					if (color == 3)
						screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_7) >> 6) + ((IOregister_BGP & BIT_6) >> 6);
					if (color == 2)
						screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_5) >> 4) + ((IOregister_BGP & BIT_4) >> 4);
					if (color == 1)
						screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_3) >> 2) + ((IOregister_BGP & BIT_2) >> 2);
					if (color == 0)
						screenData[(IOregister_LY * 160) + scanlineX] = ((IOregister_BGP & BIT_1)) + ((IOregister_BGP & BIT_0));
				}
			}
		}
	}
	// If the window was enabled this scanline.
	if ((IOregister_LCDC & BIT_5) && (IOregister_WX <= 166))
		windowScanline++;

	//----------------------------------------//
	// Draw sprites into the screen buffer.   //
	//----------------------------------------//
	if (IOregister_LCDC & BIT_1)
	{
		//----------------------------------------//
		// This decides the height of the tile    //
		// being drawn.                           //
		//----------------------------------------//
		if (IOregister_LCDC & BIT_2)
			spriteHeight = 16;
		else
			spriteHeight = 8;

		for (sprites = 0; sprites < 40; sprites++)
		{
			spriteY = emu.memory.sprite[sprites * 4] - 16;
			// If the current scanline is within the range of the sprite's position.
			if ((IOregister_LY >= spriteY) && ((spriteY + 16) > 0) && (IOregister_LY < (spriteY + spriteHeight)))
			{
				spriteX = emu.memory.sprite[(sprites * 4) + 1] - 8;
				if (emu.memory.sprite[(sprites * 4) + 3] & BIT_4)
					spritePalette = IOregister_OBP1;
				else
					spritePalette = IOregister_OBP0;

				// If the Y-Flip bit is on, invert for the tile data index.
				if (emu.memory.sprite[(sprites * 4) + 3] & BIT_6)
					tileY = (((IOregister_LY - spriteY) & (spriteHeight - 1)) ^ (spriteHeight - 1));
				else
					tileY = (IOregister_LY - spriteY) & (spriteHeight - 1);

				// If the sprite is past the 8th line, then move to the next tile.
				if (tileY >= 8)
				{
					tileNumber = (emu.memory.sprite[(sprites * 4) + 2]) + 1;
					tileY -= 8;
				}
				else
					tileNumber = emu.memory.sprite[(sprites * 4) + 2];

				for (scanlineX = spriteX; scanlineX < (spriteX + 8); scanlineX++)
				{
					if ((scanlineX >= 0) & (scanlineX < 160))
					{
						tileX = (scanlineX - spriteX) & 7;

						// If the X-Flip bit is on, invert for the tile data index.
						if (emu.memory.sprite[(sprites * 4) + 3] & BIT_5)
							tileX ^= 7;

						color = emu.io.display.tileBuffer[tileNumber][tileX][tileY];

						// Check the sprite priority.
						if (emu.memory.sprite[(sprites * 4) + 3] & BIT_7)
						{
							if (bgBuffer[scanlineX] == 0)
							{
								if (color == 3)
									screenData[(IOregister_LY * 160) + scanlineX] = ((spritePalette & BIT_7) >> 6) + ((spritePalette & BIT_6) >> 6);
								if (color == 2)
									screenData[(IOregister_LY * 160) + scanlineX] = ((spritePalette & BIT_5) >> 4) + ((spritePalette & BIT_4) >> 4);
								if (color == 1)
									screenData[(IOregister_LY * 160) + scanlineX] = ((spritePalette & BIT_3) >> 2) + ((spritePalette & BIT_2) >> 2);
							}
						}
						else
						{
							if (color == 3)
								screenData[(IOregister_LY * 160) + scanlineX] = ((spritePalette & BIT_7) >> 6) + ((spritePalette & BIT_6) >> 6);
							if (color == 2)
								screenData[(IOregister_LY * 160) + scanlineX] = ((spritePalette & BIT_5) >> 4) + ((spritePalette & BIT_4) >> 4);
							if (color == 1)
								screenData[(IOregister_LY * 160) + scanlineX] = ((spritePalette & BIT_3) >> 2) + ((spritePalette & BIT_2) >> 2);
						}
					}
				}
			}
		}
	}
}