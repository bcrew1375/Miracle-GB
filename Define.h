//----------------------------------------//
// Define the different types of Gameboys //
//----------------------------------------//
#define SYSTEM_GB  0x00
#define SYSTEM_SGB 0x03
#define SYSTEM_CGB 0x80

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
#define FLAG_Z_OFF  0x7F
#define FLAG_N_OFF  0xBF
#define FLAG_H_OFF  0xDF
#define FLAG_C_OFF  0xEF

//----------------------------------------//
// Define cycle counters.                 //
//----------------------------------------//
#define DIVIDER_INTERVAL           64
#define SOUND_LENGTH_INTERVAL      4096
#define SOUND_SWEEP_INTERVAL       8192
#define VERTICAL_RETRACE_INTERVAL  114

//----------------------------------------//
// Define flags (contents of register F)  //
//----------------------------------------//
#define FLAG_Z ((emu.cpu.regs.AF & 0x80) >> 7)
#define FLAG_N ((emu.cpu.regs.AF & 0x40) >> 6)
#define FLAG_H ((emu.cpu.regs.AF & 0x20) >> 5)
#define FLAG_C ((emu.cpu.regs.AF & 0x10) >> 4)

//----------------------------------------//
// Define the number of clock cycles to   //
// run per 1/60th of a second.            //
//----------------------------------------//
#define GB_CyclesPerFrame 70221

//----------------------------------------//
// Bit definitions.                       //
//----------------------------------------//
#define BIT_0 0x01
#define BIT_1 0x02
#define BIT_2 0x04
#define BIT_3 0x08
#define BIT_4 0x10
#define BIT_5 0x20
#define BIT_6 0x40
#define BIT_7 0x80
#define BIT_8 0x100
#define BIT_9 0x200
#define BIT_10 0x400
#define BIT_11 0x800
#define BIT_12 0x1000
#define BIT_13 0x2000
#define BIT_14 0x4000
#define BIT_15 0x8000

//----------------------------------------//
// Define I/O registers.                  //
//----------------------------------------//
#define IOregister_P1                 (emu.io.regs.P1)
#define IOregister_SB                 (emu.memory.romBank[0xFF01])
#define IOregister_SC                 (emu.memory.romBank[0xFF02])
#define IOregister_DIV                (emu.memory.romBank[0xFF04])
#define IOregister_TIMA               (emu.memory.romBank[0xFF05])
#define IOregister_TMA                (emu.memory.romBank[0xFF06])
#define IOregister_TAC                (emu.memory.romBank[0xFF07])
#define IOregister_IF                 (emu.memory.romBank[0xFF0F])
#define IOregister_NR10               (emu.memory.romBank[0xFF10])
#define IOregister_NR11               (emu.memory.romBank[0xFF11])
#define IOregister_NR12               (emu.memory.romBank[0xFF12])
#define IOregister_NR13               (emu.memory.romBank[0xFF13])
#define IOregister_NR14               (emu.memory.romBank[0xFF14])
#define IOregister_NR21               (emu.memory.romBank[0xFF16])
#define IOregister_NR22               (emu.memory.romBank[0xFF17])
#define IOregister_NR23               (emu.memory.romBank[0xFF18])
#define IOregister_NR24               (emu.memory.romBank[0xFF19])
#define IOregister_NR30               (emu.memory.romBank[0xFF1A])
#define IOregister_NR31               (emu.memory.romBank[0xFF1B])
#define IOregister_NR32               (emu.memory.romBank[0xFF1C])
#define IOregister_NR33               (emu.memory.romBank[0xFF1D])
#define IOregister_NR34               (emu.memory.romBank[0xFF1E])
#define IOregister_NR41               (emu.memory.romBank[0xFF20])
#define IOregister_NR42               (emu.memory.romBank[0xFF21])
#define IOregister_NR43               (emu.memory.romBank[0xFF22])
#define IOregister_NR44               (emu.memory.romBank[0xFF23])
#define IOregister_NR50               (emu.memory.romBank[0xFF24])
#define IOregister_NR51               (emu.memory.romBank[0xFF25])
#define IOregister_NR52               (emu.memory.romBank[0xFF26])
#define IOregister_LCDC               (emu.memory.romBank[0xFF40])
#define IOregister_STAT               (emu.memory.romBank[0xFF41])
#define IOregister_SCY                (emu.memory.romBank[0xFF42])
#define IOregister_SCX                (emu.memory.romBank[0xFF43])
#define IOregister_LY                 (emu.memory.romBank[0xFF44])
#define IOregister_LYC                (emu.memory.romBank[0xFF45])
#define IOregister_DMA                (emu.memory.romBank[0xFF46])
#define IOregister_BGP                (emu.memory.romBank[0xFF47])
#define IOregister_OBP0               (emu.memory.romBank[0xFF48])
#define IOregister_OBP1               (emu.memory.romBank[0xFF49])
#define IOregister_WY                 (emu.memory.romBank[0xFF4A])
#define IOregister_WX                 (emu.memory.romBank[0xFF4B])
#define IOregister_IE                 (emu.memory.romBank[0xFFFF])