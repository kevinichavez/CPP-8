#include <iostream>
#include <ctime>
#include "Chip8.h"

void drawScreen();

int main() {

	// Seed random number generator
	srand(time(0));

	Chip8 chip;

	chip.init();

	while (true) {
		chip.emulateCycle();

		if (chip.shouldDraw())
			drawScreen();

	}


}

void drawScreen() {

}
