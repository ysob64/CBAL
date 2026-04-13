// I could have made this file parser more "usable" but I got lazy and have <10 index, so... Bored to implement i_toChar()

#include <stdio.h>
#include <stdlib.h>

#include "../hroot/debug.h"
#include "../hroot/macros.h"
#include "../hroot/stringlib.h"

char *cuser;

void setCurrentUser(const char *user)
{
    cuser = s_copy(user);
}

void ClearUser()
{
    free(cuser);
}

char *getValue(int index)
{
	FILE* cfg;
    char dbuffer[1024]={0};

    s_Merge(dbuffer,"/home/",cuser);
    s_Merge(dbuffer,dbuffer,"/.config/CBAL/cfg.txt");

    cfg = fopen(dbuffer, "r");
    if(!cfg) return NULL;

    int cc,i,y=0;
    char oldcc;
    bool doRecord=false;

    fseek(cfg, 0, SEEK_END);
    int size = ftell(cfg);
    fseek(cfg, 0, SEEK_SET);

    char *buffer = malloc(size+1);

    if(!buffer)
    {
        printf("Couldn't allocate %d bytes !",size);
        error("Out of memory ???");
        return "NULL";
    }
    char indexc = index+'0';

    for (i = 0; ((i < size) and ((cc = fgetc(cfg)) not EOF)); i++)
    {

        if(oldcc is indexc and cc is ':')
        {
            debug("Index found !\n");
            doRecord=true;
            continue;
        }

        if(doRecord)
        {
            if(cc is '\n')
            {
                break;
            }
            buffer[y]=cc;
            y++;
        }

        oldcc=cc;
    }
 
    buffer[y] = '\0';

    if (fclose(cfg)) error("fclose failed...");

    return buffer;
}

char *getExec(FILE* dEntry,int addAllocSize)
{
    if(!dEntry) return NULL;

    int cc,i,y=0;
    char oldcc=0;
    bool doRecord=false;

    fseek(dEntry, 0, SEEK_END);
    int size = ftell(dEntry);
    fseek(dEntry, 0, SEEK_SET);

    char *buffer = malloc(size+1+addAllocSize);

    if(!buffer)
    {
        printf("Couldn't allocate %d bytes !",size);
        error("Out of memory ???");
        return NULL;
    }

    for (i = 0; ((i < size) and ((cc = fgetc(dEntry)) not EOF)); i++)
    {

        //Yes, it detects the Exec= of a desktop file with only the uppercase E
        if(oldcc is '\n' and cc is 'E')
        {
            debug("Exec found !\n");
            doRecord=true;
            continue;
        }

        if(doRecord)
        {
            if(cc is '\n')
            {
                break;
            }
            buffer[y]=cc;
            y++;
        }

        oldcc=cc;
    }

    int len = s_Lenght(buffer);

    for(int i=0;i<len;i++)
    {
        buffer[i]=buffer[i+4];
    }

    printf("%s\n",buffer);

    if (fclose(dEntry)) error("fclose failed...");

    return buffer;
}
