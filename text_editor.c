#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>
#include<wchar.h>
#include<locale.h>
//#include<wchar.h>

#define CTRL(c) ((c) & 037)

char *buffer;
static wint_t key;
unsigned int buf_idx = 0,buf_size=10;
static unsigned int width, height;

static void redraw_screen()
{
	int x=0,y=0;
	for(int i=0;i<buf_idx;i++)
    {
		if(buffer[i]==CTRL('m'))
		{
			y = (y+1)%height;
			x = 0;
			mvprintw(y,x,"");
			continue;
		}
		mvprintw(y,x,"%c",buffer[i]);
		if(x++ >= width)
		{
			x = 0;
			y = (y+1)%height;
		}
	}		
}

int main()
{
	wint_t key;
	int x=0,y=0;

	initscr(); raw(); noecho(); nonl(); keypad(stdscr, TRUE);
	setlocale(LC_ALL, ""); 
   	getmaxyx(stdscr, height, width);
	buffer = (char*) malloc(buf_size);
	
	while(1)
	{
		wget_wch(stdscr,&key);
		if(key==CTRL('q'))
			break;
		//printable key
		if(buf_idx == buf_size)
		{
			buffer = realloc(buffer,buf_size+10);
			buf_size += 10;
		}
		buffer[buf_idx++] = key;
		redraw_screen();
	}
	
	noraw(); endwin(); return 0;
}
