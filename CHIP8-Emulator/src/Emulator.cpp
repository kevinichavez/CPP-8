#include "Emulator.h"
#include <nfd.h>
#include <SDL.h>
#include <iostream>
#include <cstdint>
#include <cmath>
#include "constants.h"

Emulator::Emulator() {
	init();
}

Emulator::Emulator(uint16_t flags) {
	init();
	if (flags & DISABLE_WRAP)
		chip.disableSpriteWrap();
	if (flags & DISABLE_SDL_DELAY)
		m_useSDLdelay = false;
	if (flags & DISABLE_THROTTLE)
		m_throttleSpeed = false;
}

Emulator::~Emulator() {
	SDL_CloseAudioDevice(m_audioDev);
	SDL_Quit();
}

void Emulator::init() {

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Could not initialize SDL. SDL Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	// Setting up sound
	SDL_zero(m_spec);
	m_spec.freq = SOUND_FREQUENCY;
	m_spec.samples = SOUND_NUM_SAMPLES;
	m_spec.channels = SOUND_NUM_CHANNELS;
	m_spec.format = AUDIO_S16SYS;
	m_spec.callback = NULL;

	m_audioDev = SDL_OpenAudioDevice(NULL, 0, &m_spec, NULL, 0);

	m_gain = SOUND_DEFAULT_GAIN;

	setupWave();

	reset();
}

void Emulator::reset() {
	m_numStoredFPS = 0;
	m_scaleWidth = DEFAULT_SCALE;
	m_scaleHeight = DEFAULT_SCALE;
	m_width = CH8_WIDTH * m_scaleWidth;
	m_height = CH8_HEIGHT * m_scaleHeight;
	m_gamePath = "";
	m_totalFrames = 0;
	m_paused = false;
	m_isPlayingSound = false;
	SDL_ClearQueuedAudio(m_audioDev);
}

void Emulator::togglePause() {
	if (!m_paused) {
		SDL_PauseAudioDevice(m_audioDev, 1);
	}
	else {
		if (chip.getSoundTimer() > 0)
			SDL_PauseAudioDevice(m_audioDev, 0);
	}

	m_paused = !m_paused;
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

// TODO: Replace with code that'll allow rebinding
// Translate keyboard input to CHIP-8 buttons
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
		"CHIP-8",
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

	// Create Texture
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
	double speed = 1.0;
	uint64_t prevFrame = SDL_GetPerformanceCounter();
	uint64_t currentFrame;
	double secondsBetweenFrames, frameDiff;
	m_paused = false;
	m_numStoredFPS = 0;
	fillAudioQueue(SOUND_INITIAL_BUFFER_TIME);

	// main loop
	while (!quit) {

		// Mostly debug; will remove all except SDL_QUIT in future version
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = true;
			if (e.type == SDL_KEYDOWN) {
				if (keystate[SDL_SCANCODE_COMMA]) {
					if (speed > 0)
						speed -= 0.1;
					std::cout << "Emulation speed slowed to " << (int)(speed * 100 + 0.5) << "%\n";
				}
				else if (keystate[SDL_SCANCODE_PERIOD]) {
					if (speed < 500)
						speed += 0.1;
					std::cout << "Emulation speed sped up to " << (int)(speed * 100 + 0.5) << "%\n";
				}
				else if (keystate[SDL_SCANCODE_SLASH]) {
					std::cout << "Sprite Wrap ";
					if (!chip.wrapIsEnabled()) {
						chip.enableSpriteWrap();
						std::cout << "enabled\n";
					}
					else {
						chip.disableSpriteWrap();
						std::cout << "disabled\n";
					}
				}
				else if (keystate[SDL_SCANCODE_ESCAPE]) {
					if (m_paused)
						std::cout << "Unpaused\n";
					else std::cout << "Paused\n";
					togglePause();
				}
				else if (keystate[SDL_SCANCODE_M]) {
					std::cout << "Throttle ";
					if (m_throttleSpeed)
						std::cout << "disabled\n";
					else std::cout << "enabled\n";
					toggleThrottle();
				}

			}

		}

		if (!m_paused) {

			// Pass currently pressed keys to CHIP-8
			sendInput(keystate, keys);

			chip.emulateCycle();
			chip.decrTimers();

			if (SDL_GetQueuedAudioSize(m_audioDev) < SOUND_BUFFER_SIZE)
				pushSample();

			if (!m_isPlayingSound && chip.getSoundTimer() > 0) {
				m_isPlayingSound = true;
				SDL_PauseAudioDevice(m_audioDev, 0);
			}
			else if (chip.getSoundTimer() == 0) {
				m_isPlayingSound = false;
				SDL_PauseAudioDevice(m_audioDev, 1);
			}

			// Redraw the screen if CHIP-8 drawflag was set
			if (chip.shouldDraw()) {
				drawScreen();

				// Slow down emulation speed
				if (m_throttleSpeed) {
					if (m_useSDLdelay)
						SDL_Delay(SDL_DELAY_VALUE);

					do {
						currentFrame = SDL_GetPerformanceCounter();
						// Fill audio queue while we're waiting
						if(SDL_GetQueuedAudioSize(m_audioDev) < SOUND_BUFFER_SIZE)
							pushSample();
						frameDiff = currentFrame - prevFrame;
						secondsBetweenFrames = frameDiff / (double)SDL_GetPerformanceFrequency();
					} while (secondsBetweenFrames < TARGET_FRAMETIME_SECONDS * (1.0 / speed));

					prevFrame = currentFrame;
				}

			}
		}
		else if (m_throttleSpeed && m_useSDLdelay)
			SDL_Delay(SDL_DELAY_VALUE);
		
	}

	SDL_PauseAudioDevice(m_audioDev, 1);

	return SUCCESS;
}

int Emulator::runGame(std::string path) {
	m_gamePath = path;
	return runGame();
}

// Returns average of time between last 10 frames
double Emulator::getFPS() {
	if (m_numStoredFPS != MAX_STORED_FPS_VALS)
		return 0;
	double total = 0;
	for (int i = 0; i < MAX_STORED_FPS_VALS; i++)
		total += m_previousFPS[i];
	return (total / MAX_STORED_FPS_VALS);
}

void Emulator::fillAudioQueue(int s) {
	long numSamples = m_spec.freq * s;

	for (long i = 0; i < numSamples; i++) {
		pushSample();
	}
}

void Emulator::setupWave() {
	m_square.frequency = SOUND_DEFAULT_PLAY_FREQUENCY;
	m_square.position = 0;

	// Our epsilon is arbitrary
	const double epsilon = 0.000000001;

	double rad = 0, temp = 0;
	int16_t sample;
	for (int i = 0; i < SOUND_FREQUENCY && temp <= (1.0 - epsilon); i++) {
		rad += ((2 * M_PI) / m_square.frequency);

		// We subtract pi/2 to shift the graph so that we start at -1 and end at 1 to get our cycle
		temp = sin(rad - (M_PI / 2));

		// Now it's a square wave
		if (temp > 0)
			sample = 1;
		else if (temp < 0)
			sample = -1;
		else sample = 0;

		m_square.sampleVals.push_back(sample);
	}

}

void Emulator::pushSample() {
	int16_t sample = m_square.sampleVals[m_square.position] * m_gain;

	SDL_QueueAudio(m_audioDev, &sample, sizeof(int16_t));
	m_square.position++;

	if (m_square.position >= m_square.sampleVals.size())
		m_square.position = 0;
}