#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>

#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "hroot/io_parse.h"
#include "hroot/io_color.h"
#include "hroot/debug.h"
#include "hroot/stringlib.h"
#include "hroot/macros.h"

#define CMD_COMMANDS 8

int main()
{
	// INIT

	system("clear");

	time_t global = time(NULL);
  	struct tm *t = localtime(&global);

	char *ApplicationsSysPath=getValue(1);
	char *ApplicationsLocalPath=getValue(2);
	char *color=getValue(5);
	char *colorhigh=getValue(6);

	char *CommandList[7];

	char CommandBuffer[64]={0};
	char AppBuffer[256]={0};

	char *sysDirApps[1024]; // with extension, for file reference
	char *userDirApps[1024];

	char *sysAppsNames[1024]; // no extension, for readability
	char *userAppsNames[1024];

	int PIDS[256]={0};
	char *RunningApps[256];
	int pidsCount=0;

	bool isLeaving=false;

	for(int n=0;n<7;n++)
	{
		CommandList[n]=malloc(sizeof(char)*11);

		if(CommandList[n] is NULL)
		{
			return errorFatal("Couldn't allocate memory ! (CommandList)", -2);
		}
	}

	CommandList[0] = "help";
	CommandList[1] = "appsys";
	CommandList[2] = "appuser";
	CommandList[3] = "exit";
	CommandList[4] = "run";
	CommandList[5] = "stop";
	CommandList[6] = "time";

	if(CommandBuffer is NULL)
	{
		return errorFatal("Couldn't allocate memory for the main Command Buffer !", -2);
	}

	if(ApplicationsSysPath is NULL or ApplicationsLocalPath is NULL)
	{
		return errorFatal("Looks like cfg.txt has been deleted !\n Please create a new one in the current (executable) directory or copy-paste the default config ! (on github)\n", -1);
	}

	DIR *sysDir = opendir(ApplicationsSysPath);
	DIR *userDir = opendir(ApplicationsLocalPath);

	struct dirent *dir;

	debug(ApplicationsSysPath);
	debug(ApplicationsLocalPath);


	if(!sysDir or !userDir)
	{
		return errorFatal("The directory entered in cfg.txt doesn't exist !", -1);
	}

	int sysDirc=0;
	while ((dir = readdir(sysDir)) not NULL) 
 	{
 		bool isDs = IsDesktopShortcut(dir->d_name);
        if(isDs and isDs < 2)
        {
        	sysDirApps[sysDirc] = s_copy(dir->d_name);

        	if(sysDirApps[sysDirc] is NULL)
        	{
        		return errorFatal("Memory allocation error (type : sysDir)", -2);
        	}

        	sysAppsNames[sysDirc] = malloc(sizeof(char)*256);

        	if(sysAppsNames[sysDirc] is NULL)
        	{
        		return errorFatal("Memory allocation error (type : sysApp)", -2);
        	}

        	int i=0;
        	// 256 is the max lenght of a filename
        	while((sysDirApps[sysDirc][i] not '.') and (sysDirApps[sysDirc][i] not '\0') and i<255)
        	{
        		sysAppsNames[sysDirc][i] = sysDirApps[sysDirc][i];
        		i++;
        	}

        	sysAppsNames[sysDirc][i] = '\0';
			sysDirc++;
        }
   		else if(isDs == 2)
		{
			return errorFatal("Failed to allocate memory (from IsDesktopShortcut())", -2);
		}
    }

    if(closedir(sysDir) != 0) error("Failed to close the System Directory !");

    int userDirc=0;
    while ((dir = readdir(userDir)) not NULL) 
 	{
       	bool isDs = IsDesktopShortcut(dir->d_name);
       	if(isDs and isDs < 2)
		{
       		userDirApps[userDirc] = s_copy(dir->d_name);

       		if(userDirApps[userDirc] is NULL)
       		{
       			return errorFatal("Memory allocation error (type : userDir)", -2);
       		}

       		userAppsNames[userDirc] = malloc(sizeof(char)*256);

       		if(userAppsNames[userDirc] is NULL)
        	{
        		return errorFatal("Memory allocation error (type : userApp)", -2);
        	}

       		int i=0;
        	
        	while((userDirApps[userDirc][i] not '.') and (userDirApps[userDirc][i] not '\0') and i<255)
        	{
        		userAppsNames[userDirc][i] = userDirApps[userDirc][i];
        		i++;
        	}
        	
        	userAppsNames[userDirc][i] = '\0';
			userDirc++;
		}
		else if(isDs == 2)
		{
			return errorFatal("Failed to allocate memory (from IsDesktopShortcut())", -2);
		}
    }

    if(closedir(userDir) != 0) error("Failed to close the User Directory !");

	setColor(color[0],colorhigh[0]);

	char *TextColor = s_copy(GetColor());
	if(TextColor is NULL)
	{
		return errorFatal("Couldn't allocate memory (TextColor)",-2);
	}

	char *HighLightColor = s_copy(GetColorHighlight());
	if(HighLightColor is NULL)
	{
		return errorFatal("Couldn't allocate memory (HighLightColor)",-2);
	}

	free(color);
	free(colorhigh);

	//POST INIT
	printColor("Welcome ! type help for help !\n");
	printf("%s We are currently %d:%d:%d %s\n",TextColor,t->tm_hour,t->tm_min,t->tm_sec,COLOR_RESET);

	//MAIN
	while(true)
	{
 		if(isLeaving)
 		{
 			break;
 		}

 		printColor(">");
 		if(fgets(CommandBuffer,sizeof(CommandBuffer),stdin) == NULL)
 		{
 			 return errorFatal("fgets doesn't work ??",-1);
 			 break;
 		}

		
 		int commandSelector=0;
		while(CommandList[commandSelector] not NULL and commandSelector < CMD_COMMANDS)
		{
			int len = s_Lenght(CommandList[commandSelector]);
			if(s_StartWith(CommandBuffer,CommandList[commandSelector],len) and (CommandBuffer[len+1] == '\0' or (CommandBuffer[len] == ' ')))
			{
				commandSelector++;
				break;
			}
			commandSelector++;
		}

		if(commandSelector >= CMD_COMMANDS)
		{
			commandSelector=0;
		}
		
		switch(commandSelector)
		{
			case 0:
				if(CommandBuffer[0] not '\n' XOR (CommandBuffer[0] is ' ' or CommandBuffer[0] is'\t'))
				{
					printColor("Unknown command.\n");
				}
				break;
			case 1:
				printColor("Heres the list of all available commands :\n appuser : List all available applications for the current user.\n appsys : List system shared applications.\n run : to execute a program, by matching filename in available .desktop files.\n stop : to terminate or kill a program started with run.\n time : display the current local time.\n exit : exit this program, IMPORTANT TO USE instead of ctrl+C !!!\n");
				break;
			case 2:
				int lineCount=0;
				int pageCount=-1;

				bool isFirstPage=false;
				bool isHelp=false;

				int i=0;

				//args
				while(CommandBuffer[6+i] not '\0')
				{
					if(CommandBuffer[6+i] is 'h' and CommandBuffer[7+i] is ':')
					{
						isHelp=true;
						break;
					}

					if(CommandBuffer[6+i] is 'l' and CommandBuffer[7+i] is ':' and CommandBuffer[8+i]-'0' <= 9 and CommandBuffer[8+i]-'0' >= 0)
					{
						if(CommandBuffer[9+i] not '\0' and CommandBuffer[9+i]-'0' <= 9 and CommandBuffer[9+i]-'0' >= 0)
						{
							lineCount=CommandBuffer[9+i]-'0';
							lineCount+=(CommandBuffer[8+i]-'0')*10;
						}
						else
						{
							lineCount=CommandBuffer[8+i]-'0';
						}
					}

					if(CommandBuffer[6+i] is 'p' and CommandBuffer[7+i] is ':' and CommandBuffer[8+i]-'0' <= 9 and CommandBuffer[8+i]-'0' >= 0)
					{
						if(CommandBuffer[9+i] not '\0' and CommandBuffer[9+i]-'0' <= 9 and CommandBuffer[9+i]-'0' >= 0)
						{
							pageCount=CommandBuffer[9+i]-'0';
							pageCount+=(CommandBuffer[8+i]-'0')*10;
						}
						else
						{
							pageCount=CommandBuffer[8+i]-'0';
						}
					}

					i++;
				}

				if(isHelp)
				{
					printColor("Arguments :\n l:[Number from 0 to 99] --> Number of lines per pages\n p:[Number from 0 to 99] --> Which page do you want to see ?\n h: --> display this help\n");
					break;
				}
				
				//<0 in case some weird stuff happen, we never know with my code
				if(lineCount<=0)
				{
					
					lineCount=3000; //please don't have more than 3000 apps, thanks.
				}

				if(pageCount==-1)
				{
					isFirstPage=true;
				}

				if(pageCount<0)
				{
					pageCount=0;
				}

				if(pageCount*lineCount > 1024 and lineCount==3000)
				{
					error("Page index precised but no line per page amount precised !");
					break;
				}

				//second 'if' because if we would have an out of bound,
				//and check this condition, it segfault. 
				if(sysDirApps[pageCount*lineCount] is NULL)
				{
					error("Page doesn't exist !");
					break;
				}

				i=0;
				//printf("%sPrinting %s%d%s lines at page n°%s%d%s",GetColor(),COLOR_HIGHLIGHT,lineCount,GetColor(),COLOR_HIGHLIGHT,pageCount,COLOR_RESET);
				printf("%s[System apps (page n°%d/%d)]%s\n",HighLightColor,pageCount,sysDirc/lineCount == 0 ? sysDirc/lineCount : sysDirc/lineCount-1,COLOR_RESET);
				while(sysDirApps[i+pageCount*lineCount] not NULL and i<lineCount)
 				{
 					printColorEnd(sysAppsNames[i+pageCount*lineCount],'\n');

 					i++;
 				}
 				
 				
				//Printing the warnings at the end so if theres no scrolling avaiblable, we can still see it.
				if(lineCount==3000)
				{
					warning("number of line not precised, printed everything at once. (see appsys h !)");
				}

				if(pageCount==0 and isFirstPage)
				{
					warning("number of page not precised, printed the first one. (see appsys h !)");
				}

 				break;
				
			case 3:
				lineCount=0;
				pageCount=-1;

				isFirstPage=false;
				isHelp=false;

				i=0;

				//args
				while(CommandBuffer[6+i] not '\0')
				{
					if(CommandBuffer[6+i] is 'h' and CommandBuffer[7+i] is ':')
					{
						isHelp=true;
						break;
					}

					if(CommandBuffer[6+i] is 'l' and CommandBuffer[7+i] is ':' and CommandBuffer[8+i]-'0' <= 9 and CommandBuffer[8+i]-'0' >= 0)
					{
						if(CommandBuffer[9+i] not '\0' and CommandBuffer[9+i]-'0' <= 9 and CommandBuffer[9+i]-'0' >= 0)
						{
							lineCount=CommandBuffer[9+i]-'0';
							lineCount+=(CommandBuffer[8+i]-'0')*10;
						}
						else
						{
							lineCount=CommandBuffer[8+i]-'0';
						}
					}

					if(CommandBuffer[6+i] is 'p' and CommandBuffer[7+i] is ':' and CommandBuffer[8+i]-'0' <= 9 and CommandBuffer[8+i]-'0' >= 0)
					{
						if(CommandBuffer[9+i] not '\0' and CommandBuffer[9+i]-'0' <= 9 and CommandBuffer[9+i]-'0' >= 0)
						{
							pageCount=CommandBuffer[9+i]-'0';
							pageCount+=(CommandBuffer[8+i]-'0')*10;
						}
						else
						{
							pageCount=CommandBuffer[8+i]-'0';
						}
					}

					i++;
				}

				if(isHelp)
				{
					printColor("Arguments :\n l:[Number from 0 to 99] --> Number of lines per pages\n p:[Number from 0 to 99] --> Which page do you want to see ?\n h: --> display this help\n");
					break;
				}
				
				//<0 in case some weird stuff happen, we never know with my code
				if(lineCount<=0)
				{
					
					lineCount=3000; //please don't have more than 3000 apps, thanks.
				}

				if(pageCount==-1)
				{
					isFirstPage=true;
				}

				if(pageCount<0)
				{
					pageCount=0;
				}

				if(pageCount*lineCount > 1024 and lineCount==3000)
				{
					error("Page index precised but no line per page amount precised !");
					break;
				}

				//second 'if' because if we would have an out of bound,
				//and check this condition, it segfault. 
				if(userDirApps[pageCount*lineCount] is NULL)
				{
					error("Page doesn't exist !");
					break;
				}

				i=0;
				//printf("%sPrinting %s%d%s lines at page n°%s%d%s",GetColor(),COLOR_HIGHLIGHT,lineCount,GetColor(),COLOR_HIGHLIGHT,pageCount,COLOR_RESET);
				printf("%s[User apps (page n°%d/%d)]%s\n",HighLightColor,pageCount,userDirc/lineCount == 0 ? userDirc/lineCount : userDirc/lineCount-1,COLOR_RESET);
				while(userDirApps[i+pageCount*lineCount] not NULL and i<lineCount)
 				{
 					printColorEnd(userAppsNames[i+pageCount*lineCount],'\n');

 					i++;
 				}
 				
				//Printing the warnings at the end so if theres no scrolling avaiblable, we can still see it.
				if(lineCount==3000)
				{
					warning("number of line not precised, printed everything at once. (see appsys h !)");
				}

				if(pageCount==0 and isFirstPage)
				{
					warning("number of page not precised, printed the first one. (see appsys h !)");
				}

 				break;
			case 4:
				isLeaving=true;
				break;
			case 5:
				i=0; //terrible, but will do
				int matchCount=0,matchCountUsr=0;
				int matchPos=0;
				bool isAsync=false;
				bool isUser=false;
				bool finded=false;
				bool isListing=false;
				isHelp=false;

				//args
				while(CommandBuffer[4+i] not '\0')
				{
					if(CommandBuffer[4+i] is 'h' and CommandBuffer[5+i] is ':')
					{
						isHelp=true;
						break;
					}

					if(CommandBuffer[4+i] is 'l' and CommandBuffer[5+i] is ':')
					{
						isListing=true;
						break;
					}

					if(CommandBuffer[4+i] is 'a' and CommandBuffer[5+i] is ':' and CommandBuffer[6+i]-'0' <= 1 and CommandBuffer[6+i]-'0' >= 0)
					{
						isAsync=CommandBuffer[6+i]-'0';
					}

					if(CommandBuffer[4+i] is 'e' and CommandBuffer[5+i] is ':')
					{
						int y=0;
						while(CommandBuffer[6+i] not '\0' and CommandBuffer[6+i] not ' ' and CommandBuffer[6+i] not '\t')
						{
							AppBuffer[y]=CommandBuffer[6+i];
							y++;
							i++;
						}

						AppBuffer[6+y]='\0';
					}

					i++;
				}

				if(isHelp)
				{
					printColor("Arguments :\n e:[name] --> application name to execute.\n a:[0 or 1] --> 1 for asynchronous (you can still type in after booting up a software) or 0 for synchronous (useful for viewing logs).\n l: --> display all async. process running.\n h: --> display this help.\n");
					break;
				}

				if(isListing)
				{
					int c=0;
					printf("%s",TextColor);
					while(RunningApps[c] not NULL)
					{
						//It needs to run twice and I HAVE NO IDEA why.
						waitpid(PIDS[c], NULL, WNOHANG);
						int exist=waitpid(PIDS[c], NULL, WNOHANG);
						
						if(exist==0)
						{
							printf("%s at PID %d\n",RunningApps[c],PIDS[c]);
							c++;
						}
						else
						{
							pidsCount--;
							RunningApps[c]=":(";
							PIDS[c]=0;
							int ca=0;
							while(RunningApps[c+ca+1] not NULL and PIDS[c+ca+1] not 0)
							{
								RunningApps[c+ca]=RunningApps[c+ca+1];
								RunningApps[c+ca+1]=":(";

								PIDS[c+ca]=PIDS[c+ca+1];
								PIDS[c+ca+1]=0;

								ca++;
							}
							RunningApps[c+ca]=NULL;
							PIDS[c+ca]=0;
						}
					}

					if(c==0)
					{
						printColor("No apps are currently running asynchronously.");
					}
					printf("%s\n",COLOR_RESET);
					break;
				}

				if(AppBuffer[0] is '\0' or AppBuffer[0] is '\n' or AppBuffer[0] is ' ' or AppBuffer[0] is '\t')
				{
					error("please enter an application name to execute !");
					break;
				}

				while((userDirApps[i] not NULL) and !finded)
 				{
 					//If it exactly match, in case of a filename starting with the
 					// same words / letter sequence
 					if(s_isEqual(AppBuffer,userAppsNames[i]))
 					{
 						matchCountUsr=1;
 						matchPos=i;
 						finded=true;
 						break;
 					}


 					if(s_StartWith(AppBuffer,userAppsNames[i],s_Lenght(AppBuffer)) and !finded)
 					{
 						matchCountUsr++;
 						printColorEnd(userAppsNames[i],'\n');
 						matchPos=i;
 					}

 					i++;
 				}

 				i=0;
 				while((sysDirApps[i] not NULL) and !finded)
 				{
 					if(s_isEqual(AppBuffer,sysAppsNames[i]))
 					{
 						matchCount=1;
 						matchCountUsr=0; //A bit of spaghetti coded stuff
 						matchPos=i;
 						finded=true;
 						break;
 					}

 					if(s_StartWith(AppBuffer,sysAppsNames[i],s_Lenght(AppBuffer)) and !finded)
 					{
 						matchCount++;
 						printColorEnd(sysAppsNames[i],'\n');
 						matchPos=i;
 					}

 					i++;
 				}

 				if(matchCount+matchCountUsr>1)
 				{
 					printf("%sFounded %s%d%s matches in total !%s\n",TextColor,HighLightColor,matchCount+matchCountUsr,TextColor,COLOR_RESET);
 				}
 				else if(matchCount+matchCountUsr == 0)
 				{
 					printf("%sDid not find any applications named '%s' !%s\n",TextColor,AppBuffer,COLOR_RESET);
 				}
 				else
 				{
 					char fexec[512];

 					if(matchCountUsr==1)
 					{
						s_Merge(fexec,ApplicationsLocalPath,userDirApps[matchPos]);
						RunningApps[pidsCount]=s_copy(userAppsNames[matchPos]);
 					}
 					else
 					{
						s_Merge(fexec,ApplicationsSysPath,sysDirApps[matchPos]);
						RunningApps[pidsCount]=s_copy(sysAppsNames[matchPos]);
 					}

 					if(RunningApps[pidsCount] is NULL)
 					{
 						return errorFatal("Couldn't allocate ! (RunningApps)",-2);
 						break;
 					}

 					FILE *fp = fopen(fexec, "r");
 					if(!fp) 
 					{
 						error("Couldn't find the executable !");
 						break;
 					}
 					char *exec = malloc(sizeof(char)*256);
 					if(exec is NULL)
 					{
 						return errorFatal("Couldn't allocate memory !", -2);
 					}
 					exec = getExec(fp,4);

 					if(isAsync)
 					{
 						PIDS[pidsCount] = fork();
 						
 						if(PIDS[pidsCount] == 0)
 						{
 							setpgid(0, 0);
 							execlp("/bin/sh","/bin/sh", "-c", exec, (char *)NULL);
 						}
 						else if(PIDS[pidsCount] < 0)
 						{
 							error("Fork failed to create the child process.");
 						}

 						free(exec);

 						printf("%s Process started on PID %d\n to stop it, type %s'stop [PID]'%s\n",TextColor,PIDS[pidsCount],HighLightColor,COLOR_RESET);
 						pidsCount++;
 					}
 					else
 					{
 						system(exec);
 						free(exec);
 					}
 				}

 				break;
 			case 6:
 				isHelp=false;
 				char sPIDS[32]={0};
 				int PID=0;
 				bool doKill=false;
 				i=0;

 				while(CommandBuffer[5+i] not '\0')
				{
					if(CommandBuffer[5+i] is 'h' and CommandBuffer[6+i] is ':')
					{
						isHelp=true;
						break;
					}

					if(CommandBuffer[5+i] is 'k' and CommandBuffer[6+i] is ':')
					{
						doKill=true;
					}

					if(CommandBuffer[5+i] is 'p' and CommandBuffer[6+i] is ':')
					{
						int y=0;
						while(CommandBuffer[7+i] not '\0' and CommandBuffer[7+i] not ' ' and CommandBuffer[7+i] not '\t')
						{
							sPIDS[y]=CommandBuffer[7+i];
							y++;
							i++;
						}

						sPIDS[7+y]='\0';
					}
					i++;
				}

				if(isHelp)
				{
					printColor("Arguments :\n p:[PID number] --> (not optional) to precise the process PID\n k: --> to kill the process (sends SIGKILL instead of SIGTERM)\n h: --> display this help\n");
					break;
				}

				PID = s_toInt(sPIDS);
				
				if(PID==0)
				{
					error("Please enter a valid PID (usage p:[PID])");
					break;
				}
				
				if(doKill)
				{
					if(kill(-PID,SIGKILL) == -1)
					{
						error("Couldn't kill the process (kill sig. attempt) !");
						break;
					}
				}
				else
				{
					if(kill(-PID,SIGTERM) == -1)
					{
						error("Couldn't close the process (kill sig. attempt) ! (maybe try adding 'k' as an arg !).");
						break;
					}
				}

				if(waitpid(PID, NULL, 0) == -1)
				{
					error("Couldn't close the process (zombie process) ! (maybe try 'kill' instead)");
					break;
				}
 				break;
 			case 7:
				global = time(NULL);
				t = localtime(&global);
				printf("%s %d:%d:%d %s\n",TextColor,t->tm_hour,t->tm_min,t->tm_sec,COLOR_RESET);
				break;
		}
	}

	int cClose=0;
	while(sysDirApps[cClose] not NULL)
	{
		free(sysDirApps[cClose]);
		free(sysAppsNames[cClose]);
		cClose++;
	}

	cClose=0;
	while(userDirApps[cClose] not NULL)
	{
		free(userDirApps[cClose]);
		free(userAppsNames[cClose]);
		cClose++;
	}

	cClose=0;
	while(RunningApps[cClose] not NULL)
	{
		free(RunningApps[cClose]);
		cClose++;
	}

	free(ApplicationsSysPath);
	free(ApplicationsLocalPath);
	return 0;
}
