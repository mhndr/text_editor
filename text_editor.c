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
static unsigned int x=0,y=0;
static wint_t key;
static unsigned int width, height;

#ifdef debug
static char* debug_str = "";
void set_debug_str(char *str)
{
	strncpy(debug_str,str,strlen(str));
}
#endif 

static void redraw_screen()
{
	int _y=0,_x=0;
	line_t *line = first;
	
	clear();
	#ifdef debug
	attrset(A_REVERSE);
	mvprintw(height-2,0,"%*c",width-1," ");
	mvprintw(height-2,0,"%p <-%p->%p| %s-%d |u=%d,a=%d|x=%d,y=%d| %s",
			curr->prev,curr,curr->next,curr->text,strlen(curr->text),
			curr->usize,curr->asize,x,y,debug_str);	
	attrset(A_NORMAL);
	#endif
	while(line){
		mvprintw(_y,_x,"%s",line->text);
		_y = (_y+1)%height;
		line=line->next;
	}

	attrset(A_BOLD);
	for (int i=_y;i<height-2;i++) { 
		move(i,0);
		mvprintw(i, 0, "~"); 
	}
	attrset(A_NORMAL); 
	move(y,x);
	refresh();
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
	memmove(curr->text+x+1,curr->text+x,curr->usize-x);
	curr->text[x] = c;
	curr->usize++;
	x = (x+1)%width;
}

static void append_str(const char *str)
{
	int size = strlen(str);
	if(curr->usize + size < curr->asize){
		curr->asize += size;
		curr->text = realloc(curr->text,curr->asize);
	}
	//memmove(curr->text+x+1,curr->text+x,curr->usize-x);
	strncat(curr->text,str,size);
	curr->usize += size ;
	x = (x+size)%width;
}

static void  handle_backspace()
{
	line_t *line = curr;

	if(line->usize==0 && !line->prev) {
		return;
	}

	if( (line->usize==0 && line->prev) || x==0 ) {
		/*if the line was empty or the cursor was at SOL*/
		if(line->prev) 
			line->prev->next = line->next;
		if(line->next) 
			line->next->prev = line->prev;
		y--;	
		curr = line->prev;
		if(x==0) {
			//needs fixing
			append_str(line->text);
		}
		//x = curr->usize;
		free_line(line);
	}

	else {
		if(line->usize == x) {	
			/*if the cursor was at EOL*/
			x--;
			line->text[x]='\0';
			line->usize--;
		}
		else {
			/*if the cursor was somewhere in the middle of the line*/
			memmove(curr->text+x,curr->text+x+1,curr->usize-x);
			/*can realloc the text buffer here*/
			x--;
			line->usize--;	
		}
	}
	redraw_screen();
}

static void handle_enter()
{
	line_t *new;

	if(x < curr->usize)	{
		new = create_line(curr->text+x);
		curr->usize=x;
	}
	else {
		new = create_line("");
	}
	curr->text[x]='\0';	
	
	if(curr->next){
		curr->next->prev=new;
		new->next = curr->next;
	}
	new->prev = curr;
	curr->next = new;
	curr = new;
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
		redraw_screen();
	}
}

static void handle_keydown()
{
	if(curr->next!=NULL) {
		y++;	
		curr = curr->next;
		if(x>curr->usize-1)	
			x = curr->usize-1;
		redraw_screen();
	}
}

static void handle_keyleft()
{
	if(x > 0) {	
		x--;
		move(y,x);
		redraw_screen();
	}
}

static void handle_keyright()
{
	if(x != curr->usize){
		x++;
		move(y,x);
		redraw_screen();
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
