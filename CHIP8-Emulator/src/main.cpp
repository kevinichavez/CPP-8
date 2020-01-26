#include <iostream>
#include <ctime>
#include "Chip8.h"
#include "Emulator.h"

int main(int argc, char *argv[]) {

	// Seed random number generator
	srand(time(0));

	Emulator emu;

	if (emu.selectGame()) {
		switch (emu.runGame()) {

		default:
			break;
		}
	}

	// Cleanup
	
	return 0;
}
