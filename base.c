#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>

#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#include "hroot/io_parse.h"
#include "hroot/io_color.h"
#include "hroot/debug.h"
#include "hroot/stringlib.h"
#include "hroot/macros.h"

#define CMD_COMMANDS 7
#define VERSION 0.85f


/*
	    CCCCCCCCCCCCCBBBBBBBBBBBBBBBBB               AAA               LLLLLLLLLLL    
     CCC::::::::::::CB::::::::::::::::B             A:::A              L:::::::::L             
   CC:::::::::::::::CB::::::BBBBBB:::::B           A:::::A             L:::::::::L             
  C:::::CCCCCCCC::::CBB:::::B     B:::::B         A:::::::A            LL:::::::LL             
 C:::::C       CCCCCC  B::::B     B:::::B        A:::::::::A             L:::::L               
C:::::C                B::::B     B:::::B       A:::::A:::::A            L:::::L               
C:::::C                B::::BBBBBB:::::B       A:::::A A:::::A           L:::::L               
C:::::C                B:::::::::::::BB       A:::::A   A:::::A          L:::::L               
C:::::C                B::::BBBBBB:::::B     A:::::A     A:::::A         L:::::L               
C:::::C                B::::B     B:::::B   A:::::AAAAAAAAA:::::A        L:::::L               
C:::::C                B::::B     B:::::B  A:::::::::::::::::::::A       L:::::L               
 C:::::C       CCCCCC  B::::B     B:::::B A:::::AAAAAAAAAAAAA:::::A      L:::::L         LLLLLL
  C:::::CCCCCCCC::::CBB:::::BBBBBB::::::BA:::::A             A:::::A   LL:::::::LLLLLLLLL:::::L
   CC:::::::::::::::CB:::::::::::::::::BA:::::A               A:::::A  L::::::::::::::::::::::L
     CCC::::::::::::CB::::::::::::::::BA:::::A                 A:::::A L::::::::::::::::::::::L
        CCCCCCCCCCCCCBBBBBBBBBBBBBBBBBAAAAAAA                   AAAAAAALLLLLLLLLLLLLLLLLLLLLLLL

By ysob64

under MIT license
*/


int main()
{
	// INIT
	system("clear");
	resetexec:

	const char *user = getenv("USER");

	if(user is NULL) 
    {
        return errorFatal("env variable USER not set, aborting.",-1);
    }

    setCurrentUser(user);

	time_t global = time(NULL);
  	struct tm *t = localtime(&global);

	char *ApplicationsSysPath=getValue(1);
	char *ApplicationsLocalPath=getValue(2);
	char *color=getValue(3);
	char *colorhigh=getValue(4);
	char *LinesAmount=getValue(5);

	char *CommandList[CMD_COMMANDS];

	char CommandBuffer[64]={0};
	char AppBuffer[256]={0};

	char *sysDirApps[1024]; // with extension, for file reference
	char *userDirApps[1024];
	char *binApps[4096];

	char *sysAppsNames[1024]; // no extension, for readability
	char *userAppsNames[1024];

	int PIDS[512]={0};
	char *RunningApps[512];
	int pidsCount=0;

	bool isLeaving=false;

	if(CommandBuffer is NULL)
	{
		return errorFatal("Couldn't allocate memory for the main Command Buffer !", -2);
	}

	if(ApplicationsSysPath is NULL or ApplicationsLocalPath is NULL)
	{
		warning("Looks like cfg.txt doesn't exist !\n");
		
		char buffer[1024]={0};

		s_Merge(buffer,"/home/",user);
		const char *userhome = s_copy(buffer);
		if(userhome is NULL)
		{
			return errorFatal("Please run CBAL as a non-root user", -1);
		}

		s_Merge(buffer,userhome,"/.config/");
		const char *s_cfgdir = s_copy(buffer);

		DIR *cfgdir = opendir(s_cfgdir);
		if(!cfgdir)
		{
			if (mkdir(buffer, 0777) == -1 && errno != EEXIST) 
			{
    			free((char*)userhome);
				free((char*)s_cfgdir);

    			perror("mkdir failed");
    			return errorFatal("Cannot create config directory", -1);
			}
		}

		s_Merge(buffer,s_cfgdir,"CBAL/");

		cfgdir = opendir(buffer);
		if(!cfgdir)
		{
			if (mkdir(buffer, 0777) == -1 && errno != EEXIST) 
			{
    			free((char*)userhome);
				free((char*)s_cfgdir);
				closedir(cfgdir);

    			perror("mkdir failed");
    			return errorFatal("Cannot create config directory", -1);
			}
		}
		
		s_Merge(buffer,buffer,"cfg.txt");

		printf("Creating config file at %s...\n",buffer);

		FILE *fcfg = fopen(buffer,"w");
		if(fcfg is NULL)
		{
			free((char*)userhome);
			free((char*)s_cfgdir);
			closedir(cfgdir);
			return errorFatal("Failed writing to /home/[user]/.config/CBAL/cfg.txt",-1);
		}

		fprintf(fcfg, "1:/usr/share/applications/\n2:/home/%s/.local/share/applications/\n3:w\n4:g\n5:20\n\nColor reference : n for black (/none), r for red, g green, b blue, y yellow, m magenta, c cyan, w white.\nNote that this is in 8 bit mode for plain compatibility with all terminals.\n1 = path to system application\n2 = path to user applications\n3 = color\n4 = highlight color\n5 = How much lines do you want to see in appsys,appbin and appuser ? (by default)", user);

		free((char*)userhome);
		free((char*)s_cfgdir);
		closedir(cfgdir);
		fclose(fcfg);


		printf("Done.\n Restarting...\n");
		goto resetexec;
	}

	int lineAmountCfg = s_toInt(LinesAmount);
	free(LinesAmount);

	for(int n=0;n<CMD_COMMANDS-1;n++)
	{
		CommandList[n]=malloc(sizeof(char)*11);

		if(CommandList[n] is NULL)
		{
			return errorFatal("Couldn't allocate memory ! (CommandList)", -2);
		}
	}

	CommandList[0] = "help";
	CommandList[1] = "app";
	CommandList[2] = "exit";
	CommandList[3] = "run";
	CommandList[4] = "stop";
	CommandList[5] = "time";
	CommandList[6] = "END_CMD";

	DIR *sysDir = opendir(ApplicationsSysPath);
	DIR *userDir = opendir(ApplicationsLocalPath);
	DIR *binDir = opendir("/usr/bin");

	struct dirent *dir;

	debug(ApplicationsSysPath);
	debug(ApplicationsLocalPath);

	if(!binDir)
	{
		return errorFatal("Couldn't open /usr/bin", -1);
	}
	
	if(!sysDir or !userDir)
	{
		return errorFatal("The directory entered in cfg.txt doesn't exist !", -1);
	}

	int binC=0;
	while((dir = readdir(binDir)) not NULL)
	{
		char buffer[512]={0};
		s_Merge(buffer,"/usr/bin/",dir->d_name);
		
		if(opendir(buffer) not NULL)
		{
			continue; //we skip folder that are in /usr/bin
		}

		binApps[binC] = s_copy(dir->d_name);

        if(binApps[binC] is NULL)
        {
        	return errorFatal("Memory allocation error (type : binDir)", -2);
        }

		binC++;
	}

	if(closedir(binDir) != 0) error("Failed to close /usr/bin Directory !");

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
	printf("%sWelcome to CBAL v%2.2f ! type help for help !\n We are currently %d:%d:%d %s\n",TextColor,VERSION,t->tm_hour,t->tm_min,t->tm_sec,COLOR_RESET);
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
				printColor("Heres the list of all available commands :\n app : List all available apps/programs.\n run : to execute a program, by matching filename in available .desktop files or in /usr/bin with the b: argument.\n stop : to terminate or kill a program started with run.\n time : display the current local time.\n exit : exit this program, IMPORTANT TO USE instead of ctrl+C !!!\n");
				break;
			case 2:
				int lineCount=0;
				int pageCount=-1;

				bool isFirstPage=false;
				bool isHelp=false;

				int i=0;
				int LOCAL_OFFSET=4;

				bool isSearching=false;
				int nonMatch=0;

				char env=0;

				//args
				while(CommandBuffer[LOCAL_OFFSET+i] not '\0')
				{
					if(CommandBuffer[LOCAL_OFFSET+i] is 'h' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						isHelp=true;
						break;
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'e' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						if(CommandBuffer[LOCAL_OFFSET+2+i] is 's')
						{
							env='s';
						}
						else if(CommandBuffer[LOCAL_OFFSET+2+i] is 'u')
						{
							env='u';
						}
						else if(CommandBuffer[LOCAL_OFFSET+2+i] is 'b')
						{
							env='b';
						}
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'r' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						isSearching=true;

						int y=0;
						while(CommandBuffer[LOCAL_OFFSET+2+i] not '\0' and CommandBuffer[LOCAL_OFFSET+2+i] not ' ' and CommandBuffer[LOCAL_OFFSET+2+i] not '\t')
						{
							AppBuffer[y]=CommandBuffer[LOCAL_OFFSET+2+i];
							y++;
							i++;
						}

						AppBuffer[LOCAL_OFFSET+2+y]='\0';
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'l' and CommandBuffer[LOCAL_OFFSET+1+i] is ':' and CommandBuffer[LOCAL_OFFSET+2+i]-'0' <= 9 and CommandBuffer[LOCAL_OFFSET+2+i]-'0' >= 0)
					{
						if(CommandBuffer[LOCAL_OFFSET+3+i] not '\0' and CommandBuffer[LOCAL_OFFSET+3+i]-'0' <= 9 and CommandBuffer[LOCAL_OFFSET+3+i]-'0' >= 0)
						{
							lineCount=CommandBuffer[LOCAL_OFFSET+3+i]-'0';
							lineCount+=(CommandBuffer[LOCAL_OFFSET+2+i]-'0')*10;
						}
						else
						{
							lineCount=CommandBuffer[LOCAL_OFFSET+2+i]-'0';
						}
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'p' and CommandBuffer[LOCAL_OFFSET+1+i] is ':' and CommandBuffer[LOCAL_OFFSET+2+i]-'0' <= 9 and CommandBuffer[LOCAL_OFFSET+2+i]-'0' >= 0)
					{
						if(CommandBuffer[LOCAL_OFFSET+3+i] not '\0' and CommandBuffer[LOCAL_OFFSET+3+i]-'0' <= 9 and CommandBuffer[LOCAL_OFFSET+3+i]-'0' >= 0)
						{
							pageCount=CommandBuffer[LOCAL_OFFSET+3+i]-'0';
							pageCount+=(CommandBuffer[LOCAL_OFFSET+2+i]-'0')*10;
						}
						else
						{
							pageCount=CommandBuffer[LOCAL_OFFSET+2+i]-'0';
						}
					}

					i++;
				}

				if(isHelp)
				{
					printColor("Arguments :\ne:[letter] --> to precise the environment, which can be 's' for system apps, 'u' for the current user apps, and 'b' for programs in /usr/bin\n l:[Number from 0 to 99] --> Number of lines per pages\n p:[Number from 0 to 99] --> Which page do you want to see ?\n r:[name to search] --> search a program (ignore l: and p: args.)\n h: --> display this help\n");
					break;
				}
				
				//<0 in case some weird stuff happen, we never know with my code
				if(lineCount<=0)
				{
					
					lineCount=lineAmountCfg;
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

				if(env is 's')
				{
					//second 'if' because if we would have an out of bound,
					//and check this condition, it segfault. 
					if(sysDirApps[pageCount*lineCount] is NULL)
					{
						error("Page doesn't exist !");
						break;
					}

					i=0;
					//printf("%sPrinting %s%d%s lines at page n°%s%d%s",GetColor(),COLOR_HIGHLIGHT,lineCount,GetColor(),COLOR_HIGHLIGHT,pageCount,COLOR_RESET);
					printf("%s[System apps (page n°%d/%d)]%s\n",HighLightColor,pageCount,sysDirc%lineCount > 0 ? sysDirc%lineCount-(sysDirc%lineCount-sysDirc/lineCount) : sysDirc/lineCount-1,COLOR_RESET);
				

					if(isSearching)
					{
						printColor("r: precised, hiding non-matching app\n");
						while(sysDirApps[i] not NULL)
 						{
 							if(s_StartWith(AppBuffer,sysAppsNames[i],s_Lenght(AppBuffer)))
 							{
 								printColorEnd(sysAppsNames[i],'\n');
 							}
 							else
 							{
 								nonMatch++;
 							}

 						i++;
 					}

 					printf("%sHid %s%d%s non-matching apps%s\n",TextColor,HighLightColor,nonMatch,TextColor,COLOR_RESET);
 					}
 					else
 					{
 						while(sysDirApps[i+pageCount*lineCount] not NULL and i<lineCount)
 						{
 							printColorEnd(sysAppsNames[i+pageCount*lineCount],'\n');

 							i++;
 						}
 					}
				}
				else if(env is 'u')
				{
					//second 'if' because if we would have an out of bound,
					//and check this condition, it segfault. 
					if(userDirApps[pageCount*lineCount] is NULL)
					{
						error("Page doesn't exist !");
						break;
					}

					i=0;
					
					printf("%s[User apps (page n°%d/%d)]%s\n",HighLightColor,pageCount,userDirc%lineCount > 0 ? userDirc%lineCount-(userDirc%lineCount-userDirc/lineCount) : userDirc/lineCount-1,COLOR_RESET);
					if(isSearching)
					{
						printColor("r: precised, hiding non-matching app\n");
						while(userDirApps[i] not NULL)
 						{
 							if(s_StartWith(AppBuffer,userAppsNames[i],s_Lenght(AppBuffer)))
 							{
 								printColorEnd(userAppsNames[i],'\n');
 							}
 							else
 							{
 								nonMatch++;
 							}

 							i++;
 					}
 						printf("%sHid %s%d%s non-matching apps%s\n",TextColor,HighLightColor,nonMatch,TextColor,COLOR_RESET);
 					}
 					else
 					{
 						while(userDirApps[i+pageCount*lineCount] not NULL and i<lineCount)
 						{
 							printColorEnd(userAppsNames[i+pageCount*lineCount],'\n');

 						i++;
 						}
 					}
				}
				else if(env is 'b')
				{
					//second 'if' because if we would have an out of bound,
					//and check this condition, it segfault. 
					if(binApps[pageCount*lineCount] is NULL)
					{
						error("Page doesn't exist !");
						break;
					}

					i=0;
				
					printf("%s[/usr/bin apps (page n°%d/%d)]%s\n",HighLightColor,pageCount,binC%lineCount > 0 ? binC%lineCount-(binC%lineCount-binC/lineCount) : binC/lineCount-1,COLOR_RESET);
					if(isSearching)
					{
						printColor("r: precised, hiding non-matching app\n");
						while(binApps[i] not NULL)
 						{
 							if(s_StartWith(AppBuffer,binApps[i],s_Lenght(AppBuffer)))
 							{
 								printColorEnd(binApps[i],'\n');
 							}
 							else
 							{
 								nonMatch++;
 							}
 							i++;
 						}

 						printf("%sHid %s%d%s non-matching apps%s\n",TextColor,HighLightColor,nonMatch,TextColor,COLOR_RESET);
					}
					else
					{
						while(binApps[i+pageCount*lineCount] not NULL and i<lineCount)
 						{
 							printColorEnd(binApps[i+pageCount*lineCount],'\n');
 						
 							i++;
 						}
					}
				}
				else
				{
					error("Please precise an environment where the program/app is ! (see help with h:)");
					break;
				}
 				
				//Printing the warning at the end so if theres no scrolling avaiblable, we can still see it.
				if(pageCount==0 and isFirstPage and !isSearching)
				{
					warning("number of page not precised, printed the first one. (see app h: !)");
				}

 				break;
			case 3:
				isLeaving=true;
				break;
			case 4:
				i=0; //terrible, but will do
				int matchCount=0,matchCountUsr=0;
				int matchPos=0;
				bool isAsync=false;
				bool isUser=false;
				bool finded=false;
				bool isListing=false;
				bool isBin=false;
				isHelp=false;

				char args[1024]={0};
				args[0]='\0';

				char binRunUser=0;

				LOCAL_OFFSET=4;

				//args
				while(CommandBuffer[LOCAL_OFFSET+i] not '\0')
				{
					if(CommandBuffer[LOCAL_OFFSET+i] is 'h' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						isHelp=true;
						break;
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'l' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						isListing=true;
						break;
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'b' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						isBin=true;
						if(CommandBuffer[LOCAL_OFFSET+i+2] is 'u')
						{
							binRunUser='u';
						}
						else if(CommandBuffer[LOCAL_OFFSET+i+2] is 'r')
						{
							binRunUser='r';
						}
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'a' and CommandBuffer[LOCAL_OFFSET+1+i] is ':' and CommandBuffer[LOCAL_OFFSET+2+i]-'0' <= 1 and CommandBuffer[LOCAL_OFFSET+2+i]-'0' >= 0)
					{
						isAsync=CommandBuffer[LOCAL_OFFSET+2+i]-'0';
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'e' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						int y=0;
						
						// '\0' check as a safeguard just in case
						while(CommandBuffer[LOCAL_OFFSET+2+i] not '\0' and CommandBuffer[LOCAL_OFFSET+2+i] not '\n' and CommandBuffer[LOCAL_OFFSET+2+i] not ' ' and CommandBuffer[LOCAL_OFFSET+2+i] not '\t')
						{
							AppBuffer[y]=CommandBuffer[LOCAL_OFFSET+2+i];
							y++;
							i++;
						}

						AppBuffer[LOCAL_OFFSET+2+y]='\0';
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'p' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						int y=1;
						args[0]=' ';
						while(CommandBuffer[LOCAL_OFFSET+2+i] not '\0' and CommandBuffer[LOCAL_OFFSET+2+i] not '#' and CommandBuffer[LOCAL_OFFSET+3+i] not 'E' and CommandBuffer[LOCAL_OFFSET+4+i] not 'N' and CommandBuffer[LOCAL_OFFSET+5+i] not 'D')
						{
							args[y]=CommandBuffer[LOCAL_OFFSET+2+i];
							y++;
							i++;
						}

						args[LOCAL_OFFSET+2+y]='\0';
					}

					i++;
				}

				if(isHelp)
				{
					printColor("Arguments :\n e:[name] --> application name to execute.\n a:[0 or 1] --> 1 for asynchronous (you can still type in after booting up a software) or 0 for synchronous (useful for viewing logs).\n b:[r or u] --> execute a program from /usr/bin as user(u) or root(r)\n l: --> display all async. process running.\n p:[args] precise args to pass (end all args with #END, ex : run b:r p:update#END e:apt, not necessary if p: is the last argument of run,ex : run b:r e:apt p:update)\n h: --> display this help.\n");
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

				
				if(isBin)
				{
					if(binRunUser is 0 or (binRunUser not 'u' and binRunUser not 'r'))
					{
						error("Please enter after b: 'u' or 'r' for running as user or root !");
						break;
					}


					printColor("Searching...\n");
					while((binApps[i] not NULL) and !finded)
 					{
 						//If it exactly match, in case of a filename starting with the
 						// same words / letter sequence
 						if(s_isEqual(AppBuffer,binApps[i]))
 						{
 							matchCount=1;
 							matchPos=i;
 							finded=true;
 							break;
 						}
	

 						if(s_StartWith(AppBuffer,binApps[i],s_Lenght(AppBuffer)) and !finded)
 						{
 							matchCount++;
 							printColorEnd(binApps[i],'\n');
 							matchPos=i;
 						}

 						i++;
 					}

 					if(matchCount>1)
 					{
 						printf("%sFound %s%d%s matches in total !%s\n",TextColor,HighLightColor,matchCount+matchCountUsr,TextColor,COLOR_RESET);
 					}
 					else if(matchCount == 0)
 					{
 						printf("%sDid not find any applications named '%s' !%s\n",TextColor,AppBuffer,COLOR_RESET);
 					}
 					else
 					{
 						printColor("Found !\n");
 						char exec[512];

 					
 						s_Merge(exec,"/usr/bin/",binApps[matchPos]);
						RunningApps[pidsCount]=s_copy(binApps[matchPos]);

 						if(RunningApps[pidsCount] is NULL)
 						{
 							return errorFatal("Couldn't allocate ! (RunningApps)",-2);
 							break;
 						}

 						if(binRunUser is 'r')
 						{
 							char tempBuffer[512];
 							s_Merge(tempBuffer,"sudo ",exec);
 							for(int y=0;y<s_Lenght(tempBuffer);y++)
 							{
 								exec[y]=tempBuffer[y];
 							}
 						}

 						if(args[0] not '\0')
 						{
 							s_Merge(exec,exec,args);
 						}

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

 							printf("%s Process started on PID %d\n to stop it, type %s'stop [PID]'%s\n",TextColor,PIDS[pidsCount],HighLightColor,COLOR_RESET);
 							pidsCount++;
 						}
 						else
 						{
 							system(exec);
 						}
 					}
				}
				else
				{
					printColor("Searching...\n");
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
	 					printf("%sFound %s%d%s matches in total !%s\n",TextColor,HighLightColor,matchCount+matchCountUsr,TextColor,COLOR_RESET);
	 				}
	 				else if(matchCount+matchCountUsr == 0)
	 				{
	 					printf("%sDid not find any applications named '%s' !%s\n",TextColor,AppBuffer,COLOR_RESET);
	 				}
	 				else
	 				{
	 					printColor("Found !\n");
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

	 					if(args[0] not '\0')
 						{
 							s_Merge(exec,exec,args);
 						}

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
				}
 				break;
 			case 5:
 				isHelp=false;
 				char sPIDS[32]={0};
 				int PID=0;
 				bool doKill=false;
 				i=0;

 				LOCAL_OFFSET=5;

 				while(CommandBuffer[LOCAL_OFFSET+i] not '\0')
				{
					if(CommandBuffer[LOCAL_OFFSET+i] is 'h' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						isHelp=true;
						break;
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'k' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						doKill=true;
					}

					if(CommandBuffer[LOCAL_OFFSET+i] is 'p' and CommandBuffer[LOCAL_OFFSET+1+i] is ':')
					{
						int y=0;
						while(CommandBuffer[LOCAL_OFFSET+2+i] not '\0' and CommandBuffer[LOCAL_OFFSET+2+i] not ' ' and CommandBuffer[LOCAL_OFFSET+2+i] not '\t')
						{
							sPIDS[y]=CommandBuffer[LOCAL_OFFSET+2+i];
							y++;
							i++;
						}

						sPIDS[LOCAL_OFFSET+2+y]='\0';
					}
					i++;
				}

				if(isHelp)
				{
					printColor("Arguments :\n p:[PID number] --> (not optional) to precise the process PID\n k: --> to kill the process (sends SIGKILL instead of SIGTERM)(Always use it if running a /usr/bin app !)\n h: --> display this help\n");
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
 			case 6:
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
	ClearUser();
	return 0;
}
