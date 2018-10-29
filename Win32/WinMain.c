#include <memory.h>
#include <sdl.h>
#include <stdio.h>
#include <windows.h>

#include "resource.h"

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
extern void SystemReset();
extern void HandleSDLEvents();
extern int  InitializeSDL();
unsigned char* LoadRomFile(char filename[], unsigned long int *fileSize);
FILE *OpenLogFile();
extern void AddFPSTimer();
extern void RemoveFPSTimer();
extern int  OpenSDLWindow();
extern void ResizeScreen();
extern void RunEmulation();
extern void ClearGBMemory();
extern int EmulationInitialize(unsigned char *fileBuffer, unsigned long int fileSize);
//----------------------------------------//

//----------------------------------------//
// External variables.                    //
//----------------------------------------//

extern SDL_Window window;
extern unsigned int screenHeight;
extern unsigned int screenWidth;
//extern unsigned int screenSizeMultiplier;
//----------------------------------------//

//----------------------------------------//
// Miscellaneous variables.               //
//----------------------------------------//

HANDLE hRomFile;
HANDLE logFile;
char szFileName[MAX_PATH] = "";
unsigned int CPURunning = 0;
unsigned int FPSLimit = 1;
unsigned int logging = 0;
unsigned int emulationInitialized = 0;
//----------------------------------------//

// Display an error message if there was an invalid opcode ran.
void OpcodeError(char *errorText)
{
	MessageBox(hWnd, errorText, "Error!", MB_OK);
}

//----------------------------------------//
// Windows Callback routine.              //
//----------------------------------------//
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	unsigned char *romFileBuffer = 0;
	unsigned long int fileSize = 0;

	switch(msg)
	{
	case WM_CREATE:
		{
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
				ofn.lpstrFilter = "GB/GBC/SGB Files\0*.gb;*.gbc;*.sgb\0";
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = "";

				if(GetOpenFileName(&ofn))
				{
					romFileBuffer = LoadRomFile(szFileName, &fileSize);
					if (!romFileBuffer)
						MessageBox(hWnd, "Error opening ROM file!", "Error!", MB_OK);
					else
					{
						EmulationInitialize(romFileBuffer, fileSize);
						if (FPSLimit == 1)
							AddFPSTimer();
						RunEmulation();
					}
				}
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
			}
		break;
		case IDM_EMULATE_RESET:
			{
				//SystemReset();
			}
		break;
		case IDM_OPTIONS_USEFPSLIMIT:
			{
			if (FPSLimit == 1)
			{
				RemoveFPSTimer();
				FPSLimit = 0;
			}
			else
			{
				AddFPSTimer();
				FPSLimit = 1;
			}
			}
		break;
		/*case IDM_OPTIONS_DISPLAYSIZE_1X:
			{
				screenSizeMultiplier = 1;
				ResizeScreen();
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_2X:
			{
				screenSizeMultiplier = 2;
				ResizeScreen();
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_3X:
			{
				screenSizeMultiplier = 3;
				ResizeScreen();
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_4X:
			{
				screenSizeMultiplier = 4;
				ResizeScreen();
			}*/
		break;
		case IDM_DEBUG_STARTDEBUGGER:
			{				
				//ShowDebugger();
			}
		break;
		case IDM_DEBUG_STARTLOGGING:
			{
				logging = 1;
				logFile = CreateFile("log.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			}
		break;
		case IDM_DEBUG_STOPLOGGING:
			{
				logging = 0;
				CloseHandle(logFile);
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
// main window and message checking.      //
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
		CW_USEDEFAULT, CW_USEDEFAULT, screenWidth, screenHeight,
		NULL, NULL, hInstance, NULL);

	if (!OpenSDLWindow())
		MessageBox(NULL, "SDL Creation Failed!", "Error!",
		MB_ICONEXCLAMATION | MB_OK);

	if (hWnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
	}
	else
	{
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}

	while(GetMessage(&Msg, hWnd, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
    }
	return Msg.wParam;
}