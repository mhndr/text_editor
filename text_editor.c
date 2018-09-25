#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>
#include<locale.h>
#include<string.h>
#include<wchar.h>

#define CTRL(c) ((c) & 037)

typedef struct line 
{
	char *text;
	int usize;
	int asize;
	struct line *next,*prev;	
}line_t;

line_t *first;
line_t *curr;
int x=0,y=0;
static wint_t key;
static unsigned int width, height;

static void redraw_screen()
{
	clear();
	int y=0,x=0;
	line_t *line = first;
	while(line)
	{
		for(int i=0;i<line->usize;i++)
		{
			if(line->text[i]=='\0')
			{	
				y = (y+1)%height;
				x = 0;
				mvprintw(y,x,"");
				continue;
			}	
			mvprintw(y,x++,"%c",line->text[i]);
			if(x==width)
			{
				x=0;
				y=(y+1)%height;
			}
		}
		line=line->next;
	}
}

static line_t* create_line()
{
	line_t *new = (line_t*) malloc(sizeof(line_t));
	new->usize = 0;
	new->asize = 0; 
	new->text = (char*)malloc(new->asize);
	new->next = new->prev = NULL;
	return new;
}

static void insert_char(char c)
{
	if(curr->usize == curr->asize)
	{
		curr->asize +=1;
		curr->text = realloc(curr->text,curr->asize);
	}
	curr->text[curr->usize++]=c;
	x = (x+1)%width;
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
	redraw_screen();
}

static void handle_enter()
{
	insert_char('\0');
	line_t *new = create_line();
	new->prev = curr;
	curr->next = new;
	curr = new;
	redraw_screen();
	y = (y+1)%height;
}

static void handle_keyup()
{
	if(curr->prev!=NULL)
	{
		y--;
		curr = curr->prev;	
		move(y,x);
	}
}

static void handle_keydown()
{
	if(y != height && curr->next!=NULL)
	{	
		curr = curr->next;
		y++;	
		move(y,x);
	}
}

static void handle_keyleft()
{
	x--;
	move(y,x);
}

static void handle_keyright()
{
	if(x != width && x != curr->usize)
	{
		x++;
		move(y,x);
	}	
}

int main()
{
	wint_t key;
	initscr(); raw(); noecho(); nonl(); keypad(stdscr, TRUE);
	setlocale(LC_ALL, ""); 
   	getmaxyx(stdscr, height, width);
	first = create_line();
	curr=first;
	
	while(1)
	{
		key = wgetch(stdscr);
		if(key==CTRL('Q'))
			break;
		if(key==127)//KEY_BACKSPACE)
		{
			handle_backspace();
			continue;	
		}
		if(key == CTRL('m'))
		{
			handle_enter();
			continue;
		}
		if(key == KEY_UP)
		{
			handle_keyup();
			continue;
		}
		if(key == KEY_DOWN)
		{
			handle_keydown();
			continue;
		}
		if(key == KEY_LEFT)
		{
			handle_keyleft();
			continue;
		}	
		if(key == KEY_RIGHT)
		{
			handle_keyright();
			continue;
		}

		//printable key
	    insert_char(key);	
		redraw_screen();
	}
	
	noraw(); endwin(); return 0;
}
