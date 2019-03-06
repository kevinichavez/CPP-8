#include <iostream>
#include <ctime>
#include "Chip8.h"
#include <SDL.h>

void drawScreen(Chip8 c);

int main(int argc, char *argv[]) {

	// Seed random number generator
	srand(time(0));

	Chip8 chip;

	chip.init();

	while (true) {
		chip.emulateCycle();

		if (chip.shouldDraw())
			drawScreen(chip);

	}


}

void drawScreen(Chip8 c) {

}
