/*
 waveformgen.h
 Created by Patrick Borgeat on 21.03.2010
*/

#include <stdbool.h>

#ifndef WAVEFORMGEN_H
#define WAVEFORMGEN_H

#define WAVEFORMGEN_VERSION "0.11"

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
	
	bool drawMarkEveryMinute;

	bool scaleRms;
};

typedef struct wfg_options WFGO;

WFGO* wfg_defaultOptions();
bool wfg_generateImage(char* audioFileName, char* pictureFileName, WFGO* options);
char* wfg_lastErrorMessage();

#define WFG_PACK_RGB(_POINTER,_R,_G,_B) _POINTER[0] = _R;_POINTER[1] = _G; _POINTER[2] = _B
#define WFG_UNPACK_RGB(_POINTER) _POINTER[0], _POINTER[1], _POINTER[2]

#endif