/*
 main.c
 Created by Patrick Borgeat on 22.03.2010
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "waveformgen.h"

#define PRINT_VERSION printf("waveformgen v%s - a waveform image generator\nWritten 2010 by Patrick Borgeat (http://www.cappel-nord.de)\n\n",WAVEFORMGEN_VERSION);


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
	
	while((c = getopt(argc, argv, "o:i:d:htmjs:a:leb:r:p:c:x:g:v")) != -1)
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
			case 'e': // draw mark for every minute
				options->drawMarkEveryMinute = true;
				break;

			case 'j': // rms-only draw mode (scale rms to height)
				options->scaleRms = true;
				break;
			
			case 's': // channel spacing
				spacing = atoi(optarg);
				if(spacing > 0)
					options->channelSpacing = spacing;
				break;
				
			case 'a': // mark spacing
				spacing = atoi(optarg);
				if(spacing > 0)
					options->markSpacing = spacing;
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
			__PCOLOR('x', tlOddColor, "odd mark color");
			__PCOLOR('g', tlBgColor, "timeline background");
				
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
		fprintf(stderr, "Please specify a width greater than 10!\n");
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
	WFGO* def = wfg_defaultOptions();
	
	PRINT_VERSION;
	
	printf("usage: waveformgen [options] <infile> <outfile>\n\n\
   <infile>:  an uncompressed audio file (.wav or .aif)\n\
   <outfile>: location where to write the png file\n\
\n\
OPTIONS:\n\
   -o file    specify output file\n\
   -i file    specify input file\n\n\
   -d dim     specify dimension as [width]x[height]. Default: %dx%d\n\
   -t         transparent background\n\
   -j rms-only draw mode. Do not draw peak. Scale rms to height.\n\
   -b RRGGBB  specify background color. Default: %X%X%X\n\
   -r RRGGBB  specify rms color. Default: %X%X%X\n\
   -p RRGGBB  specify peak color. Default: %X%X%X\n\n\
   -s spc     space between channels. Default: %d\n\
   -m         mix channels\n\n\
   -l         draw a timeline\n\
   -a spc     space between marks. Default: %d\n\
   -c RRGGBB  timeline color. Default: %X%X%X\n\
   -x RRGGBB  odd mark color. Default: %X%X%X\n\
   -g RRGGBB  timeline color background color. Default: %X%X%X\n\
   -e         draw a mark for every minute.\n\n\
   -h         display help\n\
   -v         display version\n\n",
		   def->width, def->height, 
		   WFG_UNPACK_RGB(def->bgColor), WFG_UNPACK_RGB(def->rmsColor), WFG_UNPACK_RGB(def->peakColor),
		   def->channelSpacing, def->markSpacing,
		   WFG_UNPACK_RGB(def->tlColor), WFG_UNPACK_RGB(def->tlOddColor), WFG_UNPACK_RGB(def->tlBgColor)
	);
	
	free(def);
}