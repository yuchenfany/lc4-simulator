//parser output_filename first.obj second.obj third.obj …. 
// output_filename: file to output
// first.obj, ...: LC4 obj files to load & decode
// maintain array of 64k (64 * 1024) entries corresponding to 
// entire contents of LC4 memory
// before any object files are loaded, clear all to 0 (hexdump to view contents)

/* file format for lc4 obj files:
1. Code: 3-word header (xCADE, <addr>, <n>), n-word body 
2. Data: 3-word header (xDADA, <addr>, <n>) initial data values
3. Symbol: 
4. File name
5. Line number
*/

/* Output format
- list all non-zero contents of memory after all obj files loaded
- seq of lines listing address as 4-bit hex value followed by contents
- print out instruction for code section 
*/

/*
// INSN_TYPE : This enumerated type is used to represent all of the types of LC4 instructions
// Note that ADDI and ANDI are to be used to denote ADD Immediate and AND Immediate respectively.
typedef enum {ILLEGAL_INSN,
	      NOP, BRp, BRz, BRzp, BRn, BRnp, BRnz, BRnzp,
	      ADD, MUL, SUB, DIV, ADDI,
	      MOD,
	      AND, NOT, OR, XOR, ANDI,
	      LDR, STR,
	      CONST, HICONST,
	      CMP, CMPU, CMPI, CMPIU,
	      SLL, SRA, SRL,
	      JSRR, JSR,
	      JMPR, JMP,
	      TRAP, RTI} INSN_TYPE;

// Structure used to represent LC4 instructions
typedef struct {
  INSN_TYPE type;       // Type of the instruction
  int Rd, Rs, Rt;       // Register fields
  short int immediate;  // Immediate field of instruction 
}  INSN;


*/

#include <stdio.h>
#include <stdlib.h>
#include "decode.h"

const char* type_string[] = {"INVALID INSTRUCTION",
	      "NOP", "BRp", "BRz", "BRzp", "BRn", "BRnp", "BRnz", "BRnzp", //1-8
	      "ADD", "MUL", "SUB", "DIV", "ADDI", //9-13
	      "MOD", //14
	      "AND", "NOT", "OR", "XOR", "ANDI", //15-19
	      "LDR", "STR", //20-21
	      "CONST", "HICONST", //22-23
	      "CMP", "CMPU", "CMPI", "CMPIU", //24-27
	      "SLL", "SRA", "SRL", //28-30
	      "JSRR", "JSR", //31-32
	      "JMPR", "JMP", //33-34
	      "TRAP", "RTI"};  //35-36


// decode_insn : This function takes as input an unsigned 16 bit pattern and decodes that into
// an INSN structure that breaks out the various instruction components.
INSN decode_insn (unsigned short int insn) {
	unsigned short int head = (insn >> 12); // last 4 bits

	if (head == 0) { //nop/br
		return decode_branch(insn);

	} else if (head == 1) { //arithmetic
		return decode_arithmetic(insn);

	} else if (head == 10) { // mod or shifts
		return decode_shift(insn);

	} else if (head == 5) { //logical
		return decode_logic(insn);

	} else if (head == 6) { //ldr
		return decode_load(insn);

	} else if (head == 7) { //str
		return decode_store(insn);

	} else if (head == 9) { //const
		return decode_const(insn);

	} else if (head == 13) { //hiconst
		return decode_hiconst(insn);

	} else if (head == 2) { //comparison
		return decode_comp(insn);

	} else if (head == 4) { //jsrr/jsr
		return decode_jsr(insn);

	} else if (head == 12) { //jmpr/jmp
		return decode_jump(insn);

	} else if (head == 15) { //trap
		return decode_trap(insn);

	} else if (head == 8) { //rti
		return decode_rti(insn);

	} 

	//not matched -> return illegal
	INSN instr;
	instr.type = ILLEGAL_INSN;
	return instr;

}


// Print a readable version of the instruction out to the specified file
/* Formatting: 
   "Type"
   " Rd," // if not null
   " Rs," // if not null
   " Rt" // if not null
   " #immediate" // if not null

   exceptions: 
   STR Rt, Rs, IMM6
   JSRR Rs (no comma after Rs)
   JMPR Rs (no comma after Rs)
*/

void print_insn (FILE *theFile, INSN theINSN) {

	fprintf(theFile, "%s", type_string[theINSN.type]);

	if (theINSN.type == STR) { //print " Rt, Rs, #immediate"
		fprintf(theFile, " %d,", theINSN.Rt);
		fprintf(theFile, " %d,", theINSN.Rs);
		fprintf(theFile, " #%d", theINSN.immediate);
	} else if (theINSN.type == JSRR || theINSN.type == JMPR) {
		fprintf(theFile, " %d", theINSN.Rs);
	} else { //general format
		if (theINSN.Rd != null) {
		fprintf(theFIle, " %d,", theINSN.Rd);
		}
		if (theINSN.Rs != null) {
			fprintf(theFIle, " %d,", theINSN.Rs);
		}
		if (theINSN.Rt != null) {
			fprintf(theFIle, " %d", theINSN.Rt);
		}
		if (theINSN.immediate != null) {
			fprintf(theFIle, " #%d", theINSN.immediate);
		}

	}

}


INSN decode_branch(unsigned short int insn) {
	INSN instr; 
	unsigned short int code = (insn >> 9); //3-digit code 
	if (code == 0) {
		instr.type = NOP;
		return instr;
	} else if (code == 1) {
		instr.type = BRp;
	} else if (code == 2) {
		instr.type = BRz;
	} else if (code == 3) {
		instr.type = BRzp;
	} else if (code == 4) {
		instr.type = BRn;
	} else if (code == 5) {
		instr.type = BRnp;
	} else if (code == 6) {
		instr.type = BRnz;
	} else if (code == 7) {
		instr.type = BRnzp;
	}

	short int num = insn & 0x1FF;
	int sign = (num >> 8) & 1; 
	if (sign == 1) {
		num = num | 0xFE00;
	}
	instr.immediate = num;

	return instr;
}

INSN decode_arithmetic(unsigned short int insn) {
	INSN instr; 
	if (((insn >> 5) & 1) == 1) { //immediate
		instr.type = ADDI;
		instr.Rd = (insn >> 9) & 7;
		instr.Rs = (insn >> 6) & 7;

		short int num = insn & 0x1F;
		int sign = (num >> 4) & 1; 
		if (sign == 1) {
			num = num | 0xFFE0;
		}
		instr.immediate = num;

	} else {
		int code = (insn >> 3) & 7;
		instr.Rd = (insn >> 9) & 7;
		instr.Rs = (insn >> 6) & 7;
		instr.Rt = insn & 7;

		if (code == 0) {
			instr.type = ADD;
		} else if (code == 1) {
			instr.type = MUL;
			
		} else if (code == 2) {
			instr.type = SUB;

		} else if (code == 3) {
			instr.type = DIV;
		}

	}
	return instr;
}

INSN decode_shift(unsigned short int insn) { //includes shift & mod
	INSN instr; 
	int code = (insn >> 4) & 3;
	instr.Rd = (insn >> 9) & 7; 
	instr.Rs = (insn >> 6) & 7;

	if (code == 0) {
		instr.type = SLL;
		instr.immediate = (insn & 0xF); 

	} else if (code == 1) {
		instr.type = SRA;
		instr.immediate = (insn & 0xF); 

	} else if (code == 2) {
		instr.type = SRL;
		instr.immediate = (insn & 0xF); 

	} else if (code == 3) {
		instr.type = MOD;
		Rt = (insn & 7);
	}
	return instr;

}

INSN decode_logic(unsigned short int insn) {
	if (((insn >> 5) & 1) == 1) { //immediate
		instr.type = ANDI;
		instr.Rd = (insn >> 9) & 7;
		instr.Rs = (insn >> 6) & 7;

		short int num = insn & 0x1F;
		int sign = (num >> 4) & 1; 
		if (sign == 1) {
			num = num | 0xFFE0;
		}
		instr.immediate = num;
	} else {
		int code = (insn >> 3) & 7;
		instr.Rd = (insn >> 9) & 7;
		instr.Rs = (insn >> 6) & 7;

		if (code == 0) {
			instr.type = AND;
			instr.Rt = insn & 7;

		} else if (code == 1) {
			instr.type = NOT;
			
		} else if (code == 2) {
			instr.type = OR;
			instr.Rt = insn & 7;

		} else if (code == 3) {
			instr.type = XOR;
			instr.Rt = insn & 7;
		}

	}
	return instr;

}

INSN decode_load(unsigned short int insn) {
	INSN instr; 
	instr.Rd = (insn >> 9) & 7;
	instr.Rs = (insn >> 6) & 7;
	short int num = insn & 0x3F;
	int sign = (num >> 5) & 1; 
	if (sign == 1) {
		num = num | 0xFFC0;
	}
	instr.immediate = num;
	return instr;

}

INSN decode_store(unsigned short int insn) {
	INSN instr; 
	instr.Rt = (insn >> 9) & 7;
	instr.Rs = (insn >> 6) & 7;
	short int num = insn & 0x3F;
	int sign = (num >> 5) & 1; 
	if (sign == 1) {
		num = num | 0xFFC0;
	}
	instr.immediate = num;
	return instr;	
}

INSN decode_const(unsigned short int insn) {
	INSN instr; 
	instr.Rd = (insn >> 9) & 7;
	short int num = insn & 0x1FF;
	int sign = (num >> 8) & 1; 
	if (sign == 1) {
		num = num | 0xFE00;
	}
	instr.immediate = num;
	return instr;
}

INSN decode_hiconst(unsigned short int insn) {
	INSN instr; 
	instr.Rd = (insn >> 9) & 7;
	instr.immediate = insn & 0xFF;
	return instr;
}

INSN decode_comp(unsigned short int insn) {
	INSN instr; 
	instr.Rs = (insn >> 9) & 7;
	int code = (insn >> 7) & 3;
	if (code == 0) {

	} else if (code == 1) {
		instr.type = CMP;
		instr.Rt = (insn & 7);

	} else if (code == 2) {
		instr.type = CMPU;
		instr.Rt = insn & 7;

	} else if (code == 3) {
		instr.type = CMPI;
		short int num = insn & 0x7F;
		int sign = (insn >> 6) & 1;
		if (sign == 1) {
			num = num | 0xFF80;
		}
		instr.immediate = num;


	} else if (code == 4) {
		instr.type = CMPIU;
		instr.immediate = insn & 0x7F;

	}
	return instr;

}

INSN decode_jsr(unsigned short int insn) {
	INSN instr; 
	if (((insn >> 11) & 1) == 0) { //jsrr
		instr.type = JSRR;
		instr.Rs = (insn >> 6) & 7;
	} else {
		instr.type = JSR;
		short int num = insn & 0x07FF;
		int sign = (insn >> 10) & 1;
		if (sign == 1) {
			num = num | 0xF800;
		}
		instr.immediate = num;
	}
	return instr;
}

INSN decode_jump(unsigned short int insn) {
	INSN instr; 
	if (((insn >> 11) & 1) == 0) { //jmpr
		instr.type = JMPR;
		instr.Rs = (insn >> 6) & 7;
	} else { //jmp
		instr.type = JMP;
		short int num = insn & 0x07FF;
		int sign = (insn >> 10) & 1;
		if (sign == 1) {
			num = num | 0xF800;
		}
		instr.immediate = num;
	}
	return instr;
}

INSN decode_trap(unsigned short int insn) {
	INSN instr; 
	instr.type = TRAP;
	instr.immediate = insn & 0xFF;
	return instr;
}

INSN decode_rti(unsigned short int insn) {
	INSN instr; 
	instr.type = RTI;
	return instr;
}



