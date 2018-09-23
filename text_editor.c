#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>
#include<wchar.h>
#include<locale.h>
//#include<wchar.h>

#define CTRL(c) ((c) & 037)

char buffer[10];
static wint_t key;
int main()
{
	static unsigned int width, height;
	wint_t key;
	int buf_idx = 0,x=0,y=0;

	initscr(); raw(); noecho(); nonl(); keypad(stdscr, TRUE);
	setlocale(LC_ALL, ""); 
   	getmaxyx(stdscr, height, width);
	while(1)
	{
		wget_wch(stdscr,&key);
	    buffer[buf_idx++] = key;
		//redrawwin(stdscr); 			
			
		if(key==CTRL('m'))
		{
			y = (y+1)%height;
			x = 0;
			mvprintw(y,x,"");
			continue;
		}
		if(key==CTRL('q'))
			break;
		//printable key
		mvprintw(y,x,"%c",key);
		if(x++ >= width)
		{
			x = 0;
			y = (y+1)%height;
		}
	}
	
	noraw(); endwin(); return 0;
}
