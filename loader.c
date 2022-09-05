// contains routines for opening & loading object files
/* Loader

- 1 word = 2 bytes 
Use 'hexdump -C <filename>' to view contents of obj files

*/

#include <stdio.h>
#include <stdlib.h>
#include "loader.h"

void load_obj(short int* arr, int size, char* obj_file) {

	File *theFile;
	theFile = fopen(obj_file, "rb");
	if (theFile == NULL) {
		printf("Couldn't open file\n");
	}

	short int head;
	short int addr;
	short int n;
	short int line;
	short int file_index;

	while (fread(&head, sizeof(short int), 1, theFile) == 1) {
		if (head == 0xCADE) { //Code
			fread(&addr, sizeof(short int), 1, theFile); 
			fread(&n, sizeof(short int), 1, theFile);
			load_code(theFile, arr, addr, n);
		} else if (head == 0xDADA) { //Data
			fread(&addr, sizeof(short int), 1, theFile); 
			fread(&n, sizeof(short int), 1, theFile);
			load_data(theFile, arr, addr, n);
		} else if (head = 0xC3B7) { //Symbol
			fread(&addr, sizeof(short int), 1, theFile); 
			fread(&n, sizeof(short int), 1, theFile);
			read_chars(theFile, n);

		} else if (head == 0xF17E) { //File name
			fread(&n, sizeof(short int), 1, theFile);
			read_chars(theFile, n);

		} else if (head == 0x715E) { //Line number
			fread(&addr, sizeof(short int), 1, theFile); 
			fread(&line, sizeof(short int), 1, theFile);
			fread(&file_index, sizeof(short int), 1, theFile);
		}
	}

	fclose(theFile);

}


void load_code(File* fileIn, short int* arr, short int address, short int n) {
	short int instr;

	for (int i = 0; i < n; i++) {
		fread(&instr, sizeof(short int), 1, theFile);
		arr[address + i] = instr;
	}
}

void load_data(File* fileIn, short int* arr, short int address, short int n) {
	short int data;

	for (int i = 0; i < n; i++) {
		fread(&data, sizeof(short int), 1, theFile);
		arr[address + i] = data;
	}
}

void read_chars(File* fileIn, short int n) { //read n chars
	for (int i = 0; i < n; i++) {
		fgetc (theFile);
	}
}




