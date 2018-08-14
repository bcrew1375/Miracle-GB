//----------------------------------------//
// Define Gameboy registers.              //
//----------------------------------------//
typedef union
{
	struct { unsigned short int AF, BC, DE, HL, SP, PC; };
	struct { unsigned char F, A, C, B, E, D, L, H, SPL, SPH, PCL, PCH; };
} Registers;

extern Registers regs;
