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
static int x=0,y=0;
static unsigned int width, height;

#ifdef debug
static char* debug_str = "";
void set_debug_str(char *str)
{
	strncpy(debug_str,str,strlen(str));
}
#endif 

static int print_line(int row, const char *text,int len) {
	int col = 0;
	for(int i=0;i<len && col<width;col++) {
		if(text[i] == '\t') {
			mvprintw(row,col,"%*s",4,"");
			col += 4;
		}
		else {
			mvaddnwstr(row,col,text+i,1);
			col++;
		}
	}
	return col;
}

static void redraw_screen()
{
	int _y=0,_x=0;
	line_t *line = first;
	int line_count = 0;	

	clear();
	#ifdef debug
	attrset(A_REVERSE);
	mvprintw(height-2,0,"%*c",width-1," ");
	mvprintw(height-2,0,"%p <-%p->%p| %s-%d |u=%d,a=%d|x=%d,y=%d| %s",
			curr->prev,curr,curr->next,curr->text,strlen(curr->text),
			curr->usize,curr->asize,x,y,debug_str);	
	attrset(A_NORMAL);
	#endif
	while(line && line_count<=height-2){
		mvprintw(_y,_x,"%s",line->text);
		//print_line(_y,line->text,line->usize);
		_y = (_y+1)%height;
		line=line->next;
		line_count++;
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
		curr->asize +=2;
		curr->text = realloc(curr->text,curr->asize);
	}
	memmove(curr->text+x+1,curr->text+x,curr->usize-x);
	curr->text[x] = c;
	if(curr->usize == x)
		curr->text[x+1]='\0'; 
	curr->usize++;
	if(x+1 == width)
		y++;
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
	if(x+size == width)
		y++;
	x = (x+size)%width;
}

static void  handle_backspace()
{
	line_t *line = curr;
	int _x = 0;

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
		if(y<0) {
			y=0;
			first = first->prev;
		}
		curr = line->prev;
		_x = curr->usize;
		if(x==0) {
			append_str(line->text);
		}
		x = _x;
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
			memmove(curr->text+(x-1),curr->text+x,curr->usize-x);
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
	x = 0;
	y++;
	if(y>height) {
		y = height;
		first = first->next;
	}
	redraw_screen();
}

static void handle_keyup()
{
	if(curr->prev!=NULL) {
		y--;
		curr = curr->prev;
		if(x > curr->usize-1)	
			x = curr->usize;
		if(y < 0) {
			y=0;
			first = first->prev;
		}
		redraw_screen();
	}
}

static void handle_keydown()
{
	if(curr->next!=NULL) {
		y++;	
		curr = curr->next;
		if(x>curr->usize-1)	
			x = curr->usize;
		if(y>height) {
			y = height;
			first = first->next;
		}
		redraw_screen();
	}
}

static void handle_keyleft()
{
	if(x > 0) {	
		if(curr->text[x]=='\t')
			x = x-8;
		else
			x--;
		move(y,x);
		redraw_screen();
	}
}

static void handle_keyright()
{
	if(x != curr->usize){
		if(curr->text[x]=='\t')
			x=(x+8)>width?width:x+8;
		else
			x++;
		move(y,x);
		redraw_screen();
	}	
}

int open_file(char *fname) {
	FILE *fd;
	char read_buf[256]= "";
	line_t *_line;	
	
	fd = fopen(fname,"r");
	if(fd == NULL)
		return -1;	

	while(fgets(read_buf,256,fd) != NULL) {
		_line = create_line(read_buf);		
		if(!first) {
			first = _line;
			curr = _line;
			continue;
		}
		curr->next  = _line;
		_line->prev = curr;
		curr = _line;
	}
	fclose(fd);	
	curr = first;
	return 0;
}


int main(int argc, char*argv[])
{
	wint_t key;
	initscr(); raw(); noecho(); nonl(); keypad(stdscr, TRUE);
	setlocale(LC_ALL, ""); 
   	getmaxyx(stdscr, height, width);
	
	
	if(argc == 2) {
		open_file(argv[1]);		
	}
	if(!first)
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
