#include <iostream>
#include <ctime>
#include <string>
#include <SDL.h>
#include <nfd.h>
#include "Chip8.h"

void drawScreen(const uint8_t*& c);
std::string flipEscapeChars(std::string s);

int main(int argc, char *argv[]) {

	// Seed random number generator
	srand(time(0));

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Could not initialize SDL. SDL Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	// Create game window
	SDL_Window *mainWindow = SDL_CreateWindow(
		"Selecting game...",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		CH8_WIDTH * 10,
		CH8_HEIGHT * 10,
		SDL_WINDOW_OPENGL                  
	);

	if (mainWindow == NULL) {
		std::cerr << "Could not create window. SDL Error: " << SDL_GetError() << std::endl;
		exit(2);
	}

	// Create Renderer
	SDL_Renderer *renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

	Chip8 chip;

	// Select ROM file from open file dialogue
	nfdchar_t *outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog("ch8", NULL, &outPath);
	std::string gamePath;

	switch (result) {
	case NFD_OKAY:
		std::cout << "ROM path found successfully!" << std::endl;
		std::cout << outPath << std::endl;
		gamePath = outPath;
		delete outPath;
		break;

	case NFD_CANCEL:
		std::cout << "User pressed cancel." << std::endl << "Exiting program..." << std::endl;
		exit(0);
		break;

	default:
		std::cerr << "Error: " << NFD_GetError() << std::endl;
		exit(3);
	}

	SDL_SetWindowTitle(mainWindow, "CHIP-8 Emulator");

	// Reset all values in Chip8 object
	chip.init();

	// Load ROM into memory
	switch (chip.loadRom(gamePath)) {
	case 0:
		std::cout << "ROM loaded successfully!" << std::endl;
		break;

	case -1:
		exit(4);

	case -2:
		exit(5);

	default:
		std::cerr << "Something unexpected happened with the file reading process? You shouldn't ever see this!" << std::endl;
		exit(6);
	}



	const uint8_t* screen = chip.getGfxArray();

	while (true) {
		chip.emulateCycle();

		if (chip.shouldDraw())
			drawScreen(screen);
	}

	// Destroy game window
	SDL_DestroyWindow(mainWindow);

	// Cleanup
	SDL_Quit();
	return 0;
}

void drawScreen(const uint8_t*& c) {

}

std::string flipEscapeChars(std::string s) {
	for (int i = 0; i < s.size(); i++)
		if (s[i] == '\\')
			s[i] = '/';
	return s;
}
