#include <iostream>
#include <ctime>
#include <string>
#include <SDL.h>
#include "Chip8.h"

void drawScreen(const uint8_t*& c);

int main(int argc, char *argv[]) {

	// Seed random number generator
	srand(time(0));

	Chip8 chip;

	chip.init();

	std::string gameName;
	std::cout << "Enter name of game:" << std::endl;
	std::getline(std::cin, gameName);

	chip.loadGame(gameName);

	const uint8_t* screen = chip.getGfxArray();

	while (false) {
		chip.emulateCycle();

		if (chip.shouldDraw())
			drawScreen(screen);

	}

	return 0;
}

void drawScreen(const uint8_t*& c) {

}
