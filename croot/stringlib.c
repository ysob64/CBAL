#include <stdio.h>
#include <stdlib.h>
#include "../hroot/debug.h"
#include "../hroot/macros.h"

bool s_isEqual(const char *s1,const char *s2)
{
    int i=0;
    
    while((s1[i] is s2[i]) and (s1[i] not '\0') and (s2[i] not '\0'))
    {
        i++;
    }

    return (((s1[i] is '\n') or (s1[i] is '\0')) and ((s2[i] is '\n') or (s2[i] is '\0'))) ? true : false;
}

//Match = at how many character to we do stop
bool s_StartWith(const char *s1, const char *s2, const int match) //One thing, I don't why
{
    int i=0;
    
    while((i<match) and (s1[i] not '\0') and (s2[i] not '\0') and (s1[i] is s2[i]))
    {
        i++;
    }

    return i == match ? true : false;
}

int s_Lenght(const char *s)
{
    int i=0;
    
    while((s[i] not '\n') and (s[i] not '\0'))
    {
        i++;
    }

    return i;
}

//Add the second string to the end of the first one
void s_Merge(char *buffer,const char *s1,const char *s2)
{
    int i=0;
    int y=0;

    while((s1[i] not '\n') and (s1[i] not '\0'))
    {
        if(buffer[i] not s1[i])
        {
            buffer[i] = s1[i];
        }
        i++;
    }

    while((s2[y] not '\n') and (s2[y] not '\0'))
    {
        buffer[i] = s2[y];
        i++;
        y++;
    }

    buffer[i] = '\0';
    debug(buffer);
}

int s_toInt(const char *s1)
{
    int i=0;
    int result=0;

    while((s1[i] not '\n') and (s1[i] not '\0'))
    {
        if(((s1[i]-'0') > 9) or ((s1[i]-'0') < 0))
        {
            return 0;
        }
        result = (s1[i]-'0')+result*10;
        i++;
    }

    return result;
}

//NOTE : do not forget to free() after...
char *s_copy(char *s)
{
    int len = s_Lenght(s);
    char *cp = malloc(sizeof(char)*len+1);
    
    if(cp is NULL)
    {
        return NULL;
    }

    int i=0;
    while(i<len)
    {
        cp[i]=s[i];
        i++;
    }

    cp[i]='\0';

    return cp;
}

bool IsDesktopShortcut(const char *s1)
{
    int i=0;

    while(s1[i] not '.' and s1[i] not '\0')
    {
        i++;
    }

    char *copy = malloc(sizeof(char)*8);

    if(copy is NULL)
    {
        return 2; //because bool is an unsigned int
    }

    //d e s k t o p
    copy[0] = s1[i+1]; // mem garbage copy is not important here, as it would not be equal to "desktop" anyway
    copy[1] = s1[i+2];
    copy[2] = s1[i+3];
    copy[3] = s1[i+4];
    copy[4] = s1[i+5];
    copy[5] = s1[i+6];
    copy[6] = s1[i+7];
    copy[7] = '\0';
    // I could use a while loop here but I think it would have been a bit bloated for only 8 characters...

    if(s_isEqual(copy, "desktop"))
    {
        free(copy);
        return true;
    }

    free(copy);
    return false;

}
