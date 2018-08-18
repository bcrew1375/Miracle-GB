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
#define FLAG_Z ((regs.F & 0x80) >> 7)
#define FLAG_N ((regs.F & 0x40) >> 6)
#define FLAG_H ((regs.F & 0x20) >> 5)
#define FLAG_C ((regs.F & 0x10) >> 4)

//----------------------------------------//
// Define the number of cycles to run per //
// 1/60th of a second.                    //
//----------------------------------------//
#define cyclesToRun (17555)

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

//----------------------------------------//
// Define I/O registers.                  //
//----------------------------------------//
#define IOregister_P1                 (memory[0xFF00])
#define IOregister_SB                 (memory[0xFF01])
#define IOregister_SC                 (memory[0xFF02])
#define IOregister_DIV                (memory[0xFF04])
#define IOregister_TIMA               (memory[0xFF05])
#define IOregister_TMA                (memory[0xFF06])
#define IOregister_TAC                (memory[0xFF07])
#define IOregister_IF                 (memory[0xFF0F])
#define IOregister_NR10               (memory[0xFF10])
#define IOregister_NR11               (memory[0xFF11])
#define IOregister_NR12               (memory[0xFF12])
#define IOregister_NR13               (memory[0xFF13])
#define IOregister_NR14               (memory[0xFF14])
#define IOregister_NR21               (memory[0xFF16])
#define IOregister_NR22               (memory[0xFF17])
#define IOregister_NR23               (memory[0xFF18])
#define IOregister_NR24               (memory[0xFF19])
#define IOregister_NR30               (memory[0xFF1A])
#define IOregister_NR31               (memory[0xFF1B])
#define IOregister_NR32               (memory[0xFF1C])
#define IOregister_NR33               (memory[0xFF1D])
#define IOregister_NR34               (memory[0xFF1E])
#define IOregister_NR41               (memory[0xFF20])
#define IOregister_NR42               (memory[0xFF21])
#define IOregister_NR43               (memory[0xFF22])
#define IOregister_NR44               (memory[0xFF23])
#define IOregister_NR50               (memory[0xFF24])
#define IOregister_NR51               (memory[0xFF25])
#define IOregister_NR52               (memory[0xFF26])
#define IOregister_LCDC               (memory[0xFF40])
#define IOregister_STAT               (memory[0xFF41])
#define IOregister_SCY                (memory[0xFF42])
#define IOregister_SCX                (memory[0xFF43])
#define IOregister_LY                 (memory[0xFF44])
#define IOregister_LYC                (memory[0xFF45])
#define IOregister_DMA                (memory[0xFF46])
#define IOregister_BGP                (memory[0xFF47])
#define IOregister_OBP0               (memory[0xFF48])
#define IOregister_OBP1               (memory[0xFF49])
#define IOregister_WY                 (memory[0xFF4A])
#define IOregister_WX                 (memory[0xFF4B])
#define IOregister_IE                 (memory[0xFFFF])