/*#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "Win32/resource.h"
#include "Define.h"
#include "Common.h"

extern unsigned char *memory;

CALLBACK UpdateDebugger(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void ShowDebugger();
void SetupDebugger();
void DisassembleLine(unsigned int address, char *data);

char opcodeDescription[256][32];
char opcodeCBDescription[256][32];
unsigned char opcodeBytes[256] = {1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
								  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
								  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
								  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								  1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,
								  1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1,
								  2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
								  2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1};

HWND hDlg;

CALLBACK UpdateDebugger(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char data[256];
	unsigned long convertedString = "";
	unsigned int address;

	//----------------------------------------//
	// Handle messages sent to the debugger   //
	//----------------------------------------//
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			//----------------------------------------//
			// This will fill in the register		  //
			// text box values.						  //
			//----------------------------------------//
			sprintf(data, "0x%04X", emu.cpu.regs.AF);
			SetWindowText(GetDlgItem(hDlg, IDD_REGISTER_AF_EDIT), data);
			
			sprintf(data, "0x%04X", emu.cpu.regs.BC);
			SetWindowText(GetDlgItem(hDlg, IDD_REGISTER_BC_EDIT), data);
			
			sprintf(data, "0x%04X", emu.cpu.regs.DE);
			SetWindowText(GetDlgItem(hDlg, IDD_REGISTER_DE_EDIT), data);
			
			sprintf(data, "0x%04X", emu.cpu.regs.HL);
			SetWindowText(GetDlgItem(hDlg, IDD_REGISTER_HL_EDIT), data);
			
			sprintf(data, "0x%04X", emu.cpu.regs.SP);
			SetWindowText(GetDlgItem(hDlg, IDD_REGISTER_SP_EDIT), data);
			
			sprintf(data, "0x%04X", emu.cpu.regs.PC);
			SetWindowText(GetDlgItem(hDlg, IDD_REGISTER_PC_EDIT), data);
			//----------------------------------------//

			//----------------------------------------//
			// This will fill in the IO register	  //
			// text box values.						  //
			//----------------------------------------//
			sprintf(data, "0x%02X", IOregister_P1);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_P1_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_SB);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_SB_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_SC);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_SC_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_DIV);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_DIV_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_TIMA);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_TIMA_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_TMA);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_TMA_EDIT), data);

			sprintf(data, "0x%02X", IOregister_TAC);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_TAC_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_IF);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_IF_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_LCDC);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_LCDC_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_STAT);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_STAT_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_SCY);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_SCY_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_SCX);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_SCX_EDIT), data);

			sprintf(data, "0x%02X", IOregister_LY);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_LY_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_LYC);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_LYC_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_DMA);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_DMA_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_BGP);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_BGP_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_OBP0);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_OBP0_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_OBP1);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_OBP1_EDIT), data);

			sprintf(data, "0x%02X", IOregister_WY);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_WY_EDIT), data);
			
			sprintf(data, "0x%02X", IOregister_WX);
			SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_WX_EDIT), data);
			
			//sprintf(data, "0x%02X", IOregister_IE);
			//SetWindowText(GetDlgItem(hDlg, IDD_IOREGISTER_IE_EDIT), data);
			//----------------------------------------//
			for (address = 0; address < 0x10000; address++)
			{
				DisassembleLine(address, data);
				SendDlgItemMessage(hDlg, IDD_LISTBOX_DEBUGGER_CODE, LB_ADDSTRING, 0, (LPARAM)data);
			}
		}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDD_APPLYREGISTERCHANGES:
			{
				// Gets the entered value for register AF
				GetWindowText(GetDlgItem(hDlg, IDD_REGISTER_AF_EDIT), data, 256);
				if ((convertedString,strtol(data, NULL, 0)) > 0xFFFF)
					MessageBox(hDlg, "Register AF contains invalid data!", "Error", MB_OK);
				else
					emu.cpu.regs.AF,(unsigned short int)convertedString;
				
				// Gets the entered value for register BC
				GetWindowText(GetDlgItem(hDlg, IDD_REGISTER_BC_EDIT), data, 256);
				if ((convertedString,strtol(data, NULL, 0)) > 0xFFFF)
					MessageBox(hDlg, "Register BC contains invalid data!", "Error", MB_OK);
				else
					emu.cpu.regs.BC,(unsigned short int)convertedString;
				
				// Gets the entered value for register DE
				GetWindowText(GetDlgItem(hDlg, IDD_REGISTER_DE_EDIT), data, 256);
				if ((convertedString,strtol(data, NULL, 0)) > 0xFFFF)
					MessageBox(hDlg, "Register DE contains invalid data!", "Error", MB_OK);
				else
					emu.cpu.regs.DE,(unsigned short int)convertedString;
				
				// Gets the entered value for register HL
				GetWindowText(GetDlgItem(hDlg, IDD_REGISTER_HL_EDIT), data, 256);
				if ((convertedString,strtol(data, NULL, 0)) > 0xFFFF)
					MessageBox(hDlg, "Register HL contains invalid data!", "Error", MB_OK);
				else
					emu.cpu.regs.HL,(unsigned short int)convertedString;
				
				// Gets the entered value for register SP
				GetWindowText(GetDlgItem(hDlg, IDD_REGISTER_SP_EDIT), data, 256);
				if ((convertedString,strtol(data, NULL, 0)) > 0xFFFF)
					MessageBox(hDlg, "Register SP contains invalid data!", "Error", MB_OK);
				else
					emu.cpu.regs.SP,(unsigned short int)convertedString;
				
				// Gets the entered value for register PC
				GetWindowText(GetDlgItem(hDlg, IDD_REGISTER_PC_EDIT), data, 256);
				if ((convertedString,strtol(data, NULL, 0)) > 0xFFFF)
					MessageBox(hDlg, "Register PC contains invalid data!", "Error", MB_OK);
				else
					emu.cpu.regs.PC,(unsigned short int)convertedString;
			}
		break;
		}
	break;
	case WM_CLOSE:
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
	break;
	}
	//------------------------------------------------------------//

	return 0;
}
void ShowDebugger()
{
	DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DEBUGGER), hDlg, UpdateDebugger, NULL);
}

void SetupDebugger()
{
	sprintf(opcodeDescription[0x00],"NOP");
	sprintf(opcodeDescription[0x01],"LD BC, $####");
	sprintf(opcodeDescription[0x02],"LD (BC), A");
	sprintf(opcodeDescription[0x03],"INC BC");
	sprintf(opcodeDescription[0x04],"INC B");
	sprintf(opcodeDescription[0x05],"DEC B");
	sprintf(opcodeDescription[0x06],"LD B, $##");
	sprintf(opcodeDescription[0x07],"RLCA");
	sprintf(opcodeDescription[0x08],"LD ($####), SP");
	sprintf(opcodeDescription[0x09],"ADD HL, BC");
	sprintf(opcodeDescription[0x0A],"LD A, (BC)");
	sprintf(opcodeDescription[0x0B],"DEC BC");
	sprintf(opcodeDescription[0x0C],"INC C");
	sprintf(opcodeDescription[0x0D],"DEC C");
	sprintf(opcodeDescription[0x0E],"LD C, $##");
	sprintf(opcodeDescription[0x0F],"RRCA");
	sprintf(opcodeDescription[0x10],"STOP");
	sprintf(opcodeDescription[0x11],"LD DE, $####");
	sprintf(opcodeDescription[0x12],"LD (DE), A");
	sprintf(opcodeDescription[0x13],"INC DE");
	sprintf(opcodeDescription[0x14],"INC D");
	sprintf(opcodeDescription[0x15],"DEC D");
	sprintf(opcodeDescription[0x16],"LD D, $##");
	sprintf(opcodeDescription[0x17],"RLA");
	sprintf(opcodeDescription[0x18],"JR $****");
	sprintf(opcodeDescription[0x19],"ADD HL, DE");
	sprintf(opcodeDescription[0x1A],"LD A, (DE)");
	sprintf(opcodeDescription[0x1B],"DEC DE");
	sprintf(opcodeDescription[0x1C],"INC E");
	sprintf(opcodeDescription[0x1D],"DEC E");
	sprintf(opcodeDescription[0x1E],"LD E, $##");
	sprintf(opcodeDescription[0x1F],"RRA");
	sprintf(opcodeDescription[0x20],"JR NZ, $****");
	sprintf(opcodeDescription[0x21],"LD HL, $####");
	sprintf(opcodeDescription[0x22],"LDI (HL), A");
	sprintf(opcodeDescription[0x23],"INC HL");
	sprintf(opcodeDescription[0x24],"INC H");
	sprintf(opcodeDescription[0x25],"DEC H");
	sprintf(opcodeDescription[0x26],"LD H, $##");
	sprintf(opcodeDescription[0x27],"DAA");
	sprintf(opcodeDescription[0x28],"JR Z, $****");
	sprintf(opcodeDescription[0x29],"ADD HL, HL");
	sprintf(opcodeDescription[0x2A],"LDI A, (HL)");
	sprintf(opcodeDescription[0x2B],"DEC HL");
	sprintf(opcodeDescription[0x2C],"INC L");
	sprintf(opcodeDescription[0x2D],"DEC L");
	sprintf(opcodeDescription[0x2E],"LD L, $##");
	sprintf(opcodeDescription[0x2F],"CPL");
	sprintf(opcodeDescription[0x30],"JR NC, $****");
	sprintf(opcodeDescription[0x31],"LD SP, $####");
	sprintf(opcodeDescription[0x32],"LDD (HL), A");
	sprintf(opcodeDescription[0x33],"INC SP");
	sprintf(opcodeDescription[0x34],"INC (HL)");
	sprintf(opcodeDescription[0x35],"DEC (HL)");
	sprintf(opcodeDescription[0x36],"LD (HL)");
	sprintf(opcodeDescription[0x37],"SCF");
	sprintf(opcodeDescription[0x38],"JR C, $****");
	sprintf(opcodeDescription[0x39],"ADD HL, SP");
	sprintf(opcodeDescription[0x3A],"LDD A, (HL)");
	sprintf(opcodeDescription[0x3B],"DEC SP");
	sprintf(opcodeDescription[0x3C],"INC A");
	sprintf(opcodeDescription[0x3D],"DEC A");
	sprintf(opcodeDescription[0x3E],"LD A, $##");
	sprintf(opcodeDescription[0x3F],"CCF");
	sprintf(opcodeDescription[0x40],"LD B, B");
	sprintf(opcodeDescription[0x41],"LD B, C");
	sprintf(opcodeDescription[0x42],"LD B, D");
	sprintf(opcodeDescription[0x43],"LD B, E");
	sprintf(opcodeDescription[0x44],"LD B, H");
	sprintf(opcodeDescription[0x45],"LD B, L");
	sprintf(opcodeDescription[0x46],"LD B, (HL)");
	sprintf(opcodeDescription[0x47],"LD B, A");
	sprintf(opcodeDescription[0x48],"LD C, B");
	sprintf(opcodeDescription[0x49],"LD C, C");
	sprintf(opcodeDescription[0x4A],"LD C, D");
	sprintf(opcodeDescription[0x4B],"LD C, E");
	sprintf(opcodeDescription[0x4C],"LD C, H");
	sprintf(opcodeDescription[0x4D],"LD C, L");
	sprintf(opcodeDescription[0x4E],"LD C, (HL)");
	sprintf(opcodeDescription[0x4F],"LD C, A");
	sprintf(opcodeDescription[0x50],"LD D, B");
	sprintf(opcodeDescription[0x51],"LD D, C");
	sprintf(opcodeDescription[0x52],"LD D, D");
	sprintf(opcodeDescription[0x53],"LD D, E");
	sprintf(opcodeDescription[0x54],"LD D, H");
	sprintf(opcodeDescription[0x55],"LD D, L");
	sprintf(opcodeDescription[0x56],"LD D, (HL)");
	sprintf(opcodeDescription[0x57],"LD D, A");
	sprintf(opcodeDescription[0x58],"LD E, B");
	sprintf(opcodeDescription[0x59],"LD E, C");
	sprintf(opcodeDescription[0x5A],"LD E, D");
	sprintf(opcodeDescription[0x5B],"LD E, E");
	sprintf(opcodeDescription[0x5C],"LD E, H");
	sprintf(opcodeDescription[0x5D],"LD E, L");
	sprintf(opcodeDescription[0x5E],"LD E, (HL)");
	sprintf(opcodeDescription[0x5F],"LD E, A");
	sprintf(opcodeDescription[0x60],"LD H, B");
	sprintf(opcodeDescription[0x61],"LD H, C");
	sprintf(opcodeDescription[0x62],"LD H, D");
	sprintf(opcodeDescription[0x63],"LD H, E");
	sprintf(opcodeDescription[0x64],"LD H, H");
	sprintf(opcodeDescription[0x65],"LD H, L");
	sprintf(opcodeDescription[0x66],"LD H, (HL)");
	sprintf(opcodeDescription[0x67],"LD H, A");
	sprintf(opcodeDescription[0x68],"LD L, B");
	sprintf(opcodeDescription[0x69],"LD L, C");
	sprintf(opcodeDescription[0x6A],"LD L, D");
	sprintf(opcodeDescription[0x6B],"LD L, E");
	sprintf(opcodeDescription[0x6C],"LD L, H");
	sprintf(opcodeDescription[0x6D],"LD L, L");
	sprintf(opcodeDescription[0x6E],"LD L, (HL)");
	sprintf(opcodeDescription[0x6F],"LD L, A");
	sprintf(opcodeDescription[0x70],"LD (HL), B");
	sprintf(opcodeDescription[0x71],"LD (HL), C");
	sprintf(opcodeDescription[0x72],"LD (HL), D");
	sprintf(opcodeDescription[0x73],"LD (HL), E");
	sprintf(opcodeDescription[0x74],"LD (HL), H");
	sprintf(opcodeDescription[0x75],"LD (HL), L");
	sprintf(opcodeDescription[0x76],"HALT");
	sprintf(opcodeDescription[0x77],"LD (HL), A");
	sprintf(opcodeDescription[0x78],"LD A, B");
	sprintf(opcodeDescription[0x79],"LD A, C");
	sprintf(opcodeDescription[0x7A],"LD A, D");
	sprintf(opcodeDescription[0x7B],"LD A, E");
	sprintf(opcodeDescription[0x7C],"LD A, H");
	sprintf(opcodeDescription[0x7D],"LD A, L");
	sprintf(opcodeDescription[0x7E],"LD A, (HL)");
	sprintf(opcodeDescription[0x7F],"LD A, A");
	sprintf(opcodeDescription[0x80],"ADD A, B");
	sprintf(opcodeDescription[0x81],"ADD A, C");
	sprintf(opcodeDescription[0x82],"ADD A, D");
	sprintf(opcodeDescription[0x83],"ADD A, E");
	sprintf(opcodeDescription[0x84],"ADD A, H");
	sprintf(opcodeDescription[0x85],"ADD A, L");
	sprintf(opcodeDescription[0x86],"ADD A, (HL)");
	sprintf(opcodeDescription[0x87],"ADD A, A");
	sprintf(opcodeDescription[0x88],"ADC A, B");
	sprintf(opcodeDescription[0x89],"ADC A, C");
	sprintf(opcodeDescription[0x8A],"ADC A, D");
	sprintf(opcodeDescription[0x8B],"ADC A, E");
	sprintf(opcodeDescription[0x8C],"ADC A, H");
	sprintf(opcodeDescription[0x8D],"ADC A, L");
	sprintf(opcodeDescription[0x8E],"ADC A, (HL)");
	sprintf(opcodeDescription[0x8F],"ADC A, A");
	sprintf(opcodeDescription[0x90],"SUB B");
	sprintf(opcodeDescription[0x91],"SUB C");
	sprintf(opcodeDescription[0x92],"SUB D");
	sprintf(opcodeDescription[0x93],"SUB E");
	sprintf(opcodeDescription[0x94],"SUB H");
	sprintf(opcodeDescription[0x95],"SUB L");
	sprintf(opcodeDescription[0x96],"SUB (HL)");
	sprintf(opcodeDescription[0x97],"SUB A");
	sprintf(opcodeDescription[0x98],"SBC A, B");
	sprintf(opcodeDescription[0x99],"SBC A, C");
	sprintf(opcodeDescription[0x9A],"SBC A, D");
	sprintf(opcodeDescription[0x9B],"SBC A, E");
	sprintf(opcodeDescription[0x9C],"SBC A, H");
	sprintf(opcodeDescription[0x9D],"SBC A, L");
	sprintf(opcodeDescription[0x9E],"SBC A, (HL)");
	sprintf(opcodeDescription[0x9F],"SBC A, A");
	sprintf(opcodeDescription[0xA0],"AND B");
	sprintf(opcodeDescription[0xA1],"AND C");
	sprintf(opcodeDescription[0xA2],"AND D");
	sprintf(opcodeDescription[0xA3],"AND E");
	sprintf(opcodeDescription[0xA4],"AND H");
	sprintf(opcodeDescription[0xA5],"AND L");
	sprintf(opcodeDescription[0xA6],"AND (HL)");
	sprintf(opcodeDescription[0xA7],"AND A");
	sprintf(opcodeDescription[0xA8],"XOR B");
	sprintf(opcodeDescription[0xA9],"XOR C");
	sprintf(opcodeDescription[0xAA],"XOR D");
	sprintf(opcodeDescription[0xAB],"XOR E");
	sprintf(opcodeDescription[0xAC],"XOR H");
	sprintf(opcodeDescription[0xAD],"XOR L");
	sprintf(opcodeDescription[0xAE],"XOR (HL)");
	sprintf(opcodeDescription[0xAF],"XOR A");
	sprintf(opcodeDescription[0xB0],"OR B");
	sprintf(opcodeDescription[0xB1],"OR C");
	sprintf(opcodeDescription[0xB2],"OR D");
	sprintf(opcodeDescription[0xB3],"OR E");
	sprintf(opcodeDescription[0xB4],"OR H");
	sprintf(opcodeDescription[0xB5],"OR L");
	sprintf(opcodeDescription[0xB6],"OR (HL)");
	sprintf(opcodeDescription[0xB7],"OR A");
	sprintf(opcodeDescription[0xB8],"CP B");
	sprintf(opcodeDescription[0xB9],"CP C");
	sprintf(opcodeDescription[0xBA],"CP D");
	sprintf(opcodeDescription[0xBB],"CP E");
	sprintf(opcodeDescription[0xBC],"CP H");
	sprintf(opcodeDescription[0xBD],"CP L");
	sprintf(opcodeDescription[0xBE],"CP (HL)");
	sprintf(opcodeDescription[0xBF],"CP A");
	sprintf(opcodeDescription[0xC0],"RET NZ");
	sprintf(opcodeDescription[0xC1],"POP BC");
	sprintf(opcodeDescription[0xC2],"JP NZ, $####");
	sprintf(opcodeDescription[0xC3],"JP $####");
	sprintf(opcodeDescription[0xC4],"CALL NZ, $####");
	sprintf(opcodeDescription[0xC5],"PUSH BC");
	sprintf(opcodeDescription[0xC6],"ADD A, $####");
	sprintf(opcodeDescription[0xC7],"RST 00H");
	sprintf(opcodeDescription[0xC8],"RET Z");
	sprintf(opcodeDescription[0xC9],"RET");
	sprintf(opcodeDescription[0xCA],"JP Z, $####");
//	sprintf(opcodeDescription[0xCB],"");
	sprintf(opcodeDescription[0xCC],"CALL Z, $####");
	sprintf(opcodeDescription[0xCD],"CALL $####");
	sprintf(opcodeDescription[0xCE],"ADC A, $##");
	sprintf(opcodeDescription[0xCF],"RST 08H");
	sprintf(opcodeDescription[0xD0],"RET NC");
	sprintf(opcodeDescription[0xD1],"POP DE");
	sprintf(opcodeDescription[0xD2],"JP NC, $####");
	sprintf(opcodeDescription[0xD3],"Undefined");
	sprintf(opcodeDescription[0xD4],"CALL NC, $####");
	sprintf(opcodeDescription[0xD5],"PUSH DE");
	sprintf(opcodeDescription[0xD6],"SUB $##");
	sprintf(opcodeDescription[0xD7],"RST 10H");
	sprintf(opcodeDescription[0xD8],"RET C");
	sprintf(opcodeDescription[0xD9],"RETI");
	sprintf(opcodeDescription[0xDA],"JP C, $####");
	sprintf(opcodeDescription[0xDB],"Undefined");
	sprintf(opcodeDescription[0xDC],"CALL C, $####");
	sprintf(opcodeDescription[0xDD],"Undefined");
	sprintf(opcodeDescription[0xDE],"SBC A, $##");
	sprintf(opcodeDescription[0xDF],"RST 18H");
	sprintf(opcodeDescription[0xE0],"LDH ($FF00+##), A");
	sprintf(opcodeDescription[0xE1],"POP HL");
	sprintf(opcodeDescription[0xE2],"LDH ($FF##+C), A");
	sprintf(opcodeDescription[0xE3],"Undefined");
	sprintf(opcodeDescription[0xE4],"Undefined");
	sprintf(opcodeDescription[0xE5],"PUSH HL");
	sprintf(opcodeDescription[0xE6],"AND $##");
	sprintf(opcodeDescription[0xE7],"RST 20H");
	sprintf(opcodeDescription[0xE8],"ADD SP, ***");
	sprintf(opcodeDescription[0xE9],"JP (HL)");
	sprintf(opcodeDescription[0xEA],"LD ($####), A");
	sprintf(opcodeDescription[0xEB],"Undefined");
	sprintf(opcodeDescription[0xEC],"Undefined");
	sprintf(opcodeDescription[0xED],"Undefined");
	sprintf(opcodeDescription[0xEE],"XOR $##");
	sprintf(opcodeDescription[0xEF],"RST 28H");
	sprintf(opcodeDescription[0xF0],"LD A, ($FF00+##)");
	sprintf(opcodeDescription[0xF1],"POP AF");
	sprintf(opcodeDescription[0xF2],"LD A, (C)");
	sprintf(opcodeDescription[0xF3],"DI");
	sprintf(opcodeDescription[0xF4],"Undefined");
	sprintf(opcodeDescription[0xF5],"PUSH AF");
	sprintf(opcodeDescription[0xF6],"OR $##");
	sprintf(opcodeDescription[0xF7],"RST 30H");
	sprintf(opcodeDescription[0xF8],"LD HL, SP+$##");
	sprintf(opcodeDescription[0xF9],"LD SP, HL");
	sprintf(opcodeDescription[0xFA],"LD A, ($####)");
	sprintf(opcodeDescription[0xFB],"EI");
	sprintf(opcodeDescription[0xFC],"Undefined");
	sprintf(opcodeDescription[0xFD],"Undefined");
	sprintf(opcodeDescription[0xFE],"CP $##");
	sprintf(opcodeDescription[0xFF],"RST 38H");

	// CB opcode descriptions
	sprintf(opcodeCBDescription[0x00],"RLC B");
	sprintf(opcodeCBDescription[0x01],"RLC C");
	sprintf(opcodeCBDescription[0x02],"RLC D");
	sprintf(opcodeCBDescription[0x03],"RLC E");
	sprintf(opcodeCBDescription[0x04],"RLC H");
	sprintf(opcodeCBDescription[0x05],"RLC L");
	sprintf(opcodeCBDescription[0x06],"RLC (HL)");
	sprintf(opcodeCBDescription[0x07],"RLC A");
	sprintf(opcodeCBDescription[0x08],"RRC B");
	sprintf(opcodeCBDescription[0x09],"RRC C");
	sprintf(opcodeCBDescription[0x0A],"RRC D");
	sprintf(opcodeCBDescription[0x0B],"RRC E");
	sprintf(opcodeCBDescription[0x0C],"RRC H");
	sprintf(opcodeCBDescription[0x0D],"RRC L");
	sprintf(opcodeCBDescription[0x0E],"RRC (HL)");
	sprintf(opcodeCBDescription[0x0F],"RRC A");
	sprintf(opcodeCBDescription[0x10],"RL B");
	sprintf(opcodeCBDescription[0x11],"RL C");
	sprintf(opcodeCBDescription[0x12],"RL D");
	sprintf(opcodeCBDescription[0x13],"RL E");
	sprintf(opcodeCBDescription[0x14],"RL H");
	sprintf(opcodeCBDescription[0x15],"RL L");
	sprintf(opcodeCBDescription[0x16],"RL (HL)");
	sprintf(opcodeCBDescription[0x17],"RL A");
	sprintf(opcodeCBDescription[0x18],"RR B");
	sprintf(opcodeCBDescription[0x19],"RR C");
	sprintf(opcodeCBDescription[0x1A],"RR D");
	sprintf(opcodeCBDescription[0x1B],"RR E");
	sprintf(opcodeCBDescription[0x1C],"RR H");
	sprintf(opcodeCBDescription[0x1D],"RR L");
	sprintf(opcodeCBDescription[0x1E],"RR (HL)");
	sprintf(opcodeCBDescription[0x1F],"RR A");
	sprintf(opcodeCBDescription[0x20],"SLA B");
	sprintf(opcodeCBDescription[0x21],"SLA C");
	sprintf(opcodeCBDescription[0x22],"SLA D");
	sprintf(opcodeCBDescription[0x23],"SLA E");
	sprintf(opcodeCBDescription[0x24],"SLA H");
	sprintf(opcodeCBDescription[0x25],"SLA L");
	sprintf(opcodeCBDescription[0x26],"SLA (HL)");
	sprintf(opcodeCBDescription[0x27],"SLA A");
	sprintf(opcodeCBDescription[0x28],"SRA B");
	sprintf(opcodeCBDescription[0x29],"SRA C");
	sprintf(opcodeCBDescription[0x2A],"SRA D");
	sprintf(opcodeCBDescription[0x2B],"SRA E");
	sprintf(opcodeCBDescription[0x2C],"SRA H");
	sprintf(opcodeCBDescription[0x2D],"SRA L");
	sprintf(opcodeCBDescription[0x2E],"SRA (HL)");
	sprintf(opcodeCBDescription[0x2F],"SRA A");
	sprintf(opcodeCBDescription[0x30],"SWAP B");
	sprintf(opcodeCBDescription[0x31],"SWAP C");
	sprintf(opcodeCBDescription[0x32],"SWAP D");
	sprintf(opcodeCBDescription[0x33],"SWAP E");
	sprintf(opcodeCBDescription[0x34],"SWAP H");
	sprintf(opcodeCBDescription[0x35],"SWAP L");
	sprintf(opcodeCBDescription[0x36],"SWAP (HL)");
	sprintf(opcodeCBDescription[0x37],"SWAP A");
	sprintf(opcodeCBDescription[0x38],"SRL B");
	sprintf(opcodeCBDescription[0x39],"SRL C");
	sprintf(opcodeCBDescription[0x3A],"SRL D");
	sprintf(opcodeCBDescription[0x3B],"SRL E");
	sprintf(opcodeCBDescription[0x3C],"SRL H");
	sprintf(opcodeCBDescription[0x3D],"SRL L");
	sprintf(opcodeCBDescription[0x3E],"SRL (HL)");
	sprintf(opcodeCBDescription[0x3F],"SRL A");
	sprintf(opcodeCBDescription[0x40],"BIT 0, B");
	sprintf(opcodeCBDescription[0x41],"BIT 0, C");
	sprintf(opcodeCBDescription[0x42],"BIT 0, D");
	sprintf(opcodeCBDescription[0x43],"BIT 0, E");
	sprintf(opcodeCBDescription[0x44],"BIT 0, H");
	sprintf(opcodeCBDescription[0x45],"BIT 0, L");
	sprintf(opcodeCBDescription[0x46],"BIT 0, (HL)");
	sprintf(opcodeCBDescription[0x47],"BIT 0, A");
	sprintf(opcodeCBDescription[0x48],"BIT 1, B");
	sprintf(opcodeCBDescription[0x49],"BIT 1, C");
	sprintf(opcodeCBDescription[0x4A],"BIT 1, D");
	sprintf(opcodeCBDescription[0x4B],"BIT 1, E");
	sprintf(opcodeCBDescription[0x4C],"BIT 1, H");
	sprintf(opcodeCBDescription[0x4D],"BIT 1, L");
	sprintf(opcodeCBDescription[0x4E],"BIT 1, (HL)");
	sprintf(opcodeCBDescription[0x4F],"BIT 1, A");
	sprintf(opcodeCBDescription[0x50],"BIT 2, B");
	sprintf(opcodeCBDescription[0x51],"BIT 2, C");
	sprintf(opcodeCBDescription[0x52],"BIT 2, D");
	sprintf(opcodeCBDescription[0x53],"BIT 2, E");
	sprintf(opcodeCBDescription[0x54],"BIT 2, H");
	sprintf(opcodeCBDescription[0x55],"BIT 2, L");
	sprintf(opcodeCBDescription[0x56],"BIT 2, (HL)");
	sprintf(opcodeCBDescription[0x57],"BIT 2, A");
	sprintf(opcodeCBDescription[0x58],"BIT 3, B");
	sprintf(opcodeCBDescription[0x59],"BIT 3, C");
	sprintf(opcodeCBDescription[0x5A],"BIT 3, D");
	sprintf(opcodeCBDescription[0x5B],"BIT 3, E");
	sprintf(opcodeCBDescription[0x5C],"BIT 3, H");
	sprintf(opcodeCBDescription[0x5D],"BIT 3, L");
	sprintf(opcodeCBDescription[0x5E],"BIT 3, (HL)");
	sprintf(opcodeCBDescription[0x5F],"BIT 3, A");
	sprintf(opcodeCBDescription[0x60],"BIT 4, B");
	sprintf(opcodeCBDescription[0x61],"BIT 4, C");
	sprintf(opcodeCBDescription[0x62],"BIT 4, D");
	sprintf(opcodeCBDescription[0x63],"BIT 4, E");
	sprintf(opcodeCBDescription[0x64],"BIT 4, H");
	sprintf(opcodeCBDescription[0x65],"BIT 4, L");
	sprintf(opcodeCBDescription[0x66],"BIT 4, (HL)");
	sprintf(opcodeCBDescription[0x67],"BIT 4, A");
	sprintf(opcodeCBDescription[0x68],"BIT 5, B");
	sprintf(opcodeCBDescription[0x69],"BIT 5, C");
	sprintf(opcodeCBDescription[0x6A],"BIT 5, D");
	sprintf(opcodeCBDescription[0x6B],"BIT 5, E");
	sprintf(opcodeCBDescription[0x6C],"BIT 5, H");
	sprintf(opcodeCBDescription[0x6D],"BIT 5, L");
	sprintf(opcodeCBDescription[0x6E],"BIT 5, (HL)");
	sprintf(opcodeCBDescription[0x6F],"BIT 5, A");
	sprintf(opcodeCBDescription[0x70],"BIT 6, B");
	sprintf(opcodeCBDescription[0x71],"BIT 6, C");
	sprintf(opcodeCBDescription[0x72],"BIT 6, D");
	sprintf(opcodeCBDescription[0x73],"BIT 6, E");
	sprintf(opcodeCBDescription[0x74],"BIT 6, H");
	sprintf(opcodeCBDescription[0x75],"BIT 6, L");
	sprintf(opcodeCBDescription[0x76],"BIT 6, (HL)");
	sprintf(opcodeCBDescription[0x77],"BIT 6, A");
	sprintf(opcodeCBDescription[0x78],"BIT 7, B");
	sprintf(opcodeCBDescription[0x79],"BIT 7, C");
	sprintf(opcodeCBDescription[0x7A],"BIT 7, D");
	sprintf(opcodeCBDescription[0x7B],"BIT 7, E");
	sprintf(opcodeCBDescription[0x7C],"BIT 7, H");
	sprintf(opcodeCBDescription[0x7D],"BIT 7, L");
	sprintf(opcodeCBDescription[0x7E],"BIT 7, (HL)");
	sprintf(opcodeCBDescription[0x7F],"BIT 7, A");
	sprintf(opcodeCBDescription[0x80],"RES 0, B");
	sprintf(opcodeCBDescription[0x81],"RES 0, C");
	sprintf(opcodeCBDescription[0x82],"RES 0, D");
	sprintf(opcodeCBDescription[0x83],"RES 0, E");
	sprintf(opcodeCBDescription[0x84],"RES 0, H");
	sprintf(opcodeCBDescription[0x85],"RES 0, L");
	sprintf(opcodeCBDescription[0x86],"RES 0, (HL)");
	sprintf(opcodeCBDescription[0x87],"RES 0, A");
	sprintf(opcodeCBDescription[0x88],"RES 1, B");
	sprintf(opcodeCBDescription[0x89],"RES 1, C");
	sprintf(opcodeCBDescription[0x8A],"RES 1, D");
	sprintf(opcodeCBDescription[0x8B],"RES 1, E");
	sprintf(opcodeCBDescription[0x8C],"RES 1, H");
	sprintf(opcodeCBDescription[0x8D],"RES 1, L");
	sprintf(opcodeCBDescription[0x8E],"RES 1, (HL)");
	sprintf(opcodeCBDescription[0x8F],"RES 1, A");
	sprintf(opcodeCBDescription[0x90],"RES 2, B");
	sprintf(opcodeCBDescription[0x91],"RES 2, C");
	sprintf(opcodeCBDescription[0x92],"RES 2, D");
	sprintf(opcodeCBDescription[0x93],"RES 2, E");
	sprintf(opcodeCBDescription[0x94],"RES 2, H");
	sprintf(opcodeCBDescription[0x95],"RES 2, L");
	sprintf(opcodeCBDescription[0x96],"RES 2, (HL)");
	sprintf(opcodeCBDescription[0x97],"RES 2, A");
	sprintf(opcodeCBDescription[0x98],"RES 3, B");
	sprintf(opcodeCBDescription[0x99],"RES 3, C");
	sprintf(opcodeCBDescription[0x9A],"RES 3, D");
	sprintf(opcodeCBDescription[0x9B],"RES 3, E");
	sprintf(opcodeCBDescription[0x9C],"RES 3, H");
	sprintf(opcodeCBDescription[0x9D],"RES 3, L");
	sprintf(opcodeCBDescription[0x9E],"RES 3, (HL)");
	sprintf(opcodeCBDescription[0x9F],"RES 3, A");
	sprintf(opcodeCBDescription[0xA0],"RES 4, B");
	sprintf(opcodeCBDescription[0xA1],"RES 4, C");
	sprintf(opcodeCBDescription[0xA2],"RES 4, D");
	sprintf(opcodeCBDescription[0xA3],"RES 4, E");
	sprintf(opcodeCBDescription[0xA4],"RES 4, H");
	sprintf(opcodeCBDescription[0xA5],"RES 4, L");
	sprintf(opcodeCBDescription[0xA6],"RES 4, (HL)");
	sprintf(opcodeCBDescription[0xA7],"RES 4, A");
	sprintf(opcodeCBDescription[0xA8],"RES 5, B");
	sprintf(opcodeCBDescription[0xA9],"RES 5, C");
	sprintf(opcodeCBDescription[0xAA],"RES 5, D");
	sprintf(opcodeCBDescription[0xAB],"RES 5, E");
	sprintf(opcodeCBDescription[0xAC],"RES 5, H");
	sprintf(opcodeCBDescription[0xAD],"RES 5, L");
	sprintf(opcodeCBDescription[0xAE],"RES 5, (HL)");
	sprintf(opcodeCBDescription[0xAF],"RES 5, A");
	sprintf(opcodeCBDescription[0xB0],"RES 6, B");
	sprintf(opcodeCBDescription[0xB1],"RES 6, C");
	sprintf(opcodeCBDescription[0xB2],"RES 6, D");
	sprintf(opcodeCBDescription[0xB3],"RES 6, E");
	sprintf(opcodeCBDescription[0xB4],"RES 6, H");
	sprintf(opcodeCBDescription[0xB5],"RES 6, L");
	sprintf(opcodeCBDescription[0xB6],"RES 6, (HL)");
	sprintf(opcodeCBDescription[0xB7],"RES 6, A");
	sprintf(opcodeCBDescription[0xB8],"RES 7, B");
	sprintf(opcodeCBDescription[0xB9],"RES 7, C");
	sprintf(opcodeCBDescription[0xBA],"RES 7, D");
	sprintf(opcodeCBDescription[0xBB],"RES 7, E");
	sprintf(opcodeCBDescription[0xBC],"RES 7, H");
	sprintf(opcodeCBDescription[0xBD],"RES 7, L");
	sprintf(opcodeCBDescription[0xBE],"RES 7, (HL)");
	sprintf(opcodeCBDescription[0xBF],"RES 7, A");
	sprintf(opcodeCBDescription[0xC0],"SET 0, B");
	sprintf(opcodeCBDescription[0xC1],"SET 0, C");
	sprintf(opcodeCBDescription[0xC2],"SET 0, D");
	sprintf(opcodeCBDescription[0xC3],"SET 0, E");
	sprintf(opcodeCBDescription[0xC4],"SET 0, H");
	sprintf(opcodeCBDescription[0xC5],"SET 0, L");
	sprintf(opcodeCBDescription[0xC6],"SET 0, (HL)");
	sprintf(opcodeCBDescription[0xC7],"SET 0, A");
	sprintf(opcodeCBDescription[0xC8],"SET 1, B");
	sprintf(opcodeCBDescription[0xC9],"SET 1, C");
	sprintf(opcodeCBDescription[0xCA],"SET 1, D");
	sprintf(opcodeCBDescription[0xCB],"SET 1, E");
	sprintf(opcodeCBDescription[0xCC],"SET 1, H");
	sprintf(opcodeCBDescription[0xCD],"SET 1, L");
	sprintf(opcodeCBDescription[0xCE],"SET 1, (HL)");
	sprintf(opcodeCBDescription[0xCF],"SET 1, A");
	sprintf(opcodeCBDescription[0xD0],"SET 2, B");
	sprintf(opcodeCBDescription[0xD1],"SET 2, C");
	sprintf(opcodeCBDescription[0xD2],"SET 2, D");
	sprintf(opcodeCBDescription[0xD3],"SET 2, E");
	sprintf(opcodeCBDescription[0xD4],"SET 2, H");
	sprintf(opcodeCBDescription[0xD5],"SET 2, L");
	sprintf(opcodeCBDescription[0xD6],"SET 2, (HL)");
	sprintf(opcodeCBDescription[0xD7],"SET 2, A");
	sprintf(opcodeCBDescription[0xD8],"SET 3, B");
	sprintf(opcodeCBDescription[0xD9],"SET 3, C");
	sprintf(opcodeCBDescription[0xDA],"SET 3, D");
	sprintf(opcodeCBDescription[0xDB],"SET 3, E");
	sprintf(opcodeCBDescription[0xDC],"SET 3, H");
	sprintf(opcodeCBDescription[0xDD],"SET 3, L");
	sprintf(opcodeCBDescription[0xDE],"SET 3, (HL)");
	sprintf(opcodeCBDescription[0xDF],"SET 3, A");
	sprintf(opcodeCBDescription[0xE0],"SET 4, B");
	sprintf(opcodeCBDescription[0xE1],"SET 4, C");
	sprintf(opcodeCBDescription[0xE2],"SET 4, D");
	sprintf(opcodeCBDescription[0xE3],"SET 4, E");
	sprintf(opcodeCBDescription[0xE4],"SET 4, H");
	sprintf(opcodeCBDescription[0xE5],"SET 4, L");
	sprintf(opcodeCBDescription[0xE6],"SET 4, (HL)");
	sprintf(opcodeCBDescription[0xE7],"SET 4, A");
	sprintf(opcodeCBDescription[0xE8],"SET 5, B");
	sprintf(opcodeCBDescription[0xE9],"SET 5, C");
	sprintf(opcodeCBDescription[0xEA],"SET 5, D");
	sprintf(opcodeCBDescription[0xEB],"SET 5, E");
	sprintf(opcodeCBDescription[0xEC],"SET 5, H");
	sprintf(opcodeCBDescription[0xED],"SET 5, L");
	sprintf(opcodeCBDescription[0xEE],"SET 5, (HL)");
	sprintf(opcodeCBDescription[0xEF],"SET 5, A");
	sprintf(opcodeCBDescription[0xF0],"SET 6, B");
	sprintf(opcodeCBDescription[0xF1],"SET 6, C");
	sprintf(opcodeCBDescription[0xF2],"SET 6, D");
	sprintf(opcodeCBDescription[0xF3],"SET 6, E");
	sprintf(opcodeCBDescription[0xF4],"SET 6, H");
	sprintf(opcodeCBDescription[0xF5],"SET 6, L");
	sprintf(opcodeCBDescription[0xF6],"SET 6, (HL)");
	sprintf(opcodeCBDescription[0xF7],"SET 6, A");
	sprintf(opcodeCBDescription[0xF8],"SET 7, B");
	sprintf(opcodeCBDescription[0xF9],"SET 7, C");
	sprintf(opcodeCBDescription[0xFA],"SET 7, D");
	sprintf(opcodeCBDescription[0xFB],"SET 7, E");
	sprintf(opcodeCBDescription[0xFC],"SET 7, H");
	sprintf(opcodeCBDescription[0xFD],"SET 7, L");
	sprintf(opcodeCBDescription[0xFE],"SET 7, (HL)");
	sprintf(opcodeCBDescription[0xFF],"SET 7, A");
}

void DisassembleLine(unsigned int address, char *data)
{
	char replaceData[10];
	int opcodeAddress;
	char *stringPointer;

	//----------------------------------------//
	// This code will print information into  //
	// the listbox and account for opcodes	  //
	// that take more than 1 byte.			  //
	//----------------------------------------//
	if (opcodeBytes[emu.memory.intRamaddress]] == 3)
	{		
		sprintf(data, "0x%04X: %02X %02X %02X   %s", address, emu.memory.intRamaddress], emu.memory.intRamaddress + 1], emu.memory.intRamaddress + 2], opcodeDescription[emu.memory.intRamaddress]]);
		stringPointer = strstr(data, "####");

		if (stringPointer != NULL)
		{
			opcodeAddress = (emu.memory.intRamaddress + 2] << 8) + emu.memory.intRamaddress + 1];

			sprintf(replaceData, "%04X", opcodeAddress);

			*stringPointer = replaceData[0];
			*(stringPointer + 1) = replaceData[1];
			*(stringPointer + 2) = replaceData[2];
			*(stringPointer + 3) = replaceData[3];
		}
		address += 2;
	}
	else if (opcodeBytes[emu.memory.intRamaddress]] == 2)
	{
		sprintf(data, "0x%04X: %02X %02X      %s", address, emu.memory.intRamaddress], emu.memory.intRamaddress + 1], opcodeDescription[emu.memory.intRamaddress]]);
		if ((stringPointer = strstr(data, "##")) != NULL)
		{
			opcodeAddress = emu.memory.intRamaddress + 1];

			sprintf(replaceData, "%02X", opcodeAddress);

			*stringPointer = replaceData[0];
			*(stringPointer + 1) = replaceData[1];
		}
		//----------------------------------------//
		// This handles the JP and JR instructions//
		//----------------------------------------//
		else if ((stringPointer = strstr(data, "****")) != NULL)
		{
			opcodeAddress = address + (signed char)emu.memory.intRamaddress + 1] + 2;

			sprintf(replaceData, "%04X", opcodeAddress);

			*stringPointer = replaceData[0];
			*(stringPointer + 1) = replaceData[1];
			*(stringPointer + 2) = replaceData[2];
			*(stringPointer + 3) = replaceData[3];
		}
		else if ((stringPointer = strstr(data, "***")) != NULL)
		{
			//----------------------------------------//
			// Since sprintf apparently doesn't handle//
			// signed values, I had to use this crappy//
			// implementation to add a - or + sign to //
			// the ADD SP opcode					  //
			//----------------------------------------//
			opcodeAddress = emu.memory.intRamaddress + 1];

			if (opcodeAddress > 128)
			{
				opcodeAddress ^= 255;
				opcodeAddress++;

				sprintf(replaceData, "%03X", opcodeAddress);

				opcodeAddress--;
				opcodeAddress ^= 255;
			}
			else
			{
				sprintf(replaceData, "%03X", opcodeAddress);
			}

			if (opcodeAddress >= 0)
				replaceData[0] = 0x2B;

			if (opcodeAddress >= 128)
			{
				replaceData[0] = 0x2D;
			}

			*stringPointer = replaceData[0];
			*(stringPointer + 1) = replaceData[1];
			*(stringPointer + 2) = replaceData[2];
		}
		else if (emu.memory.intRamaddress] == 0xCB)
		{
			sprintf(data, "0x%04X: %02X %02X      %s", address, emu.memory.intRamaddress], emu.memory.intRamaddress + 1], opcodeCBDescription[emu.memory.intRamaddress]]);
		}
		address++;
	}
	else
	{
		sprintf(data, "0x%04X: %02X         %s", address, emu.memory.intRamaddress], opcodeDescription[emu.memory.intRamaddress]]);
	}
}*/