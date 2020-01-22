#include <iostream>
#include <ctime>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <nfd.h>
#include "Chip8.h"
#include "constants.h"
#include "Emulator.h"

int main(int argc, char *argv[]) {

	// Seed random number generator
	srand(time(0));

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Could not initialize SDL. SDL Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	Emulator emu;

	if (emu.selectGame()) {
		switch (emu.runGame()) {

		default:
			break;
		}
	}

	// Cleanup
	SDL_Quit();
	return 0;
}
