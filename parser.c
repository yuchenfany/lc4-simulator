#include <stdio.h>
#include <stdlib.h>

#include "loader.h"
#include "decode.h"
// parser output_filename first.obj second.obj third.obj

int main(int argc, char **argv) {

	if (argc <= 3) { //must have at least 2 args
		return 0; 
	} 

	int array_size = 65536;
	short int contents[array_size];

	for (int i = 2; i < argc; i++) { //obj files
		load_obj(contents, array_size, argv[i]);
	}

	//user code: 0x0000-0x1FFF
	//os code: 0x8000-0x9FFF

	FILE *theFile;
	theFile = fopen(argc[1], "w");

	if (theFile == NULL) {
		printf("Couldn't open output file\n");
		return 1;
	}

	
	for (int i = 0; i < array_size; i++) {
		if (contents[i] != 0) {
			fprintf(theFile, "%04X : %04X", i, contents[i]);
			if (i <= 0x1FFF || (i >= 0x8000 && i <= 0x9FFF)) { //within user code or os code
				INSN instruction = decode_insn(contents[i]);
				fprintf(theFile, " -> ");
				print_insn(theFile, instruction);
			}
			fprintf(theFIle, "\n");
		}
	}

	fclose(theFile);

	return 0;
}

