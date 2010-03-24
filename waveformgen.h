/*
 waveformgen.h
 Created by Patrick Borgeat on 21.03.2010
 http://www.cappel-nord.de
 
 This file is part of waveformgen.
 
 waveformgen is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 waveformgen is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with waveformgen. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdbool.h>

#ifndef WAVEFORMGEN_H
#define WAVEFORMGEN_H

#define WAVEFORMGEN_VERSION "0.1"

struct wfg_options {
	
	int width;
	int height;
	bool transparentBg;
	
	int bgColor[3];
	int rmsColor[3];
	int peakColor[3];
	
	bool mixChannels;
	int channelSpacing;
	
	bool drawTimeline;
	int tlColor[3];
	int tlOddColor[3];
	int tlBgColor[3];
	int markSpacing;
};

typedef struct wfg_options WFGO;

WFGO* wfg_defaultOptions();
bool wfg_generateImage(char* audioFileName, char* pictureFileName, WFGO* options);
char* wfg_lastErrorMessage();

#define WFG_FILL_INT_COLOR_ARRAY(_POINTER,_R,_G,_B) _POINTER[0] = _R;_POINTER[1] = _G; _POINTER[2] = _B
#define WFG_THREE_INTS_FROM_ARRAY(_POINTER) _POINTER[0], _POINTER[1], _POINTER[2]

#endif