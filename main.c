/*
 main.c
 Created by Patrick Borgeat on 22.03.2010
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "waveformgen.h"

#define PRINT_VERSION printf("waveformgen %s - a waveform image generator\nWritten 2010 by Patrick Borgeat (http://www.cappel-nord.de)\n\n",WAVEFORMGEN_VERSION);


// private
void displayHelp();
bool parseDimension(char* string, WFGO* options);
bool parseColor(char* string, int* color);

int main (int argc, char *argv[])
{
	char* inFile = NULL;
	char* outFile = NULL;
	
	if(argc < 2)
	{
		displayHelp();
		return EXIT_FAILURE;
	}
	
	WFGO* options = wfg_defaultOptions();
	
	// 	http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_22.html#SEC388
	
	int c;
	int spacing;
	
	while((c = getopt(argc, argv, "o:i:d:htmslb:r:p:c:v")) != -1)
	{
		switch (c)
		{
			case 'h': // help
				displayHelp();
				return EXIT_SUCCESS;
				break;
			case 'v': // version
				PRINT_VERSION;
				return EXIT_SUCCESS;
				break;
			case 'o': // output
				outFile = optarg;
				break;
			case 'i': // input
				inFile = optarg;
				break;
			
			case 't': // transparent
				options->transparentBg = true;
				break;
			case 'm': // mix channels
				options->mixChannels = true;
				break;
			case 'l': // draw timeline
				options->drawTimeline = true;
				break;
			
			case 's': // channel spacing
				spacing = atoi(optarg);
				if(spacing > 0)
					options->channelSpacing = spacing;
				break;
			
			case 'd': // dimension
				if(!parseDimension(optarg, options))
				{
					fprintf(stderr, "Could not parse dimensions!\n");
					return EXIT_FAILURE;
				}
				break;
				
#define __PCOLOR(_CHAR, _FIELD, _NAME)\
case _CHAR: \
if(!parseColor(optarg, options->_FIELD))\
{\
	fprintf(stderr, "Could not parse %s color!\n",_NAME); \
	return EXIT_FAILURE; \
}\
break;\
			
			__PCOLOR('b', bgColor, "background");
			__PCOLOR('r', rmsColor, "rms");
			__PCOLOR('p', peakColor, "peak");
			__PCOLOR('c', tlColor, "timeline");
		}
	}
	
	if(argc > optind)
	{
		inFile = argv[optind];
	}
	
	if(argc > optind+1)
	{
		outFile = argv[optind+1];
	}
	
	if(inFile == NULL)
	{
		fprintf(stderr, "You have to specify an input file!\n");
		return EXIT_FAILURE;
	}
	
	if(outFile == NULL)
	{
		fprintf(stderr, "You have to specify an output file!\n");
		return EXIT_FAILURE;
	}
	
	// a too small width would make the audio file buffer quite large.
	if(options->width < 10)
	{
		fprintf(stderr, "Please specify a widht greater than 10!\n");
		return EXIT_FAILURE;
	}
	
	bool ret = wfg_generateImage(inFile, outFile, options);
	
	if(!ret)
	{
		fprintf(stderr, "ERROR: %s\n", wfg_lastErrorMessage());
		return EXIT_FAILURE;
	}
	
	printf("Saved waveform image to %s\n", outFile);	
	return EXIT_SUCCESS;
}

bool parseDimension(char* string, WFGO* options)
{
	int w = 0;
	int h = 0;
	
	sscanf(string, "%dx%d", &w, &h);
	
	if(w > 0 && h > 0)
	{
		options->width = w;
		options->height = h;
		return true;
	}
	
	return false;
}

bool parseColor(char* string, int* color)
{
	int temp[3];
	char buf[2];
	
	int i = 0;
	int ci = 0;
	
	char c;
	
	bool tooShort = false;
	
	for(i = 0; i < 3; i++)
	{
		for(int bi = 0; bi < 2; bi++)
		{
			c = string[ci];
			
			if(c == 0)
			{
				tooShort = true;
				break;
			}
			
			buf[bi] = c;
			ci++;
		}
		
		int ret = sscanf(buf, "%x", &temp[i]);
		
		if(ret == 0 || tooShort == true)
		{
			return false;
		}
	}
	
	color[0] = temp[0];
	color[1] = temp[1];
	color[2] = temp[2];
	
	return true;
}

void displayHelp()
{
	PRINT_VERSION;
	
	printf("usage: waveformgen [options] <infile> <outfile>\n\n\
   <infile>:  an uncompressed audio file (.wav or .aif)\n\
   <outfile>: location where to write the png file\n\
\n\
OPTIONS:\n\
   -o file    specify output file\n\
   -i file    specify input file\n\n\
   -d dim     specify dimension as [width]x[height]. Default: 800x120\n\
   -t         transparent background\n\
   -b RRGGBB  specify background color. Default: FFFFFF\n\
   -r RRGGBB  specify rms color. Default: 141414\n\
   -p RRGGBB  specify peak color. Default: 505050\n\n\
   -s spc     space between channels. Default: 5\n\
   -m         mix channels\n\n\
   -l         draw a timeline\n\
   -c RRGGBB  timeline color.  Default: 141414\n\n\
   -h         display help\n\
   -v         display version\n\n");
}