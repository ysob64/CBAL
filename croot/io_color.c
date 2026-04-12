#include <stdlib.h>
#include <stdio.h>

#include "../hroot/debug.h"

char colorbuffer[6];
char colorhbuffer[6];

void setColor(char color,char colorh)
{
	switch(color)
	{
		default :
			printf("Invalid color character !");
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[37m");
			return;
			break;
		case 'n': //for none / black, because it would have been the same as blue else
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[30m");
			break;
		case 'r':
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[31m");
			break;
		case 'g':
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[32m");
			break;
		case 'y':
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[33m");
			break;
		case 'b':
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[34m");
			break;
		case 'm': //magenta
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[35m");
			break;
		case 'c': //cyan
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[36m");
			break;
		case 'w': //white
			snprintf(colorbuffer, sizeof(colorbuffer), "\e[37m");
			break;

	}

	char todebug[40] = "\0";
	snprintf(todebug, sizeof(todebug), "Text color has been set to %c\n",colorh);
	debug(todebug);

	switch(colorh)
	{
		default :
			printf("Invalid color character !");
			snprintf(colorbuffer, sizeof(colorhbuffer), "\e[32m");
			return;
			break;
		case 'n': //for none / black, because it would have been the same as blue else
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[30m");
			break;
		case 'r':
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[31m");
			break;
		case 'g':
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[32m");
			break;
		case 'y':
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[33m");
			break;
		case 'b':
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[34m");
			break;
		case 'm': //magenta
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[35m");
			break;
		case 'c': //cyan
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[36m");
			break;
		case 'w': //white
			snprintf(colorhbuffer, sizeof(colorhbuffer), "\e[37m");
			break;

	}

	snprintf(todebug, sizeof(todebug), "Highlight color has been set to %c\n",colorh);
	debug(todebug);
}

char *GetColor()
{
	return colorbuffer;
}

char *GetColorHighlight()
{
	return colorhbuffer;
}

void printColor(const char *msg) //simple func to avoid rewriting color code each time
{
	if(colorbuffer[0] == 0)
	{
		error("SetColor hasn't be called, default color is not defined !");
		snprintf(colorbuffer, sizeof(colorbuffer), '\0');
	}

	printf("%s %s \e[0m",colorbuffer,msg);
}


void printColorEnd(const char *msg, const char end)
{
	if(colorbuffer[0] == 0)
	{
		error("SetColor hasn't be called, default color is not defined !");
		snprintf(colorbuffer, sizeof(colorbuffer), '\0');
	}

	printf("%s %s \e[0m %c",colorbuffer,msg,end);
}