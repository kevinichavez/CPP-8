#include "Chip8.h"
#include <iostream>

const uint16_t CH8_FONTSET[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void unknownOpcode(uint16_t opcode);

Chip8::Chip8() {
	// Clear first 0x200 bytes of memory
	for (int i = 0; i < 0x200; i++)
		m_memory[i] = 0;

	// Load in fontset
	for (int i = 0; i < 80; i++)
		m_memory[i] = CH8_FONTSET[i];
}

void Chip8::init() {
	// Program counter starts at 0x200
	m_pc = 0x200;
	
	// Reset index register
	m_I = 0;

	// Reset stack pointer
	m_sp = 0;

	// Clear display
	clearDisp();

	// Clear registers
	for (int i = 0; i < 0xF; i++)
		m_V[i] = 0;

	// Clear stack
	for (int i = 0; i < STACK_SIZE; i++)
		m_stack[i] = 0;

	// Clear memory
	for (int i = m_pc; i < CH8_MEM_SIZE; i++)
		m_memory[i] = 0;

	// Reset delay timer
	m_dTimer = 0;

	// Reset sound timer
	m_sTimer = 0;

	// Reset key state
	for (int i = 0; i < 0xF; i++)
		m_keys[i] = false;
}

void Chip8::emulateCycle() {
	// Get opcode
	uint16_t opcode = m_memory[m_pc] << 8 | m_memory[m_pc + 1];

	// Decode opcode
	switch (opcode & 0xF000) {
	case 0x000:
		switch (opcode & 0x00FF) {
		case 0x00E0:
			// 00E0: Clears the screen
			clearDisp();
			m_pc += 2;
			break;

		case 0x00EE:
			// 00EE: Return from subroutine
			m_pc = m_stack[--m_sp];
			m_pc += 2;
			break;

		default:
			std::cerr << "Trying to call RCA 1802 at " << std::hex << opcode << std::dec << " (?)" << std::endl;
		}
		break;

	case 0xA000:
		// ANNN: Sets I to the address NNN
		m_I = opcode & 0x0FFF;
		m_pc += 2;
		break;

	default:
		unknownOpcode(opcode);
	}


}

void Chip8::clearDisp() {
	for (int i = 0; i < CH8_RESOLUTION; i++)
		m_gfx[i] = false;
}

void unknownOpcode(uint16_t opcode) {
	std::cerr << "Unknown opcode: " << std::hex << opcode << std::dec << std::endl;
}