/*
Copyright © 2011-2012 Clint Bellanger
Copyright © 2013-2014 Henrik Andersson
Copyright © 2013 Kurt Rinnert

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>

using namespace std;

#include "main.h"
#include "Settings.h"
#include "Stats.h"
#include "GameSwitcher.h"
#include "SharedResources.h"
#include "UtilsFileSystem.h"

GameSwitcher *gswitch;

/**
 * Game initialization.
 */
void init(const std::string render_device_name) {
	srand((unsigned int)time(NULL));
	setPaths();
	setStatNames();

	// SDL Inits
	if ( SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0 ) {
		fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	// Shared Resources set-up

	mods = new ModManager();

	if (!mods->haveFallbackMod()) {
		fprintf(stderr, "Could not find the default mod in the following locations:\n");
		if (dirExists(PATH_DATA + "mods")) fprintf(stderr, "%smods/\n", PATH_DATA.c_str());
		if (dirExists(PATH_USER + "mods")) fprintf(stderr, "%smods/\n", PATH_USER.c_str());
		fprintf(stderr, "\nA copy of the default mod is in the \"mods\" directory of the flare-engine repo.\n");
		fprintf(stderr, "The repo is located at: https://github.com/clintbellanger/flare-engine\n");
		fprintf(stderr, "Try again after copying the default mod to one of the above directories.\nExiting.\n");
		exit(1);
	}

	if (!loadSettings()) {
		fprintf(stderr, "%s",
				("Could not load settings file: ‘" + PATH_CONF + FILE_SETTINGS + "’.\n").c_str());
		exit(1);
	}

	msg = new MessageEngine();
	font = new FontEngine();
	anim = new AnimationManager();
	comb = new CombatText();
	inpt = new InputState();
	icons = NULL;

	// Load tileset options (must be after ModManager is initialized)
	loadTilesetSettings();

	// Load miscellaneous settings
	loadMiscSettings();

	// Create render Device and Rendering Context.
	render_device = getRenderDevice(render_device_name);
	int status = render_device->createContext(VIEW_W, VIEW_H);

	if (status == -1) {

		fprintf (stderr, "Error during SDL_SetVideoMode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	// initialize share icons resource
	SharedResources::loadIcons();

	// Set Gamma
	if (CHANGE_GAMMA)
		render_device->setGamma(GAMMA);

	if (AUDIO && Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 1024)) {
		fprintf (stderr, "Error during Mix_OpenAudio: %s\n", SDL_GetError());
		AUDIO = false;
	}

	snd = new SoundManager();

	// initialize Joysticks
	if(SDL_NumJoysticks() == 1) {
		printf("1 joystick was found:\n");
	}
	else if(SDL_NumJoysticks() > 1) {
		printf("%d joysticks were found:\n", SDL_NumJoysticks());
	}
	else {
		printf("No joysticks were found.\n");
		ENABLE_JOYSTICK = false;
	}
	for(int i = 0; i < SDL_NumJoysticks(); i++) {
		printf("  Joy %d) %s\n", i, inpt->getJoystickName(i).c_str());
	}
	if ((ENABLE_JOYSTICK) && (SDL_NumJoysticks() > 0)) {
		joy = SDL_JoystickOpen(JOYSTICK_DEVICE);
		printf("Using joystick #%d.\n", JOYSTICK_DEVICE);
	}

	// Set sound effects volume from settings file
	if (AUDIO)
		Mix_Volume(-1, SOUND_VOLUME);

	gswitch = new GameSwitcher();

	curs = new CursorManager();
}

int simulate(int logic_ticks, bool debug_event, int delay) {
	int now_ticks = SDL_GetTicks();
	int loops = 0;
	while (now_ticks > logic_ticks && loops < MAX_FRAMES_PER_SEC) {
		// Frames where data loading happens (GameState switching and map loading)
		// take a long time, so our loop here will think that the game "lagged" and
		// try to compensate. To prevent this compensation, we mark those frames as
		// "loading frames" and update the logic ticker without actually executing logic.
		if (gswitch->isLoadingFrame()) {
			logic_ticks = now_ticks;
			break;
		}

		SDL_PumpEvents();
		inpt->handle(debug_event);
		gswitch->logic();
		inpt->resetScroll();

		logic_ticks += delay;
		loops++;
	}
	return logic_ticks;
}

void render(int prev_ticks, int delay) {
	render_device->blankScreen();
	gswitch->render();

	// display the FPS counter
	// if the frame completed quickly, we estimate the delay here
	int now_ticks = SDL_GetTicks();
	int delay_ticks = 0;
	if (now_ticks - prev_ticks < delay) {
		delay_ticks = delay - (now_ticks - prev_ticks);
	}
	if (now_ticks+delay_ticks - prev_ticks != 0) {
		gswitch->showFPS(1000 / (now_ticks+delay_ticks - prev_ticks));
	}

	render_device->commitFrame();
}

void delay_loop(int prev_ticks, int delay) {
	int now_ticks = SDL_GetTicks();
	if (now_ticks - prev_ticks < delay) {
		SDL_Delay(delay - (now_ticks - prev_ticks));
	}
}

int game_ticks() {
    return SDL_GetTicks();
}

bool done() {
    return gswitch->done || inpt->done;
}

void mainLoop (bool debug_event) {
	int delay = int(floor((1000.f/MAX_FRAMES_PER_SEC)+0.5f));
	int logic_ticks = SDL_GetTicks();

	while ( !done() ) {
		int now_ticks = SDL_GetTicks();
		int prev_ticks = now_ticks;

		// Execute the game logic
		logic_ticks = simulate(logic_ticks, debug_event, delay);

		// Render to screen
		render(prev_ticks, delay);

		// delay quick frames
		delay_loop(prev_ticks, delay);
	}
}

void cleanup() {
	delete gswitch;

	delete anim;
	delete comb;
	delete font;
	delete inpt;
	delete mods;
	delete msg;
	delete snd;
	delete curs;

	Mix_CloseAudio();

	if (render_device)
		render_device->destroyContext();
	delete render_device;

	SDL_Quit();
}

std::string parseArg(const std::string &arg) {
	std::string result = "";

	// arguments must start with '--'
	if (arg.length() > 2 && arg[0] == '-' && arg[1] == '-') {
		for (unsigned i = 2; i < arg.length(); ++i) {
			if (arg[i] == '=') break;
			result += arg[i];
		}
	}

	return result;
}

std::string parseArgValue(const std::string &arg) {
	std::string result = "";
	bool found_equals = false;

	for (unsigned i = 0; i < arg.length(); ++i) {
		if (found_equals) {
			result += arg[i];
		}
		if (arg[i] == '=') found_equals = true;
	}

	return result;
}

int main(int argc, char *argv[]) {
	bool debug_event = false;
	bool done = false;
	std::string render_device_name = "";

	for (int i = 1 ; i < argc; i++) {
		std::string arg = std::string(argv[i]);
		if (parseArg(arg) == "debug-event") {
			debug_event = true;
		}
		else if (parseArg(arg) == "data-path") {
			CUSTOM_PATH_DATA = parseArgValue(arg);
			if (!CUSTOM_PATH_DATA.empty() && CUSTOM_PATH_DATA.at(CUSTOM_PATH_DATA.length()-1) != '/')
				CUSTOM_PATH_DATA += "/";
		}
		else if (parseArg(arg) == "version") {
			printf("%s\n", getVersionString().c_str());
			done = true;
		}
		else if (parseArg(arg) == "renderer") {
			render_device_name = parseArgValue(arg);
		}
		else if (parseArg(arg) == "help") {
			printf("\
--help           Prints this message.\n\n\
--version        Prints the release version.\n\n\
--data-path      Specifies an exact path to look for mod data.\n\n\
--debug-event    Prints verbose hardware input information.\n\n\
--renderer       Specifies the rendering backend to use. The default is 'sdl'.\n");
			done = true;
		}
	}

	if (!done) {
		init(render_device_name);
		mainLoop(debug_event);
		cleanup();
	}

	return 0;
}
