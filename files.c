/*****************************************************************************
 * Copyright (C) Rohan Yogi. yogir15.comp@coep.ac.in
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#include "files.h"
#include "queue.h"
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdarg.h>
#define READING 100
#define WRITING 200
files iob[OPEN_MAX];
int Fillbuffer(files *fp, int attribute) {// to allocate the buffer and check errors accordingly
	if(!strcmp(fp->mode,"w") || !strcmp(fp->mode, "a"))
		return INT_MAX;
	if(fp-> bufstate == _UNBUF) { //currently unbuffered
		fp->base = (char *)malloc(BUFSIZE);
		if(fp->base  == NULL) {
			return EOF;	
		}
	}
	lseek(fp->fd, fp->total_r, 0);
	fp->ptr = fp->base;//resetting the buffer pointer position
	fp->cnt = read(fp->fd, fp->base, BUFSIZE);
	if(--fp->cnt < 0 ) {
		fp->cnt = 0;
		if(attribute == READING) {
			fp->exceed = '1';
			return EOF;
		}
		if(attribute == WRITING) {
			if( fp->mode_c == '0') {
			//have set flags here except for a and w modes
			if(strcmp(fp -> mode, "r")==0)
				fp -> flag = _READ;
			else if(!strcmp(fp -> mode , "r+") || !strcmp(fp -> mode , "w+") || !strcmp(fp -> mode, "a+")) 
				fp ->flag = _RDWR;
			fp -> bufstate = _BUF; //this is primarily for r and all '+' modes
			fp->mode_c = '1';
			}
			return INT_MIN;
		}
	}
	fp->cnt ++;
	if(fp -> mark_c == '0') { // not yet calculated or can say Fillbuffer is called for the first time(compared with fseek)
		if(fp->cnt < BUFSIZE) {
			fp -> mark = fp->cnt + fp->total_r;
			fp->mark_c = '1';
		}
		else {
			fp->mark_c = '1';
			int m = fp->total_r, ct = 0;
			char ch;
			lseek(fp->fd, fp->total_r, SEEK_SET);
			while(read(fp->fd, &ch, 1)) {
				ct ++;
			}
			//lseek(fp->fd, fp->total_r, SEEK_SET);
			fp->mark = m + ct;
			//calculated
		}
	}
	lseek(fp->fd, fp->total_r, SEEK_SET);
	if( fp->mode_c == '0') {
		//have set flags here except for a and w modes
		if(strcmp(fp -> mode, "r")==0)
			fp -> flag = _READ;
		else if(!strcmp(fp -> mode , "r+") || !strcmp(fp -> mode , "w+") || !strcmp(fp -> mode, "a+")) 
			fp ->flag = _RDWR;
		fp -> bufstate = _BUF; //this is primarily for r and all '+' modes
		fp->mode_c = '1';
	}
	return (int)(*fp->ptr); //check this
}	
int Fgetc(files *fp) {
	char ch, x;
	if(fp->flag == _WRITE || fp->exceed == '1')
		return EOF;
	if(!qempty(&fp->q1)) {
		ch = dequeue(&fp->q1);
		fp -> total_r ++;
		fp->cnt --;
		return (int)ch;
	}
	if(--fp->cnt < 0 ) {
		lseek(fp->fd, fp->total_r, SEEK_SET); 
		if((x = Fillbuffer(fp, READING)) == EOF ) {
			return EOF;
		}
		else {
			fp->ptr ++;
			fp->cnt --;
			fp->total_r ++;
			return x;
		}
	}
	fp->total_r ++;
	return *(fp->ptr)++;	
}
int Ungetc(char ch, files *fp) {
	if(!qfull(&fp->q1)) {
		fp -> cnt ++;
		enqueue(&fp->q1, ch);
		fp -> total_r --;
		return (int)ch;
	}
 	return EOF;
}
files *Fopen(char *filename, char *mode) {
	static int i = 0;
	int j;
	// checked for invalid modes
	switch(mode[0]) {
		case 'a':
		case 'r': 
		case 'w': break;
		default : printf("Invalid mode %s\n", mode);
			  return NULL;
	}
	if(i == 0) {
		for(j = 0; j <= OPEN_MAX-1; j++) {
			iob[j].flag = -1;
			iob[j].bufstate = _UNBUF;
			iob[j].mode = NULL;
			qinit(&(iob[j].q1));
		}
		i = 1;
	}	
	files *fp;
	int fd, count = 0, n;
	for(n = 0; count <= OPEN_MAX - 1; n = (n + 1) % OPEN_MAX) {
		count ++;
		if(iob[n].mode == NULL)
			break;
	}
	if(count >= OPEN_MAX) {
		return NULL;
	}	
	fp = &iob[n];
	if(mode[0] == 'r') {
		if(mode[1] == '\0') {
			fd = open(filename, O_RDONLY);
			if(fd == -1) {
				return NULL;
			}
			fp->mode = mode;
		}
		else if(mode[1] == '+') {
			fd = open(filename, O_RDWR);
			if(fd == -1) {
				return NULL;
			}	
			fp-> mode = mode;
		}
		else
			return NULL; //any character after r
		lseek(fd, 0, 0);
	}
	else if(mode[0] == 'w') {
		if(mode[1] == '\0') {
			fd = creat(filename, PERMS);
			if(fd == -1)
				return NULL;
			fp->mode = mode;
			fp -> flag = _WRITE;
		}
		else if(mode[1] == '+') {
			fd = creat(filename, 0666);
			close(fd);
			fd = open(filename, O_RDWR);
			fp->mode = mode;
		}
		else
			return NULL;
	}
	else if(mode[0] == 'a') {
		if(mode[1] == '\0') {
			if(fd = open(filename, O_WRONLY) == -1)
					fd = creat(filename, 0666);
			lseek(fd, 0, SEEK_END);
				// no buffer for this mode. As no reading functions can be implemented in this mode.
			fp->mode = mode;
			fp -> flag = _WRITE;	
		}
		else if(mode[1] == '+') {
			if(fd = open(filename, O_RDWR) == -1) {
				fd = creat(filename, 0666);
			}
			close(fd);
			fd = open(filename, O_RDWR);	
			fp-> mode = mode;		
            /*KEY STEP*/lseek(fd, 0, SEEK_SET);// enabled for reading. For writing the position will be changed manually...
		}
		else
			return NULL;
	}
	// setting initial values to all data members
	fp -> fd = fd;
	fp->mark_c = fp->bufcnt = '0';
	fp -> name = filename;
	fp -> total_r = fp->mark = 0;
	//following parameters shall be disabled for non-reading modes...like w or a
	if(strcmp(mode, "a")!=0 && strcmp(mode, "w")!=0) {
		fp -> base = fp->ptr = NULL;
		fp -> cnt = 0;
		fp -> exceed = fp -> mode_c = '0';
	}
	return fp;
}	
int Fclose(files *fp) { //handle return values
	// set all the attributes of the file pointer to zero and NULL appropriately
	if(fp == NULL) {
		return EOF;
	}
	free(fp->base);
	fp->ptr = fp->base = fp->mode = NULL;
	fp->cnt = fp->total_r = fp->mark = 0;
	while(!qempty(&fp->q1)) {
		dequeue(&fp->q1);
	}
	fp->bufstate = -1;
	fp->flag = -1;
	fp->fd = -1;
	fp->exceed = '0';
	fp->name = NULL;
	fp->base = NULL;
	fp->mark_c = '0';
	fp = NULL; //removed the pointer from the array index it was earlier pointing to
	return 0;
}	
int Ftell(files *fp) {
	return fp->total_r;
}
int Fseek(files *fp, long offset, int whence) { //POSSIBLE ERRORS : INVALID WHENCE
	switch(whence) {
		case SEEK_SET :
		case SEEK_CUR :
		case SEEK_END : break;
		default : return -1;
	}
	if(fp->flag == _WRITE) {
		//fp->mark calculation
		if(fp->mark_c == '0' && strcmp("a", fp->mode)==0) {
			close(fp->fd);
			fp->fd = open(fp->name, O_RDONLY);
			int i = 0;
			char ch;
			while(read(fp->fd, &ch, 1))
				i++;
			fp->mark = i;
			close(fp->fd);
			fp->fd = open(fp->name, O_WRONLY);
			lseek(fp->fd, fp->total_r, SEEK_SET);
			fp->mark_c = '1';
		}	
		lseek(fp->fd, offset, whence);
		if(fp->total_r + offset < 0)
			fp->total_r = 0;
		else {
			if(whence == 0) {
				fp->total_r = offset;
			}
			else
			if(whence == 1) {
				fp->total_r += offset;
			}
			else
			if(whence == 2) {
				fp->total_r = fp->mark + offset;//offset can be negative too;	
			}
			else
 
				return -1;
		}
		return 0;
	}
	if(fp -> mark_c == '0') { //data member not yet calculated	
		fp->mark_c = '1';// calculated
		Fillbuffer(fp, READING);
		lseek(fp->fd, fp->total_r, 0); // just to be safe this shall be total_r = 0 as in this case only shall mark be uncalculated.
		int i=0;
		char ch;
		while(read(fp->fd, &ch, 1)) {
			i ++;
		}
		lseek(fp->fd, fp->total_r, SEEK_SET);
		fp->mark = fp->total_r + i;	
	}
	while(!qempty(&fp->q1)) {
		dequeue(&fp->q1);
	}
	int i = 0, x;
	char ch;
	if(whence == 0) {
		if(fp->exceed == '1')
			fp->exceed = '0';
		if(offset <= 0) {
			lseek(fp->fd, 0, SEEK_SET);
			fp->total_r = 0;
			Fillbuffer(fp, READING);
			return 0;
		}
		else if(offset >= fp->mark ) {
			lseek(fp->fd, 0, 2);
			fp->exceed = '1';
			fp->base = NULL;
			fp ->ptr = NULL;
			fp->cnt = 0;
		}
		else {
			lseek(fp->fd, 0, SEEK_SET);
			lseek(fp->fd, offset, 0);
			Fillbuffer(fp, READING);
		}
		fp->total_r = offset;
	}
	else if(whence == 1) {
		if(fp->exceed != '1') {
			//implies total_r is valid position
			lseek(fp ->fd, fp -> total_r, SEEK_SET);// to update the buffer location to what it should be
			if(offset <= 0) {
				if(fabs(offset) >= fp->total_r) { //bring it to first position
					lseek(fp->fd, 0, 0);
					fp->total_r = 0;
					Fillbuffer(fp, 100);
				}
				else {
					lseek(fp->fd, offset, SEEK_CUR);
					fp->total_r += offset;
					Fillbuffer(fp, 100);
				}	
				return 0;
			}if(offset > 0) {
				if(fp->total_r + offset > fp->mark) { //limit exceeded
					lseek(fp->fd, 0, 2);
					fp->exceed = '1';
					fp->base = fp->ptr = NULL;
					fp->cnt = 0;
					fp->total_r += offset; //VERIFY THIS.
				}
				else if((fp->total_r + offset) <= fp->mark) {
					lseek(fp->fd, offset, 1);
					fp->total_r += offset;
					Fillbuffer(fp, 100);
				}
				
			}
		}
		else if(fp->exceed == '1'){ //no characters left..end of file reached.
			if(offset >= 0) {
				fp->total_r = fp->total_r + offset;
			}
			else if (offset < 0) {
				if(fp->total_r + offset < fp->mark) {
					fp->total_r += offset;
					fp->exceed = '0';
					lseek(fp->fd, offset, SEEK_CUR);
					Fillbuffer(fp,100);
				}
				else
					fp ->total_r += offset;		
			}
		}
	}
	else if(whence == 2) { //worked on this
		if(offset < 0) {
			if(fp->exceed == '1') {
				fp->exceed = 0;
			}
			if(abs(offset) >= fp->mark) {
				lseek(fp->fd, 0, 0);
				fp->total_r = 0;
				Fillbuffer(fp, 100);
			}
			else {
				lseek(fp->fd, offset, SEEK_END);
				fp->total_r = fp->mark + offset;
				Fillbuffer(fp, 100);
			}
		}
		else if(offset >= 0) {
			lseek(fp->fd, 0, 2);
			fp->total_r = offset + fp->mark;
			fp->exceed = '1';
			fp->cnt = 0;
			fp->ptr = fp->base = NULL;
		}	
	}
	return 0;
}
void Rewind(files *fp) {
	Fseek(fp, 0, SEEK_SET);
}
int Fgetpos(files *fp, fpos_t *pos) {
	//this function is similar to Ftell(); therefore this shall access the total_r member
	pos->__pos = fp->total_r;// as defined in _G_config.h
	return 0;
}
int Fsetpos(files *fp, const fpos_t *pos) {//the value of pos is obtained from the function Fgetpos();
	//similar to Fseek with whence SEEK_SET
	Fseek(fp, pos->__pos, SEEK_SET);		
	return 0;
}
char *Fgets(char *arr, int size, files* fp) {
	if(fp->flag == _WRITE || size < 0) {
		return NULL;
	}
	// this should affect fp->cnt, fp->total_r
	arr[0] = '\0'; 
	int x = 0;
	char var;
	Fseek(fp, fp->total_r, 0);
	while(x <= size - 2) {
		if(fp->cnt == 0) {
			lseek(fp->fd, fp->total_r, 0);
			var = Fillbuffer(fp, READING);
			if(var == '\n' ){
				arr[x++] = '\n';
				arr[x] = '\0';
				fp->ptr ++;
				fp->cnt --;
				fp->total_r ++;
				return arr;
			}
			else if(var == EOF) {
				arr[x] = '\0';
				return arr;
			}
			else {
				arr[x] = var;
				x++;
				fp->ptr ++;
				fp->cnt --;
				fp->total_r ++;
			}
		}
		else { 
			if(*(fp->ptr) == '\n') {
				arr[x++] = '\n';
				fp->total_r ++;
				fp->cnt --;
				fp->ptr ++;
				break;
			}
			arr[x] = *(fp->ptr)++;
			x ++;
			fp->cnt--;// if it is at the last character it will get to zero and then in the next loop call, Fillbuffer will be called.
			fp->total_r ++;
		}	
	}
	arr[x] = '\0';	
	return arr;
}
int Putc(int c, files *fp) { // fseek works separately for write too
	char ch = c; 
	if(fp->bufstate == _UNBUF && fp->flag!=_WRITE) {
		Fillbuffer(fp, WRITING);
	}
	if(fp->flag == _READ)
		return EOF;
	if(fp->flag == _WRITE) {
		if(strcmp(fp->mode, "a") == 0) {
			// for this both the total_r and mark shall increase. cuz adding always happens at the end.
			Fseek(fp, 0, 2);
			write(fp->fd, &ch, 1);
			fp->total_r ++;
			fp->mark ++;
		}
		else if(!strcmp(fp->mode, "w")) {
			// if something is getting overwritten then no increase in mark.
			write(fp->fd, &ch, 1);
			if(fp->total_r == fp->mark) {
				fp->total_r ++;
				fp->mark++;
			}
			else
				fp->total_r ++;
		}
	}
	if(fp->flag == _RDWR) {
		if(!strcmp(fp->mode, "r+") || !strcmp(fp->mode, "w+")) {//both do overwriting from current position
			lseek(fp->fd, fp->total_r, 0);
			write(fp->fd, &ch, 1);// from the current position only
			//if(!strcmp(fp->mode, "r+") || fp->mark == fp->total_r){
			if(fp->mark == fp->total_r) {
				if(!strcmp(fp->mode, "r+")) {
					if(fp->total_r==0)
						fp->total_r ++;
					fp->mark ++; //no change to fp->total_r cuz it is not the characters read in history

				}
				else if(!strcmp(fp->mode, "w+")) {
					if(fp->total_r==0)
						fp->total_r ++;
					fp->mark ++;
				}
			}
			else if(!strcmp(fp->mode, "r+")) {
				// all those thingys related to reading fromthe buffer
				fp->total_r ++;//indirect change to current character
				fp->cnt --;
				if(fp->cnt < 0) {
					fp->cnt = 0;
				}
				fp->ptr++;
			}
			else {
 				fp->total_r ++;
				fp->cnt --;
				if(fp->cnt < 0) {
					fp->cnt = 0;
				}
				fp->ptr++;
			}
		}
		else if(!strcmp(fp->mode, "a+")) {
			Fseek(fp, 0, 2);
			write(fp->fd, &ch, 1);
			fp->total_r ++;
			fp->mark ++;
		}
	}
	//printf("%d %d", fp->mark, fp->total_r);
	return (int)c;
}
int Fputs(char *str, files *fp) {
	int x = strlen(str);
	if(x == 0) {
		return EOF;
	}
	if(fp->bufstate == _UNBUF && fp->flag!=_WRITE) {
		Fillbuffer(fp, WRITING);
	}
	if(fp->flag == _READ) {
		return EOF;
	}
	if(fp->flag == _WRITE) {
		if(!strcmp(fp->mode, "a")) {
			Fseek(fp, 0, SEEK_END);
			int f = write(fp->fd, str, x);//excluding the null character, otherwise it will become a binary file
			fp->total_r += f;
			fp->mark += f;
		}
		else if(!strcmp(fp->mode, "w")) {
			int f = write(fp->fd, str, x);
			if(fp->mark == fp->total_r) {
				fp->total_r += f;
				fp->mark += f;
			}
			else
				fp->total_r += f;
		}
	}
	//now
	if(fp->flag == _RDWR) {
		if(!strcmp(fp->mode, "r+") || !strcmp(fp->mode, "w+")) {//both do overwriting from current position
			Fseek(fp, fp->total_r, 0);
			int f = write(fp->fd, str, x);// from the current position only
			//if(!strcmp(fp->mode, "r+") || fp->mark == fp->total_r){
			if(fp->mark == fp->total_r) {
				if(!strcmp(fp->mode, "r+")) {
					fp->total_r += f;
					fp->mark += f; //no change to fp->total_r cuz it is not the characters read in history

				}
				else if(!strcmp(fp->mode, "w+")) {//this is the entire problem
					fp->total_r += f;
					fp->mark += f;
				}
			}
			else if(!strcmp(fp->mode, "r+")) {
				// all those thingys related to reading fromthe buffer
				fp->total_r += f;//indirect change to current character
				fp->cnt -= f;
				if(fp->cnt < 0) {
					fp->cnt = 0;
				}
				fp->ptr += f;
			}
			else {
 				fp->total_r += f;
				fp->cnt -= f;
				if(fp->cnt < 0) {
					fp->cnt == 0;
				}
				fp->ptr += f;
			}
		}
		else if(!strcmp(fp->mode, "a+")) {
			Fseek(fp, 0, 2);
			int f = write(fp->fd, str, x);
			fp->total_r += f;
			fp->mark += f;
		}
	}
	return 1;
}
int Fprintf(files* fp, const char *format, ...) { // To be modified and still a lot to be done
	va_list list;
	queue q1;
	qinit(&q1);
	int i, n = strlen(format),x = 0 ;// x shall be used to insert characters into the text string
	char str[100];
	if(n == 1 && format[0] == '%') {
		printf("Error : Spurious trailing '%%' in the second parameter.");
		return EOF;
	}
	int count = 0, retcnt = 0;
	for(i = 0; i <= n-2; i ++) {
		switch(format[i]) {
			case'%' : switch(format[i+1]) {
					case 'f':count ++; break;
					case 'c':count ++; break;
					case 's':count ++; break;
					case 'd':count ++; break;
					case 'l': switch(format[i+2]) {
							case 'd':count++; break;
							case 'f':count++; break;
							default : break;
						  }
						 break;
					default :break;
				}
				break;
			default : break;
		}
	}
	if(count == 0) {
		char a[100];
		strcpy(a, format);
		//simple fputs of the second parmeter to the file pointed to by fp
		Fputs(a, fp);
		return strlen(a);
	}
	va_start(list, count);
	for(i = 0; i <= n - 1; i++) {
		if(format[i] != '%') {
			str[x++] = format[i];
		}
		else
		if(format[i] == '%' ) {
			if(i == n-1) {
				if(x == 0) {
					Fputs("%", fp);
				}
				else 
					str[x++] == '%';
			}
			else if(format[i+1] != 'd' && format[i+1] != 'f' && format[i+1] != 's' && format[i+1] != 'c' && format[i+1] !='l') {
				//it is just a % sign with no format specifier. add it to the string. 
				str[x++] = '%';
			}
			else if (format[i+1] == 'd') {
				str[x] = '\0';
				if(Fputs(str, fp) == 1) {
					retcnt += strlen(str);
				}
				str[0] = '\0';
				char s[15];
				int m = va_arg(list, int);
				sprintf(s, "%d", m);
				int n = strlen(s);
				if(Fputs(s, fp) == 1) {
					retcnt += n;
				}
				i += 1;
				x = 0;
			}
			else if (format[i+1] == 'f') {
				str[x] = '\0';
				if(Fputs(str, fp) == 1) {
					retcnt += strlen(str);
				}
				str[0] = '\0';
				char s[20];
				double d = va_arg(list, double);
				float f = d;
				sprintf(s, "%f", f);
				int n = strlen(s);
				if(Fputs(s, fp) == 1)
					retcnt += n;
				i += 1;
				x = 0;
			}
			else if (format[i+1] == 'c') {
				str[x] = '\0';
				if(Fputs(str, fp) == 1) {
					retcnt += strlen(str);
				}
				str[0] = '\0';
				int l = va_arg(list, int);
				char ch = l;
				Putc(ch, fp);
				retcnt += 1;
				i += 1;
				x = 0;
			}
			else if (format[i+1] == 's') {
				str[x] = '\0';
				if(Fputs(str, fp) == 1) {
					retcnt += strlen(str);
				}
				str[0] = '\0';
				char *p = va_arg(list, char *);
				int n = strlen(p);
				if(Fputs(p, fp) == 1)
					retcnt += n;
				i += 1;
				x = 0;
			}
			else if (format[i+1] == 'l') {
				if(i == n-2) {
					i +=1;// since this will result in a warning
				}
				else {
					char s[15];
					if(format[i+2] == 'd' || format[i+2] == 'f') {
						if(format[i+2] == 'd') { //long
							str[x] = '\0';
							if(Fputs(str, fp) == 1) {
								retcnt += strlen(str);
							}
							str[0] = '\0';
							long l = va_arg(list, long);
							sprintf(s, "%ld", l);
							if(Fputs(s, fp) == 1) {
								retcnt += strlen(s);
							}
							i += 2;
							x = 0;
						}
						else if(format[i+2] == 'f') { // double
							str[x] = '\0';
							if(Fputs(str, fp) == 1) {
								retcnt += strlen(str);
							}
							str[0] = '\0';
							double d = va_arg(list, double);
							sprintf(s, "%lf", d);
							if(Fputs(s, fp) == 1) {
								retcnt += strlen(s);
							}
							i += 2;
							x = 0;	
						}
					}
					else {
						i += 1;// cuz u will read the character after %l. The function works that waay.
					}
				}
			}
		}
	}
	str[x] = '\0';
	Fputs(str, fp);
	va_end(list);	
	return retcnt ;
}
int Feof(files *fp) {
	return (fp->exceed == '1');
}
int Fscanf(files *fp, char *format, ...) {
	//use fgetc extensively
	va_list list;
	int i, n = strlen(format);
	char s[30];
	queue q1;
	qinit(&q1);
	int count = 0;
	if(format[n-1] == '%' || format[n-2] == '%' && format[n-1] == 'l') {
		printf("Bad format specifier.. Unrecognised data type. ");
		return EOF;
	}
	for(i = 0; i <= n-2; i ++) {
		if(format[i]=='%' && format[i+1] == ' ') {
			printf("Bad format specifier.. Unrecognised data type. ");
			return EOF;
		}
		switch(format[i]) {
			case'%' : switch(format[i+1]) {
					case 'f':enqueue(&q1, 'f'); count ++; break;
					case 'c':enqueue(&q1, 'c'); count ++; break;
					case 's':enqueue(&q1, 's'); count ++; break;
					case 'd':enqueue(&q1, 'i'); count ++; break;
					case 'l': switch(format[i+2]) {
							case 'd':enqueue(&q1, 'l'); count++; break;
							case 'f':enqueue(&q1, 'd'); count++; break;
							default : while(!qempty(&q1)) {
									dequeue(&q1);
								 }
								printf("Bad format specifier.. Unrecognised data type. ");
								return EOF;//dequeued everything inserted till now
						  }
						 break;
					default :while(!qempty(&q1)) {
							dequeue(&q1);
						 }
						printf("Bad format specifier.. Unrecognised data type. ");
						return EOF;//dequeued everything inserted till now
				}
				break;
			default : break;
		}
	}
	if(count == 0) {
		printf("Undefined format specifier as second parameter to function ");
		return EOF;
	}	
	va_start(list, count);
	//everything enqueued successfully.
	//now it is time to read stuff in a character form.
	while(!qempty(&q1)) {
		char c = dequeue(&q1);
		char ch;
		//take everything onto strings
		if(c == 'i') {
			char str[15];
			str[0] = '\0';
			int a;
			int x = 0, ctr = 0;
			while((ch = Fgetc(fp)) != EOF) {
				if((ch == ' ' || ch == '\n' || ch == '\t') && ctr == 0) {
					//ignoring all initial spaces and \n's
					while((ch = Fgetc(fp)) != EOF && (ch == ' ' || ch == '\n' || ch == '\t'));
					if(ch == EOF) {
						str[x] = '\0';
						 a = atoi(str);
						break;
					}
					else
						Ungetc(ch, fp);
					ctr = 1;
				}
				else if((ch == ' ' || ch =='\n' || ch == '\t') && ctr == 1) {
					Ungetc(ch, fp);
					str[x] = '\0';
					a = atoi(str);
					break;
				}						
				if(isdigit(ch))	{
					str[x ++] = ch;
					if(ctr == 0) {
						ctr = 1;
					}
				}
				else {
					str[x] = '\0';
					Ungetc(ch, fp);
					 a = atoi(str);
					break;
				}
			}
			int *p = va_arg(list, int*);
			*p = a;
		}
		if(c == 'c') {
			char ch;
			ch = Fgetc(fp);
			if(ch == EOF) {
				return EOF;
			}
			char *p = va_arg(list, char*);
			*p = ch;
		}
		if(c == 'l') {
			char str[15];
			str[0] = '\0';
			long a;
			int x = 0, ctr = 0;
			while((ch = Fgetc(fp)) != EOF) {
				if((ch == ' ' || ch == '\n' || ch =='\t')&& ctr == 0) {
					//ignoring all initial spaces and \n's
					while((ch = Fgetc(fp)) != EOF && (ch == ' ' || ch == '\n' || ch == '\t'));
					if(ch == EOF) {
						str[x] = '\0';
						 a = atoi(str);
						break;
					}
					else
						Ungetc(ch, fp);
					ctr = 1;
				}						
				if(isdigit(ch))	{
					str[x ++] = ch;
					if(ctr == 0) {
						ctr = 1;
					}
				}
				else {
					str[x] = '\0';
					Ungetc(ch, fp);
					 a = atol(str);
					break;
				}
			}
			long *p = va_arg(list, long*);
			*p = a;
		}
		if(c == 'f') {
			char str[15];
			str[0] = '\0';
			float a;
			int x = 0, ctr = 0;
			while((ch = Fgetc(fp)) != EOF) {
				if((ch == ' ' || ch == '\n' || ch =='\t') && ctr == 0) {
					//ignoring all initial spaces and \n's
					while((ch = Fgetc(fp)) != EOF && (ch == ' ' || ch == '\n' || ch =='\t'));
					if(ch == EOF) {
						str[x] = '\0';
						 a = atof(str);
						break;
					}
					else
						Ungetc(ch, fp);
					ctr = 1;
				}					
				else if(isdigit(ch))	{
					str[x ++] = ch;
					if(ctr == 0) {
						ctr = 1;
					}
				}
				else if(ch == '.') {
					str[x++] = ch;
					if(ctr == 0) {
						ctr = 1;
					}
				}
				else {
					str[x] = '\0';
					Ungetc(ch, fp);
					a = atof(str);
				 	break;
				}
			}
			float *p = va_arg(list, float*);
			*p = a;
		}
		if(c == 'd') {
			char str[15];
			str[0] = '\0';
			double a;
			float f;
			int x = 0, ctr = 0;
			while((ch = Fgetc(fp)) != EOF) {
				if((ch == ' ' || ch == '\n' || ch =='\t') && ctr == 0) {
					//ignoring all initial spaces and \n's
					while((ch = Fgetc(fp)) != EOF && (ch == ' ' || ch == '\n' || ch =='\t'));
					if(ch == EOF) {
						str[x] = '\0';
						 f = atof(str);
						a = f;
						break;
					}
					else
						Ungetc(ch, fp);
					ctr = 1;
				}				
				else if(isdigit(ch))	{
					str[x ++] = ch;
					if(ctr == 0) {
						ctr = 1;
					}
				}
				else if(ch == '.') {
					str[x++] = ch;
					if(ctr == 0) {
						ctr = 1;
					}
				}
				else {
					str[x] = '\0';
					Ungetc(ch, fp);
					f = atof(str);
					a = f;
					break;
				}
			}
			double *p = va_arg(list, double*);
			*p = a;
		}
		if(c == 's') {
			char ch;
			s[0] = '\0';
			int x = 0;
			while((ch = Fgetc(fp)) != EOF && ch != ' ' && ch != '\n' && ch!='\t') {
				s[x++] = ch;
			}
			if(ch==EOF) {
				s[x] = '\0';
			}
			else {
				Ungetc(ch, fp);
				s[x] = '\0';
			}
			char *p = va_arg(list, char*);
			strcpy(p,s);
			
		}
	}
	va_end(list);
	return 1;
}
size_t Fwrite(const void *ptr, size_t size , size_t nmemb, files *fp){
	if(size < 0 || nmemb < 0) {
		return 0;
	}
	if(fp->bufstate == _UNBUF && fp->flag!=_WRITE) {
		Fillbuffer(fp, WRITING);
	}
	if(fp->flag == _READ) {
		return 0;
	}
	int w;
	if(fp->flag == _WRITE) {
		if(!strcmp(fp->mode, "a")) {
			Fseek(fp, 0, SEEK_END);
			int w = write(fp->fd, ptr, size*nmemb);//excluding the null character, otherwise it will become a binary file
			fp->total_r += w;
			fp->mark += w;
		}
		else if(!strcmp(fp->mode, "w")) {
			w = write(fp->fd, ptr, size*nmemb);
			if(fp->mark == fp->total_r) {
				fp->total_r += w;
				fp->mark += w;
			}
			else
				fp->total_r += w;
		}
	}
	//now
	if(fp->flag == _RDWR) {
		if(!strcmp(fp->mode, "r+") || !strcmp(fp->mode, "w+")) {//both do overwriting from current position
			lseek(fp->fd, fp->total_r, 0);
			w = write(fp->fd, ptr, size*nmemb);// from the current position only
			//if(!strcmp(fp->mode, "r+") || fp->mark == fp->total_r){
			if(fp->mark == fp->total_r) {
				if(!strcmp(fp->mode, "r+")) {
					if(fp->total_r==0)
						fp->total_r += w;
					fp->mark += w; //no change to fp->total_r cuz it is not the characters read in history

				}
				else if(!strcmp(fp->mode, "w+")) {
					if(fp->total_r==0)
						fp->total_r += w;
					fp->mark += w;
				}
			}
			else if(!strcmp(fp->mode, "r+")) {
				// all those thingys related to reading fromthe buffer
				fp->total_r += w;//indirect change to current character
				fp->cnt -= w;
				if(fp->cnt < 0) {
					fp->cnt = 0;
				}
				fp->ptr += w;
			}
			else {
 				fp->total_r += w;
				fp->cnt -= w;//woah
				if(fp->cnt < 0) {
					fp->cnt == 0;
				}
				fp->ptr += w;
			}
		}
		else if(!strcmp(fp->mode, "a+")) {
			Fseek(fp, 0, 2);
			w = write(fp->fd, ptr, size * nmemb);
			fp->total_r += w;
			fp->mark += w;
		}
	}
	//printf("%d %d", fp->mark, fp->total_r);
	return w;	
}
size_t Fread(void *ptr, size_t size, size_t nmemb, files *fp) { // does not distinguish between end of file and error. returns a zero for both. job of the user to check what exactly happened if at all this function returns a 0
	if(fp->flag == _WRITE || size < 0 || nmemb < 0) {
		return 0;
	}
	if(fp->bufcnt == '0') {
		Fillbuffer(fp, READING);
		fp->bufcnt = '1';
	}
	if(fp->total_r >= fp->mark) {
		Fseek(fp, 0, 2);
	}
	else {
		Fseek(fp, fp->total_r , 0);
	}
	size_t r = read(fp->fd, ptr, size * nmemb);// read returns how many bytes it successfully read.
	if(r == -1 || r == 0) {
		return 0;
	}
	fp -> cnt -= r;
	fp ->total_r += r;
	fp -> ptr += r;
	return r;
}
