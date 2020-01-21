#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include "Chip8.h"

class Emulator {
public:
	int selectGame();
	void reset();
private:
	Chip8 chip;

};

#endif