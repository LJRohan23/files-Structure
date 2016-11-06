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
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
void Usage() {
	int fd;
	char ch;
	fd = open("help.c", O_RDONLY);
	while(read(fd, &ch, 1))
		putchar(ch);
}
int main(int argc, char *argv[]) {
	if(argc == 2) {
		if(strcmp(argv[1], "-h") == 0) {
			Usage();
			printf("\n");
			return 0;
		}
		else {
			errno = EINVAL;
			perror("Invalid syntax : ");
			return errno;
		}
	}
	files * fs = Fopen("123.c", "r");
	if(fs == NULL) {
		errno = EINVAL;
		perror(" Unable to open the file: ");
	}
	else
		Fclose(fs);
	files * fp = Fopen("demo.c", "w+");
	if(fp == NULL) {
		errno = EINVAL;
		perror(" Unable to open the file: ");
		return errno;
	}
	int x = 405;
	int d;
	Fwrite(&x, 4, 1, fp);
	Fseek(fp,0,0);
	Fread(&d, 4, 1, fp);
	printf("Integer value in demo.c %d \n", x);
	Fclose(fp);

	files *fq = Fopen("Hello.c", "a");
	if(fq == NULL) {
		errno = EINVAL;
		perror(" Unable to open the file: ");
		return errno;
	}
	Fprintf(fq, "Rohan");
	int m = Ftell(fq);
	printf("Hello.c File pointer position %d \n", m);
	Fclose(fq);

	files *fr = Fopen("demo.c", "r+");
	if(fr == NULL) {
		errno = EINVAL;
		perror(" Unable to open the file: ");
		return errno;
	}
	Ungetc('+', fr);
	char arr[20];
	Fscanf(fr, "%s", arr);
	Fclose(fr);

	printf("String read from demo.c : %s\n", arr);
	return 0;
}
