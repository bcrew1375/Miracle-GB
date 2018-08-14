#include <memory.h>
#include <sdl.h>
#include <stdio.h>
#include <windows.h>

#include "Win32/resource.h"

HWND hWnd;
WNDCLASSEX wc;
MSG Msg;

//----------------------------------------//
// External data                          //
//----------------------------------------//

//----------------------------------------//
// External functions.                    //
//----------------------------------------//

extern void CloseSDL();
extern void CPUReset();
extern int  InitializeSDL();
extern int  LoadRomFile(char filename[]);
extern int  OpenSDLWindow();
extern void ResizeScreen();
extern void RunEmulation();
extern void ClearGBMemory();
extern void UpdateDebugger();
extern void ShowDebugger();
extern void SetupDebugger();
//----------------------------------------//

//----------------------------------------//
// External variables.                    //
//----------------------------------------//

extern unsigned int screenHeight;
extern unsigned int screenWidth;
//----------------------------------------//

//----------------------------------------//
// Miscellaneous variables.               //
//----------------------------------------//

char szFileName[MAX_PATH] = "";

unsigned int CPURunning = 0;
unsigned int FPSLimit = 1;
unsigned int logging = 0;
//----------------------------------------//

//----------------------------------------//
// Miscellaneous pointers.                //
//----------------------------------------//

unsigned char *bootBuffer;
unsigned char *romBuffer;
unsigned char *memory;
//----------------------------------------//

//----------------------------------------//
// Windows Callback routine.              //
//----------------------------------------//
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		{
			//----------------------------------------//
			// Make sure we can allocate enough memory//
			// for the largest Gameboy ROM.           //
			//----------------------------------------//
			if ((!(romBuffer = (unsigned char *)(malloc(0x200000)))) || (!(bootBuffer = (unsigned char *)(malloc(0x100)))) || (!(memory = (unsigned char *)(malloc(0x10000)))))
			{
				MessageBox(hWnd, "Could not allocate memory for buffer!", "Error!", MB_OK);
				DestroyWindow(hWnd);
				exit(0);
			}
			//----------------------------------------//
			// In the case SDL could not be started,  //
			// complain and then exit.                //
			//----------------------------------------//
			if (InitializeSDL() == -1)
			{
				MessageBox(hWnd, "Could not initialize SDL!", "Error!", MB_OK);
				DestroyWindow(hWnd);
				exit(0);
			}
			//----------------------------------------//
			// Clear Gameboy Memory					  //
			//----------------------------------------//
			ClearGBMemory();
			//----------------------------------------//
			// Reset the Gameboy CPU				  //
			//----------------------------------------//
			CPUReset();
			//----------------------------------------//
			// Setup the debugger					  //
			//----------------------------------------//
			SetupDebugger();
		}
	break;
	case WM_CLOSE:
		{
			CloseSDL();
			DestroyWindow(hWnd);
			exit(0);
		}
	break;
	case WM_DESTROY:
		{
			CloseSDL();
			PostQuitMessage(0);
		}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_FILE_OPEN:
			{
				OPENFILENAME ofn;

				ZeroMemory(&ofn, sizeof(ofn));

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = "GB Files (*.gb)\0*.gb\0GBC Files (*.gbc)\0*.gbc\0";
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = "";

				if(GetOpenFileName(&ofn))
				{
					switch(LoadRomFile(szFileName))
					{
					case -1:
						{
							MessageBox(hWnd, "Could not open file in Read-Only Binary mode!", "Error!", MB_OK);
						}
					break;
					case -2:
						{
							MessageBox(hWnd, "Could not read from file!", "Error!", MB_OK);
						}
					break;
					}
				}
				CPUReset();
			}
		break;
		case IDM_FILE_EXIT:
			{
				CloseSDL();
				DestroyWindow(hWnd);
				exit(0);
			}
		break;
		case IDM_EMULATE_RUN:
			{
				if (OpenSDLWindow() == -1)
					MessageBox(hWnd, "Could not open SDL window", "Error!", MB_OK);

				CPURunning = 1;
				RunEmulation();
			}
		break;
		case IDM_EMULATE_RESET:
			{
				CPUReset();
			}
		break;
		case IDM_OPTIONS_USEFPSLIMIT:
			{
				FPSLimit ^= 1;
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_1X:
			{
				screenWidth = 160;
				screenHeight = 144;

				ResizeScreen();
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_2X:
			{
				screenWidth = 320;
				screenHeight = 288;

				ResizeScreen();
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_3X:
			{
				screenWidth = 480;
				screenHeight = 432;
				
				ResizeScreen();
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_4X:
			{
				screenWidth = 640;
				screenHeight = 576;
				
				ResizeScreen();
			}
		break;
		case IDM_DEBUG_STARTDEBUGGER:
			{				
				ShowDebugger();
			}
		break;
		case IDM_DEBUG_STARTLOGGING:
			{
				logging = 1;
			}
		break;
		case IDM_DEBUG_STOPLOGGING:
			{
				logging = 0;
			}
		break;
		}
	break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	break;
	}
	return 0;
}

//----------------------------------------//
// The main function.  This setups the    //
// main window.                           //
//----------------------------------------//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName = "Window Class";
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"Window Class",
		"Miracle GB",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL);

	if(hWnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
		MB_ICONEXCLAMATION | MB_OK);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while(GetMessage(&Msg, hWnd, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
    }
	return Msg.wParam;
}
