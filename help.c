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

THIS IS A PROGRAM TO IMPLEMENT THE "files" STRUCTURE IN C

This structure is essential to implement text or binary files.

Functions using this structure have prototypes as follows :

files *Fopen(char *filename, char *mode);

* TO OPEN A FILE IN A PARTICULAR MODE

List of modes

r : Opens the file for reading. The stream is positioned at the beginning of the file.

r+ : opens the file for both reading and writing. Stream is positioned
 at the beginning of the file.

w : Truncate the file to zero length or create the file if it does not exist.
Stream is positioned at the beginning of the file.

w+ : Open file for reading and writing. File created if does not exist
 else the file is truncated to zero length. 
Stream is positioned at the beginning of the file.

a : Open the file for appending. File is created if it does not exist.
 The sream is positioned at the end of the file.

a+ : Open file for reading and appending. The file is created if it does not exist. 
The initial file position for reading is at the beginning of the file.
But output is always written to the end of the file.

files *Fclose(char *filename);

* TO CLOSE A PARTICULAR FILE MENTIONED AS A PARAMETER

files *Fseek(files *stream, long offset, int whence);

* TO MOVE THE FILE POINTER TO THE SPECIFIED POSITION offset FROM WHENCE

Possible values of whence

SEEK_SET (value = 0) : beginning

SEEK_CUR (value = 1) : current position

SEEK_END (value = 2) : end of the file

int Fgetpos(files *stream, fpos_t *pos);

* SIMILAR TO ftell(). USED TO OBTAIN THE FILE POINTER POSITION. 

int Fsetpos(files *stream, const fpos_t *pos);

* SIMILAR TO fseek() with whence parameter set as SEEK_SET

void *Rewind(char *stream);

* SET FILE POINTER TO THE BEGINNING OF THE FILE

files *Ftell(char *filename);

* TO  DETERMINE FILE POINTER POSITION

int Fscanf(files *stream, char *format, ...);

* READ DATA FROM A FILE, DEPENDING ON THE FORMAT SPECIFIERS.
USUALLY USED FOR TEXT FILES

int Fprintf(files *stream, const char *format, ...);

* WRITE DATA INTO A FILE, DEPENDING ON THE FORMAT SPECIFIERS. USED FOR TEXT FILES.

size_t Fread(void *ptr, size_t size, size_t nmemb, files *stream);

* PART OF BINARY I/O. READS nmemb ITEMS OF DATA, EACH size BYTES LONG,
FROM THE STREAM POINTED TO BY stream, STORING THEM AT LOCATION ptr.

size_t Fwrite(const void *ptr, size_t size, size_t nmemb, files *stream);

* PART OF BINARY I/O. WRITES nmemb ITEMS OF DATA, EACH size BYTES LONG,
TO THE STREAM POINTED BY stream, OBTAINING THEM FROM THE LOCATION GIVEN BY ptr.
