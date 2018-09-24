#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>
#include<wchar.h>
#include<locale.h>
#include<string.h>
//#include<wchar.h>

#define CTRL(c) ((c) & 037)
#define BUF_INCR 10

typedef struct line 
{
	char *text;
	int usize;
	int asize;
	struct line *next,*prev;	
}line_t;

line_t *first;
line_t *curr;
static wint_t key;
static unsigned int width, height;

static void redraw_screen()
{
	clear();
	int y = 0;
	line_t *line = first;
	while(line)
	{
		mvprintw(y++,0,"%s",line->text);
		line=line->next;
	}
}

static line_t* new_line()
{
	line_t *new = (line_t*) malloc(sizeof(line_t));
	new->usize = 0;
	new->asize = BUF_INCR; 
	new->text = (char*)malloc(new->asize);
	new->next = new->prev = NULL;
	return new;
}

static void insert_char(char c)
{
	if(curr->usize == curr->asize)
	{
		curr->asize +=BUF_INCR;
		curr->text = realloc(curr->text,curr->asize);
	}
	curr->text[curr->usize++]=c;
}

static void  handle_backspace()
{
	line_t *line = curr;
	while(line && line->usize==0)
		line = line->prev;
	if(line)
	{
		line->text[line->usize]='\0';
		line->usize--;
	}
}

int main()
{
	wint_t key;
	int x=0,y=0;

	initscr(); raw(); noecho(); nonl(); keypad(stdscr, TRUE);
	setlocale(LC_ALL, ""); 
   	getmaxyx(stdscr, height, width);
	first = new_line();
	curr=first;
	
	while(1)
	{
		wget_wch(stdscr,&key);
		if(key==CTRL('Q'))
			break;
		if(key==127)//KEY_BACKSPACE)
		{
			handle_backspace();
			redraw_screen();
			continue;	
		}
		if(key == CTRL('m'))
		{
			insert_char('\0');
			line_t *new = new_line();
			new->prev = curr;
			curr->next = new;
			curr = new;
			redraw_screen();
			continue;
		}
		//printable key
	    insert_char(key);	
		redraw_screen();
	}
	
	noraw(); endwin(); return 0;
}
