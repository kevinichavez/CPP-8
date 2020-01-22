#ifndef EMULATOR_H
#define EMULATOR_H

#include <string>
#include "Chip8.h"
#include <SDL.h>

class Emulator {
public:
	Emulator();
	~Emulator();

	bool selectGame();
	int runGame();
	int runGame(std::string path);
	void reset();
private:
	Chip8 chip;
	std::string gamePath;
	SDL_Window* gameWindow;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	int width, height, scaleWidth, scaleHeight;

	void drawScreen();
	void sendInput(const uint8_t* ks, bool keys[]);
};

#endif