//----------------------------------------//
// Define the different types of Gameboys //
//----------------------------------------//
#define SYSTEM_GB  0x00		// Classic Gameboy
#define SYSTEM_SGB 0x03		// Super Gameboy
#define SYSTEM_CGB 0x80		// Gameboy Color

/* Taken from Mark Rawer's site here: http://marc.rawer.de/Gameboy/index.html
0 - ROM ONLY					12 - ROM + MBC3 + RAM
1 - ROM + MBC1					13 - ROM + MBC3 + RAM + BATT
2 - ROM + MBC1 + RAM			19 - ROM + MBC5
3 - ROM + MBC1 + RAM + BATT		1A - ROM + MBC5 + RAM
5 - ROM + MBC2					1B - ROM + MBC5 + RAM + BATT
6 - ROM + MBC2 + BATTERY		1C - ROM + MBC5 + RUMBLE
8 - ROM + RAM					1D - ROM + MBC5 + RUMBLE + SRAM
9 - ROM + RAM + BATTERY			1E - ROM + MBC5 + RUMBLE + SRAM + BATT
B - ROM + MMM01					1F - Pocket Camera
C - ROM + MMM01 + SRAM			FD - Bandai TAMA5
D - ROM + MMM01 + SRAM + BATT	FE - Hudson HuC - 3
F - ROM + MBC3 + TIMER + BATT	FF - Hudson HuC - 1
10 - ROM + MBC3 + TIMER + RAM + BATT
11 - ROM + MBC3 */

//----------------------------------------//
// Define the different Memory Bank       //
// Controller types                       //
//----------------------------------------//
#define ROM_ONLY				0x00
#define ROM_MBC_1				0x01
#define ROM_MBC_1_RAM			0x02
#define ROM_MBC_1_RAM_BATT		0x03
#define ROM_MBC_2				0x05
#define ROM_MBC_2_BATT			0x06
#define ROM_RAM					0x08
#define ROM_RAM_BATT			0x09
#define ROM_MMM1				0x0B
#define ROM_MMM1_SRAM			0x0C
#define ROM_MMM1_SRAM_BATT		0x0D
#define ROM_MBC3_TIMER_BATT		0x0F
#define ROM_MBC3_TIMER_RAM_BATT	0x10
#define ROM_MBC3				0x11
#define ROM_MBC3_RAM			0x12
#define ROM_MBC3_RAM_BATT		0x13
#define ROM_MBC5				0x19
#define ROM_MBC5_RAM			0x1A
#define ROM_MBC5_RAM_BATT		0x1B
#define ROM_MBC5_RUMB			0x1C
#define ROM_MBC5_RUMB_SRAM		0x1D
#define ROM_MBC5_RUMB_SRAM_BATT	0x1E
#define ROM_POCKET_CAMERA		0x1F
#define ROM_BANDAI_TAMA5		0xFD
#define ROM_HUDSON_HUC_3		0xFE
#define ROM_HUDSON_HUC_1		0xFF

//----------------------------------------//
// Define flag on/off switches.           //
//----------------------------------------//
#define FLAG_Z_ON   0x80
#define FLAG_N_ON   0x40
#define FLAG_H_ON   0x20
#define FLAG_C_ON   0x10
#define FLAG_Z_OFF  0x70
#define FLAG_N_OFF  0xB0
#define FLAG_H_OFF  0xD0
#define FLAG_C_OFF  0xE0

//----------------------------------------//
// Define flags (contents of register F)  //
//----------------------------------------//
#define FLAG_Z ((emu.cpu.regs.F & 0x80) >> 7)
#define FLAG_N ((emu.cpu.regs.F & 0x40) >> 6)
#define FLAG_H ((emu.cpu.regs.F & 0x20) >> 5)
#define FLAG_C ((emu.cpu.regs.F & 0x10) >> 4)

//----------------------------------------//
// Define the number of clock cycles to   //
// run per 1/60th of a second.            //
//----------------------------------------//
#define GB_CyclesPerFrame	70224
#define GB_CyclesSTAT		456
#define GB_CyclesDIV		256
#define GB_CyclesPerSecond	4194304

//----------------------------------------//
// Cycles for TIMA register's different   //
// clock settings.                        //
//----------------------------------------//
#define GB_CyclesTIMA_4096		1024
#define GB_CyclesTIMA_16384		256
#define GB_CyclesTIMA_65536		64
#define GB_CyclesTIMA_262144	16

//----------------------------------------//
// Bit definitions.                       //
//----------------------------------------//
#define BIT_0		0x01
#define BIT_1		0x02
#define BIT_2		0x04
#define BIT_3		0x08
#define BIT_4		0x10
#define BIT_5		0x20
#define BIT_6		0x40
#define BIT_7		0x80
#define BIT_8		0x100
#define BIT_9		0x200
#define BIT_10		0x400
#define BIT_11		0x800
#define BIT_12		0x1000
#define BIT_13		0x2000
#define BIT_14		0x4000
#define BIT_15		0x8000
#define BIT_0_OFF	0xFE
#define BIT_1_OFF	0xFD
#define BIT_2_OFF	0xFB
#define BIT_3_OFF	0xF7
#define BIT_4_OFF	0xEF
#define BIT_5_OFF	0xDF
#define BIT_6_OFF	0xBF
#define BIT_7_OFF	0x7F

//----------------------------------------//
// Define I/O registers.                  //
//----------------------------------------//
#define IOregister_P1                 (emu.memory.ioRegs[0x00])
#define IOregister_SB                 (emu.memory.ioRegs[0x01])
#define IOregister_SC                 (emu.memory.ioRegs[0x02])
#define IOregister_DIV                (emu.memory.ioRegs[0x04])
#define IOregister_TIMA               (emu.memory.ioRegs[0x05])
#define IOregister_TMA                (emu.memory.ioRegs[0x06])
#define IOregister_TAC                (emu.memory.ioRegs[0x07])
#define IOregister_IF				  (emu.memory.ioRegs[0x0F])
#define IOregister_NR10               (emu.memory.ioRegs[0x10])
#define IOregister_NR11               (emu.memory.ioRegs[0x11])
#define IOregister_NR12               (emu.memory.ioRegs[0x12])
#define IOregister_NR13               (emu.memory.ioRegs[0x13])
#define IOregister_NR14               (emu.memory.ioRegs[0x14])
#define IOregister_NR21               (emu.memory.ioRegs[0x16])
#define IOregister_NR22               (emu.memory.ioRegs[0x17])
#define IOregister_NR23               (emu.memory.ioRegs[0x18])
#define IOregister_NR24               (emu.memory.ioRegs[0x19])
#define IOregister_NR30               (emu.memory.ioRegs[0x1A])
#define IOregister_NR31               (emu.memory.ioRegs[0x1B])
#define IOregister_NR32               (emu.memory.ioRegs[0x1C])
#define IOregister_NR33               (emu.memory.ioRegs[0x1D])
#define IOregister_NR34               (emu.memory.ioRegs[0x1E])
#define IOregister_NR41               (emu.memory.ioRegs[0x20])
#define IOregister_NR42               (emu.memory.ioRegs[0x21])
#define IOregister_NR43               (emu.memory.ioRegs[0x22])
#define IOregister_NR44               (emu.memory.ioRegs[0x23])
#define IOregister_NR50               (emu.memory.ioRegs[0x24])
#define IOregister_NR51               (emu.memory.ioRegs[0x25])
#define IOregister_NR52               (emu.memory.ioRegs[0x26])
#define IOregister_LCDC               (emu.memory.ioRegs[0x40])
#define IOregister_STAT               (emu.memory.ioRegs[0x41])
#define IOregister_SCY                (emu.memory.ioRegs[0x42])
#define IOregister_SCX                (emu.memory.ioRegs[0x43])
#define IOregister_LY                 (emu.memory.ioRegs[0x44])
#define IOregister_LYC                (emu.memory.ioRegs[0x45])
#define IOregister_DMA                (emu.memory.ioRegs[0x46])
#define IOregister_BGP                (emu.memory.ioRegs[0x47])
#define IOregister_OBP0               (emu.memory.ioRegs[0x48])
#define IOregister_OBP1               (emu.memory.ioRegs[0x49])
#define IOregister_WY                 (emu.memory.ioRegs[0x4A])
#define IOregister_WX                 (emu.memory.ioRegs[0x4B])
#define IOregister_IE                 (emu.memory.intrpFlags)

#define IntrpMasterEnable			  (emu.state.intrpEnable)