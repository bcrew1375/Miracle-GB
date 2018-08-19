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
extern int  InitializeSDL();
extern int  LoadRomFile(char filename[]);
extern FILE *OpenLogFile();
extern int  OpenSDLWindow();
extern void ResizeScreen();
extern void RunEmulation();
extern void ClearGBMemory();
extern int EmulationInitialize(unsigned char *fileBuffer, unsigned int fileSize);
//----------------------------------------//

//----------------------------------------//
// External variables.                    //
//----------------------------------------//

extern SDL_Window window;
extern unsigned int screenHeight;
extern unsigned int screenWidth;
extern unsigned int maxFileSize;
//extern unsigned int screenSizeMultiplier;
//----------------------------------------//

//----------------------------------------//
// Miscellaneous variables.               //
//----------------------------------------//

HANDLE hRomFile;
char szFileName[MAX_PATH] = "";
unsigned char *romFileBuffer;

PLARGE_INTEGER hRomFileSize = NULL;
unsigned int CPURunning = 0;
unsigned int FPSLimit = 1;
unsigned int logging = 0;
unsigned int emulationInitialized = 0;
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
				ofn.lpstrFilter = "GB Files (*.gb)\0*.gb\0GBC Files (*.gbc)\0*.gbc\0";
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = "";

				if(GetOpenFileName(&ofn))
				{
					hRomFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
					if (!hRomFile)
						MessageBox(hWnd, "Can't open file!", "Error!", MB_OK);
					else
					{
						if (!GetFileSizeEx(hRomFile, &hRomFileSize))
						{
							MessageBox(hWnd, "Can't get file size!", "Error", MB_OK);
						}
						if (hRomFileSize > maxFileSize)
						{
							MessageBox(hWnd, "The file is too large and likely not a valid ROM", "Error!", MB_OK);
						}
						else
						{
							LPDWORD bytesRead;
							romFileBuffer = malloc(hRomFileSize);
							if (!romFileBuffer)
								MessageBox(hWnd, "Can't allocate ROM buffer!", "Error!", MB_OK);
							if (!ReadFile(hRomFile, romFileBuffer, hRomFileSize, NULL, NULL))
								MessageBox(hWnd, "Can't read file!", "Error!", MB_OK);
							else
								EmulationInitialize(romFileBuffer, hRomFileSize);
						}
						
						CloseHandle(hRomFile);
					}

					//fileBuffer = LoadRomFile(szFileName, GetFileSizeEx(szFileName, &fileSize));
					/*switch(fileBuffer)
					{
					case -1:
						{
							MessageBox(hWnd, "Could not open file!", "Error!", MB_OK);
							fileBuffer = 0;
						}
					break;
					case -2:
						{
							MessageBox(hWnd, "File too large!  Likely not a valid ROM.", "Error!", MB_OK);
							fileBuffer = 0;
						}
					break;
					case -3:
						{
							MessageBox(hWnd, "Could not allocate RAM for file!", "Error!", MB_OK);
							fileBuffer = 0;
						}
					break;
					case -4:
						{
							MessageBox(hWnd, "Could not read from file!", "Error!", MB_OK);
							fileBuffer = 0;
						}
					break;
					}*/
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
				RunEmulation();
			}
		break;
		case IDM_EMULATE_RESET:
			{
				SystemReset();
			}
		break;
		case IDM_OPTIONS_USEFPSLIMIT:
			{
				FPSLimit ^= 1;
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
				if(OpenLogFile() != 0)
					MessageBox(hWnd, "Open log file failed!", "Error", MB_OK);
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
		CW_USEDEFAULT, CW_USEDEFAULT, screenWidth, screenHeight,
		NULL, NULL, hInstance, NULL);

	if(hWnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
		MB_ICONEXCLAMATION | MB_OK);
	}

	OpenSDLWindow();

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while(GetMessage(&Msg, hWnd, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
    }
	return Msg.wParam;
}
