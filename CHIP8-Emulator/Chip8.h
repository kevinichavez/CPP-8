#pragma once

#include <cstdint>

const int CH8_RESOLUTION = 64 * 32;
const int STACK_SIZE = 24;
const int CH8_MEM_SIZE = 4096;

class Chip8 {

public:
	// Constructor, clears first 0x200 bytes in memory and loads in fontset
	Chip8();

	// Reset all variables to default starting values
	void init();

	// Emulate one cycle
	void emulateCycle();

	// Get state of drawing flag
	const bool shouldDraw() { return m_drawFlag; }

	// Passes input to emulator
	void setKeys();


private:
	// Opcodes on CHIP-8 are two bytes long
	uint16_t m_opcode;

	// Hardware CHIP-8 is on typically has 4096 8-bit memory locations
	uint8_t m_memory[CH8_MEM_SIZE];

	// CHIP-8 has 16 8-bit data registers named V0 through VF
	uint8_t m_V[0xF];

	// CHIP-8 has address register I which is 16 bits wide
	uint16_t m_I;

	// Program Counter
	uint16_t m_pc;

	// Stack with 48 bytes for up to 24 levels of nesting
	uint16_t m_stack[STACK_SIZE];

	// Stack Pointer
	uint8_t m_sp;

	// Delay timer, used for timings of events in games
	uint16_t m_dTimer;

	// Sound timer, beeping noise made when value is non-zero
	uint16_t m_sTimer;

	// Flag that determines whether screen should be redrawn
	bool m_drawFlag;

	// Array that stores current state of pixels on 64 * 32 screen
	bool m_gfx[CH8_RESOLUTION];

	// Store whether key is currently being pressed
	bool m_keys[0xF];

	// Clear the screen
	void clearDisp();
};