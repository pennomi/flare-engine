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

#pragma once
#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cmath>
#include <ctime>

using namespace std;

void init(const std::string render_device_name);
void mainLoop (bool debug_event);
void cleanup();
bool done();
int game_ticks();
int simulate(int logic_ticks, bool debug_event, int delay);
void render(int prev_ticks, int delay);
void delay_loop(int prev_ticks, int delay);


std::string parseArg(const string &arg);
std::string parseArgValue(const string &arg);
int main(int argc, char *argv[]);

#endif