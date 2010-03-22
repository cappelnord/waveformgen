/*
 waveformgen.c
 Created by Patrick Borgeat on 20.03.2010
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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sndfile.h>
#include <gd.h>

#include "waveformgen.h"

// private
char* lastErrorMessage = "No Error so far.";
void drawTimeline(gdImagePtr im, WFGO* options);

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
	
	long framesPerLine = (long) sfinfo.frames / width;
	long samplesPerLine = framesPerLine * sfinfo.channels;
	
	
	// although one could think, that these values are flexible, the loops
	// below only work in these configurations (1/all or all/1)
	
	int channelsPerDrawing = 1;
	int drawnChannels = sfinfo.channels;
	
	if(options->mixChannels)
	{
		channelsPerDrawing = sfinfo.channels;
		drawnChannels = 1;
	}
		
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
	
	// calculate how large one drawing should be
	
	int drawHeight = height;
	
	// leave space for the timeline
	if(options->drawTimeline)
		drawHeight -= 10;
	
	// subtract spacing
	drawHeight = drawHeight - ((drawnChannels - 1) * options->channelSpacing);
	
	// divide by drawnChannels
	drawHeight = drawHeight / drawnChannels;
	
	// background color
	int bgColor = gdImageColorAllocate(im,WFG_THREE_INTS_FROM_ARRAY(options->bgColor));
	
	if(options->transparentBg) {
		gdImageColorTransparent(im,bgColor);
	}
	
	int rmsColor =  gdImageColorAllocate(im, WFG_THREE_INTS_FROM_ARRAY(options->rmsColor));
	int peakColor = gdImageColorAllocate(im, WFG_THREE_INTS_FROM_ARRAY(options->peakColor));
	
	// too many nested loops ...
	for(int i = 0; i < width; i++)
	{
		if(sf_read_float(sfile, buffer, samplesPerLine) != samplesPerLine)
		{
			lastErrorMessage = "Could not read samples from audio file!";
			sf_close(sfile);
			free(buffer);
			gdImageDestroy(im);
			return false;			
		}
		
		int drawOffset = 0;

		for(int d = 0; d < drawnChannels; d++)
		{
			double val = 0.0;
			
			float peakP = 0.0;
			float peakM = 0.0;
			
			for(long e = 0; e < framesPerLine; e++)
			{
				for(int f = 0; f < channelsPerDrawing; f++)
				{
					float smpl = buffer[e * drawnChannels + d];
					val = val + (smpl*smpl);
					
					if(peakM > smpl)
						peakM = smpl;
					
					if(peakP < smpl)
						peakP = smpl;
				}
			}
			
			val = val / (float) (framesPerLine * channelsPerDrawing);
			val = sqrt(val);
			
			int peakPP = drawHeight/2 -  peakP * (drawHeight/2);
			int peakMP = drawHeight/2 + (peakM * -1.0 * (drawHeight/2));
						
			// avoid rounding errors when peak is very small
			if(peakP > 0.001 || peakM < -0.001) 
				gdImageLine(im, i,peakPP + drawOffset,i,peakMP + drawOffset, peakColor);
			
			int rmsSize;
			rmsSize = val * (double) (drawHeight/2);
			gdImageLine(im, i,drawHeight/2 - rmsSize + drawOffset,i,drawHeight/2 + rmsSize + drawOffset, rmsColor);
			
			drawOffset += drawHeight + options->channelSpacing;
		}
	}

	
	sf_close(sfile);
	free(buffer);
	
	if(options->drawTimeline)
		drawTimeline(im, options);
	
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

void drawTimeline(gdImagePtr im, WFGO* options)
{
	// TODO: implement
}

WFGO* wfg_defaultOptions()
{
	WFGO* options = malloc(sizeof(struct wfg_options));
	memset(options,0,sizeof(struct wfg_options));
	
	options->width = 800;
	options->height = 120;
	options->transparentBg = false;
		
	WFG_FILL_INT_COLOR_ARRAY(options->bgColor, 255, 255, 255);
	WFG_FILL_INT_COLOR_ARRAY(options->rmsColor, 20, 20, 20);
	WFG_FILL_INT_COLOR_ARRAY(options->peakColor, 80, 80, 80);
	
	options->mixChannels = false;
	options->channelSpacing = 5;
	
	options->drawTimeline = false;
	WFG_FILL_INT_COLOR_ARRAY(options->tlColor, 20, 20, 20);
	
	return options;
}

char* wfg_lastErrorMessage()
{
	return lastErrorMessage;
}