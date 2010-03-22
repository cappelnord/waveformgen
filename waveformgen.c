/*
 waveformgen.c
 Created by Patrick Borgeat on 20.03.2010
 http://www.cappel-nord.de
 
 This file is part of waveformgen.
 
 Foobar is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Foobar is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with waveformgen. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sndfile.h>
#include <gd.h>

#include "waveformgen.h"

char* lastErrorMessage = "No Error so far.";

int main (int argc, char *argv[])
{

}

bool wfg_generateImage(char* audioFileName, char* pictureFileName, WFGO* options)
{	
	int width = options->width;
	int height = options->height;

	// Initial audio part
	
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));
	
	SNDFILE* sfile =  sf_open(audioFileName, SFM_READ, &sfinfo);
	
	if(sfile == NULL)
	{
		lastErrorMessage = "Could not open input file!";
		return false;
	}
	
	long samplesPerLine = (((long) sfinfo.frames) * sfinfo.channels) / width;
	
	// we need to assure, that we read a complete frames
	samplesPerLine = samplesPerLine - (samplesPerLine % sfinfo.channels);
		
	float * buffer = malloc(sizeof(float) * samplesPerLine);
	
	// malloc fail
	if(buffer == NULL)
	{
		lastErrorMessage = "Could not allocate memory!";
		sf_close(sfile);
		return false;
	}
	
	// Allocate Image
	gdImagePtr im = gdImageCreate(width,height);
	
	if(im == NULL)
	{
		lastErrorMessage = "Could not allocate image!";
		free(buffer);
		sf_close(sfile);
		return false;
	}
	
	if(options->drawTimeline) {
		// leave space for the timeline
		height -= 10;
	}
	
	// background color
	int bgColor = gdImageColorAllocate(im,WFG_THREE_INTS_FROM_ARRAY(options->bgColor));
	
	if(options->transparentBg) {
		gdImageColorTransparent(im,bgColor);
	}
	
	int rmsColor =  gdImageColorAllocate(im, WFG_THREE_INTS_FROM_ARRAY(options->rmsColor));
	int peakColor = gdImageColorAllocate(im, WFG_THREE_INTS_FROM_ARRAY(options->peakColor));
		
	for(int i = 0; i < width; i++)
	{
		double val = 0.0;
		
		float peakP = 0.0;
		float peakM = 0.0;
		
		if(sf_read_float(sfile, buffer, samplesPerLine) != samplesPerLine)
		{
			lastErrorMessage = "Could not read samples from audio file!";
			sf_close(sfile);
			free(buffer);
			gdImageDestroy(im);
			return false;			
		}
		
		for(long e = 0; e < samplesPerLine; e++)
		{
			val = val + (buffer[e] * buffer[e]);
			
			if(peakM < buffer[e])
				peakM = buffer[e];
			
			if(peakP > buffer[e])
				peakP = buffer[e];
		}
		
		val = val / (float) samplesPerLine;
		val = sqrt(val);
		
		
		int peakPP = height/2 -  peakP * (height/2);
		int peakMP = height/2 + (peakM * -1.0 * (height/2));
		
		gdImageLine(im, i,peakPP,i,peakMP, peakColor);
		
		int rmsSize;
		rmsSize = val * (double) (height/2);
		gdImageLine(im, i,height/2 - rmsSize,i,height/2 + rmsSize, rmsColor);
		
	}
	
	sf_close(sfile);
	free(buffer);
	
	// write out file
	FILE* file = fopen(pictureFileName,"wb");
	if(file == NULL)
	{
		lastErrorMessage = "Could not open output file!";
		gdImageDestroy(im);
	}
	
	gdImagePng(im,file);
	
	fclose(file);
	gdImageDestroy(im);
	
	return true;
}

WFGO* wfg_defaultOptions()
{
	WFGO* options = malloc(sizeof(struct wfg_options));
	memset(options,0,sizeof(struct wfg_options));
	
	options->width = 800;
	options->height = 100;
	options->transparentBg = false;
		
	WFG_FILL_INT_COLOR_ARRAY(options->bgColor, 255, 255, 255);
	WFG_FILL_INT_COLOR_ARRAY(options->rmsColor, 20, 20, 20);
	WFG_FILL_INT_COLOR_ARRAY(options->peakColor, 80, 80, 80);
	
	options->drawTimeline = true;
	
	WFG_FILL_INT_COLOR_ARRAY(options->tlColor, 20, 20, 20);
	
	return options;
}

char* wfg_lastErrorMessage()
{
	return lastErrorMessage;
}