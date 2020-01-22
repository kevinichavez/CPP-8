#include "Chip8.h"
#include <iostream>
#include <fstream>
#include <SDL.h>
#include "constants.h"

void unknownOpcode(uint16_t opcode);

Chip8::Chip8() {
	init();
}

void Chip8::init() {

	// Program counter starts at 0x200
	pc = 0x200;
	
	// Reset index register
	I = 0;

	// Reset stack pointer
	sp = 0;

	// Clear display
	clearDisp();

	// Clear registers
	for (int i = 0; i < 0xF; i++)
		V[i] = 0;

	// Clear stack
	for (int i = 0; i < CH8_STACK_SIZE; i++)
		stack[i] = 0;

	// Clear memory
	for (int i = pc; i < CH8_MEM_SIZE; i++)
		memory[i] = 0;

	// Reset delay timer
	dTimer = 0;

	// Reset sound timer
	sTimer = 0;

	// Reset key state
	for (int i = 0; i < 0xF; i++)
		keys[i] = false;

	// Reset wrap flag
	wrapFlag = false;

	// Reset timer counter
	lastTime = SDL_GetTicks();
	accumulator = 0;

	// Clear first 0x200 bytes of memory
	for (int i = 0; i < 0x200; i++)
		memory[i] = 0;

	// Load in fontset
	for (int i = 0; i < 80; i++)
		memory[i] = CH8_FONTSET[i];
}

void Chip8::emulateCycle() {

	// Reset drawing flag
	drawFlag = false;

	// Get opcode
	uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];

	// Get V register identifiers
	uint8_t x = (opcode >> 8) & 0x0F;
	uint8_t y = (opcode >> 4) & 0x0F;

	// Decode opcode
	switch (opcode & 0xF000) {
	case 0x000:
		switch (opcode & 0x00FF) {
		case 0x00E0:
			// 00E0: Clears the screen
			clearDisp();
			drawFlag = true;
			incrPC();
			break;

		case 0x00EE:
			// 00EE: Return from subroutine
			pc = stack[--sp];
			incrPC();
			break;

		default:
			std::cerr << "Trying to call RCA 1802 at " << std::hex << (0x0FFF & opcode) << std::dec << " (?)" << std::endl;
			exit(-1);
		}
		break;

	case 0x1000:
		// 1NNN: Jumps to address NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000:
		// 2NNN: Call function at NNN
		stack[sp++] = pc;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000:
		// 3XNN: Skips the next instruction if VX equals NN
		if (V[x] == (opcode & 0x00FF))
			incrPC();
		incrPC();
		break;

	case 0x4000:
		// 4XNN: Skips the next instruction if VX doesn't equal NN
		if (V[x] != (opcode & 0x00FF))
			incrPC();
		incrPC();
		break;

	case 0x5000:
		// 5XY0: Skips the next instruction if VX equals VY
		if (V[x] == V[y])
			incrPC();
		incrPC();
		break;

	case 0x6000:
		// 6XNN: Sets VX to NN
		V[x] = opcode & 0x00FF;
		incrPC();
		break;

	case 0x7000:
		// 7XNN: Adds NN to VX (carry flag unchanged)
		V[x] += (opcode & 0x00FF);
		incrPC();
		break;

	case 0x8000:
		switch (opcode & 0x000F) {
		case 0x0000:
			// 8XY0: Sets VX to value of VY
			V[x] = V[y];
			incrPC();
			break;

		case 0x0001:
			// 8XY1: Sets VX to VX OR VY
			V[x] |= V[y];
			incrPC();
			break;

		case 0x0002:
			// 8XY2: Sets VX to VX AND VY
			V[x] &= V[y];
			incrPC();
			break;

		case 0x0003:
			// 8XY3: Sets VX to VX XOR VY
			V[x] ^= V[y];
			incrPC();
			break;

		case 0x0004: {
			// 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
			uint16_t sum = V[x] + V[y];
			if (sum > 0xFF)
				V[0xF] = 1;
			else V[0xF] = 0;
			V[x] = sum;
			incrPC();
			break;
		}

		case 0x0005:
			// 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (V[x] < V[y])
				V[0xF] = 0;
			else V[0xF] = 1;
			V[x] = V[x] - V[y];
			incrPC();
			break;

		case 0x0006:
			// 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
			V[0xF] = V[x] & 0x01;
			V[x] >>= 1;
			incrPC();
			break;

		case 0x0007:
			// 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
			if (V[x] > V[y])
				V[0xF] = 0;
			else V[0xF] = 1;
			V[x] = V[y] - V[x];;
			incrPC();
			break;

		case 0x000E:
			// 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1
			V[0xF] = V[x] >> 7;
			V[x] <<= 1;
			incrPC();
			break;

		default:
			unknownOpcode(opcode);
		}
		break;

	case 0x9000:
		// 9XY0: Skips the next instruction if VX doesn't equal VY
		if (V[x] != V[y])
			incrPC();
		incrPC();

	case 0xA000:
		// ANNN: Sets I to the address NNN
		I = opcode & 0x0FFF;
		incrPC();
		break;

	case 0xB000:
		// BNNN: Jumps to the address NNN plus V0
		pc = V[0x0] + (opcode & 0x0FFF);
		break;

	case 0xC000: {
		// CXNN: Sets VX to the result of a bitwise AND operation on a random number between 0 and 255 and NN
		uint8_t rng = rand() % 0xFF;
		V[x] = rng & (opcode & 0x00FF);
		incrPC();
		break;
	}

	case 0xD000: {
		// DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
		// Each row of 8 pixels is read as bit-coded starting from memory location I
		// I value doesn’t change after the execution of this instruction
		// VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen

		// Get height of sprite
		int height = opcode & 0x0F;

		// Reset VF Register since we don't know if there was collision yet
		V[0xF] = 0;

		// Loop for number of rows the sprite takes up
		for (int row = 0; row < height; row++) {

			// Get one row of sprite at a time
			uint8_t spriteRow = memory[I + row];

			// Each sprite is 8 pixels wide
			for (int col = 0; col < 8; col++) {

				// Get bit to check if it's set
				uint8_t bit = spriteRow & (0x80 >> col);

				// Check if bit is set
				if (bit) {

					// Check for wraparound
					if (V[x] + col >= CH8_WIDTH) {
						if (wrapFlag) {
							// TODO: Fix this!
							// Wrap X value to other side of screen
							V[x] = CH8_WIDTH % V[x];
						}
						else continue;
					}

					if (V[y] + row > CH8_HEIGHT)
						continue;

					// Check if bit is already set
					if (gfx[V[x] + col][V[y + row]] == 1)
						V[0xF] = 1;

					// XOR the bit using 1 to flip it
					gfx[V[x] + col][V[y] + row] ^= 1;
				}

			}
		}

		drawFlag = true;
		incrPC();
		break;
	}

	case 0xE000:
		switch (opcode & 0x00FF) {
		case 0x009E:
			// EX9E: Skips the next instruction if the key stored in VX is pressed
			if (keys[V[x]])
				incrPC();
			incrPC();
			break;
			
		case 0x00A1:
			// EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (!keys[V[x]])
				incrPC();
			incrPC();
			break;

		default:
			unknownOpcode(opcode);
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF) {
		case 0x0007:
			// FX07: Sets VX to the value of the delay timer
			V[x] = dTimer;
			incrPC();
			break;

		case 0x000A: {
			// FX0A: A key press is awaited, and then stored in VX. Halt all instruction until key press
			bool keyIsPressed = false;
			for (int i = 0; i < 0xF; i++)
				if (keys[i]) {
					keyIsPressed = true;
					V[x] = keys[i];
					i = 0xF;
				}
			if (keyIsPressed)
				return;
			incrPC();
			break;
		}

		case 0x0015:
			// FX15: Sets the delay timer to VX
			dTimer = V[x];
			incrPC();
			break;

		case 0x0018:
			// FX18: Sets the sound timer to VX
			sTimer = V[x];
			incrPC();
			break;

		case 0x001E: {
			// FX1E: Adds VX to I. VF is set to 1 when there is a range overflow and 0 when there isn't
			int vxisum = V[x] + I;
			if (vxisum > 0x0FFF)
				V[0xF] = 1;
			else V[0xF] = 0;
			I = vxisum;
			incrPC();
			break;
		}

		case 0x0029:
			// FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
			I = V[x] * 5;
			incrPC();
			break;

		case 0x0033:
			// FX33: Take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2
			memory[I] = V[x] / 100;
			memory[I + 1] = (V[x] % 100 ) / 10;
			memory[I + 2] = V[x] % 10;
			incrPC();
			break;

		case 0x0055:
			// FX55: Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
			for (int i = 0; i <= x; i++)
				memory[I + i] = V[i];
			incrPC();
			break;

		case 0x0065:
			// FX65: Fills V0 to VX (including VX) with values from memory starting at address I. I is left unmodified
			for (int i = 0; i <= x; i++)
				V[i] = memory[I + i];
			incrPC();
			break;

		default:
			unknownOpcode(opcode);
		}
		break;

	default:
		unknownOpcode(opcode);
	}

}

void Chip8::setKeys(bool a[]) {
	for (int i = 0; i < 16; i++)
		keys[i] = a[i];
}

int Chip8::loadRom(std::string name) {

	// First 0x200 bytes reserved for interpretter (font in our case)
	const int MAX_ROM_SIZE = CH8_MEM_SIZE - 0x200;

	char tempBuffer[MAX_ROM_SIZE];

	// Read ROM file into memory
	std::ifstream rom(name, std::ios::in | std::ios::binary);

	// Check if file was read into stream correctly
	if ((rom.rdstate() & std::ifstream::failbit) != 0 || (rom.rdstate() & std::ifstream::badbit) != 0) {
		std::cerr << "Error opening " << name << std::endl;
		return -1;
	}

	// Check file size
	rom.seekg(0, rom.end);
	int romSize = rom.tellg();
	if (romSize > MAX_ROM_SIZE) {
		std::cerr << "File too large!";
		return ERR_ROM_TOO_BIG;
	}

	// Reset 
	rom.seekg(0, rom.beg);

	// Reset stream flags
	rom.clear();

	// Read file into temporary buffer
	rom.read(tempBuffer, romSize);

	// Check if ROM was read into temporary buffer successfully
	if (rom)
		std::cout << "all " << rom.gcount() << " bytes read successfully.";
	else {
		std::cerr << "error: only " << rom.gcount() << "bytes could be read";
		return ERR_ROM_READ;
	}

	rom.close();

	// Read into memory
	for (int i = 0; i < romSize; i++)
		memory[0x200 + i] = tempBuffer[i];

	return SUCCESS;
}

void Chip8::decrTimers() {

	uint32_t curTime = SDL_GetTicks();

	uint32_t timeDelta = curTime - lastTime;

	// Caps deltaTime in case program is suspended
	if (timeDelta > 100)
		timeDelta = 100;

	lastTime = curTime;

	accumulator += timeDelta;

	// Targets counting down at 60 Hz
	while (accumulator > TIMER_RATE) {
		if (sTimer > 0)
			sTimer--;
		if (dTimer > 0)
			dTimer--;
		accumulator -= TIMER_RATE;
	}
}

void Chip8::clearDisp() {
	for (int i = 0; i < CH8_WIDTH; i++)
		for (int j = 0; j < CH8_HEIGHT; j++)
			gfx[i][j] = 0;
}

void unknownOpcode(uint16_t opcode) {
	std::cerr << "Unknown opcode: " << std::hex << opcode << std::dec << std::endl;
	exit(-3);
}