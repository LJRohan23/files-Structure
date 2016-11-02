#ifndef __FILES_H
#define __FILES_H
#include <stdio.h>
#include <stdarg.h>
#include "queue.h"
#define BUFSIZE 1024
#define OPEN_MAX 20
#define PERMS 0666
typedef struct files {
	int total_r; //total read characters (for ftell() access)
	char *ptr; //buffer pointer location (to be used by several functions)
	char *base; //location of buffer
	queue q1; //the queue data type for ungetc(), Will be referred by other functions too
	int cnt; //number of characters remaining in the buffer. All reading functions will modify this counter
	int fd; //file descriptor
	int bufstate; //to denote the buffer existence for the file
	int flag; //denotes the type of the file
	char *mode; //mode of file opening...
	int mark; //location of the last character of the file. All writing functions will modify this mark
	char exceed; //flag for determining EOF encounter
	char *name; //name of the file passed as parameter
	char mark_c;//status of mark calculation
	char mode_c;//status of mode calculation
	char bufcnt;
}files;
enum FLAGS {
	_READ ,
	_WRITE ,
	_RDWR ,
	_EOF ,
	_ERR ,
	_UNBUF ,//
	_BUF //All reading files will check for these conditions
};
int fillbuffer(files *);
files *Fopen(char *, char *);
int Ftell(files *);
void Rewind(files *);
int Fseek(files *, long, int);
int Fgetpos(files *, fpos_t*);
int Fsetpos(files *, const fpos_t*);
size_t Fread(void *, size_t , size_t , files *);
size_t Fwrite(const void *, size_t , size_t , files *);
int Fscanf(files *, char *, ...);
int Fprintf(files *, const char *, ...);
int Fclose(files *);
int Fgetc(files *);
int Putc(int, files *);
char *Fgets(char *, int, files *);
int Ungetc(char, files*);
int Fputs(char *, files*);//used for writing text files 
int Feof(files*);
#endif
