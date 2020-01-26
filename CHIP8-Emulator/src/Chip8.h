#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <string>
#include "constants.h"

class Chip8 {

public:
	// Constructor, clears first 0x200 bytes in memory and loads in fontset
	Chip8();

	// Reset all variables to default starting values
	void init();

	// Emulate one cycle
	void emulateCycle();

	// Get state of drawing flag
	bool shouldDraw() const { return drawFlag; }

	// Passes input to emulator
	void setKeys(bool a[]);

	// Array that stores current state of pixels on 64 * 32 screen
	uint8_t gfx[CH8_WIDTH][CH8_HEIGHT];

	// Loads ROM file into memory
	int loadRom(std::string name);

	// Get current value of sound timer
	uint16_t getSoundTimer() const { return sTimer; }

	// Check if the soundTimer was given a new value since it was last read
	bool isAudioUpdated();

	// Turn sprite wrapping to the other side of the screen on
	void enableSpriteWrap() { wrapFlag = true; }

	// Turn sprite wrapping off
	void disableSpriteWrap() { wrapFlag = false; }

	// Check if sprite wrapping is on or off
	bool wrapIsEnabled() { return wrapFlag; }

	// Decrement the timers
	void decrTimers();

private:
	// Hardware CHIP-8 is on typically has 4096 8-bit memory locations
	uint8_t memory[CH8_MEM_SIZE];

	// CHIP-8 has 16 8-bit data registers named V0 through VF
	uint8_t V[16];

	// CHIP-8 has address register I which is 16 bits wide
	uint16_t I;

	// Program Counter
	uint16_t pc;

	// Stack for calling subroutines with 48 bytes for up to 24 levels of nesting
	uint16_t stack[CH8_STACK_SIZE];

	// Stack Pointer
	uint8_t sp;

	// Delay timer, used for timings of events in games
	uint16_t dTimer;

	// Sound timer, beeping noise made when value is non-zero
	uint16_t sTimer;

	// Keeps track of how many milliseconds passed when timers last updated
	uint32_t lastTime;

	// Flag that determines whether screen should be redrawn
	bool drawFlag;

	// Store whether key is currently being pressed
	bool keys[16];

	// Determines what should happen if sprite goes off screen on the x-axis
	bool wrapFlag;

	// Clear the screen
	void clearDisp();

	// Increment program counter
	void incrPC() { pc += 2; }

	//
	bool soundTimerIsUpdated;
};

#endif