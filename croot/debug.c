//FANCY STUFF FOR INDENTIFYING MESSAGES

#include <stdio.h>
#include <stdbool.h>

//local to debug.c
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0
#endif

void debug(const char *msg)
{
	if(DEBUG_ENABLED) printf("\e[32mDEBUG : \e[0m %s \n",msg);
}

void warning(const char *msg)
{
	printf("\e[33mWARNING : \e[0m %s\n",msg);
}

int errorFatal(const char *msg, int exitCode)
{
	printf("\e[31mFATAL_ERROR : \e[0m %s \n",msg);
	return exitCode;
}

void error(const char *msg)
{
	printf("\e[31mERROR : \e[0m %s \n",msg);
}