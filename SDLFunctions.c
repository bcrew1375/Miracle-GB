#include <sdl.h>
#include <windows.h>

//----------------------------------------//
// SDL defined type variables, arrays, etc//
//----------------------------------------//
SDL_Color colors[4];
SDL_Event event;
SDL_Window *sdlWindow;
SDL_Surface *screen;
SDL_Texture *texture;
SDL_Renderer *renderer;
SDL_DisplayMode displayMode;
static SDL_TimerID fpsTimerID = NULL;
//----------------------------------------//

void HandleSDLEvents();
void SetupSDLTimers();
void UpdateScreen();

//----------------------------------------//
// External data.                         //
//----------------------------------------//

// External functions.
extern int InitializeSound();

// External variables and arrays.
extern HWND hWnd;

extern unsigned char joyState[8];
extern unsigned char *memory;
extern unsigned char screenData[0x5A00];
extern unsigned int CPURunning;
extern unsigned long long int FPS;
extern unsigned int FPSLimit;
extern unsigned int SpeedKey;

extern unsigned char szFileName[];
//----------------------------------------//


//----------------------------------------//
// Constants.                             //
//----------------------------------------//

const unsigned int SCREEN_BPP = 8;
//----------------------------------------//


//----------------------------------------//
// Miscellaneous variables.               //
//----------------------------------------//

unsigned int screenHeight = 144;
unsigned int screenWidth = 160;

unsigned int screenSizeMultiplier = 3;

boolean fullScreenOn = 0;

//----------------------------------------//


//----------------------------------------//
// Miscellaneous pointers.                //
//----------------------------------------//

unsigned char *screen_ptr;
//----------------------------------------//


//----------------------------------------//
// Screen buffer arrays.                  //
//----------------------------------------//

//unsigned char oldScreenData[160 * 144];
//unsigned char screenData[160 * 144];
//----------------------------------------//


//----------------------------------------//
// Initialize the functions that will be  //
// used in the program.                   //
//----------------------------------------//
int InitializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1)
		return -1;

	return 0;
}


//----------------------------------------//
// Change the width and height of the     //
// display.                               //
//----------------------------------------//
void ResizeScreen()
{
	//rectangleWidth = screenWidth / 160;
	//rectangleHeight = screenHeight / 144;

	SDL_SetWindowSize(sdlWindow, screenWidth * screenSizeMultiplier, screenHeight * screenSizeMultiplier);
}

//----------------------------------------//
// Open the SDL window, set up the palette//
//----------------------------------------//
int OpenSDLWindow()
{
	sdlWindow = SDL_CreateWindowFrom(hWnd);//"Miracle GB", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth * screenSizeMultiplier, screenHeight * screenSizeMultiplier, SDL_WINDOW_OPENGL);
	if (sdlWindow == NULL)
		return 0;
	renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL)
		return 0;
	SDL_RenderSetLogicalSize(renderer, screenWidth, screenHeight);

	// Set up a pointer to the screen
	//screen_ptr = (unsigned char *)screen->pixels;

	ResizeScreen();

	//memset(&oldScreenData, 0, 0x5A00);
	//memset(&screenData, 0, 0x5A00);

	screen = SDL_CreateRGBSurfaceFrom(&screenData, screenWidth, screenHeight, 8, screenWidth, 0, 0, 0, 0);

	colors[0].r = 0xFF;
	colors[0].g = 0xFF;
	colors[0].b = 0xFF;
	
	colors[1].r = 0xAA;
	colors[1].g = 0xAA;
	colors[1].b = 0xAA;

	colors[2].r = 0x55;
	colors[2].g = 0x55;
	colors[2].b = 0x55;

	colors[3].r = 0x00;
	colors[3].g = 0x00;
	colors[3].b = 0x00;

	SDL_SetPaletteColors(screen->format->palette, colors, 0, 4);

	return -1;
}

//----------------------------------------//
// Close down all SDL functions.          //
//----------------------------------------//
void CloseSDL()
{
	SDL_Quit();
}

//----------------------------------------//
// This updates the FPS display in the    //
// window's title bar.                    //
//----------------------------------------//
unsigned int UpdateFPS(Uint32 interval, void *param)
{
	char *windowTitle[256];
	
	//*windowTitle = SDL_GetWindowTitle(window);
	
	sprintf(&windowTitle, "Miracle GB   FPS: %d", FPS);
	SDL_SetWindowTitle(sdlWindow, windowTitle);

	FPS = 0;
	
	return interval;
}

void AddFPSTimer()
{
	if (!fpsTimerID)
		fpsTimerID = SDL_AddTimer(1000, UpdateFPS, 0);
}

void RemoveFPSTimer()
{
	if (fpsTimerID)
		fpsTimerID = SDL_RemoveTimer(fpsTimerID);
}

//----------------------------------------//
// This will lock the emulation to        //
// roughly 60 frames a second.            //
//----------------------------------------//
void LimitFPS()
{
	static unsigned long int emulationTimer = 0;
	unsigned int timeDifference;
	
	if (!emulationTimer)
		emulationTimer = SDL_GetTicks();

	timeDifference = SDL_GetTicks() - emulationTimer;

	// Display one frame per 1/60 of a second.
	if (timeDifference < 16.6)
		SDL_Delay(16.6 - timeDifference);

	emulationTimer = SDL_GetTicks();
}

//----------------------------------------//
// This pauses or resumes the emulation   //
// and removes the FPS timer.             //
//----------------------------------------//
void PauseEmu()
{
}


//----------------------------------------//
// This gets the current state of the     //
// keyboard and, depending on the keys,   //
// takes the appropriate actions.         //
//----------------------------------------//
void GetKeys()
{
	// This will determine the keys that are currently pressed.
	Uint8 *keyState = SDL_GetKeyboardState(NULL);

	// Check System keystates.
	if (keyState[SDL_SCANCODE_ESCAPE])
	{
		CPURunning ^= 1;
		PauseEmu();
	}
	if (keyState[SDL_SCANCODE_TAB])
	{
		SpeedKey = 1;
	}
	else
	{
		SpeedKey = 0;
	}
	/*if (keyState[SDL_SCANCODE_F4])
	{
		if (fullScreenOn == 0)
		{
			//if (SDL_GetDesktopDisplayMode(0, &displayMode))
			//{
			//	SDL_SetWindowSize(window, displayMode.w, displayMode.h);
			//	fullScreenOn = 1;
			//}
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			fullScreenOn = 1;
		}
		else
		{
			//SDL_SetWindowSize(window, screenWidth * screenSizeMultiplier, screenHeight * screenSizeMultiplier);
			SDL_SetWindowFullscreen(window, 0);
			fullScreenOn = 0;
		}
	}*/

	// Check Gameboy keystates.
	if (keyState[SDL_SCANCODE_DOWN])
		joyState[0] = 1;
	else
		joyState[0] = 0;

	if (keyState[SDL_SCANCODE_UP])
		joyState[1] = 1;
	else
		joyState[1] = 0;

	if (keyState[SDL_SCANCODE_LEFT])
		joyState[2] = 1;
	else
		joyState[2] = 0;

	if (keyState[SDL_SCANCODE_RIGHT])
		joyState[3] = 1;
	else
		joyState[3] = 0;

	if (keyState[SDL_SCANCODE_RETURN])
		joyState[4] = 1;
	else
		joyState[4] = 0;

	if (keyState[SDL_SCANCODE_SPACE])
		joyState[5] = 1;
	else
		joyState[5] = 0;

	if (keyState[SDL_SCANCODE_D])
		joyState[6] = 1;
	else
		joyState[6] = 0;

	if (keyState[SDL_SCANCODE_F])
		joyState[7] = 1;
	else
		joyState[7] = 0;
}

//----------------------------------------//
// This function draws the actual screen  //
// data to the surface then displays it.  //
//----------------------------------------//
void UpdateScreen()
{
	SDL_memcpy(screen->pixels, screenData, 0x5A00);
	texture = SDL_CreateTextureFromSurface(renderer, screen);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, 0, 0);
	SDL_DestroyTexture(texture);
	SDL_RenderPresent(renderer);
}

//----------------------------------------//
// Update the SDL event queue.            //
//----------------------------------------//
void HandleSDLEvents()
{
	SDL_PumpEvents();
	GetKeys();
}
