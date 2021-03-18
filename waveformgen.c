/*
 waveformgen.c
 Created by Patrick Borgeat on 20.03.2010
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sndfile.h>
#include <gd.h>

#include "waveformgen.h"
#include "minifont.h"

#define WFG_STRING_BUFFER_SIZE 7
#define ZERO_CHAR 48 // ascii char 0

// private
char* lastErrorMessage = "No Error so far.";
void drawTimeline(gdImagePtr im, WFGO* options, int seconds);
void drawNumber(gdImagePtr im, int number, int x, int y, int color);
int drawNumberString(gdImagePtr im, char* buffer, int x, int y, int color, bool doDraw);
void fillStringWithTime(char * string, int seconds);


int markSpacings[] = {1,2,5,10,15,30,1 * 60,2 * 60,5 * 60,10 * 60,15 * 60,30 * 60,60 * 60};

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
	
	int seconds = sfinfo.frames / sfinfo.samplerate;
	
	
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
		drawHeight -= 13;
	
	// subtract spacing
	drawHeight = drawHeight - ((drawnChannels - 1) * options->channelSpacing);
	
	// divide by drawnChannels
	drawHeight = drawHeight / drawnChannels;
	
	// background color
	int bgColor = gdImageColorAllocate(im,WFG_UNPACK_RGB(options->bgColor));
	
	if(options->transparentBg) {
		gdImageColorTransparent(im,bgColor);
	}
	
	int rmsColor =  gdImageColorAllocate(im, WFG_UNPACK_RGB(options->rmsColor));
	int peakColor = gdImageColorAllocate(im, WFG_UNPACK_RGB(options->peakColor));
	
	int maxRms = 0;
	int rmsSizes[width];
	int drawOffsets[width];

	int centerPoint = drawHeight/2;
	
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
			
			int rmsSize = val * (double) (centerPoint);

			if (rmsSize > maxRms)
				maxRms = rmsSize;

			if(!options->scaleRms) 
			{
				double ddrawHeight = drawHeight;
				int peakPP = centerPoint - round(peakP * (ddrawHeight/2.0));
				int peakMP = centerPoint + round(peakM * -1.0 * (ddrawHeight/2.0));

				// render immediately if not scaling
				gdImageLine(im, i,peakPP + drawOffset,i,peakMP + drawOffset, peakColor);
				gdImageLine(im, i, centerPoint + rmsSize + drawOffset, i, centerPoint - rmsSize + drawOffset, rmsColor);
			}
			else 
			{
				// record our values for later rendering when we are scaling
				rmsSizes[i] = rmsSize;
				drawOffsets[i] = drawOffset;
			}
			
			drawOffset += drawHeight + options->channelSpacing;
		}
	}

	if(options->scaleRms)
	{
		float scaleFactor = centerPoint / (float)maxRms;

		for(int i = 0; i < width; i++)
		{
			int scaledRms = ceil(rmsSizes[i] * scaleFactor);
			int y1 = centerPoint - scaledRms + drawOffsets[i];
			int y2 = centerPoint + scaledRms + drawOffsets[i];
			gdImageLine(im, i, y1, i, y2, rmsColor);
		}
	}
	
	sf_close(sfile);
	free(buffer);
	
	if(options->drawTimeline)
		drawTimeline(im, options, seconds);
	
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

void drawTimeline(gdImagePtr im, WFGO* options, int seconds)
{
	int color =  gdImageColorAllocate(im, WFG_UNPACK_RGB(options->tlColor));
	int oddColor = gdImageColorAllocate(im, WFG_UNPACK_RGB(options->tlOddColor));
	int bgColor =  gdImageColorAllocate(im, WFG_UNPACK_RGB(options->tlBgColor));

	
	char cbuf[WFG_STRING_BUFFER_SIZE];	
	
	int y = options->height - 8;
	int w = options->width;
	
	gdImageFilledRectangle(im, 0,y -3 ,w,options->height,bgColor);
	
	fillStringWithTime(cbuf, 0);
	drawNumberString(im, cbuf, 1,y,color,true);
	gdImageLine(im, 0,y-4,0,y-2,color);
	
	fillStringWithTime(cbuf, seconds);
	drawNumberString(im, cbuf, w - drawNumberString(im,cbuf,0,0,color,false),y, color, true);
	gdImageLine(im, w-1,y-4,w-1,y-2,color);

	
	if(options->drawMarkEveryMinute)
	{
		for(int i = 0; i < seconds; i += 60)
		{
			int x = ((float) i / (float) seconds) * w;
			gdImageLine(im, x, y-4, x, y-2, color);
		}
	}
		
	
	int num = (w / options->markSpacing);
	int div = 1;
	int parts = 0;
	
	for(int i = 0; i < sizeof(markSpacings) / sizeof(int); i++)
	{
		div = markSpacings[i];
		parts = seconds / div;
		
		if(parts <= num)
		{
			break;
		}
	}
	
	for(int i = 1; i < parts + 1; i++)
	{
		int markSec = div * i;
		fillStringWithTime(cbuf, markSec);
		int x = ((float) markSec / (float) seconds) * w;
		int xf = x - drawNumberString(im,cbuf,0,0,color,false) / 2;
		
		int thisColor = color;
		if(markSec % 60 != 0)
			thisColor = oddColor;
		
		if(x < w - (options->markSpacing/2) && x < w - 50)
		{
			drawNumberString(im, cbuf, xf,y,thisColor,true);
			gdImageLine(im, x,y-4,x,y-2,thisColor);
		}
		
	}
	
	// this would draw equals spaced marks
	/*
	for(int i = 1; i < num; i++)
	{
		fillStringWithTime(cbuf, seconds * ((float) i / (float) num));
		drawNumberString(im, cbuf, (w * ((float) i / (float) num)) - (drawNumberString(im,cbuf,0,0,color,false) / 2),y, color, true);
	}
	 */
}

WFGO* wfg_defaultOptions()
{
	WFGO* options = malloc(sizeof(struct wfg_options));
	memset(options,0,sizeof(struct wfg_options));
	
	options->width = 800;
	options->height = 120;
	options->transparentBg = false;
		
	WFG_PACK_RGB(options->bgColor, 255, 255, 255);
	WFG_PACK_RGB(options->rmsColor, 80, 80, 80);
	WFG_PACK_RGB(options->peakColor, 20, 20, 20);

	options->mixChannels = false;
	options->channelSpacing = 3;
	
	options->drawTimeline = false;
	WFG_PACK_RGB(options->tlColor, 20, 20, 20);
	WFG_PACK_RGB(options->tlOddColor, 80, 80, 80);
	WFG_PACK_RGB(options->tlBgColor, 192, 192, 192);
	options->markSpacing = 80;
	
	options->drawMarkEveryMinute = false;

	return options;
}

char* wfg_lastErrorMessage()
{
	return lastErrorMessage;
}

/*
 I think i did the whole font drawing for fun.
*/

void drawNumber(gdImagePtr im, int number, int x, int y, int color)
{
	int i = number*35;
	
	for(int cy = y; cy < (y + 7);cy++)
	{
		for(int cx = x; cx < (x + 5);cx++)
		{
			if(wfg_char_table[i] == 1)
			{
				gdImageSetPixel(im, cx, cy, color);
			}
			i++;
		}
	}	
}

int drawNumberString(gdImagePtr im, char* buffer, int x, int y, int color, bool doDraw)
{
	char c;
	int offsetX = 0;
	
	int i = 0;
	
	while((c = buffer[i]) != 0)
	{
		int number = c - ZERO_CHAR;
		
		if(c == ':')
		{
			number = 10;
			offsetX -= 2;
		}
		
		if(c == '1')
		{
			offsetX -= 1;
		}
		
		if(number < 0 || number > 10)
			number = 0;
		
		if(doDraw)
			drawNumber(im, number, offsetX + x, y, color);
		
		switch(c)
		{
			case ':':
				offsetX += 4;
				break;
			default:
				offsetX += 6;
				break;
		}
		
		i++;
	}
	
	return offsetX;
}

void fillStringWithTime(char * string, int seconds)
{
	memset(string, 0, sizeof(char) * WFG_STRING_BUFFER_SIZE);
	
	int minutes = seconds/60;
	int rest = seconds%60;
	int i = sprintf(string,"%d",minutes);
	
	string[i] = ':';
	
	string[i+1] = (rest / 10) + ZERO_CHAR;
	string[i+2] = (rest % 10) + ZERO_CHAR;
}
