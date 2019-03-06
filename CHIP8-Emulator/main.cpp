#include <iostream>
#include <ctime>
#include "Chip8.h"

void drawScreen(const uint8_t*& c);

int main(int argc, char *argv[]) {

	// Seed random number generator
	srand(time(0));



	Chip8 chip;

	chip.init();

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
