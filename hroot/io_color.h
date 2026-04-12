#define COLOR_RESET "\e[37m"
#define COLOR_HIGHLIGHT "\e[32m"

void setColor(char color,char colorh);
char *GetColor();
char *GetColorHighlight();
void printColor(const char *msg); //simple func to avoid rewriting color code each time
void printColorEnd(const char *msg,const char end);