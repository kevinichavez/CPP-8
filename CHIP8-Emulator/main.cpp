#include <iostream>
#include <ctime>
#include <string>
#include "SDL.h"
#include "nfd.h"
#include "Chip8.h"

void drawScreen(const Chip8 &c, SDL_Renderer *renderer, int scalex, int scaley);
void setKeys(Chip8 &c, const uint8_t *ks, bool keys[]);

int main(int argc, char *argv[]) {

	// Seed random number generator
	srand(time(0));

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Could not initialize SDL. SDL Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	// Snapshot of keyboard's current state
	const uint8_t *keystate = SDL_GetKeyboardState(NULL);

	// Store values of keyboard keys mapped to CHIP-8
	bool keys[16];

	// Create game window
	SDL_Window *mainWindow = SDL_CreateWindow(
		"Selecting game...",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		CH8_WIDTH * 10,
		CH8_HEIGHT * 10,
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_SHOWN
	);

	if (mainWindow == NULL) {
		std::cerr << "Could not create window. SDL Error: " << SDL_GetError() << std::endl;
		exit(2);
	}

	// Create Renderer
	SDL_Renderer *renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

	// Create texture
	SDL_Texture *texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		CH8_WIDTH * 10,
		CH8_HEIGHT * 10);

	SDL_Event e;

	Chip8 chip;

	// Select ROM file from open file dialogue
	nfdchar_t *outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog("ch8", NULL, &outPath);
	std::string gamePath;

	switch (result) {
	case NFD_OKAY:
		std::cout << "ROM path found successfully!" << std::endl;
		std::cout << outPath << std::endl;
		gamePath = outPath;
		delete outPath;
		break;

	case NFD_CANCEL:
		std::cout << "User pressed cancel." << std::endl << "Exiting program..." << std::endl;
		exit(0);
		break;

	default:
		std::cerr << "Error: " << NFD_GetError() << std::endl;
		exit(3);
	}

	SDL_SetWindowTitle(mainWindow, "CHIP-8 Emulator");

	// Reset all values in Chip8 object
	chip.init();

	// Load ROM into memory
	switch (chip.loadRom(gamePath)) {
	case 0:
		std::cout << "ROM loaded successfully!" << std::endl;
		break;

	case -1:
		exit(4);

	case -2:
		exit(5);

	case -3:
		exit(6);

	default:
		std::cerr << "Something unexpected happened with the file reading process? You shouldn't ever see this!" << std::endl;
		exit(7);
	}

	bool quit = false;
	uint32_t ticks = 0;

	// TODO: Slow down emulation
	while (!quit) {

		// Update which keys are pressed on keyboard
		SDL_PumpEvents();

		// Emulate one CPU cycle
		chip.emulateCycle();
		ticks++;

		// Redraw the screen if CHIP-8 drawflag was set
		if (chip.shouldDraw())
			drawScreen(chip, renderer, 10, 10);

		// Pass currently pressed keys to CHIP-8
		setKeys(chip, keystate, keys);

		// Check if user pressed X button or escape
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT || e.type == SDLK_ESCAPE)
				quit = true;

	}

	// Destroy game window
	SDL_DestroyWindow(mainWindow);

	// Cleanup
	SDL_Quit();
	return 0;
}

void drawScreen(const Chip8 &c, SDL_Renderer *renderer, int scalex, int scaley) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	for (int i = 0; i < CH8_HEIGHT; i++) {
		for (int j = 0; j < CH8_WIDTH; j++) {
			if (c.gfx[j][i]) {
				SDL_Rect pixel;
				pixel.x = j * scalex;
				pixel.y = i * scaley;
				pixel.w = scalex;
				pixel.h = scaley;

				SDL_RenderFillRect(renderer, &pixel);
			}
		}
	}

	SDL_RenderPresent(renderer);
}

void setKeys(Chip8 &c, const uint8_t *ks, bool keys[]) {
	keys[0x1] = ks[SDL_SCANCODE_1]; keys[0x2] = ks[SDL_SCANCODE_2]; keys[0x3] = ks[SDL_SCANCODE_3]; keys[0xC] = ks[SDL_SCANCODE_4];
	keys[0x4] = ks[SDL_SCANCODE_Q]; keys[0x5] = ks[SDL_SCANCODE_W]; keys[0x6] = ks[SDL_SCANCODE_E]; keys[0xD] = ks[SDL_SCANCODE_R];
	keys[0x7] = ks[SDL_SCANCODE_A]; keys[0x8] = ks[SDL_SCANCODE_S]; keys[0x9] = ks[SDL_SCANCODE_D]; keys[0xE] = ks[SDL_SCANCODE_F];
	keys[0xA] = ks[SDL_SCANCODE_Z]; keys[0x0] = ks[SDL_SCANCODE_X]; keys[0xB] = ks[SDL_SCANCODE_C]; keys[0xF] = ks[SDL_SCANCODE_V];

	c.setKeys(keys);
}
