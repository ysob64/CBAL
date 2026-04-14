CBAL
==

CBAL (Console Based application launcher) is a c based application launcher. It can launch applications via all .desktop shortcut (System shared applications or locally installed for the current user).
It is made only for linux based os and will still forever be.

Commands
--

(from the help command)  
Heres the list of all available commands :  
 * app : List all available applications/programs.    
 * run : to execute a program, by matching filename in available .desktop files or in /usr/bin with the b: argument.
 * stop : to terminate or kill a program started with run.  
 * time : display the current local time.  
 * exit : exit this program, IMPORTANT TO USE instead of ctrl+C !!!

As you may see, there is no search command since 'run' automatically search the app by matching them.
If more than one app match the research, then it displays all app that matched the typed text.

Arguments
--

They are written under the form letter:value , if no value is required, then it is simply letter: .

Configuration file
--
It can be found at /home/[user]/.config/CBAL/cfg.txt

Required dependency (on runtime only)
--
'sudo' (any version) for running as root.

FAQ
--

### Why ?
I made this for people that doesn't uses an application menu on extremely lightweight distros., as an alternative to dmenu.
Even if dmenu is very lightweight, and very fast in his search, I was tired to have EVERY available executable from /usr/bin 
and other directories I don't even know they where existing.

TODO
--

- [x] First setup detection to automatically create cfg.txt in /home/[user]/.config/CBAL/.
- [ ] arm architecture builds. (when v1 will be out)
- [x] /usr/bin search & run command. (useful for true async instead of using the & operand in terminal (which kill the process anyway when the terminal closes))
- [x] Add args support for 'run'.
- [ ] make 'run' runs in a external terminal.
- [ ] add args support (argc, argv) to, for a first feature, automatically boot it up on a(multiple) specified command(s).
