#pragma once

#include <cstdint>

const int CH8_WIDTH = 64;
const int CH8_HEIGHT = 32;
const int CH8_STACK_SIZE = 24;
const int CH8_MEM_SIZE = 0x1000;

class Chip8 {

public:
	// Constructor, clears first 0x200 bytes in memory and loads in fontset
	Chip8();

	// Reset all variables to default starting values
	void init();

	// Emulate one cycle
	void emulateCycle();

	// Get state of drawing flag
	bool shouldDraw() const { return m_drawFlag; }

	// Passes input to emulator
	void setKeys();

	// Returns pointer to first address of graphics array
	const uint8_t* getGfxArray() const { return &m_gfx[0][0]; }

	// Turn sprite wrapping to the other side of the screen on
	void enableSpriteWrap() { m_wrapFlag = true; }

	// Turn sprite wrapping off
	void disableSpriteWrap() { m_wrapFlag = false; }

private:
	// Hardware CHIP-8 is on typically has 4096 8-bit memory locations
	uint8_t m_memory[CH8_MEM_SIZE];

	// CHIP-8 has 16 8-bit data registers named V0 through VF
	uint8_t m_V[16];

	// CHIP-8 has address register I which is 16 bits wide
	uint16_t m_I;

	// Program Counter
	uint16_t m_pc;

	// Stack for calling subroutines with 48 bytes for up to 24 levels of nesting
	uint16_t m_stack[CH8_STACK_SIZE];

	// Stack Pointer
	uint8_t m_sp;

	// Delay timer, used for timings of events in games
	uint16_t m_dTimer;

	// Sound timer, beeping noise made when value is non-zero
	uint16_t m_sTimer;

	// Flag that determines whether screen should be redrawn
	bool m_drawFlag;

	// Array that stores current state of pixels on 64 * 32 screen
	uint8_t m_gfx[CH8_WIDTH][CH8_HEIGHT];

	// Store whether key is currently being pressed
	bool m_keys[16];

	// Determines what should happen if sprite goes off screen
	bool m_wrapFlag;

	// Clear the screen
	void clearDisp();

	// Increment program counter
	void incrPC() { m_pc += 2; }
};