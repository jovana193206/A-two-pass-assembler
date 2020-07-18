#ifndef DECLARATIONS_H
#define DECLARATIONS_H

enum Condition {
	EQ = 0, 
	NE = 1, 
	GT = 2, 
	AL = 3
};

enum Opcode {
	ADD = 0,
	SUB = 1,
	MUL = 2,
	DIV = 3,
	CMP = 4,
	AND = 5,
	OR = 6,
	NOT = 7,
	TEST = 8,
	PUSH = 9,
	POP = 10,
	CALL = 11,
	IRET = 12,
	MOV = 13,
	SHL = 14,
	SHR = 15,
	UNDEF = 16
};

enum AddressingCode {
	IMM_PSW = 0,
	REGDIR = 1,
	MEM = 2,
	REGIND = 3
};

enum AddressingMode {IMM, AMPERSAND, PSW, REGDIR_MODE, MEMDIR, ASTERISK, REGIND_POM, REGIND_SYMB, PCREL, ERR};




#endif
