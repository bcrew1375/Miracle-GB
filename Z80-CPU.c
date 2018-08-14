#include <stdio.h>
#include <conio.h>
#include <sdl.h>
#include <windows.h>

#include "define.h"
#include "Common.h"
//----------------------------------------//
// Sound Library stuff                    //
//----------------------------------------//
//#include "gbsound/audio.h"

//AUDIO aud;
Registers regs;

//----------------------------------------//
// External data                          //
//----------------------------------------//
extern unsigned char *bootBuffer;
extern unsigned char *romBuffer;
extern unsigned char *memory;
extern unsigned char oldScreenData[160 * 144];
extern unsigned char screenData[160 * 144];
extern unsigned char MBCType;
extern unsigned char modeFlag;

extern unsigned int bootRomPresent;
extern unsigned int CPURunning;
extern unsigned int FPSLimit;
extern unsigned int logging;

//----------------------------------------//
// External functions                     //
//----------------------------------------//
extern void CheckSDLEvents();
extern void GetKeys();
extern unsigned int UpdateFPS(Uint32 interval, void *param);
extern void UpdateScreen();
extern void WriteMemory(unsigned short int address, unsigned char data);

signed long int OAMCounter;

// Taken from Visual Boy Advance source.
unsigned short int daaTable[2048] = {
	0x0080,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,
	0x0800,0x0900,0x1020,0x1120,0x1220,0x1320,0x1420,0x1520,
	0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,
	0x1800,0x1900,0x2020,0x2120,0x2220,0x2320,0x2420,0x2520,
	0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,
	0x2800,0x2900,0x3020,0x3120,0x3220,0x3320,0x3420,0x3520,
	0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,
	0x3800,0x3900,0x4020,0x4120,0x4220,0x4320,0x4420,0x4520,
	0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,
	0x4800,0x4900,0x5020,0x5120,0x5220,0x5320,0x5420,0x5520,
	0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,0x5600,0x5700,
	0x5800,0x5900,0x6020,0x6120,0x6220,0x6320,0x6420,0x6520,
	0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600,0x6700,
	0x6800,0x6900,0x7020,0x7120,0x7220,0x7320,0x7420,0x7520,
	0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,0x7600,0x7700,
	0x7800,0x7900,0x8020,0x8120,0x8220,0x8320,0x8420,0x8520,
	0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,0x8600,0x8700,
	0x8800,0x8900,0x9020,0x9120,0x9220,0x9320,0x9420,0x9520,
	0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,0x9600,0x9700,
	0x9800,0x9900,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
	0x0810,0x0910,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
	0x1810,0x1910,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
	0x2810,0x2910,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
	0x3810,0x3910,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
	0x4810,0x4910,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
	0x5810,0x5910,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,0x6610,0x6710,
	0x6810,0x6910,0x7030,0x7130,0x7230,0x7330,0x7430,0x7530,
	0x7010,0x7110,0x7210,0x7310,0x7410,0x7510,0x7610,0x7710,
	0x7810,0x7910,0x8030,0x8130,0x8230,0x8330,0x8430,0x8530,
	0x8010,0x8110,0x8210,0x8310,0x8410,0x8510,0x8610,0x8710,
	0x8810,0x8910,0x9030,0x9130,0x9230,0x9330,0x9430,0x9530,
	0x9010,0x9110,0x9210,0x9310,0x9410,0x9510,0x9610,0x9710,
	0x9810,0x9910,0xA030,0xA130,0xA230,0xA330,0xA430,0xA530,
	0xA010,0xA110,0xA210,0xA310,0xA410,0xA510,0xA610,0xA710,
	0xA810,0xA910,0xB030,0xB130,0xB230,0xB330,0xB430,0xB530,
	0xB010,0xB110,0xB210,0xB310,0xB410,0xB510,0xB610,0xB710,
	0xB810,0xB910,0xC030,0xC130,0xC230,0xC330,0xC430,0xC530,
	0xC010,0xC110,0xC210,0xC310,0xC410,0xC510,0xC610,0xC710,
	0xC810,0xC910,0xD030,0xD130,0xD230,0xD330,0xD430,0xD530,
	0xD010,0xD110,0xD210,0xD310,0xD410,0xD510,0xD610,0xD710,
	0xD810,0xD910,0xE030,0xE130,0xE230,0xE330,0xE430,0xE530,
	0xE010,0xE110,0xE210,0xE310,0xE410,0xE510,0xE610,0xE710,
	0xE810,0xE910,0xF030,0xF130,0xF230,0xF330,0xF430,0xF530,
	0xF010,0xF110,0xF210,0xF310,0xF410,0xF510,0xF610,0xF710,
	0xF810,0xF910,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
	0x0810,0x0910,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
	0x1810,0x1910,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
	0x2810,0x2910,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
	0x3810,0x3910,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
	0x4810,0x4910,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
	0x5810,0x5910,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x0600,0x0700,0x0800,0x0900,0x0A00,0x0B00,0x0C00,0x0D00,
	0x0E00,0x0F00,0x1020,0x1120,0x1220,0x1320,0x1420,0x1520,
	0x1600,0x1700,0x1800,0x1900,0x1A00,0x1B00,0x1C00,0x1D00,
	0x1E00,0x1F00,0x2020,0x2120,0x2220,0x2320,0x2420,0x2520,
	0x2600,0x2700,0x2800,0x2900,0x2A00,0x2B00,0x2C00,0x2D00,
	0x2E00,0x2F00,0x3020,0x3120,0x3220,0x3320,0x3420,0x3520,
	0x3600,0x3700,0x3800,0x3900,0x3A00,0x3B00,0x3C00,0x3D00,
	0x3E00,0x3F00,0x4020,0x4120,0x4220,0x4320,0x4420,0x4520,
	0x4600,0x4700,0x4800,0x4900,0x4A00,0x4B00,0x4C00,0x4D00,
	0x4E00,0x4F00,0x5020,0x5120,0x5220,0x5320,0x5420,0x5520,
	0x5600,0x5700,0x5800,0x5900,0x5A00,0x5B00,0x5C00,0x5D00,
	0x5E00,0x5F00,0x6020,0x6120,0x6220,0x6320,0x6420,0x6520,
	0x6600,0x6700,0x6800,0x6900,0x6A00,0x6B00,0x6C00,0x6D00,
	0x6E00,0x6F00,0x7020,0x7120,0x7220,0x7320,0x7420,0x7520,
	0x7600,0x7700,0x7800,0x7900,0x7A00,0x7B00,0x7C00,0x7D00,
	0x7E00,0x7F00,0x8020,0x8120,0x8220,0x8320,0x8420,0x8520,
	0x8600,0x8700,0x8800,0x8900,0x8A00,0x8B00,0x8C00,0x8D00,
	0x8E00,0x8F00,0x9020,0x9120,0x9220,0x9320,0x9420,0x9520,
	0x9600,0x9700,0x9800,0x9900,0x9A00,0x9B00,0x9C00,0x9D00,
	0x9E00,0x9F00,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
	0x0E10,0x0F10,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
	0x1E10,0x1F10,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
	0x2E10,0x2F10,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
	0x3E10,0x3F10,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
	0x4E10,0x4F10,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
	0x5E10,0x5F10,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x6610,0x6710,0x6810,0x6910,0x6A10,0x6B10,0x6C10,0x6D10,
	0x6E10,0x6F10,0x7030,0x7130,0x7230,0x7330,0x7430,0x7530,
	0x7610,0x7710,0x7810,0x7910,0x7A10,0x7B10,0x7C10,0x7D10,
	0x7E10,0x7F10,0x8030,0x8130,0x8230,0x8330,0x8430,0x8530,
	0x8610,0x8710,0x8810,0x8910,0x8A10,0x8B10,0x8C10,0x8D10,
	0x8E10,0x8F10,0x9030,0x9130,0x9230,0x9330,0x9430,0x9530,
	0x9610,0x9710,0x9810,0x9910,0x9A10,0x9B10,0x9C10,0x9D10,
	0x9E10,0x9F10,0xA030,0xA130,0xA230,0xA330,0xA430,0xA530,
	0xA610,0xA710,0xA810,0xA910,0xAA10,0xAB10,0xAC10,0xAD10,
	0xAE10,0xAF10,0xB030,0xB130,0xB230,0xB330,0xB430,0xB530,
	0xB610,0xB710,0xB810,0xB910,0xBA10,0xBB10,0xBC10,0xBD10,
	0xBE10,0xBF10,0xC030,0xC130,0xC230,0xC330,0xC430,0xC530,
	0xC610,0xC710,0xC810,0xC910,0xCA10,0xCB10,0xCC10,0xCD10,
	0xCE10,0xCF10,0xD030,0xD130,0xD230,0xD330,0xD430,0xD530,
	0xD610,0xD710,0xD810,0xD910,0xDA10,0xDB10,0xDC10,0xDD10,
	0xDE10,0xDF10,0xE030,0xE130,0xE230,0xE330,0xE430,0xE530,
	0xE610,0xE710,0xE810,0xE910,0xEA10,0xEB10,0xEC10,0xED10,
	0xEE10,0xEF10,0xF030,0xF130,0xF230,0xF330,0xF430,0xF530,
	0xF610,0xF710,0xF810,0xF910,0xFA10,0xFB10,0xFC10,0xFD10,
	0xFE10,0xFF10,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
	0x0E10,0x0F10,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
	0x1E10,0x1F10,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
	0x2E10,0x2F10,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
	0x3E10,0x3F10,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
	0x4E10,0x4F10,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
	0x5E10,0x5F10,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x00C0,0x0140,0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,
	0x0840,0x0940,0x0440,0x0540,0x0640,0x0740,0x0840,0x0940,
	0x1040,0x1140,0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,
	0x1840,0x1940,0x1440,0x1540,0x1640,0x1740,0x1840,0x1940,
	0x2040,0x2140,0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,
	0x2840,0x2940,0x2440,0x2540,0x2640,0x2740,0x2840,0x2940,
	0x3040,0x3140,0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,
	0x3840,0x3940,0x3440,0x3540,0x3640,0x3740,0x3840,0x3940,
	0x4040,0x4140,0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,
	0x4840,0x4940,0x4440,0x4540,0x4640,0x4740,0x4840,0x4940,
	0x5040,0x5140,0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,
	0x5840,0x5940,0x5440,0x5540,0x5640,0x5740,0x5840,0x5940,
	0x6040,0x6140,0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,
	0x6840,0x6940,0x6440,0x6540,0x6640,0x6740,0x6840,0x6940,
	0x7040,0x7140,0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,
	0x7840,0x7940,0x7440,0x7540,0x7640,0x7740,0x7840,0x7940,
	0x8040,0x8140,0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,
	0x8840,0x8940,0x8440,0x8540,0x8640,0x8740,0x8840,0x8940,
	0x9040,0x9140,0x9240,0x9340,0x9440,0x9540,0x9640,0x9740,
	0x9840,0x9940,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x4050,0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,
	0x4850,0x4950,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x5050,0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,
	0x5850,0x5950,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x6050,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,
	0x6850,0x6950,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,
	0x7850,0x7950,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x8050,0x8150,0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,
	0x8850,0x8950,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x9050,0x9150,0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,
	0x9850,0x9950,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
	0xA050,0xA150,0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,
	0xA850,0xA950,0xA450,0xA550,0xA650,0xA750,0xA850,0xA950,
	0xB050,0xB150,0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,
	0xB850,0xB950,0xB450,0xB550,0xB650,0xB750,0xB850,0xB950,
	0xC050,0xC150,0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,
	0xC850,0xC950,0xC450,0xC550,0xC650,0xC750,0xC850,0xC950,
	0xD050,0xD150,0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,
	0xD850,0xD950,0xD450,0xD550,0xD650,0xD750,0xD850,0xD950,
	0xE050,0xE150,0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,
	0xE850,0xE950,0xE450,0xE550,0xE650,0xE750,0xE850,0xE950,
	0xF050,0xF150,0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,
	0xF850,0xF950,0xF450,0xF550,0xF650,0xF750,0xF850,0xF950,
	0x00D0,0x0150,0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,
	0x0850,0x0950,0x0450,0x0550,0x0650,0x0750,0x0850,0x0950,
	0x1050,0x1150,0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,
	0x1850,0x1950,0x1450,0x1550,0x1650,0x1750,0x1850,0x1950,
	0x2050,0x2150,0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,
	0x2850,0x2950,0x2450,0x2550,0x2650,0x2750,0x2850,0x2950,
	0x3050,0x3150,0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,
	0x3850,0x3950,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x4050,0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,
	0x4850,0x4950,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x5050,0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,
	0x5850,0x5950,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x6050,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,
	0x6850,0x6950,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,
	0x7850,0x7950,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x8050,0x8150,0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,
	0x8850,0x8950,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x9050,0x9150,0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,
	0x9850,0x9950,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
	0xFA60,0xFB60,0xFC60,0xFD60,0xFE60,0xFF60,0x00C0,0x0140,
	0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,0x0840,0x0940,
	0x0A60,0x0B60,0x0C60,0x0D60,0x0E60,0x0F60,0x1040,0x1140,
	0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,0x1840,0x1940,
	0x1A60,0x1B60,0x1C60,0x1D60,0x1E60,0x1F60,0x2040,0x2140,
	0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,0x2840,0x2940,
	0x2A60,0x2B60,0x2C60,0x2D60,0x2E60,0x2F60,0x3040,0x3140,
	0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,0x3840,0x3940,
	0x3A60,0x3B60,0x3C60,0x3D60,0x3E60,0x3F60,0x4040,0x4140,
	0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,0x4840,0x4940,
	0x4A60,0x4B60,0x4C60,0x4D60,0x4E60,0x4F60,0x5040,0x5140,
	0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,0x5840,0x5940,
	0x5A60,0x5B60,0x5C60,0x5D60,0x5E60,0x5F60,0x6040,0x6140,
	0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,0x6840,0x6940,
	0x6A60,0x6B60,0x6C60,0x6D60,0x6E60,0x6F60,0x7040,0x7140,
	0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,0x7840,0x7940,
	0x7A60,0x7B60,0x7C60,0x7D60,0x7E60,0x7F60,0x8040,0x8140,
	0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,0x8840,0x8940,
	0x8A60,0x8B60,0x8C60,0x8D60,0x8E60,0x8F60,0x9040,0x9140,
	0x9240,0x9340,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x3A70,0x3B70,0x3C70,0x3D70,0x3E70,0x3F70,0x4050,0x4150,
	0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x4A70,0x4B70,0x4C70,0x4D70,0x4E70,0x4F70,0x5050,0x5150,
	0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x5A70,0x5B70,0x5C70,0x5D70,0x5E70,0x5F70,0x6050,0x6150,
	0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x6A70,0x6B70,0x6C70,0x6D70,0x6E70,0x6F70,0x7050,0x7150,
	0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x7A70,0x7B70,0x7C70,0x7D70,0x7E70,0x7F70,0x8050,0x8150,
	0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x8A70,0x8B70,0x8C70,0x8D70,0x8E70,0x8F70,0x9050,0x9150,
	0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
	0x9A70,0x9B70,0x9C70,0x9D70,0x9E70,0x9F70,0xA050,0xA150,
	0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,0xA850,0xA950,
	0xAA70,0xAB70,0xAC70,0xAD70,0xAE70,0xAF70,0xB050,0xB150,
	0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,0xB850,0xB950,
	0xBA70,0xBB70,0xBC70,0xBD70,0xBE70,0xBF70,0xC050,0xC150,
	0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,0xC850,0xC950,
	0xCA70,0xCB70,0xCC70,0xCD70,0xCE70,0xCF70,0xD050,0xD150,
	0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,0xD850,0xD950,
	0xDA70,0xDB70,0xDC70,0xDD70,0xDE70,0xDF70,0xE050,0xE150,
	0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,0xE850,0xE950,
	0xEA70,0xEB70,0xEC70,0xED70,0xEE70,0xEF70,0xF050,0xF150,
	0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,0xF850,0xF950,
	0xFA70,0xFB70,0xFC70,0xFD70,0xFE70,0xFF70,0x00D0,0x0150,
	0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,0x0850,0x0950,
	0x0A70,0x0B70,0x0C70,0x0D70,0x0E70,0x0F70,0x1050,0x1150,
	0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,0x1850,0x1950,
	0x1A70,0x1B70,0x1C70,0x1D70,0x1E70,0x1F70,0x2050,0x2150,
	0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,0x2850,0x2950,
	0x2A70,0x2B70,0x2C70,0x2D70,0x2E70,0x2F70,0x3050,0x3150,
	0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x3A70,0x3B70,0x3C70,0x3D70,0x3E70,0x3F70,0x4050,0x4150,
	0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x4A70,0x4B70,0x4C70,0x4D70,0x4E70,0x4F70,0x5050,0x5150,
	0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x5A70,0x5B70,0x5C70,0x5D70,0x5E70,0x5F70,0x6050,0x6150,
	0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x6A70,0x6B70,0x6C70,0x6D70,0x6E70,0x6F70,0x7050,0x7150,
	0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x7A70,0x7B70,0x7C70,0x7D70,0x7E70,0x7F70,0x8050,0x8150,
	0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x8A70,0x8B70,0x8C70,0x8D70,0x8E70,0x8F70,0x9050,0x9150,
	0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
};

unsigned char cpTable[512] = {
	0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
	0x50,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xC0,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x60,
};

unsigned char zeroTable[256] = {
	0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

signed short int dividerCounter;
signed short int retraceCounter;
signed short int timerCounter;
signed short int LYCounter;

unsigned short int timerInterval;

unsigned char tilePosXWin = 0, tilePosYWin = 0;
unsigned char WYTemp = 0;

//----------------------------------------//
// Gameboy status variables.              //
//----------------------------------------//
unsigned char halted;
unsigned char interruptMasterEnable;
unsigned char stopped;
unsigned char instructionRepeat;

//----------------------------------------//
// Gameboy status arrays.                 //
//----------------------------------------//
unsigned char joyState[8];

//----------------------------------------//
// Display arrays.                        //
//----------------------------------------//
unsigned char bgDataBuffer[160];
unsigned char bgPaletteBuffer[160];
unsigned char winDataBuffer[160];
unsigned char winPaletteBuffer[160];

//----------------------------------------//
// Display variables.                     //
//----------------------------------------//
signed short int tileNumber, sprites;
signed short int x, y, i, j;
signed short int WinX, WinY;

unsigned char data1, data2, color;
unsigned char scrollX, scrollY;
unsigned char borderXBack, borderYBack;
unsigned char borderXWin, borderYWin;
unsigned char SpriteHeight;

unsigned char OAMTransferEnabled;

unsigned short int spritePalette;
unsigned short int plotX, plotY;
unsigned short int tilePosXBack = 0, tilePosYBack = 0;
unsigned short int WindowMapData;
unsigned short int BGTileData;
unsigned short int BGMapData;

//----------------------------------------//
// Miscellaneous variables                //
//----------------------------------------//
signed char interruptLatencyCounter;

unsigned char cycles;
unsigned char opcode;
unsigned char spriteHeight;

unsigned int FPS = 0;
unsigned int SpeedKey = 0;
unsigned int LCDCInterruptExecuted = 0;
unsigned int previousPCvalue = 0;

unsigned int cycleTotal = 0;

unsigned long int emulationTimer;

SDL_TimerID FPSTimerID;

//----------------------------------------//
// Miscellaneous arrays                   //
//----------------------------------------//
char errorText[256];

//----------------------------------------//
// Define all functions used for the Z80  //
// emulation.                             //
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
void z80_CALL_condition_immediate(unsigned char condition);
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
void z80_JP_condition_immediate(unsigned char condition);
void z80_JP_immediate();
void z80_JR_condition_offset(unsigned char condition);
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
void z80_LD_reg16_immediate(unsigned char *reg1, unsigned char *reg2);
void z80_LD_location_reg16_A(unsigned short int *reg);
void z80_LD_reg8_immediate(unsigned char *reg);
void z80_LD_reg8_location_HL(unsigned char *reg);
void z80_LD_reg8_reg8(unsigned char *reg1, unsigned char *reg2);
void z80_LD_SP_HL();
void z80_LD_0xFF00_C_A();
void z80_LD_0xFF00_immediate_A();
void z80_NOP();
void z80_OR_immediate();
void z80_OR_reg8(unsigned char *reg);
void z80_POP_reg16(unsigned char *reg1, unsigned char *reg2);
void z80_PUSH_reg16(unsigned char *reg1, unsigned char *reg2);
void z80_RES_bit_reg8(unsigned char bit, unsigned char *reg);
void z80_RET();
void z80_RETI();
void z80_RET_condition(unsigned char condition);
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
void CPUReset();
void Draw8PixelLine();
void DrawScanline();
void FillBackgroundBuffer();
void HandleInterrupts();
void RunEmulation();
void UpdateIORegisters();
void UpdateJoypadIORegister();
void UpdateStatusIORegister();
void UpdateLCD();
void UpdateLCDRegisters();
void WriteMemory(unsigned short int address, unsigned char data);
void WriteToLog();
void ZeroGBMemory();

//----------------------------------------//
// File handle for log file.              //
//----------------------------------------//
FILE *handle;

//----------------------------------------//
// This function will return the CPU to   //
// its start-up state.                    //
//----------------------------------------//
void CPUReset()
{
	handle = fopen("log.txt", "w");

	//----------------------------------------//
	// Set the 16-bit user register startup   //
	// values.                                //
	//----------------------------------------//
	regs.AF = 0x01B0;
	regs.BC = 0x0013;
	regs.DE = 0x00D8;
	regs.HL = 0x014D;

	//----------------------------------------//
	// Set the 16-bit pointer register startup//
	// values.                                //
	//----------------------------------------//
	regs.SP = 0xFFFE;

	//----------------------------------------//
	// Reset the cycle timing counters.       //
	//----------------------------------------//
	retraceCounter = 20;

	dividerCounter = DIVIDER_INTERVAL;

	timerCounter = 0;

	interruptLatencyCounter = 0;

	//----------------------------------------//
	// Reset the cycle counter.               //
	//----------------------------------------//
	cycles = 0;

	//----------------------------------------//
	// Make sure the system isn't halted or   //
	// stopped.                               //
	//----------------------------------------//
	halted = 0;
	instructionRepeat = 0;
	stopped = 0;

	modeFlag = 2;

	//----------------------------------------//
	// Do other stuff.                        //
	//----------------------------------------//
	OAMTransferEnabled = 0;

	//----------------------------------------//
	// Disable interrupts.                    //
	//----------------------------------------//
	interruptMasterEnable = 0;

	//----------------------------------------//
	// Load the base ROM (32K).               //
	//----------------------------------------//
	//printf("%d", romBuffer[0x101]);
	//getch();

	memcpy(&memory[0x0000], &romBuffer[0x0000], 0x8000);
	
	// See if boot ROM is present.  PC is set based on boot ROM's presence.
	/*if (bootRomPresent == 1)
	{
		memcpy(&memory[0x0000], &bootBuffer[0x0000], 0x100);
		
		regs.PC = 0x0000;
	}
	else*/
	{	
		regs.PC = 0x0100;
	}

	switch(memory[0x0147])
	{
	case 0x01:
	case 0x02:
	case 0x03:
		MBCType = 1;
	break;
	break;
	break;
	case 0x05:
	case 0x06:
		MBCType = 2;
	break;
	break;
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		MBCType = 3;
	break;
	break;
	break;
	break;
	break;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		MBCType = 5;
	break;
	break;
	break;
	break;
	break;
	break;
	}

	//----------------------------------------//
	// Set other startup values.  (Still not  //
	// sure if this is actually needed.)      //
	//----------------------------------------//
	IOregister_P1               = 0xFF;
	IOregister_SB               = 0x00;
	IOregister_SC               = 0x7E;
//	memory[0xFF03]              = 0xFF;
	IOregister_DIV              = 0xAD;
	IOregister_TIMA             = 0x00;
	IOregister_TMA              = 0x00;
	IOregister_TAC              = 0xF8;
//	memory[0xFF08]              = 0xFF; memory[0xFF09] = 0xFF; memory[0xFF0A] = 0xFF; memory[0xFF0B] = 0xFF; memory[0xFF0C] = 0xFF; memory[0xFF0D] = 0xFF; memory[0xFF0E] = 0xFF;
	IOregister_IF               = 0xE1;
	IOregister_NR10             = 0x80;
	IOregister_NR11             = 0xBF;
	IOregister_NR12             = 0xF0;
	IOregister_NR13             = 0xFF;
	IOregister_NR14             = 0xBF;
//	memory[0xFF15]              = 0xFF;
	IOregister_NR21             = 0x3F;
	IOregister_NR22             = 0x00;
	IOregister_NR23             = 0xFF;
	IOregister_NR24             = 0xBF;
	IOregister_NR30             = 0x7F;
    IOregister_NR31             = 0xFF;
	IOregister_NR32             = 0x9F;
	IOregister_NR33             = 0xFF;
	IOregister_NR34             = 0xBF;
//	memory[0xFF1F]              = 0xFF;
	IOregister_NR41             = 0xFF;
	IOregister_NR42             = 0x00;
	IOregister_NR43             = 0x00;
	IOregister_NR44             = 0xBF;
	IOregister_NR50             = 0x77;
	IOregister_NR51             = 0xF3;
	IOregister_NR52             = 0xF1;
//	memory[0xFF27]              = 0x00; memory[0xFF28] = 0x00; memory[0xFF29] = 0x00; memory[0xFF2A] = 0x00; memory[0xFF2B] = 0x00; memory[0xFF2C] = 0x00; memory[0xFF2D] = 0x00; memory[0xFF2E] = 0x00; memory[0xFF2F] = 0x00;
	memory[0xFF30]              = 0x06; memory[0xFF31] = 0xFE; memory[0xFF32] = 0x0E; memory[0xFF33] = 0x7F; memory[0xFF34] = 0x00; memory[0xFF35] = 0xFF; memory[0xFF36] = 0x58; memory[0xFF37] = 0xDF;
	memory[0xFF38]              = 0x00; memory[0xFF39] = 0xEC; memory[0xFF3A] = 0x00; memory[0xFF3B] = 0xBF; memory[0xFF3C] = 0x0C; memory[0xFF3D] = 0xED; memory[0xFF3E] = 0x03; memory[0xFF3F] = 0xF7;
	IOregister_LCDC             = 0x91;
	IOregister_STAT             = 0x80;
	IOregister_SCY              = 0x00;
	IOregister_SCX              = 0x00;
	IOregister_LY               = 0x90;
	IOregister_LYC              = 0x00;
	IOregister_DMA              = 0x00;
	IOregister_BGP              = 0xFC;
	IOregister_OBP0             = 0xFF;
	IOregister_OBP1             = 0xFF;
	IOregister_WY               = 0x00;
	IOregister_WX               = 0x07;
/*	memory[0xFF4C]              = 0xFF;
	memory[0xFF4D]              = 0x7E;
	memory[0xFF4E]              = 0xFF;
	memory[0xFF4F]              = 0xFE;
	memory[0xFF50]              = 0xFF;
	memory[0xFF51]              = 0x00;
	memory[0xFF52]              = 0x00;
	memory[0xFF53]              = 0x00;
	memory[0xFF54]              = 0x00;
	memory[0xFF55]              = 0xFF;
	memory[0xFF56]              = 0x00;
	memory[0xFF68]              = 0xC0;
	memory[0xFF69]              = 0xFF;
	memory[0xFF6A]              = 0xC1;
	memory[0xFF6B]              = 0x20;
	memory[0xFF70]              = 0xF8;*/
	IOregister_IE               = 0x00;

	//aud.Initialize();
}

//----------------------------------------//
// This instruction will add a given value//
// plus the carry flag to register A.     //
//----------------------------------------//
void z80_ADC_A_immediate()
{
	// If there will be a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((regs.A & 0xF) + (memory[regs.PC] & 0xF) + FLAG_C) > 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((regs.A + memory[regs.PC] + FLAG_C) > 0xFF)
	{
		regs.A += memory[regs.PC] + FLAG_C;
		regs.F |= FLAG_C_ON;
	}
	else
	{
		regs.A += memory[regs.PC] + FLAG_C;
		regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Flag N is reset.
	regs.F &= FLAG_N_OFF;
	
	// Increment Program Counter.
	regs.PC++;

	// Instruction uses 2 machine cycles.
	cycles += 2;
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
	if (((regs.A & 0xF) + (*reg & 0xF) + FLAG_C) > 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((regs.A + *reg + FLAG_C) > 0xFF)
	{
		regs.A += *reg + FLAG_C;
		regs.F |= FLAG_C_ON;
	}
	else
	{
		regs.A += *reg + FLAG_C;
		regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Flag N is reset.
	regs.F &= FLAG_N_OFF;

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x8E)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will add a given value//
// to register A.                         //
//----------------------------------------//
void z80_ADD_A_immediate()
{
	// If there will be a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((regs.A & 0xF) + (memory[regs.PC] & 0xF)) > 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((regs.A + memory[regs.PC]) > 0xFF)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;
	
	// Add *reg to register A.
	regs.A += memory[regs.PC];

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Flag N is reset.
	regs.F &= FLAG_N_OFF;

	regs.PC++;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will add an 8-bit     //
// register's value to register A.        //
//----------------------------------------//
void z80_ADD_A_reg8(unsigned char *reg)
{
	// If there will be a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((regs.A & 0xF) + (*reg & 0xF)) > 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 7, set flag C, otherwise
	// reset it.
	if ((regs.A + *reg) > 0xFF)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;
	
	// Add *reg to register A.
	regs.A += *reg;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Flag N is reset.
	regs.F &= FLAG_N_OFF;

	// Machine cycles used is dependent on opcode
	if (opcode == 0x86)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will add the value of //
// a 16-bit register to register HL.      //
//----------------------------------------//
void z80_ADD_HL_reg16(unsigned short int *reg)
{
	// If there will be a carry from bit 3, set flag H, otherwise
	// reset it.
	if (((regs.H & 0xF) <= 0xF) && (((regs.H + *reg) & 0x10) >= 0x10))
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// If there will be a carry from bit 15, set flag C, otherwise
	// reset it.
	if ((regs.HL + *reg) > 0xFFFF)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;
	
	// Add *reg to HL.
	regs.HL += *reg;

	// Flag N is reset.
	regs.F &= FLAG_N_OFF;

	// Instruction uses 2 machine cycles
	cycles += 2;
}

//----------------------------------------//
// This instruction will add a given,     //
// signed 8-bit value to the              //
// Stack pointer.                         //
//----------------------------------------//
void z80_ADD_SP_immediate()
{
	// Turn on flag C if the result is above 0xFFFF, or below 0x0000.
	if (((regs.SP + (signed char)memory[regs.PC]) > 0xFFFF) || ((regs.SP + (signed char)memory[regs.PC]) < 0x0000))
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	regs.SP += (signed char)memory[regs.PC];

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.SP];

	// Flag N is reset.
	regs.F &= FLAG_N_OFF;

	// Increment Program Counter.
	regs.PC++;

	// Instruction uses 4 machine cycles.
	cycles += 4;
}

//----------------------------------------//
// This instruction will logical AND a    //
// given value with register A.           //
//----------------------------------------//
void z80_AND_immediate()
{
	// Logically AND register A with an immediate value.
	regs.A &= memory[regs.PC];

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Increment Program Counter.
	regs.PC++;
	
	// Flag_H is set.
	regs.F |= FLAG_H_ON;

	// Flags N and C are reset.
	regs.F &= (FLAG_Z_ON | FLAG_H_ON);

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will logical AND a    //
// given value with an 8-bit register.    //
//----------------------------------------//
void z80_AND_reg8(unsigned char *reg)
{
	// Logically AND register A with *reg.
	regs.A &= *reg;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Flag_H is set.
	regs.F |= FLAG_H_ON;

	// Flags N and C are reset.
	regs.F &= (FLAG_Z_ON | FLAG_H_ON);

	// Machine cycles used is dependant on opcode
	if (opcode == 0xA6)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will test a bit of an //
// 8-bit register.                        //
//----------------------------------------//
void z80_BIT_bit_reg8(unsigned char bit, unsigned char *reg)
{
	// Test the bit.  If it is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg & bit];

	// Flag N is reset.
	regs.F &= FLAG_N_OFF;

	// Flag H is set.
	regs.F |= FLAG_H_ON;

	// Flag C might need adjusted.

	// Machine cycles used is dependant on opcode.
	if (((opcode & 0x06) == 0x06) || ((opcode & 0x0E) == 0x0E))
		cycles += 3;
	else
		cycles += 2;
}

//----------------------------------------//
// This instruction will push the current //
// value of the Program Counter onto the  //
// stack and jump to a new address, but   //
// only if the given condition is met.    //
//----------------------------------------//
void z80_CALL_condition_immediate(unsigned char condition)
{
	unsigned char conditionTrue = 0;

	switch (condition)
	{
	case 0x01:
		{
			if (!FLAG_Z)
			{
				conditionTrue = 1;
			}
		}
	break;
	case 0x02:
		{
			if (FLAG_Z)
			{
				conditionTrue = 1;
			}
		}
	break;
	case 0x03:
		{
			if (!FLAG_C)
			{
				conditionTrue = 1;
			}
		}
	break;
	case 0x04:
		{
			if (FLAG_C)
			{
				conditionTrue = 1;
			}
		}
	break;
	}

	// Skip over the 16-bit address.
	regs.PC += 2;

	if(conditionTrue == 1)
	{
		// Store the address of the next instruction on the stack.
		regs.SP--;
		WriteMemory(regs.SP, regs.PCH);
		regs.SP--;
		WriteMemory(regs.SP, regs.PCL);

		// Reset PC to it's original value to get the jump address.
		regs.PC -= 2;
		
		regs.PC = (memory[regs.PC + 1] << 8) + memory[regs.PC];

		cycles += 3;
	}

	cycles += 3;
}

//----------------------------------------//
// This instruction will push the current //
// value of the Program Counter onto the  //
// stack and jump to a new address.       //
//----------------------------------------//
void z80_CALL_immediate()
{
	// Skip over the 16-bit address.
	regs.PC += 2;

	// Push address of next instruction onto the stack.
	regs.SP--;
	WriteMemory(regs.SP, regs.PCH);
	regs.SP--;
	WriteMemory(regs.SP, regs.PCL);

	// Reset PC to it's original value to get the jump address.
	regs.PC -= 2;

	// Load new address into Program Counter.
	regs.PC = (memory[regs.PC + 1] << 8) + memory[regs.PC];

	// Instruction uses 6 cycles.
	cycles += 6;
}

//----------------------------------------//
// This instruction will complement(flip) //
// the carry flag.                        //
//----------------------------------------//
void z80_CCF()
{
	// Complement flag C.
	regs.F ^= 0x10;

	// Flags N and H are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will compare a given  //
// value with register A.                 //
//----------------------------------------//
void z80_CP_immediate()
{
	// Adding 255 makes sure the index is never below 0.
	regs.F = cpTable[(regs.A - memory[regs.PC]) + 255];

	// Increment Program Counter.
	regs.PC++;

	// Machine cycles used is dependant on opcode.
	cycles += 2;
}

//----------------------------------------//
// This instruction will compare an 8-bit //
// register with register A.              //
//----------------------------------------//
void z80_CP_reg8(unsigned char *reg)
{
	// Adding 255 makes sure the index is never below 0.
	regs.F = cpTable[(regs.A - *reg) + 255];	

	// Machine cycles used is dependant on opcode.
	if (opcode == 0xBE)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will flip all of      //
// register A's bits.                     //
//----------------------------------------//
void z80_CPL()
{
	// Exclusive OR register A with 0xFF, this will flip the bits.
	regs.A ^= 0xFF;

	// Flags N and H are set
	regs.F |= (FLAG_N_ON | FLAG_H_ON);

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will convert register //
// A to its packed-BCD representation.    //
//----------------------------------------//
void z80_DAA()
{
	unsigned short int lookupIndex = regs.A;

	if (FLAG_C)
		lookupIndex |= 256;
	if (FLAG_H)
		lookupIndex |= 512;
	if (FLAG_N)
		lookupIndex |= 1024;

	regs.AF = daaTable[lookupIndex];

	//----------------------------------------//
	// This will no longer be used, as a      //
	// lookup table is faster.  This is left  //
	// for those that are wondering about the //
	// the mathmatics.                        //
	//----------------------------------------//

	/*unsigned char correctionValue = 0x00;
	
	if (FLAG_N) // If Negative Flag Set 
	{
		if ((regs.A & 0x0F) > 0x09 || FLAG_H) 
		{
			regs.A -=0x06; //Half borrow: (0-1) = (0xF-0x6) = 9 
			if ((regs.A & 0xF0) == 0xF0)
				regs.F |= FLAG_C_ON;
			else
				regs.F &= FLAG_C_OFF;
		}
		
		if ((regs. A & 0xF0) > 0x90 || FLAG_C)
			regs.A -= 0x60;
	}
	else
	{
		if ((regs.A & 0x0F) > 0x09 || FLAG_H)
		{
			regs.A += 0x06; //Half carry: (9+1) = (0xA+0x6) = 10
			if ((regs.A & 0xF0) == 0)
				regs.F |= FLAG_C_ON;
			else
				regs.F &= FLAG_C_OFF; 
        }
		
		if ((regs.A & 0xF0) > 0x90 || FLAG_C)
			regs.A += 0x60;
	}

    if(regs.A == 0)
		regs.F |= FLAG_Z_ON;
	else
		regs.F &= FLAG_Z_OFF;
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];*/

	cycles += 1;
}

//----------------------------------------//
// This instruction will decrease the     //
// value at memory location (HL) by 1.    //
//----------------------------------------//
void z80_DEC_location_HL()
{	
	WriteMemory(regs.HL, memory[regs.HL] - 1);

	if ((memory[regs.HL] & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[memory[regs.HL]];

	// Instruction turns on flag N.
	regs.F |= FLAG_N_ON;
	
	// Instruction uses 3 machine cycles.
	cycles += 3;
}

//----------------------------------------//
// This instruction decreases a given     //
// 16-bit register's value by 1.          //
//----------------------------------------//
void z80_DEC_reg16(unsigned short int *reg)
{
	// Decrement *reg.
	*reg -= 1;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction decreases a given     //
// 8-bit register's value by 1.                 //
//----------------------------------------//
void z80_DEC_reg8(unsigned char *reg)
{
	// Decrement *reg
	*reg -= 1;

	if ((*reg & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];
	
	// Turn on flag N.
	regs.F |= FLAG_N_ON;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction turns off the         //
// Interrupt Master Enable flag.          //
//----------------------------------------//
void z80_DI()
{
	// Disable interrupts.
	interruptMasterEnable = 0;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction turns on the          //
// Interrupt Master Enable flag.          //
//----------------------------------------//
void z80_EI()
{
	// Enable interrupts.
	//interruptMasterEnable = 1;
	interruptLatencyCounter = 2;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will halt the GB CPU  //
// until an interrupt occurs.             //
//----------------------------------------//
void z80_HALT()
{
	// Make sure interrupts are enabled before
	// halting the system.
	if (interruptMasterEnable == 1)
	{
		halted = 1;
		if ((IOregister_IF & IOregister_IE) > 0)
		{
			regs.PC--;
		}
		else
		{
			halted = 0;
		}
	}

	if ((interruptMasterEnable == 0) && (IOregister_IF > 0) && (IOregister_IE > 0))
		instructionRepeat = 1;

	// If interrupts are disabled, this should suspend
	// the Program Counter for one instruction.

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will increase the     //
// value at memory location (HL) by 1.    //
//----------------------------------------//
void z80_INC_location_HL()
{
	// See if there will be a carry from bit 3, if there was, set flag H.
	if ((memory[regs.HL] & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// Add 1 to the value at the address in register HL.
	WriteMemory(regs.HL, memory[regs.HL] + 1);

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[memory[regs.HL]];
	
	// Turn off flag N.
	regs.F &= FLAG_N_OFF;

	// Instruction uses 3 machine cycles.
	cycles += 3;
}

//----------------------------------------//
// This instruction increases a given     //
// 16-bit register's value by 1.          //
//----------------------------------------//
void z80_INC_reg16(unsigned short int *reg)
{
	*reg += 1;
	
	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction increases a given     //
// 8-bit register's value by 1.           //
//----------------------------------------//
void z80_INC_reg8(unsigned char *reg)
{
	if ((*reg & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// Add 1 to *reg.
	*reg += 1;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];
	
	// Turn off flag N.
	regs.F &= FLAG_N_OFF;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will relocate the     //
// Program Counter(PC) to a given address,//
// but only if the given condition is met.//
//----------------------------------------//
void z80_JP_condition_immediate(unsigned char condition)
{
	switch (condition)
	{
	// If the Z flag is off, jump to the address.
	case 0x01:
		{
			if (!FLAG_Z)
			{
				regs.PC = (memory[regs.PC + 1] << 8) + memory[regs.PC];
				cycles += 1;
			}
			else
				regs.PC += 2;
		}
	break;
	// If the Z flag is on, jump to the address.
	case 0x02:
		{
			if (FLAG_Z)
			{
				regs.PC = (memory[regs.PC + 1] << 8) + memory[regs.PC];
				cycles += 1;
			}
			else
				regs.PC += 2;
		}
	break;
	// If the C flag is off, jump to the address.
	case 0x03:
		{
			if (!FLAG_C)
			{
				regs.PC = (memory[regs.PC + 1] << 8) + memory[regs.PC];
				cycles += 1;
			}
			else
				regs.PC += 2;
		}
	break;
	// If the C flag is on, jump to the address.
	case 0x04:
		{
			if (FLAG_C)
			{
				regs.PC = (memory[regs.PC + 1] << 8) + memory[regs.PC];
				cycles += 1;
			}
			else
				regs.PC += 2;
		}
	break;
	}

	cycles += 3;
}

//----------------------------------------//
// This instruction will relocate the    //
// Program Counter(PC) to a given address.//
//----------------------------------------//
void z80_JP_immediate()
{
	if (opcode == 0xE9)  // JP HL
	{
		regs.PC = regs.HL;
		
		cycles += 1;
	}
	else  // JP, nnnn
	{
		regs.PC = (memory[regs.PC + 1] << 8) + memory[regs.PC];

		cycles += 4;
	}
}

//----------------------------------------//
// This instruction will add a signed     //
// 8-bit offset to the Program Counter    //
// only if the given condition is met.    //
//----------------------------------------//
void z80_JR_condition_offset(unsigned char condition)
{
	// condition decides the jump condition to look for.
	switch(condition)
	{
	case 0x01:			//JR NZ, offset
		{
			if (!FLAG_Z)
			{
				regs.PC += (signed char)memory[regs.PC];
				cycles += 1;
			}
		}
	break;
	case 0x02:			//JR Z, offset
		{
			if (FLAG_Z)
			{
				regs.PC += (signed char)memory[regs.PC];
				cycles += 1;
			}
		}
	break;
	case 0x03:			//JR NC, offset
		{
			if (!FLAG_C)
			{
				regs.PC += (signed char)memory[regs.PC];
				cycles += 1;
			}
		}
	break;
	case 0x04:			//JR C, offset
		{
			if (FLAG_C)
			{
				regs.PC += (signed char)memory[regs.PC];
				cycles += 1;
			}
		}
	break;
	}

	// Increment the Program Counter.
	regs.PC++;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will add a signed     //
// 8-bit offset to the Program Counter.   //
//----------------------------------------//
void z80_JR_offset()
{
	// Disposition Program Counter.
	regs.PC += (signed char)memory[regs.PC];

	// Increment Program Counter.
	regs.PC++;

	// Instruction uses 3 machine cycles.
	cycles += 3;
}

//----------------------------------------//
// This instruction will load the value at//
// a given memory location into A.        //
//----------------------------------------//
void z80_LD_A_location_immediate()
{
	// Load data at location into register A.
	regs.A = memory[(memory[regs.PC + 1] << 8) + memory[regs.PC]];
	// Skip over the 16-bit address.
	regs.PC += 2;

	// Instruction uses 4 machine cycles.
	cycles += 4;
}

//----------------------------------------//
// This instruction loads the value at    //
// memory location 0xFF00 plus register C //
// into register A.                       //
//----------------------------------------//
void z80_LD_A_0xFF00_C()
{
	// Load the value at 0xFF00 + register C into register A.
	regs.A = memory[0xFF00 + regs.C];

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will load the value   //
// at memory location 0xFF00 plus an 8-bit//
// value into register A.                 //
//----------------------------------------//
void z80_LD_A_0xFF00_immediate()
{
	// Load the value at 0xFF00 + immediate into register A.
	regs.A = memory[0xFF00 + memory[regs.PC]];

	// Increment Program Counter.
	regs.PC++;

	// Flags might need edited.

	// Instruction uses 4 machine cycles.
	cycles += 4;
}

//----------------------------------------//
// This instruction will load the value in//
// memory at (reg16) into register A.     //
//----------------------------------------//
void z80_LD_A_location_reg16(unsigned short int *reg)
{
	// Load A with the value at the address in *reg.
	regs.A = memory[*reg];

	// If the register was HL, increment it.
	if (opcode == 0x2A)
		regs.HL++;
	else if (opcode == 0x3A)
		regs.HL--;
	
	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// Load the Stack Pointer plus a given,   //
// signed 8-bit value into memory         //
// location (HL).                         //
//----------------------------------------//
void z80_LD_HL_SP_immediate()
{
	// Load the Stack Pointer + value into register HL.
	regs.HL = regs.SP + (signed char)memory[regs.PC];

	// Flags H and C need to be adjusted.

	// Reset flags Z and N.
	regs.F &= (FLAG_H_ON | FLAG_C_ON);

	// Increment Program Counter.
	regs.PC++;

	// Instruction uses 3 cycles.
	cycles += 3;
}

//----------------------------------------//
// This instruction will load a given     //
// value into memory location (HL).       //
//----------------------------------------//
void z80_LD_location_HL_immediate()
{
	// Load the address in HL with the immediate value
	WriteMemory(regs.HL, memory[regs.PC]);

	// Increment the Program Counter to skip over the 8-bit value.
	regs.PC++;

	// Instruction uses 3 machine cycles
	cycles += 3;
}

//----------------------------------------//
// This instruction will load the value of//
// an 8-bit register into memory location //
// (HL).                                  //
//----------------------------------------//
void z80_LD_location_HL_reg8(unsigned char *reg)
{
	// Load *reg into the address in register HL
	WriteMemory(regs.HL, *reg);

	// Instruction uses 2 cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will load the value of//
// register A into memory at a given      //
// location.                              //
//----------------------------------------//
void z80_LD_location_immediate_A()
{
	// Load register A into the location.
	WriteMemory(((memory[regs.PC + 1]) << 8) + memory[regs.PC], regs.A);

	// Skip over the 16-bit address.
	regs.PC += 2;
	
	// Instruction uses 4 machine cycles.
	cycles += 4;
}

//----------------------------------------//
// This will load the Stack Pointer into a//
// given memory location.                 //
//----------------------------------------//
void z80_LD_location_SP()
{
	WriteMemory((memory[regs.PC + 1] << 8) + memory[regs.PC], regs.SPL);
	WriteMemory((memory[regs.PC + 1] << 8) + memory[regs.PC] + 1, regs.SPH);

	// Skip over the 16-bit address.
	regs.PC += 2;

	// Instruction uses 5 machine cycles.
	cycles += 5;
}

//----------------------------------------//
// This instruction will load a 16-bit    //
// register with a given value.           //
//----------------------------------------//
void z80_LD_reg16_immediate(unsigned char *reg1, unsigned char *reg2)
{
	// Load the 16-bit value into address.
	*reg2 = memory[regs.PC];
	regs.PC++;
	*reg1 = memory[regs.PC];
	regs.PC++;

	// Instruction uses 3 machine cycles
	cycles += 3;
}

//----------------------------------------//
// This instruction will load a 16-bit    //
// register with the value at location A. //
//----------------------------------------//
void z80_LD_location_reg16_A(unsigned short int *reg)
{
	WriteMemory(*reg, regs.A);
	
	// If the opcode was HLI, increment HL.
	if (opcode == 0x22)
		regs.HL++;
	// If the opcode was HLD, decrement HL.
	else if (opcode == 0x32)
		regs.HL--;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will load an 8-bit    //
// register with a given value.           //
//----------------------------------------//
void z80_LD_reg8_immediate(unsigned char *reg)
{
	// Load *reg with the 8-bit value immediately after it.
	*reg = memory[regs.PC];
	
	// Increment the program counter to skip the value.
	regs.PC++;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will load an 8-bit    //
// register with the value at memory      //
// location (HL).                         //
//----------------------------------------//
void z80_LD_reg8_location_HL(unsigned char *reg)
{
	// Load the value at the address in HL into *reg.
	*reg = memory[regs.HL];

	// Instruction uses 2 machine cycles.
	cycles += 2;
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

	// Instruction uses 1 clock cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will load the Stack   //
// Pointer with the value of register HL. //
//----------------------------------------//
void z80_LD_SP_HL()
{
	// Load register HL into Stack Pointer.
	regs.SP = regs.HL;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction loads register A into //
// memory location 0xFF00 plus register C.//
//----------------------------------------//
void z80_LD_0xFF00_C_A()
{
	WriteMemory(0xFF00 + regs.C, regs.A);

	cycles += 2;
}

//----------------------------------------//
// This instruction will load the value   //
// of register A into memory at the       //
// location of 0xFF00 plus an 8-bit value.//
//----------------------------------------//
void z80_LD_0xFF00_immediate_A()
{
	// Write the value of register A into memory.
	WriteMemory(0xFF00 + memory[regs.PC], regs.A);

	// Increment Program Counter
	regs.PC++;

	// Instruction uses 3 machine cycles.
	cycles += 3;
}

//----------------------------------------//
// This instruction idles the CPU for 1   //
// machine cycle.                         //
//----------------------------------------//
void z80_NOP()
{
	// Idle for 1 machine cycle
	cycles += 1;
}

//----------------------------------------//
// This instruction will logical OR a     //
// given value with register A.           //
//----------------------------------------//
void z80_OR_immediate()
{
	regs.A |= memory[regs.PC];

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// The other three flags are reset.
	regs.F &= FLAG_Z_ON;

	// Increment Program Counter.
	regs.PC++;

	// Machine cycles used is dependant on opcode.
	if (opcode == 0xB6)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will logical OR a     //
// 8-bit register and register A.         //
//----------------------------------------//
void z80_OR_reg8(unsigned char *reg)
{
	// Logically Inclusive OR register A and *reg.
	regs.A |= *reg;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// The other three flags are reset.
	regs.F &= FLAG_Z_ON;

	// Machine cycles used is dependant on opcode.
	if (opcode == 0xB6)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will pop a value off  //
// the stack into a 16-bit register.      //
//----------------------------------------//
void z80_POP_reg16(unsigned char *reg1, unsigned char *reg2)
{
	// Pop two bytes from the stack into *reg.
	*reg2 = memory[regs.SP];
	regs.SP++;
	*reg1 = memory[regs.SP];
	regs.SP++;

	// Instruction uses 3 cycles.
	cycles += 3;
}

//----------------------------------------//
// This instruction will push the value of//
// a 16-bit register onto the stack.      //
//----------------------------------------//
void z80_PUSH_reg16(unsigned char *reg1, unsigned char *reg2)
{
	// Push the 16-bit register's two bytes onto the stack. Little
	// Endian order.
	regs.SP--;
	WriteMemory(regs.SP, *reg1);
	regs.SP--;
	WriteMemory(regs.SP, *reg2);
	
	// Instruction uses 4 cycles.
	cycles += 4;
}

//----------------------------------------//
// This instruction will reset(turn off) a//
// given bit of an 8-bit register.        //
//----------------------------------------//
void z80_RES_bit_reg8(unsigned char bit, unsigned char *reg)
{
	*reg &= (0xFF - bit);

	if (((opcode & 0x06) == 0x06) || ((opcode & 0x0E) == 0x0E))
		cycles += 3;
	else
		cycles += 2;
}

//----------------------------------------//
// This instruction will pop two bytes off//
// the stack and jump to that address.    //
//----------------------------------------//
void z80_RET()
{
	// Set Program Counter to the first 16-bit address in the stack.
	regs.PCL = memory[regs.SP];
	regs.SP++;
	regs.PCH = memory[regs.SP];
	regs.SP++;

	cycles += 4;
}

//----------------------------------------//
// This instruction will pop two bytes off//
// the stack, jump to that address, and   //
// turn on the Interrupt Master Enable.   //
//----------------------------------------//
void z80_RETI()
{
	// Load 16-bit address from stack into Program Counter.
	regs.PCL = memory[regs.SP];
	regs.SP++;
	regs.PCH = memory[regs.SP];
	regs.SP++;

	// Enable interrupts.
	interruptMasterEnable = 1;

	cycles += 4;
}

//----------------------------------------//
// This instruction will pop two bytes off//
// the stack and jump to that address, but//
// only if the given condtion is met.     //
//----------------------------------------//
void z80_RET_condition(unsigned char condition)
{
	unsigned char conditionTrue = 0;

	switch (condition)
	{
	case 0x01:
		{
			if (!FLAG_Z)
			{
				conditionTrue = 1;
			}
		}
	break;
	case 0x02:
		{
			if (FLAG_Z)
			{
				conditionTrue = 1;
			}
		}
	break;
	case 0x03:
		{
			if (!FLAG_C)
			{
				conditionTrue = 1;
			}
		}
	break;
	case 0x04:
		{
			if (FLAG_C)
			{
				conditionTrue = 1;
			}
		}
	break;
	}

	if (conditionTrue == 1)
	{
		// Change the program counter to the address on the stack.
		regs.PCL = memory[regs.SP];
		regs.SP++;
		regs.PCH = memory[regs.SP];
		regs.SP++;

		cycles += 3;
	}

	// Instruction uses 2 cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction rotates an 8-bit      //
// register left by 1 bit.                //
//----------------------------------------//
void z80_RL_reg8(unsigned char *reg)
{
	if (*reg & BIT_7)
	{
		if (FLAG_C)
			*reg = (*reg << 1) | 0x01;
		else
			*reg <<= 1;
		
		regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C)
			*reg = (*reg << 1) | 0x01;
		else
			*reg <<= 1;
		
		regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];

	// Flags H and N are all reset
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x16)
		cycles += 4;
	else
		cycles += 2;
}

//----------------------------------------//
// This instruction rotates register A    //
// left by 1 bit.                         //
//----------------------------------------//
void z80_RLA()
{
	if (regs.A & BIT_7)
	{
		if (FLAG_C)
			regs.A = (regs.A << 1) | 0x01;
		else
			regs.A <<= 1;

		regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C)
			regs.A = (regs.A << 1) | 0x01;
		else
			regs.A <<= 1;

		regs.F &= FLAG_C_OFF;
	}

	// The other three flags are reset.  Only the carry flag
	// remains unchanged.
	regs.F &= FLAG_C_ON;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will rotate an 8-bit  //
// register left through the carry flag.  //
//----------------------------------------//
void z80_RLC_reg8(unsigned char *reg)
{
	// See if the old bit 7 data is on or off and set flag C accordingly.
	if (*reg & BIT_7)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Shift *reg 1 bit to the left.  Old bit 7 goes to bit 0.
	*reg <<= 1;
	
	// Add flag C to *reg to make up for the shifted out bit.
	if (FLAG_C)
		*reg |= 1;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];
	
	// Flags N and H are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x06)
		cycles += 4;
	else
		cycles += 2;
}

//----------------------------------------//
// This instruction rotates register A    //
// by 1 bit through the carry flag.       //
//----------------------------------------//
void z80_RLCA()
{
	if (regs.A & BIT_7)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Shift register A 1 bit to the left.  Old bit 7 goes to bit 0.
	regs.A <<= 1;
	
	// Add flag C to register A to make up for the shifted out bit.
	if (FLAG_C)
		regs.A |= 0x01;

	// The other three flags are reset.  Only the carry flag
	// remains unchanged.
	regs.F &= FLAG_C_ON;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction rotates an 8-bit      //
// register right by 1 bit.                //
//----------------------------------------//
void z80_RR_reg8(unsigned char *reg)
{
	if (*reg & BIT_0)
	{
		if (FLAG_C)
			*reg = (*reg >> 1) | 0x80;
		else
			*reg >>= 1;
		
		regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C)
			*reg = (*reg >> 1) | 0x80;
		else
			*reg >>= 1;
		
		regs.F &= FLAG_C_OFF;
	}
	
	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];

	// Flags N and H are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x1E)
		cycles += 4;
	else
		cycles += 2;
}

//----------------------------------------//
// This instruction rotates register A    //
// to the right 1 bit.                    //
//----------------------------------------//
void z80_RRA()
{
	if (regs.A & BIT_0)
	{
		if (FLAG_C)
			regs.A = (regs.A >> 1) | 0x80;
		else
			regs.A >>= 1;

		regs.F |= FLAG_C_ON;
	}
	else
	{
		if (FLAG_C)
			regs.A = (regs.A >> 1) | 0x80;
		else
			regs.A >>= 1;

		regs.F &= FLAG_C_OFF;
	}

	// The other three flags are reset.
	regs.F &= FLAG_C_ON;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will rotate an 8-bit  //
// register right through the carry flag. //
//----------------------------------------//
void z80_RRC_reg8(unsigned char *reg)
{
	// See if the old bit 0 data was on or off and set flag C accordingly.
	if (*reg & BIT_0)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Shift *reg 1 bit to the right.  Old bit 0 goes to bit 7.
	*reg >>= 1;

	// If flag C is on, add 0x80 to *reg to make up for the
	// lost bit.
	if (FLAG_C)
		*reg |= 0x80;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];

	// Flags N and H are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x0E)
		cycles += 4;
	else
		cycles += 2;
}

//----------------------------------------//
// This instruction rotates register A    //
// through the carry flag.                //
//----------------------------------------//
void z80_RRCA()
{
	// See if the old bit 0 data was on or off and set flag C accordingly.
	if (regs.A & BIT_0)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Shift register A 1 bit to the right.  Old bit 0 goes to bit 7.
	regs.A >>= 1;

	// If flag C is on, add 0x80 to register A to make up for the
	// lost bit.
	if (FLAG_C)
		regs.A |= 0x80;

	// The other three flags are reset.
	regs.F &= FLAG_C_ON;

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will push the current //
// Program Counter onto the stack and     //
// jumps to a new address(This is used    //
// mainly for interrupts).                //
//----------------------------------------//
void z80_RST(unsigned char address)
{
	// Push the Program Counter into the stack in little-endian
	// (low byte first) order.
	regs.SP--;
	WriteMemory(regs.SP, regs.PCH);
	regs.SP--;
	WriteMemory(regs.SP, regs.PCL);

	// Set Program Counter to address.
	regs.PC = address;

	// Instruction uses 4 machine cycles.
	cycles += 4;
}

//----------------------------------------//
// This instruction will subtract a given //
// value and the carry flag to register A.//
//----------------------------------------//
void z80_SBC_A_immediate()
{
	// Need to check for borrow from bit 4.
	if (((regs.A - memory[regs.PC] - FLAG_C) & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((regs.A - memory[regs.PC] - FLAG_C) < 0)
	{
		regs.A -= memory[regs.PC] + FLAG_C;
		regs.F |= FLAG_C_ON;
	}
	else
	{
		regs.A -= memory[regs.PC] + FLAG_C;
		regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Increment Program Counter.
	regs.PC++;

	// Flag N is set.
	regs.F |= FLAG_N_ON;

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x9E)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will subtract an 8-bit//
// register and the carry flag from       //
// register A.                            //
//----------------------------------------//
void z80_SBC_A_reg8(unsigned char *reg)
{
	// Need to check for borrow from bit 4.
	if (((regs.A - *reg - FLAG_C) & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((regs.A - *reg - FLAG_C) < 0)
	{
		regs.A -= *reg + FLAG_C;
		regs.F |= FLAG_C_ON;
	}
	else
	{
		regs.A -= *reg + FLAG_C;
		regs.F &= FLAG_C_OFF;
	}

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Flag N is set.
	regs.F |= FLAG_N_ON;

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x9E)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction will set the carry    //
// flag.                                  //
//----------------------------------------//
void z80_SCF()
{
	// Set flag C.
	regs.F |= FLAG_C_ON;

	// Flags N and H are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Instruction uses 1 machine cycle.
	cycles += 1;
}

//----------------------------------------//
// This instruction will set(turn on) a   //
// given bit of an 8-bit register.        //
//----------------------------------------//
void z80_SET_bit_reg8(unsigned char bit, unsigned char *reg)
{
	*reg |= bit;

	if (((opcode & 0x06) == 0x06) || ((opcode & 0x0E) == 0x0E))
		cycles += 3;
	else
		cycles += 2;
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
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Shift *reg left once.  0 is shifted in from right.
	*reg <<= 1;

	// Make sure the least significant bit is 0.
	*reg &= (255 - BIT_0);

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];

	// Flags H and N are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x26)
		cycles += 4;
	else
		cycles += 2;
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
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Shift *reg right once.  0 is shifted in from left.  If bit 7
	// is set, make sure it stays set.
	if (*reg & BIT_7)
		*reg = (*reg >> 1) + BIT_7;
	else
		*reg >>= 1;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];

	// Flags H and N are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x2E)
		cycles += 4;
	else
		cycles += 2;
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
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Shift *reg right once.  0 is shifted in from right.
	*reg >>= 1;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];

	// Flags H and N are reset.
	regs.F &= (FLAG_Z_ON | FLAG_C_ON);

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x3E)
		cycles += 4;
	else
		cycles += 2;
}

//----------------------------------------//
// This halts the GB CPU until a button is//
// pressed.                               //
//----------------------------------------//
void z80_STOP()
{
	// Skip over the extra 0x00
	regs.PC++;

	// Set stopped status to on.
    stopped = 1;

	// Instruction uses 1 machine cycle
	cycles += 1;
}

//----------------------------------------//
// This instruction will subtract a given //
// value from register A.                 //
//----------------------------------------//
void z80_SUB_immediate()
{
	// Need to check for borrow from bit 4.
	if (((regs.A - memory[regs.PC]) & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((regs.A - memory[regs.PC]) < 0)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;

	// Subtract *reg from register A.
	regs.A -= memory[regs.PC];

	// If the result was 0, turn on flag Z.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];
	
	// Increment Program Counter.
	regs.PC++;
	
	// Turn on flag N.
	regs.F |= FLAG_N_ON;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This instruction will subtract an 8-bit//
// register's value from register A.      //
//----------------------------------------//
void z80_SUB_reg8(unsigned char *reg)
{
	// Need to check for borrow from bit 4.
	if (((regs.A - *reg) & 0xF) == 0xF)
		regs.F |= FLAG_H_ON;
	else
		regs.F &= FLAG_H_OFF;

	// Set flag C if result will be below 0.
	if ((regs.A - *reg) < 0)
		regs.F |= FLAG_C_ON;
	else
		regs.F &= FLAG_C_OFF;
	
	// Subtract *reg from register A.
	regs.A -= *reg;

	// If the result was 0, turn on flag Z.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];
	
	// Turn on flag N.
	regs.F |= FLAG_N_ON;

	// Machine cycles used is dependant on opcode.
	if (opcode == 0x96)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This instruction swaps the higher and  //
// lower 4-bits of an 8-bit register.     //
//----------------------------------------//
void z80_SWAP_reg8(unsigned char *reg)
{
	// Swap the upper and lower nybbles of *reg.
	*reg = (((*reg & 0xF0) >> 4) | ((*reg & 0x0F) << 4));

	// If the result was 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[*reg];

	// The other three flags are reset.
	regs.F &= FLAG_Z_ON;

	// Machines cycles used is dependant on opcode.
	if (opcode == 0x36)
		cycles += 4;
	else
		cycles += 2;
}

//----------------------------------------//
// This will exclusive OR a given value   //
// and register A.                        //
//----------------------------------------//
void z80_XOR_immediate()
{
	// Logically Exclusive OR register A and *reg.
	regs.A ^= memory[regs.PC];

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// Increment Program Counter.
	regs.PC++;
	
	// The other three flags are reset.
	regs.F &= FLAG_Z_ON;

	// Instruction uses 2 machine cycles.
	cycles += 2;
}

//----------------------------------------//
// This will exclusive OR an 8-bit        //
// register and register A.               //
//----------------------------------------//
void z80_XOR_reg8(unsigned char *reg)
{
	// Logically Exclusive OR register A and *reg.
	regs.A ^= *reg;

	// If result is 0, set flag Z, otherwise reset it.
	regs.F &= 127;
	regs.F |= zeroTable[regs.A];

	// The other three flags are reset.
	regs.F &= FLAG_Z_ON;

	// Machine cycles used is dependant on opcode.
	if (opcode == 0xAE)
		cycles += 2;
	else
		cycles += 1;
}

//----------------------------------------//
// This function takes care of the main GB//
// CPU processes.                         //
//----------------------------------------//
void RunEmulation()
{
	// Initialize audio library
	//aud.Initialize();

	// Setup a function callback to display Frames Per Second.
	FPSTimerID = SDL_AddTimer(1000, UpdateFPS, NULL);

	emulationTimer = SDL_GetTicks();

	while (CPURunning)
	{
	while (cycleTotal < cyclesToRun)
	{
		if (halted == 0)
		{
			if (stopped == 0)
			{
				// Fetch an opcode from the ROM buffer.
				opcode = memory[regs.PC];

				// Increment the Program Counter.
				if (instructionRepeat == 0)
				{
					regs.PC++;
				}
				else
				{
					instructionRepeat = 0;
				}

				// Use the opcode to call the appropriate function.
				switch(opcode)
				{
				case 0x00: z80_NOP(); break;
				case 0x01: z80_LD_reg16_immediate(&regs.B, &regs.C); break;
				case 0x02: z80_LD_location_reg16_A(&regs.BC); break;
				case 0x03: z80_INC_reg16(&regs.BC); break;
				case 0x04: z80_INC_reg8(&regs.B); break;
				case 0x05: z80_DEC_reg8(&regs.B); break;
				case 0x06: z80_LD_reg8_immediate(&regs.B); break;
				case 0x07: z80_RLCA(); break;
				case 0x08: z80_LD_location_SP(); break;
				case 0x09: z80_ADD_HL_reg16(&regs.BC); break;
				case 0x0A: z80_LD_A_location_reg16(&regs.BC); break;
				case 0x0B: z80_DEC_reg16(&regs.BC); break;
				case 0x0C: z80_INC_reg8(&regs.C); break;
				case 0x0D: z80_DEC_reg8(&regs.C); break;
				case 0x0E: z80_LD_reg8_immediate(&regs.C); break;
				case 0x0F: z80_RRCA(); break;
				case 0x10: z80_STOP(); break;
				case 0x11: z80_LD_reg16_immediate(&regs.D, &regs.E); break;
				case 0x12: z80_LD_location_reg16_A(&regs.DE); break;
				case 0x13: z80_INC_reg16(&regs.DE); break;
				case 0x14: z80_INC_reg8(&regs.D); break;
				case 0x15: z80_DEC_reg8(&regs.D); break;
				case 0x16: z80_LD_reg8_immediate(&regs.D); break;
				case 0x17: z80_RLA(); break;
				case 0x18: z80_JR_offset(); break;
				case 0x19: z80_ADD_HL_reg16(&regs.DE); break;
				case 0x1A: z80_LD_A_location_reg16(&regs.DE); break;
				case 0x1B: z80_DEC_reg16(&regs.DE); break;
				case 0x1C: z80_INC_reg8(&regs.E); break;
				case 0x1D: z80_DEC_reg8(&regs.E); break;
				case 0x1E: z80_LD_reg8_immediate(&regs.E); break;
				case 0x1F: z80_RRA(); break;
				case 0x20: z80_JR_condition_offset(0x01); break;
				case 0x21: z80_LD_reg16_immediate(&regs.H, &regs.L); break;
				case 0x22: z80_LD_location_reg16_A(&regs.HL); break;
				case 0x23: z80_INC_reg16(&regs.HL); break;
				case 0x24: z80_INC_reg8(&regs.H); break;
				case 0x25: z80_DEC_reg8(&regs.H); break;
				case 0x26: z80_LD_reg8_immediate(&regs.H); break;
				case 0x27: z80_DAA(); break;
				case 0x28: z80_JR_condition_offset(0x02); break;
				case 0x29: z80_ADD_HL_reg16(&regs.HL); break;
				case 0x2A: z80_LD_A_location_reg16(&regs.HL); break;
				case 0x2B: z80_DEC_reg16(&regs.HL); break;
				case 0x2C: z80_INC_reg8(&regs.L); break;
				case 0x2D: z80_DEC_reg8(&regs.L); break;
				case 0x2E: z80_LD_reg8_immediate(&regs.L); break;
				case 0x2F: z80_CPL(); break;
				case 0x30: z80_JR_condition_offset(0x03); break;
				case 0x31: z80_LD_reg16_immediate(&regs.SPH, &regs.SPL); break;
				case 0x32: z80_LD_location_reg16_A(&regs.HL); break;
				case 0x33: z80_INC_reg16(&regs.SP); break;
				case 0x34: z80_INC_location_HL(); break;
				case 0x35: z80_DEC_location_HL(); break;
				case 0x36: z80_LD_location_HL_immediate(); break;
				case 0x37: z80_SCF(); break;
				case 0x38: z80_JR_condition_offset(0x04); break;
				case 0x39: z80_ADD_HL_reg16(&regs.SP); break;
				case 0x3A: z80_LD_A_location_reg16(&regs.HL); break;
				case 0x3B: z80_DEC_reg16(&regs.SP); break;
				case 0x3C: z80_INC_reg8(&regs.A); break;
				case 0x3D: z80_DEC_reg8(&regs.A); break;
				case 0x3E: z80_LD_reg8_immediate(&regs.A); break;
				case 0x3F: z80_CCF(); break;
				case 0x40: z80_LD_reg8_reg8(&regs.B, &regs.B); break;
				case 0x41: z80_LD_reg8_reg8(&regs.B, &regs.C); break;
				case 0x42: z80_LD_reg8_reg8(&regs.B, &regs.D); break;
				case 0x43: z80_LD_reg8_reg8(&regs.B, &regs.E); break;
				case 0x44: z80_LD_reg8_reg8(&regs.B, &regs.H); break;
				case 0x45: z80_LD_reg8_reg8(&regs.B, &regs.L); break;
				case 0x46: z80_LD_reg8_location_HL(&regs.B); break;
				case 0x47: z80_LD_reg8_reg8(&regs.B, &regs.A); break;
				case 0x48: z80_LD_reg8_reg8(&regs.C, &regs.B); break;
				case 0x49: z80_LD_reg8_reg8(&regs.C, &regs.C); break;
				case 0x4A: z80_LD_reg8_reg8(&regs.C, &regs.D); break;
				case 0x4B: z80_LD_reg8_reg8(&regs.C, &regs.E); break;
				case 0x4C: z80_LD_reg8_reg8(&regs.C, &regs.H); break;
				case 0x4D: z80_LD_reg8_reg8(&regs.C, &regs.L); break;
				case 0x4E: z80_LD_reg8_location_HL(&regs.C); break;
				case 0x4F: z80_LD_reg8_reg8(&regs.C, &regs.A); break;
				case 0x50: z80_LD_reg8_reg8(&regs.D, &regs.B); break;
				case 0x51: z80_LD_reg8_reg8(&regs.D, &regs.C); break;
				case 0x52: z80_LD_reg8_reg8(&regs.D, &regs.D); break;
				case 0x53: z80_LD_reg8_reg8(&regs.D, &regs.E); break;
				case 0x54: z80_LD_reg8_reg8(&regs.D, &regs.H); break;
				case 0x55: z80_LD_reg8_reg8(&regs.D, &regs.L); break;
				case 0x56: z80_LD_reg8_location_HL(&regs.D); break;
				case 0x57: z80_LD_reg8_reg8(&regs.D, &regs.A); break;
				case 0x58: z80_LD_reg8_reg8(&regs.E, &regs.B); break;
				case 0x59: z80_LD_reg8_reg8(&regs.E, &regs.C); break;
				case 0x5A: z80_LD_reg8_reg8(&regs.E, &regs.D); break;
				case 0x5B: z80_LD_reg8_reg8(&regs.E, &regs.E); break;
				case 0x5C: z80_LD_reg8_reg8(&regs.E, &regs.H); break;
				case 0x5D: z80_LD_reg8_reg8(&regs.E, &regs.L); break;
				case 0x5E: z80_LD_reg8_location_HL(&regs.E); break;
				case 0x5F: z80_LD_reg8_reg8(&regs.E, &regs.A); break;
				case 0x60: z80_LD_reg8_reg8(&regs.H, &regs.B); break;
				case 0x61: z80_LD_reg8_reg8(&regs.H, &regs.C); break;
				case 0x62: z80_LD_reg8_reg8(&regs.H, &regs.D); break;
				case 0x63: z80_LD_reg8_reg8(&regs.H, &regs.E); break;
				case 0x64: z80_LD_reg8_reg8(&regs.H, &regs.H); break;
				case 0x65: z80_LD_reg8_reg8(&regs.H, &regs.L); break;
				case 0x66: z80_LD_reg8_location_HL(&regs.H); break;
				case 0x67: z80_LD_reg8_reg8(&regs.H, &regs.A); break;
				case 0x68: z80_LD_reg8_reg8(&regs.L, &regs.B); break;
				case 0x69: z80_LD_reg8_reg8(&regs.L, &regs.C); break;
				case 0x6A: z80_LD_reg8_reg8(&regs.L, &regs.D); break;
				case 0x6B: z80_LD_reg8_reg8(&regs.L, &regs.E); break;
				case 0x6C: z80_LD_reg8_reg8(&regs.L, &regs.H); break;
				case 0x6D: z80_LD_reg8_reg8(&regs.L, &regs.L); break;
				case 0x6E: z80_LD_reg8_location_HL(&regs.L); break;
				case 0x6F: z80_LD_reg8_reg8(&regs.L, &regs.A); break;
				case 0x70: z80_LD_location_HL_reg8(&regs.B); break;
				case 0x71: z80_LD_location_HL_reg8(&regs.C); break;
				case 0x72: z80_LD_location_HL_reg8(&regs.D); break;
				case 0x73: z80_LD_location_HL_reg8(&regs.E); break;
				case 0x74: z80_LD_location_HL_reg8(&regs.H); break;
				case 0x75: z80_LD_location_HL_reg8(&regs.L); break;
				case 0x76: z80_HALT(); break;
				case 0x77: z80_LD_location_HL_reg8(&regs.A); break;
				case 0x78: z80_LD_reg8_reg8(&regs.A, &regs.B); break;
				case 0x79: z80_LD_reg8_reg8(&regs.A, &regs.C); break;
				case 0x7A: z80_LD_reg8_reg8(&regs.A, &regs.D); break;
				case 0x7B: z80_LD_reg8_reg8(&regs.A, &regs.E); break;
				case 0x7C: z80_LD_reg8_reg8(&regs.A, &regs.H); break;
				case 0x7D: z80_LD_reg8_reg8(&regs.A, &regs.L); break;
				case 0x7E: z80_LD_reg8_location_HL(&regs.A); break;
				case 0x7F: z80_LD_reg8_reg8(&regs.A, &regs.A); break;
				case 0x80: z80_ADD_A_reg8(&regs.B); break;
				case 0x81: z80_ADD_A_reg8(&regs.C); break;
				case 0x82: z80_ADD_A_reg8(&regs.D); break;
				case 0x83: z80_ADD_A_reg8(&regs.E); break;
				case 0x84: z80_ADD_A_reg8(&regs.H); break;
				case 0x85: z80_ADD_A_reg8(&regs.L); break;
				case 0x86: z80_ADD_A_reg8(&memory[regs.HL]); break;
				case 0x87: z80_ADD_A_reg8(&regs.A); break;
				case 0x88: z80_ADC_A_reg8(&regs.B); break;
				case 0x89: z80_ADC_A_reg8(&regs.C); break;
				case 0x8A: z80_ADC_A_reg8(&regs.D); break;
				case 0x8B: z80_ADC_A_reg8(&regs.E); break;
				case 0x8C: z80_ADC_A_reg8(&regs.H); break;
				case 0x8D: z80_ADC_A_reg8(&regs.L); break;
				case 0x8E: z80_ADC_A_reg8(&memory[regs.HL]); break;
				case 0x8F: z80_ADC_A_reg8(&regs.A); break;
				case 0x90: z80_SUB_reg8(&regs.B); break;
				case 0x91: z80_SUB_reg8(&regs.C); break;
				case 0x92: z80_SUB_reg8(&regs.D); break;
				case 0x93: z80_SUB_reg8(&regs.E); break;
				case 0x94: z80_SUB_reg8(&regs.H); break;
				case 0x95: z80_SUB_reg8(&regs.L); break;
				case 0x96: z80_SUB_reg8(&memory[regs.HL]); break;
				case 0x97: z80_SUB_reg8(&regs.A); break;
				case 0x98: z80_SBC_A_reg8(&regs.B); break;
				case 0x99: z80_SBC_A_reg8(&regs.C); break;
				case 0x9A: z80_SBC_A_reg8(&regs.D); break;
				case 0x9B: z80_SBC_A_reg8(&regs.E); break;
				case 0x9C: z80_SBC_A_reg8(&regs.H); break;
				case 0x9D: z80_SBC_A_reg8(&regs.L); break;
				case 0x9E: z80_SBC_A_reg8(&memory[regs.HL]); break;
				case 0x9F: z80_SBC_A_reg8(&regs.A); break;
				case 0xA0: z80_AND_reg8(&regs.B); break;
				case 0xA1: z80_AND_reg8(&regs.C); break;
				case 0xA2: z80_AND_reg8(&regs.D); break;
				case 0xA3: z80_AND_reg8(&regs.E); break;
				case 0xA4: z80_AND_reg8(&regs.H); break;
				case 0xA5: z80_AND_reg8(&regs.L); break;
				case 0xA6: z80_AND_reg8(&memory[regs.HL]); break;
				case 0xA7: z80_AND_reg8(&regs.A); break;
				case 0xA8: z80_XOR_reg8(&regs.B); break;
				case 0xA9: z80_XOR_reg8(&regs.C); break;
				case 0xAA: z80_XOR_reg8(&regs.D); break;
				case 0xAB: z80_XOR_reg8(&regs.E); break;
				case 0xAC: z80_XOR_reg8(&regs.H); break;
				case 0xAD: z80_XOR_reg8(&regs.L); break;
				case 0xAE: z80_XOR_reg8(&memory[regs.HL]); break;
				case 0xAF: z80_XOR_reg8(&regs.A); break;
				case 0xB0: z80_OR_reg8(&regs.B); break;
				case 0xB1: z80_OR_reg8(&regs.C); break;
				case 0xB2: z80_OR_reg8(&regs.D); break;
				case 0xB3: z80_OR_reg8(&regs.E); break;
				case 0xB4: z80_OR_reg8(&regs.H); break;
				case 0xB5: z80_OR_reg8(&regs.L); break;
				case 0xB6: z80_OR_reg8(&memory[regs.HL]); break;
				case 0xB7: z80_OR_reg8(&regs.A); break;
				case 0xB8: z80_CP_reg8(&regs.B); break;
				case 0xB9: z80_CP_reg8(&regs.C); break;
				case 0xBA: z80_CP_reg8(&regs.D); break;
				case 0xBB: z80_CP_reg8(&regs.E); break;
				case 0xBC: z80_CP_reg8(&regs.H); break;
				case 0xBD: z80_CP_reg8(&regs.L); break;
				case 0xBE: z80_CP_reg8(&memory[regs.HL]); break;
				case 0xBF: z80_CP_reg8(&regs.A); break;
				case 0xC0: z80_RET_condition(0x01); break;
				case 0xC1: z80_POP_reg16(&regs.B, &regs.C); break;
				case 0xC2: z80_JP_condition_immediate(0x01); break;
				case 0xC3: z80_JP_immediate(); break;
				case 0xC4: z80_CALL_condition_immediate(0x01); break;
				case 0xC5: z80_PUSH_reg16(&regs.B, &regs.C); break;
				case 0xC6: z80_ADD_A_immediate(); break;
				case 0xC7: z80_RST(0x00); break;
				case 0xC8: z80_RET_condition(0x02); break;
				case 0xC9: z80_RET(); break;
				case 0xCA: z80_JP_condition_immediate(0x02); break;
				// Cover the special 0xCB opcodes.
				case 0xCB:
					{
						opcode = memory[regs.PC];
						regs.PC++;
						switch (opcode)
						{
						case 0x00: z80_RLC_reg8(&regs.B); break;
						case 0x01: z80_RLC_reg8(&regs.C); break;
						case 0x02: z80_RLC_reg8(&regs.D); break;
						case 0x03: z80_RLC_reg8(&regs.E); break;
						case 0x04: z80_RLC_reg8(&regs.H); break;
						case 0x05: z80_RLC_reg8(&regs.L); break;
						case 0x06: z80_RLC_reg8(&memory[regs.HL]); break;
						case 0x07: z80_RLC_reg8(&regs.A); break;
						case 0x08: z80_RRC_reg8(&regs.B); break;
						case 0x09: z80_RRC_reg8(&regs.C); break;
						case 0x0A: z80_RRC_reg8(&regs.D); break;
						case 0x0B: z80_RRC_reg8(&regs.E); break;
						case 0x0C: z80_RRC_reg8(&regs.H); break;
						case 0x0D: z80_RRC_reg8(&regs.L); break;
						case 0x0E: z80_RRC_reg8(&memory[regs.HL]); break;
						case 0x0F: z80_RRC_reg8(&regs.A); break;
						case 0x10: z80_RL_reg8(&regs.B); break;
						case 0x11: z80_RL_reg8(&regs.C); break;
						case 0x12: z80_RL_reg8(&regs.D); break;
						case 0x13: z80_RL_reg8(&regs.E); break;
						case 0x14: z80_RL_reg8(&regs.H); break;
						case 0x15: z80_RL_reg8(&regs.L); break;
						case 0x16: z80_RL_reg8(&memory[regs.HL]); break;
						case 0x17: z80_RL_reg8(&regs.A); break;
						case 0x18: z80_RR_reg8(&regs.B); break;
						case 0x19: z80_RR_reg8(&regs.C); break;
						case 0x1A: z80_RR_reg8(&regs.D); break;
						case 0x1B: z80_RR_reg8(&regs.E); break;
						case 0x1C: z80_RR_reg8(&regs.H); break;
						case 0x1D: z80_RR_reg8(&regs.L); break;
						case 0x1E: z80_RR_reg8(&memory[regs.HL]); break;
						case 0x1F: z80_RR_reg8(&regs.A); break;
						case 0x20: z80_SLA_reg8(&regs.B); break;
						case 0x21: z80_SLA_reg8(&regs.C); break;
						case 0x22: z80_SLA_reg8(&regs.D); break;
						case 0x23: z80_SLA_reg8(&regs.E); break;
						case 0x24: z80_SLA_reg8(&regs.H); break;
						case 0x25: z80_SLA_reg8(&regs.L); break;
						case 0x26: z80_SLA_reg8(&memory[regs.HL]); break;
						case 0x27: z80_SLA_reg8(&regs.A); break;
						case 0x28: z80_SRA_reg8(&regs.B); break;
						case 0x29: z80_SRA_reg8(&regs.C); break;
						case 0x2A: z80_SRA_reg8(&regs.D); break;
						case 0x2B: z80_SRA_reg8(&regs.E); break;
						case 0x2C: z80_SRA_reg8(&regs.H); break;
						case 0x2D: z80_SRA_reg8(&regs.L); break;
						case 0x2E: z80_SRA_reg8(&memory[regs.HL]); break;
						case 0x2F: z80_SRA_reg8(&regs.A); break;
						case 0x30: z80_SWAP_reg8(&regs.B); break;
						case 0x31: z80_SWAP_reg8(&regs.C); break;
						case 0x32: z80_SWAP_reg8(&regs.D); break;
						case 0x33: z80_SWAP_reg8(&regs.E); break;
						case 0x34: z80_SWAP_reg8(&regs.H); break;
						case 0x35: z80_SWAP_reg8(&regs.L); break;
						case 0x36: z80_SWAP_reg8(&memory[regs.HL]); break;
						case 0x37: z80_SWAP_reg8(&regs.A); break;
						case 0x38: z80_SRL_reg8(&regs.B); break;
						case 0x39: z80_SRL_reg8(&regs.C); break;
						case 0x3A: z80_SRL_reg8(&regs.D); break;
						case 0x3B: z80_SRL_reg8(&regs.E); break;
						case 0x3C: z80_SRL_reg8(&regs.H); break;
						case 0x3D: z80_SRL_reg8(&regs.L); break;
						case 0x3E: z80_SRL_reg8(&memory[regs.HL]); break;
						case 0x3F: z80_SRL_reg8(&regs.A); break;
						case 0x40: z80_BIT_bit_reg8(0x01, &regs.B); break;
						case 0x41: z80_BIT_bit_reg8(0x01, &regs.C); break;
						case 0x42: z80_BIT_bit_reg8(0x01, &regs.D); break;
						case 0x43: z80_BIT_bit_reg8(0x01, &regs.E); break;
						case 0x44: z80_BIT_bit_reg8(0x01, &regs.H); break;
						case 0x45: z80_BIT_bit_reg8(0x01, &regs.L); break;
						case 0x46: z80_BIT_bit_reg8(0x01, &memory[regs.HL]); break;
						case 0x47: z80_BIT_bit_reg8(0x01, &regs.A); break;
						case 0x48: z80_BIT_bit_reg8(0x02, &regs.B); break;
						case 0x49: z80_BIT_bit_reg8(0x02, &regs.C); break;
						case 0x4A: z80_BIT_bit_reg8(0x02, &regs.D); break;
						case 0x4B: z80_BIT_bit_reg8(0x02, &regs.E); break;
						case 0x4C: z80_BIT_bit_reg8(0x02, &regs.H); break;
						case 0x4D: z80_BIT_bit_reg8(0x02, &regs.L); break;
						case 0x4E: z80_BIT_bit_reg8(0x02, &memory[regs.HL]); break;
						case 0x4F: z80_BIT_bit_reg8(0x02, &regs.A); break;
						case 0x50: z80_BIT_bit_reg8(0x04, &regs.B); break;
						case 0x51: z80_BIT_bit_reg8(0x04, &regs.C); break;
						case 0x52: z80_BIT_bit_reg8(0x04, &regs.D); break;
						case 0x53: z80_BIT_bit_reg8(0x04, &regs.E); break;
						case 0x54: z80_BIT_bit_reg8(0x04, &regs.H); break;
						case 0x55: z80_BIT_bit_reg8(0x04, &regs.L); break;
						case 0x56: z80_BIT_bit_reg8(0x04, &memory[regs.HL]); break;
						case 0x57: z80_BIT_bit_reg8(0x04, &regs.A); break;
						case 0x58: z80_BIT_bit_reg8(0x08, &regs.B); break;
						case 0x59: z80_BIT_bit_reg8(0x08, &regs.C); break;
						case 0x5A: z80_BIT_bit_reg8(0x08, &regs.D); break;
						case 0x5B: z80_BIT_bit_reg8(0x08, &regs.E); break;
						case 0x5C: z80_BIT_bit_reg8(0x08, &regs.H); break;
						case 0x5D: z80_BIT_bit_reg8(0x08, &regs.L); break;
						case 0x5E: z80_BIT_bit_reg8(0x08, &memory[regs.HL]); break;
						case 0x5F: z80_BIT_bit_reg8(0x08, &regs.A); break;
						case 0x60: z80_BIT_bit_reg8(0x10, &regs.B); break;
						case 0x61: z80_BIT_bit_reg8(0x10, &regs.C); break;
						case 0x62: z80_BIT_bit_reg8(0x10, &regs.D); break;
						case 0x63: z80_BIT_bit_reg8(0x10, &regs.E); break;
						case 0x64: z80_BIT_bit_reg8(0x10, &regs.H); break;
						case 0x65: z80_BIT_bit_reg8(0x10, &regs.L); break;
						case 0x66: z80_BIT_bit_reg8(0x10, &memory[regs.HL]); break;
						case 0x67: z80_BIT_bit_reg8(0x10, &regs.A); break;
						case 0x68: z80_BIT_bit_reg8(0x20, &regs.B); break;
						case 0x69: z80_BIT_bit_reg8(0x20, &regs.C); break;
						case 0x6A: z80_BIT_bit_reg8(0x20, &regs.D); break;
						case 0x6B: z80_BIT_bit_reg8(0x20, &regs.E); break;
						case 0x6C: z80_BIT_bit_reg8(0x20, &regs.H); break;
						case 0x6D: z80_BIT_bit_reg8(0x20, &regs.L); break;
						case 0x6E: z80_BIT_bit_reg8(0x20, &memory[regs.HL]); break;
						case 0x6F: z80_BIT_bit_reg8(0x20, &regs.A); break;
						case 0x70: z80_BIT_bit_reg8(0x40, &regs.B); break;
						case 0x71: z80_BIT_bit_reg8(0x40, &regs.C); break;
						case 0x72: z80_BIT_bit_reg8(0x40, &regs.D); break;
						case 0x73: z80_BIT_bit_reg8(0x40, &regs.E); break;
						case 0x74: z80_BIT_bit_reg8(0x40, &regs.H); break;
						case 0x75: z80_BIT_bit_reg8(0x40, &regs.L); break;
						case 0x76: z80_BIT_bit_reg8(0x40, &memory[regs.HL]); break;
						case 0x77: z80_BIT_bit_reg8(0x40, &regs.A); break;
						case 0x78: z80_BIT_bit_reg8(0x80, &regs.B); break;
						case 0x79: z80_BIT_bit_reg8(0x80, &regs.C); break;
						case 0x7A: z80_BIT_bit_reg8(0x80, &regs.D); break;
						case 0x7B: z80_BIT_bit_reg8(0x80, &regs.E); break;
						case 0x7C: z80_BIT_bit_reg8(0x80, &regs.H); break;
						case 0x7D: z80_BIT_bit_reg8(0x80, &regs.L); break;
						case 0x7E: z80_BIT_bit_reg8(0x80, &memory[regs.HL]); break;
						case 0x7F: z80_BIT_bit_reg8(0x80, &regs.A); break;
						case 0x80: z80_RES_bit_reg8(0x01, &regs.B); break;
						case 0x81: z80_RES_bit_reg8(0x01, &regs.C); break;
						case 0x82: z80_RES_bit_reg8(0x01, &regs.D); break;
						case 0x83: z80_RES_bit_reg8(0x01, &regs.E); break;
						case 0x84: z80_RES_bit_reg8(0x01, &regs.H); break;
						case 0x85: z80_RES_bit_reg8(0x01, &regs.L); break;
						case 0x86: z80_RES_bit_reg8(0x01, &memory[regs.HL]); break;
						case 0x87: z80_RES_bit_reg8(0x01, &regs.A); break;
						case 0x88: z80_RES_bit_reg8(0x02, &regs.B); break;
						case 0x89: z80_RES_bit_reg8(0x02, &regs.C); break;
						case 0x8A: z80_RES_bit_reg8(0x02, &regs.D); break;
						case 0x8B: z80_RES_bit_reg8(0x02, &regs.E); break;
						case 0x8C: z80_RES_bit_reg8(0x02, &regs.H); break;
						case 0x8D: z80_RES_bit_reg8(0x02, &regs.L); break;
						case 0x8E: z80_RES_bit_reg8(0x02, &memory[regs.HL]); break;
						case 0x8F: z80_RES_bit_reg8(0x02, &regs.A); break;
						case 0x90: z80_RES_bit_reg8(0x04, &regs.B); break;
						case 0x91: z80_RES_bit_reg8(0x04, &regs.C); break;
						case 0x92: z80_RES_bit_reg8(0x04, &regs.D); break;
						case 0x93: z80_RES_bit_reg8(0x04, &regs.E); break;
						case 0x94: z80_RES_bit_reg8(0x04, &regs.H); break;
						case 0x95: z80_RES_bit_reg8(0x04, &regs.L); break;
						case 0x96: z80_RES_bit_reg8(0x04, &memory[regs.HL]); break;
						case 0x97: z80_RES_bit_reg8(0x04, &regs.A); break;
						case 0x98: z80_RES_bit_reg8(0x08, &regs.B); break;
						case 0x99: z80_RES_bit_reg8(0x08, &regs.C); break;
						case 0x9A: z80_RES_bit_reg8(0x08, &regs.D); break;
						case 0x9B: z80_RES_bit_reg8(0x08, &regs.E); break;
						case 0x9C: z80_RES_bit_reg8(0x08, &regs.H); break;
						case 0x9D: z80_RES_bit_reg8(0x08, &regs.L); break;
						case 0x9E: z80_RES_bit_reg8(0x08, &memory[regs.HL]); break;
						case 0x9F: z80_RES_bit_reg8(0x08, &regs.A); break;
						case 0xA0: z80_RES_bit_reg8(0x10, &regs.B); break;
						case 0xA1: z80_RES_bit_reg8(0x10, &regs.C); break;
						case 0xA2: z80_RES_bit_reg8(0x10, &regs.D); break;
						case 0xA3: z80_RES_bit_reg8(0x10, &regs.E); break;
						case 0xA4: z80_RES_bit_reg8(0x10, &regs.H); break;
						case 0xA5: z80_RES_bit_reg8(0x10, &regs.L); break;
						case 0xA6: z80_RES_bit_reg8(0x10, &memory[regs.HL]); break;
						case 0xA7: z80_RES_bit_reg8(0x10, &regs.A); break;
						case 0xA8: z80_RES_bit_reg8(0x20, &regs.B); break;
						case 0xA9: z80_RES_bit_reg8(0x20, &regs.C); break;
						case 0xAA: z80_RES_bit_reg8(0x20, &regs.D); break;
						case 0xAB: z80_RES_bit_reg8(0x20, &regs.E); break;
						case 0xAC: z80_RES_bit_reg8(0x20, &regs.H); break;
						case 0xAD: z80_RES_bit_reg8(0x20, &regs.L); break;
						case 0xAE: z80_RES_bit_reg8(0x20, &memory[regs.HL]); break;
						case 0xAF: z80_RES_bit_reg8(0x20, &regs.A); break;
						case 0xB0: z80_RES_bit_reg8(0x40, &regs.B); break;
						case 0xB1: z80_RES_bit_reg8(0x40, &regs.C); break;
						case 0xB2: z80_RES_bit_reg8(0x40, &regs.D); break;
						case 0xB3: z80_RES_bit_reg8(0x40, &regs.E); break;
						case 0xB4: z80_RES_bit_reg8(0x40, &regs.H); break;
						case 0xB5: z80_RES_bit_reg8(0x40, &regs.L); break;
						case 0xB6: z80_RES_bit_reg8(0x40, &memory[regs.HL]); break;
						case 0xB7: z80_RES_bit_reg8(0x40, &regs.A); break;
						case 0xB8: z80_RES_bit_reg8(0x80, &regs.B); break;
						case 0xB9: z80_RES_bit_reg8(0x80, &regs.C); break;
						case 0xBA: z80_RES_bit_reg8(0x80, &regs.D); break;
						case 0xBB: z80_RES_bit_reg8(0x80, &regs.E); break;
						case 0xBC: z80_RES_bit_reg8(0x80, &regs.H); break;
						case 0xBD: z80_RES_bit_reg8(0x80, &regs.L); break;
						case 0xBE: z80_RES_bit_reg8(0x80, &memory[regs.HL]); break;
						case 0xBF: z80_RES_bit_reg8(0x80, &regs.A); break;
						case 0xC0: z80_SET_bit_reg8(0x01, &regs.B); break;
						case 0xC1: z80_SET_bit_reg8(0x01, &regs.C); break;
						case 0xC2: z80_SET_bit_reg8(0x01, &regs.D); break;
						case 0xC3: z80_SET_bit_reg8(0x01, &regs.E); break;
						case 0xC4: z80_SET_bit_reg8(0x01, &regs.H); break;
						case 0xC5: z80_SET_bit_reg8(0x01, &regs.L); break;
						case 0xC6: z80_SET_bit_reg8(0x01, &memory[regs.HL]); break;
						case 0xC7: z80_SET_bit_reg8(0x01, &regs.A); break;
						case 0xC8: z80_SET_bit_reg8(0x02, &regs.B); break;
						case 0xC9: z80_SET_bit_reg8(0x02, &regs.C); break;
						case 0xCA: z80_SET_bit_reg8(0x02, &regs.D); break;
						case 0xCB: z80_SET_bit_reg8(0x02, &regs.E); break;
						case 0xCC: z80_SET_bit_reg8(0x02, &regs.H); break;
						case 0xCD: z80_SET_bit_reg8(0x02, &regs.L); break;
						case 0xCE: z80_SET_bit_reg8(0x02, &memory[regs.HL]); break;
						case 0xCF: z80_SET_bit_reg8(0x02, &regs.A); break;
						case 0xD0: z80_SET_bit_reg8(0x04, &regs.B); break;
						case 0xD1: z80_SET_bit_reg8(0x04, &regs.C); break;
						case 0xD2: z80_SET_bit_reg8(0x04, &regs.D); break;
						case 0xD3: z80_SET_bit_reg8(0x04, &regs.E); break;
						case 0xD4: z80_SET_bit_reg8(0x04, &regs.H); break;
						case 0xD5: z80_SET_bit_reg8(0x04, &regs.L); break;
						case 0xD6: z80_SET_bit_reg8(0x04, &memory[regs.HL]); break;
						case 0xD7: z80_SET_bit_reg8(0x04, &regs.A); break;
						case 0xD8: z80_SET_bit_reg8(0x08, &regs.B); break;
						case 0xD9: z80_SET_bit_reg8(0x08, &regs.C); break;
						case 0xDA: z80_SET_bit_reg8(0x08, &regs.D); break;
						case 0xDB: z80_SET_bit_reg8(0x08, &regs.E); break;
						case 0xDC: z80_SET_bit_reg8(0x08, &regs.H); break;
						case 0xDD: z80_SET_bit_reg8(0x08, &regs.L); break;
						case 0xDE: z80_SET_bit_reg8(0x08, &memory[regs.HL]); break;
						case 0xDF: z80_SET_bit_reg8(0x08, &regs.A); break;
						case 0xE0: z80_SET_bit_reg8(0x10, &regs.B); break;
						case 0xE1: z80_SET_bit_reg8(0x10, &regs.C); break;
						case 0xE2: z80_SET_bit_reg8(0x10, &regs.D); break;
						case 0xE3: z80_SET_bit_reg8(0x10, &regs.E); break;
						case 0xE4: z80_SET_bit_reg8(0x10, &regs.H); break;
						case 0xE5: z80_SET_bit_reg8(0x10, &regs.L); break;
						case 0xE6: z80_SET_bit_reg8(0x10, &memory[regs.HL]); break;
						case 0xE7: z80_SET_bit_reg8(0x10, &regs.A); break;
						case 0xE8: z80_SET_bit_reg8(0x20, &regs.B); break;
						case 0xE9: z80_SET_bit_reg8(0x20, &regs.C); break;
						case 0xEA: z80_SET_bit_reg8(0x20, &regs.D); break;
						case 0xEB: z80_SET_bit_reg8(0x20, &regs.E); break;
						case 0xEC: z80_SET_bit_reg8(0x20, &regs.H); break;
						case 0xED: z80_SET_bit_reg8(0x20, &regs.L); break;
						case 0xEE: z80_SET_bit_reg8(0x20, &memory[regs.HL]); break;
						case 0xEF: z80_SET_bit_reg8(0x20, &regs.A); break;
						case 0xF0: z80_SET_bit_reg8(0x40, &regs.B); break;
						case 0xF1: z80_SET_bit_reg8(0x40, &regs.C); break;
						case 0xF2: z80_SET_bit_reg8(0x40, &regs.D); break;
						case 0xF3: z80_SET_bit_reg8(0x40, &regs.E); break;
						case 0xF4: z80_SET_bit_reg8(0x40, &regs.H); break;
						case 0xF5: z80_SET_bit_reg8(0x40, &regs.L); break;
						case 0xF6: z80_SET_bit_reg8(0x40, &memory[regs.HL]); break;
						case 0xF7: z80_SET_bit_reg8(0x40, &regs.A); break;
						case 0xF8: z80_SET_bit_reg8(0x80, &regs.B); break;
						case 0xF9: z80_SET_bit_reg8(0x80, &regs.C); break;
						case 0xFA: z80_SET_bit_reg8(0x80, &regs.D); break;
						case 0xFB: z80_SET_bit_reg8(0x80, &regs.E); break;
						case 0xFC: z80_SET_bit_reg8(0x80, &regs.H); break;
						case 0xFD: z80_SET_bit_reg8(0x80, &regs.L); break;
						case 0xFE: z80_SET_bit_reg8(0x80, &memory[regs.HL]); break;
						case 0xFF: z80_SET_bit_reg8(0x80, &regs.A); break;
						default:
							{
								sprintf(errorText, "Error in 0xCB instruction at %x!  Opcode number is %x", regs.PC, opcode);
								MessageBox(NULL, errorText, "Error!", MB_OK);
							}
						break;
						}
					}
				break;
				case 0xCC: z80_CALL_condition_immediate(0x02); break;
				case 0xCD: z80_CALL_immediate(); break;
				case 0xCE: z80_ADC_A_immediate(); break;
				case 0xCF: z80_RST(0x08); break;
				case 0xD0: z80_RET_condition(0x03); break;
				case 0xD1: z80_POP_reg16(&regs.D, &regs.E); break;
				case 0xD2: z80_JP_condition_immediate(0x03); break;
				//case 0xD3:  Maybe doesn't exist?
				case 0xD4: z80_CALL_condition_immediate(0x03); break;
				case 0xD5: z80_PUSH_reg16(&regs.D, &regs.E); break;
				case 0xD6: z80_SUB_immediate(); break;
				case 0xD7: z80_RST(0x10); break;
				case 0xD8: z80_RET_condition(0x04); break;
				case 0xD9: z80_RETI(); break;
				case 0xDA: z80_JP_condition_immediate(0x04); break;
				//case 0xDB:  Maybe doesn't exist?
				case 0xDC: z80_CALL_condition_immediate(0x04); break;
				//case 0xDD:  Maybe doesn't exist?
				case 0xDE: z80_SBC_A_immediate(); break;
				case 0xDF: z80_RST(0x18); break;
				case 0xE0: z80_LD_0xFF00_immediate_A(); break;
				case 0xE1: z80_POP_reg16(&regs.H, &regs.L); break;
				case 0xE2: z80_LD_0xFF00_C_A(); break;
				//case 0xE3:  Maybe doesn't exist?
				//case 0xE4:  Maybe doesn't exist?
				case 0xE5: z80_PUSH_reg16(&regs.H, &regs.L); break;
				case 0xE6: z80_AND_immediate(); break;
				case 0xE7: z80_RST(0x20); break;
				case 0xE8: z80_ADD_SP_immediate(); break;
				case 0xE9: z80_JP_immediate(); break;
				case 0xEA: z80_LD_location_immediate_A(); break;
				//case 0xEB:  Maybe doesn't exist?
				//case 0xEC:  Maybe doesn't exist?
				//case 0xED:  Maybe doesn't exist?
				case 0xEE: z80_XOR_immediate(); break;
				case 0xEF: z80_RST(0x28); break;
				case 0xF0: z80_LD_A_0xFF00_immediate(); break;
				case 0xF1: z80_POP_reg16(&regs.A, &regs.F); break;
				case 0xF2: z80_LD_A_0xFF00_C(); break;
				case 0xF3: z80_DI(); break;
				//case 0xF4:  Maybe doesn't exist?
				case 0xF5: z80_PUSH_reg16(&regs.A, &regs.F); break;
				case 0xF6: z80_OR_immediate(); break;
				case 0xF7: z80_RST(0x30); break;
				case 0xF8: z80_LD_HL_SP_immediate(); break;
				case 0xF9: z80_LD_SP_HL(); break;
				case 0xFA: z80_LD_A_location_immediate(); break;
				case 0xFB: z80_EI(); break;
				//case 0xFC:  Maybe doesn't exist?				//case 0xFD:  Maybe doesn't exist?
				case 0xFE: z80_CP_immediate(); break;
				case 0xFF: z80_RST(0x38); break;
				default:
					{
						sprintf(errorText, "Error in instruction at %x!  Opcode number is %x.", regs.PC, opcode);
						MessageBox(NULL, errorText, "Error!", MB_OK);
					}
				break;
			}
		}
	}

	//----------------------------------------//
	// Update the counters.                   //
	//----------------------------------------//
	if (halted == 1)
		cycles += 6;
	if (stopped == 1)
		cycles += 6;

	if (IOregister_LCDC & BIT_7)
	{
		retraceCounter -= cycles;
		LYCounter -= cycles;
	}

	if (interruptLatencyCounter > 0)
	{
		interruptLatencyCounter--;

		if (interruptLatencyCounter == 0)
		{
			interruptMasterEnable = 1;
		}
	}

	HandleInterrupts();

	dividerCounter -= cycles;

	if (IOregister_TAC & BIT_2)
	{
		timerCounter -= cycles;
	}

	/*if (OAMTransferEnabled == 1)
	{
		OAMCounter -= cycles;

		if (OAMCounter <= 0)
		{
			OAMTransferEnabled = 0;
		}
	}*/

	UpdateIORegisters();

	//Get pressed keyboard keys.
	GetKeys();
	UpdateJoypadIORegister();
	UpdateStatusIORegister();
	
	if (IOregister_IF > 0)
		interruptLatencyCounter -= cycles;
	if (interruptLatencyCounter < 0)
		interruptLatencyCounter = 0;

	//----------------------------------------//
	// See if an interrupt was requested      //
	// before calling the handle function.    //
	//----------------------------------------//
	if ((IOregister_IF > 0) && (interruptLatencyCounter == 0))
	{
		interruptLatencyCounter = 7;
	}

	cycleTotal += cycles;

	//----------------------------------------//
	// Reset the cycle counter.               //
	//----------------------------------------//
	cycles = 0;

	if (logging == 1)
		WriteToLog();
}

cycleTotal -= cyclesToRun;

/*IOregister_LY = 0;
modeFlag = 2;
retraceCounter = 20
WYTemp = IOregister_WY;*/

//aud.EndFrame();

// This keeps the emulation as close to normal speed as possible.
if (SpeedKey == 0)
{
	if (FPSLimit == 1)
	{
		while ((SDL_GetTicks() - emulationTimer) < 15) {}
	}
}

FPS++;
UpdateScreen();

emulationTimer = SDL_GetTicks();

}

}

//----------------------------------------//
// This function will check if an         //
// interrupt has occured, and take the    //
// appropriate action.                    //
//----------------------------------------//
void HandleInterrupts()
{
	if (interruptMasterEnable == 1)
	{
		if ((IOregister_IE & BIT_0) && (IOregister_IF & BIT_0))
		{
			interruptMasterEnable = 0;
			halted = 0;
			z80_RST(0x40);
			IOregister_IF &= 254;
		}
		else if ((IOregister_IE & BIT_1) && (IOregister_IF & BIT_1))
		{
			interruptMasterEnable = 0;
			halted = 0;
			z80_RST(0x48);
			IOregister_IF &= 253;
		}
		else if ((IOregister_IE & BIT_2) && (IOregister_IF & BIT_2))	
		{
			interruptMasterEnable = 0;
			halted = 0;
			z80_RST(0x50);
			IOregister_IF &= 251;
		}
		else if ((IOregister_IE & BIT_3) && (IOregister_IF & BIT_3))
		{
			interruptMasterEnable = 0;
			halted = 0;
			z80_RST(0x58);
			IOregister_IF &= 247;
		}
		else if ((IOregister_IE & BIT_4) && (IOregister_IF & BIT_4))
		{
			interruptMasterEnable = 0;
			stopped = 0;
			halted = 0;
			z80_RST(0x60);
			IOregister_IF &= 239;
		}
	}
}

void UpdateJoypadIORegister()
{			
	// If P14 was turned on
	if ((IOregister_P1 & BIT_4) == 0)
	{
		// If Down was pressed.
		if (joyState[0] == 1)
			IOregister_P1 &= 247;
		else
			IOregister_P1 |= BIT_3;

		// If Up was pressed.
		if (joyState[1] == 1)
			IOregister_P1 &= 251;
		else
			IOregister_P1 |= BIT_2;

		// If Left was pressed.
		if (joyState[2] == 1)
			IOregister_P1 &= 253;
		else
			IOregister_P1 |= BIT_1;

		// If Right was pressed.
		if (joyState[3] == 1)
			IOregister_P1 &= 254;
		else
			IOregister_P1 |= BIT_0;
	}

	// If P15 was turned on.
	else if ((IOregister_P1 & BIT_5) == 0)
	{
		// If Start was pressed.
		if (joyState[4] == 1)
			IOregister_P1 &= 247;
		else
			IOregister_P1 |= BIT_3;

		// If Select was pressed.
		if (joyState[5] == 1)
			IOregister_P1 &= 251;
		else
			IOregister_P1 |= BIT_2;

		// If B was pressed.
		if (joyState[6] == 1)
			IOregister_P1 &= 253;
		else
			IOregister_P1 |= BIT_1;

		// If A was pressed.
		if (joyState[7] == 1)
			IOregister_P1 &= 254;
		else
			IOregister_P1 |= BIT_0;
	}


	// If a button was pressed, turn off STOP and generate an interrupt.
	if ((IOregister_P1 & 0x0F) < 0x0F)
	{
		stopped = 0;
		IOregister_IF |= BIT_4;
	}
}

void UpdateStatusIORegister()
{
	//----------------------------------------//
	// Determine the current state of the LCD //
	// and set the mode flag accordingly.     //
	//----------------------------------------//
	if (IOregister_LCDC & BIT_7)
	{
		if (retraceCounter <= 0)
		{
			//if (IOregister_LY < 144)
			{
				switch(modeFlag)
				{
					// Go here if STAT is in mode 0 (H-Blank).
					case 3:
					{
						modeFlag = 0;

						if (IOregister_LY < 144)
						{
							if ((IOregister_STAT & BIT_3) && (LCDCInterruptExecuted == 0))
							{
								IOregister_IF |= BIT_1;
								LCDCInterruptExecuted = 1;
							}
						}

						retraceCounter += 51;
					}
					break;
					// Go here if STAT is in mode 2 (OAM Search).
					case 0:
					{
						modeFlag = 2;

						IOregister_LY++;
						
						LCDCInterruptExecuted = 0;

						CheckLYC();

						// See if it's time for V-Blank.
						if ((IOregister_LY == 144))
						{
							modeFlag = 1;
											
							IOregister_IF |= BIT_0;

							retraceCounter += 114;
						}
						else//if (IOregister_LY < 144)
						{
							if (IOregister_STAT & BIT_5)
							{
								IOregister_IF |= BIT_1;
								LCDCInterruptExecuted = 1;
							}
						
							retraceCounter += 20;
						}
					}
					break;
					case 1:
					{
						IOregister_LY++;

						if (IOregister_LY == 154)
						{
							IOregister_LY = 0;
							modeFlag = 2;

							if (IOregister_LY < 144)
							{
								if ((IOregister_STAT & BIT_3) && (LCDCInterruptExecuted == 0))
								{
									IOregister_IF |= BIT_1;
									LCDCInterruptExecuted = 1;
								}
							}

							//retraceCounter += 51;
							
							tilePosYWin = 0;

							// Set the WYTemp variable to the current value of WY.
							WYTemp = IOregister_WY;							
						}
						
						if (IOregister_LY == 153)
						{
							retraceCounter += 20;
						}
						else
						{
							retraceCounter += 114;
						}

						CheckLYC();
					}
					break;
					// Go here if STAT is in mode 3 (OAM Transfer).
					case 2:
					{
						modeFlag = 3;

						DrawScanline();
						//x = 0;

						//FillBackgroundBuffer();

						retraceCounter += 43;
					}
					break;
				}
			}
		}
		
		IOregister_STAT &= 252;
		IOregister_STAT |= modeFlag;
	}
	else
	{
		IOregister_STAT &= 252;
	}
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
		{
			IOregister_IF |= BIT_1;
		}
	}
	else
	{
		IOregister_STAT &= 251;
	}
}

//----------------------------------------//
// This function will update the I/O      //
// registers.                             //
//----------------------------------------//
void UpdateIORegisters()
{
	if (dividerCounter <= 0)
	{
		IOregister_DIV++;

		dividerCounter += DIVIDER_INTERVAL;
	}

	//----------------------------------------//
	// If the timer counter has expired,      //
	// increment the TIMA register.           //
	//----------------------------------------//
	if ((timerCounter <= 0) && (IOregister_TAC & BIT_2))
	{ 
		//----------------------------------------//
		// This generates an interrupt when it    //
		// exceeds 255.                           //
		//----------------------------------------//
		IOregister_TIMA++;

		if (IOregister_TIMA == 0)
		{
			IOregister_IF |= BIT_2;
			//----------------------------------------//
			// This loads TIMA with TMA.              //
			// Not quite sure if this is correct yet. //
			//----------------------------------------//
			IOregister_TIMA = IOregister_TMA;
		}

		//----------------------------------------//
		// Add the timer interval to the timer    //
		// counter.                               //
		//----------------------------------------//
		timerCounter += timerInterval;
	}
}

void DrawScanline()
{
	//----------------------------------------//
	// Make sure the LCD is active before     //
	// going any further.                     //
	//----------------------------------------//
	if (IOregister_LCDC & BIT_7)
	{
		if (IOregister_LCDC & BIT_6)
			WindowMapData = 0x9C00;
		else
			WindowMapData = 0x9800;
		//----------------------------------------//
		// This decides where to get background   //
		// tile patterns from.                    //
		//----------------------------------------//
		if (IOregister_LCDC & BIT_4)
			BGTileData = 0x8000;
		else
			// Technically should be 0x8800, but this
			// makes addressing simpler.
			BGTileData = 0x9000;

		//----------------------------------------//
		// This decides where to get tile numbers //
		// to be displayed from.                  //
		//----------------------------------------//
		if (IOregister_LCDC & BIT_3)
			BGMapData = 0x9C00;
		else
			BGMapData = 0x9800;

		//----------------------------------------//
		// This decides the height of the tile    //
		// being drawn.                           //
		//----------------------------------------//
		if (IOregister_LCDC & BIT_2)
			SpriteHeight = 16;
		else
			SpriteHeight = 8;

		x = 0;
		y = IOregister_LY;

		scrollX = IOregister_SCX;
		scrollY = (y + IOregister_SCY);

		tilePosXBack = scrollX >> 3;
		tilePosYBack = (scrollY >> 3) << 5;

		borderXBack = 1 << (7 - (scrollX & 7));
		borderYBack = scrollY & 7;

		if (BGTileData == 0x8000)
			tileNumber = memory[BGMapData + tilePosYBack + tilePosXBack];
		else
			tileNumber = (signed char) memory[BGMapData + tilePosYBack + tilePosXBack];

		//----------------------------------------//
		// Make sure the background is enabled    //
		// before drawing.                        //
		//----------------------------------------//
		if (IOregister_LCDC & BIT_0)
		{
			//----------------------------------------//
			// Draw the background into the screen    //
			// buffer.  These are always 8 x 8.       //
			//----------------------------------------//	
			while (x < 160)
			{
				data1 = memory[BGTileData + (tileNumber << 4) + (borderYBack << 1)];
				data2 = memory[BGTileData + (tileNumber << 4) + (borderYBack << 1) + 1];

				while (borderXBack > 0)
				{
					color = (data1 & borderXBack) ? 1 : 0;
					color += (data2 & borderXBack) ? 2 : 0;
						
					if (color == 3)
						screenData[((y << 7) + (y << 5)) + x] = ((IOregister_BGP & BIT_7) >> 6) + ((IOregister_BGP & BIT_6) >> 6);
					if (color == 2)
						screenData[((y << 7) + (y << 5)) + x] = ((IOregister_BGP & BIT_5) >> 4) + ((IOregister_BGP & BIT_4) >> 4);
					if (color == 1)
						screenData[((y << 7) + (y << 5)) + x] = ((IOregister_BGP & BIT_3) >> 2) + ((IOregister_BGP & BIT_2) >> 2);
					if (color == 0)
						screenData[((y << 7) + (y << 5)) + x] = ((IOregister_BGP & BIT_1)) + ((IOregister_BGP & BIT_0));

					bgDataBuffer[x] = color;
					
					x++;

					if (x >= 160)
						break;

					borderXBack >>= 1;
				}
				
				tilePosXBack++;

				if (tilePosXBack == 32)
					tilePosXBack = 0;

				borderXBack = 128;

				if (BGTileData == 0x8000)
					tileNumber = memory[BGMapData + tilePosYBack + tilePosXBack];
				else
					tileNumber = (signed char) memory[BGMapData + tilePosYBack + tilePosXBack];
			}
		}
		else
		{
			memset(&screenData[((IOregister_LY << 7) + (IOregister_LY << 5))], 0, 160);
		}

		WinX = IOregister_WX - 7;
		if (WinX < 0)
			WinX = 0;

		WinY = IOregister_LY;

		tilePosXWin = 0;
		//if (IOregister_LY >= IOregister_WY)
		//	tilePosYWin = (IOregister_LY - IOregister_WY) >> 3;
		
		borderXWin = 128;

		if (BGTileData == 0x8000)
			tileNumber = memory[WindowMapData + (tilePosYWin << 5) + tilePosXWin];
		else
			tileNumber = (signed char) memory[WindowMapData + (tilePosYWin << 5) + tilePosXWin];

		//----------------------------------------//
		// Make sure the window is enabled        //
		// before drawing.                        //
		//----------------------------------------//
		if (IOregister_LCDC & BIT_5)
		{
			if ((WinY >= WYTemp) && (WinX < 160) && (WinX >= 0))
			{
				borderYWin = (WinY - WYTemp) & 7;
				
				//----------------------------------------//
				// Draw the window into the screen        //
				// buffer.  These are always 8 x 8.       //
				//----------------------------------------//	
				while (WinX < 160)
				{
					data1 = memory[BGTileData + (tileNumber << 4) + (borderYWin << 1)];
					data2 = memory[BGTileData + (tileNumber << 4) + (borderYWin << 1) + 1];

					while (borderXWin > 0)
					{
						color = (data1 & borderXWin) ? 1 : 0;
						color += (data2 & borderXWin) ? 2 : 0;

						if (color == 3)
							screenData[((WinY << 7) + (WinY << 5)) + WinX] = ((IOregister_BGP & BIT_7) >> 6) + ((IOregister_BGP & BIT_6) >> 6);
						if (color == 2)
							screenData[((WinY << 7) + (WinY << 5)) + WinX] = ((IOregister_BGP & BIT_5) >> 4) + ((IOregister_BGP & BIT_4) >> 4);
						if (color == 1)
							screenData[((WinY << 7) + (WinY << 5)) + WinX] = ((IOregister_BGP & BIT_3) >> 2) + ((IOregister_BGP & BIT_2) >> 2);
						if (color == 0)
							screenData[((WinY << 7) + (WinY << 5)) + WinX] = ((IOregister_BGP & BIT_1)) + ((IOregister_BGP & BIT_0));

						bgDataBuffer[WinX] = color;

						WinX++;

						if (WinX >= 160)
							break;

						borderXWin >>= 1;
					}
					
					tilePosXWin++;

					if (tilePosXWin == 20)
						tilePosXWin = 0;

					borderXWin = 128;

					if (BGTileData == 0x8000)
						tileNumber = memory[WindowMapData + (tilePosYWin << 5) + tilePosXWin];
					else
						tileNumber = (signed char) memory[WindowMapData + (tilePosYWin << 5) + tilePosXWin];
				}
				if (borderYWin == 7)
					tilePosYWin++;
			}
		}

		//----------------------------------------//
		// Draw sprites into the screen buffer.   //
		//----------------------------------------//
		if (IOregister_LCDC & BIT_1)
		{
		for (sprites = 39; sprites >= 0; sprites--)
		{
			y = memory[0xFE00 + (sprites << 2)] - 16;
			if ((IOregister_LY >= y) && (IOregister_LY < (y + SpriteHeight)))
			{
				x = memory[0xFE00 + (sprites << 2) + 1] - 8;
				if (SpriteHeight == 16)
					tileNumber = (memory[0xFE00 + (sprites << 2) + 2]) & 254;
				else
					tileNumber = memory[0xFE00 + (sprites << 2) + 2];
				
				spritePalette = 0xFF48 + ((memory[0xFE00 + (sprites << 2) + 3] & 16) >> 4);
				
				// If the Y-Flip bit is on, invert for the tile data index.
				if (memory[0xFE00 + (sprites << 2) + 3] & 64)
				{
					data1 = memory[0x8000 + (tileNumber << 4) + (((memory[0xFF44] - y) ^ (SpriteHeight - 1)) << 1)];
					data2 = memory[0x8000 + (tileNumber << 4) + (((memory[0xFF44] - y) ^ (SpriteHeight - 1)) << 1) + 1];
				}
				else
				{
					data1 = memory[0x8000 + (tileNumber << 4) + ((memory[0xFF44] - y) << 1)];
					data2 = memory[0x8000 + (tileNumber << 4) + ((memory[0xFF44] - y) << 1) + 1];
				}

				plotY = IOregister_LY;
				
				for (i = 0; i < 8; i++)
				{
					plotX = x + i;

					if ((plotX >= 0) && (plotX < 160))
					{
						// If X-Flip bit is on, invert i for the pixel check.
						if (memory[0xFE00 + (sprites << 2) + 3] & BIT_5)
							i ^= 7;

						// Check the sprite priority.
						if (memory[0xFE00 + (sprites << 2) + 3] & BIT_7)
						{
							if(bgDataBuffer[plotX] == 0)
							{
								// Determine the color of the pixel.
								if (((data1 & (0x80 >> i)) != 0) && (data2 & (0x80 >> i)) != 0)
									screenData[((plotY << 7) + (plotY << 5)) + plotX] = ((memory[spritePalette] & 128) >> 6) + ((memory[spritePalette] & 64) >> 6);
								if (((data1 & (0x80 >> i)) == 0) && (data2 & (0x80 >> i)) != 0)
									screenData[((plotY << 7) + (plotY << 5)) + plotX] = ((memory[spritePalette] & 32) >> 4) + ((memory[spritePalette] & 16) >> 4);
								if (((data1 & (0x80 >> i)) != 0) && (data2 & (0x80 >> i)) == 0)
									screenData[((plotY << 7) + (plotY << 5)) + plotX] = ((memory[spritePalette] & 8) >> 2) + ((memory[spritePalette] & 4) >> 2);
								if (((data1 & (0x80 >> i)) == 0) && (data2 & (0x80 >> i)) == 0)
								{}	//screenBuffer[(plotY * 160) + plotX] = ((memory[spritePalette] & 2)) + ((memory[spritePalette] & 1));
							}
						}
						else
						{
							// Determine the color of the pixel.
							if (((data1 & (0x80 >> i)) != 0) && (data2 & (0x80 >> i)) != 0)
								screenData[((plotY << 7) + (plotY << 5)) + plotX] = ((memory[spritePalette] & 128) >> 6) + ((memory[spritePalette] & 64) >> 6);
							if (((data1 & (0x80 >> i)) == 0) && (data2 & (0x80 >> i)) != 0)
								screenData[((plotY << 7) + (plotY << 5)) + plotX] = ((memory[spritePalette] & 32) >> 4) + ((memory[spritePalette] & 16) >> 4);
							if (((data1 & (0x80 >> i)) != 0) && (data2 & (0x80 >> i)) == 0)
								screenData[((plotY << 7) + (plotY << 5)) + plotX] = ((memory[spritePalette] & 8) >> 2) + ((memory[spritePalette] & 4) >> 2);
							if (((data1 & (0x80 >> i)) == 0) && (data2 & (0x80 >> i)) == 0)
							{}	//screenBuffer[(plotY * 160) + plotX] = ((memory[spritePalette] & 2)) + ((memory[spritePalette] & 1));
						}
						
						// If i was inverted for the pixel check,
						// return it to its original value.
						if (memory[0xFE00 + (sprites << 2) + 3] & BIT_5)
							i ^= 7;
					}
				}
			}
		}
		}
	}
}

//----------------------------------------//
// This function writes to the log file.  //
//----------------------------------------//
void WriteToLog()
{
	fprintf(handle, "PC = 0x%x  ", regs.PC);
	fprintf(handle, "SP = 0x%x  ", regs.SP);
	fprintf(handle, "AF = 0x%x  ", regs.AF);
	fprintf(handle, "BC = 0x%x  ", regs.BC);
	fprintf(handle, "DE = 0x%x  ", regs.DE);
	fprintf(handle, "HL = 0x%x  ", regs.HL);
	fprintf(handle, "Interrupts = %x  ", interruptMasterEnable);
	fprintf(handle, "Opcode = 0x%x ", opcode);
	fprintf(handle, "\n");
}