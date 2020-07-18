#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "declarations.h"
#include <vector>
#include <string>

using namespace std;

class Assembler;


class InstructionHandler {
public:
	static int handle(bool firstCall, vector<string> tokens, Assembler* assembler);

	static Condition getCondition(string condStr);
	static bool isTwoOpInstr(string instr);
	static bool isSrcOpInstr(string instr);
	static bool isDstOpInstr(string instr);
	static bool isNoOpInstr(string instr);
	static int getInstrLength(AddressingMode dstAddrMode, AddressingMode srcAddrMode);
	static int extractRegNum(string operand); //For REGDIR_MODE, REGIND_POM, REGIND_SYMB
	static unsigned int getOperandBits(string operand, AddressingMode addrMode);
	static unsigned int processAdditionalBytes(string operand, AddressingMode addrMode, Assembler* assembler);

	static int twoOpHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
	static int srcOpHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
	static int dstOpHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
	static int iretHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
	static int retHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
	static int jmpHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
};


#endif
