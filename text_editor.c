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
	int _y=0,_x=0;
	line_t *line = first;
	
	clear();
	mvprintw(height-2,0,"%p-%s",curr,curr->text);	
	while(line){
		for(int i=0;i<line->usize;i++){
			if(line->text[i]=='\0'){	
				_y = (_y+1)%height;
				_x = 0;
				mvprintw(_y,_x,"");
				continue;
			}	
			mvprintw(_y,_x++,"%c",line->text[i]);
			if(_x==width){
				_x=0;
				_y=(_y+1)%height;
			}
		}
		line=line->next;
	}

	attrset(A_BOLD);
	for (int i=_y+1;i<height-2;i++) { 
		move(i,0);
		//clrtoeol();
		mvprintw(i, 0, "~"); 
	}
	attrset(A_NORMAL); 
	move(_y,_x);
	//refresh();

}

static line_t* create_line(const char *str)
{
	line_t *new = (line_t*) malloc(sizeof(line_t));
	new->usize = 0;
	new->asize = strlen(str); 
	new->text = (char*)malloc(new->asize);
	strcpy(new->text,str);
	new->usize = new->asize;
	new->next = new->prev = NULL;
	return new;
}

static void free_line(line_t *line)
{
	if(line) {
		if(line->text)
			free(line->text);
		free(line);
	}
}

static void insert_char(char c)
{
	if(curr->usize == curr->asize){
		curr->asize +=1;
		curr->text = realloc(curr->text,curr->asize);
	}
	curr->text[curr->usize++]=c;
	x = (x+1)%width;
}

static void  handle_backspace()
{
	line_t *line = curr;

	if(line->usize==0 && line->prev) {
		if(line->prev) 
			line->prev->next = line->next;
		if(line->next) 
			line->next->prev = line->prev;
		y--;	
		curr = line->prev;
		free_line(line);
	}
	else 
		line->usize--;

	x = line->usize;
	redraw_screen();
}

static void handle_enter()
{
	line_t *new;

	insert_char('\0');
	if(x < curr->usize)	{
		new = create_line(curr->text+x);
		curr->usize=x;
	}
	else {
		new = create_line("");
	}

	if(curr->next){
		curr->next->prev=new;
		new->next = curr->next;
	}
	else {	
		new->prev = curr;
	}

	curr->next = new;
	curr = new;
	insert_char('\0');
	y = (y+1)%height;
	x = 0;
	redraw_screen();
}

static void handle_keyup()
{
	if(curr->prev!=NULL) {
		y--;
		curr = curr->prev;
		if(x>curr->usize-1)	
			x = curr->usize-1;
		move(y,x);
	}
}

static void handle_keydown()
{
	if(curr->next!=NULL) {
		y++;	
		curr = curr->next;
		x = curr->usize-1;
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
	if(x != curr->usize){
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
	first = create_line("");
	curr=first;
	redraw_screen();
	while(1){
		key = wgetch(stdscr);
		if(key==CTRL('Q'))
			break;
		if(key==127) {//KEY_BACKSPACE)
			handle_backspace();
			continue;	
		}
		if(key == CTRL('m')){
			handle_enter();
			continue;
		}
		if(key == KEY_UP){
			handle_keyup();
			continue;
		}
		if(key == KEY_DOWN){
			handle_keydown();
			continue;
		}
		if(key == KEY_LEFT){
			handle_keyleft();
			continue;
		}	
		if(key == KEY_RIGHT){
			handle_keyright();
			continue;
		}

		//printable key
	    insert_char(key);	
		redraw_screen();
	}
	
	noraw(); endwin(); return 0;
}
