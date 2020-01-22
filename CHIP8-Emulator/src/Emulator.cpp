#include "Emulator.h"
#include <nfd.h>
#include <SDL.h>
#include <iostream>
#include <cstdint>
#include "constants.h"

Emulator::Emulator() {
	reset();
}

Emulator::~Emulator() {
	SDL_Quit();
}

void Emulator::reset() {
	m_fpsTimer.stop();
	m_scaleWidth = DEFAULT_SCALE;
	m_scaleHeight = DEFAULT_SCALE;
	m_width = CH8_WIDTH * m_scaleWidth;
	m_height = CH8_HEIGHT * m_scaleHeight;
	m_gamePath = "";
	m_totalFrames = 0;
}

bool Emulator::selectGame() {
	// Select ROM file from open file dialogue
	nfdchar_t* outPath = nullptr;
	nfdresult_t result = NFD_OpenDialog("ch8", NULL, &outPath);

	switch (result) {
	case NFD_OKAY:
		std::cout << "ROM path found successfully!" << std::endl;
		std::cout << outPath << std::endl;
		m_gamePath = outPath;
		delete outPath;
		return true;
		break;

	case NFD_CANCEL:
		std::cout << "User pressed cancel." << std::endl << "Exiting program..." << std::endl;
		return false;
		break;

	default:
		std::cerr << "Error: " << NFD_GetError() << std::endl;
		return false;
	}
}

void Emulator::drawScreen() {
	// Set render fill color to black
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

	// Clear all current pixels by making the screen black
	SDL_RenderClear(m_renderer);

	// Draw pixels in white
	SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

	// Make every non-zero value in the graphics array a white rectangle
	for (int i = 0; i < CH8_HEIGHT; i++) {
		for (int j = 0; j < CH8_WIDTH; j++) {
			if (chip.gfx[j][i]) {
				SDL_Rect pixel;
				pixel.x = j * m_scaleWidth;
				pixel.y = i * m_scaleHeight;
				pixel.w = m_scaleWidth;
				pixel.h = m_scaleHeight;

				SDL_RenderFillRect(m_renderer, &pixel);
			}
		}
	}

	// Present the pixel positions to the user
	SDL_RenderPresent(m_renderer);

	// Add one frame to the total
	++m_totalFrames;
}

void Emulator::sendInput(const uint8_t* ks, bool keys[]) {
	keys[0x1] = ks[SDL_SCANCODE_1]; keys[0x2] = ks[SDL_SCANCODE_2]; keys[0x3] = ks[SDL_SCANCODE_3]; keys[0xC] = ks[SDL_SCANCODE_4];
	keys[0x4] = ks[SDL_SCANCODE_Q]; keys[0x5] = ks[SDL_SCANCODE_W]; keys[0x6] = ks[SDL_SCANCODE_E]; keys[0xD] = ks[SDL_SCANCODE_R];
	keys[0x7] = ks[SDL_SCANCODE_A]; keys[0x8] = ks[SDL_SCANCODE_S]; keys[0x9] = ks[SDL_SCANCODE_D]; keys[0xE] = ks[SDL_SCANCODE_F];
	keys[0xA] = ks[SDL_SCANCODE_Z]; keys[0x0] = ks[SDL_SCANCODE_X]; keys[0xB] = ks[SDL_SCANCODE_C]; keys[0xF] = ks[SDL_SCANCODE_V];

	chip.setKeys(keys);
}

int Emulator::runGame() {
	// Load ROM into memory
	int result = chip.loadRom(m_gamePath);
	if (result != SUCCESS) {
		return result;
	}

	m_gameWindow = SDL_CreateWindow(
		"CHIP 8",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_width,
		m_height,
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_SHOWN
	);

	if (m_gameWindow == NULL) {
		std::cerr << "Could not create window. SDL Error: " << SDL_GetError() << std::endl;
		return ERR_INIT_SDL;
	}

	// Create Renderer
	m_renderer = SDL_CreateRenderer(m_gameWindow, -1, SDL_RENDERER_ACCELERATED);

	// Create m_texture
	m_texture = SDL_CreateTexture(m_renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		m_width,
		m_height);

	// Snapshot of keyboard's current state
	const uint8_t* keystate = SDL_GetKeyboardState(NULL);

	// Store values of keyboard keys mapped to CHIP-8
	bool keys[16];

	SDL_Event e;

	bool quit = false;
	uint32_t ticks = 0;
	uint16_t speed = 0;
	uint32_t currentTime, lastTime, deltaTime, accumulator;
	accumulator = 0;
	currentTime = SDL_GetTicks();
	lastTime = currentTime;
	std::string title;
	m_fpsTimer.start();

	// main loop
	while (!quit) {

		// Reduce delay and sound timers
		chip.decrTimers();

		// Update which keys are pressed on keyboard
		SDL_PumpEvents();

		// while (ticks % speed) {
			// Emulate one CPU cycle
		chip.emulateCycle();
		chip.decrTimers();
		ticks++;

		// Redraw the screen if CHIP-8 drawflag was set
		if (chip.shouldDraw()) {
			drawScreen();
			SDL_Delay(1);
	}
		// }

		/* Old framerate regulating code
		// Slows down emulation
		currentTime = SDL_GetTicks();
		deltaTime = currentTime - lastTime;
		if (deltaTime > 100)
			deltaTime = 100;
		lastTime = currentTime;
		accumulator += deltaTime;

		while (accumulator > TARGET_FRAMERATE) {
			if (!(ticks % speed))
				ticks++;
			accumulator -= TARGET_FRAMERATE;
		}
		*/

		while (getAvgFPS() > TARGET_FRAMERATE + speed) {

		}

		// Pass currently pressed keys to CHIP-8
		sendInput(keystate, keys);

		// Check if user pressed X button or escape
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = true;
			if (e.type == SDL_KEYDOWN) {
				if (keystate[SDL_SCANCODE_COMMA]) {
					if (speed > 1)
						speed -= 1;
					std::cout << "Emulation speed slowed to " << speed << std::endl;
				}
				else if (keystate[SDL_SCANCODE_PERIOD]) {
					if (speed < 500)
						speed += 1;
					std::cout << "Emulation speed sped up to " << speed << std::endl;
				}
				else if (keystate[SDL_SCANCODE_SLASH]) {
					std::cout << "Current FPS: " << (int)(getAvgFPS() + 0.5) << '\n';
				}
				else if (keystate[SDL_SCANCODE_ESCAPE]) {
					quit = true;
				}

			}

		}
	}

	return SUCCESS;
}

int Emulator::runGame(std::string path) {
	m_gamePath = path;
	return runGame();
}

double Emulator::getAvgFPS() {
	// we divide by 1000 to get time passed in seconds
	double avg = m_totalFrames / (m_fpsTimer.getTicks() / 1000.0);

	// When total number of ticks is low, average will be too high due to the math, so we correct it here
	if (avg > 2000000)
		return 0;

	return avg;
}