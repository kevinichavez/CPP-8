#include <iostream>
#include "Chip8.h"

void drawScreen();

int main() {

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
