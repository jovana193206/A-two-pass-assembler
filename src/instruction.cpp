#include <stdexcept>
#include "instruction.h"
#include "assembler.h"
#include "symtab.h"
#include "relocation.h"
#include "section.h"


bool InstructionHandler::isTwoOpInstr(string instr) {
	if ((instr.compare("add") == 0) || (instr.compare("ADD") == 0) || (instr.compare("sub") == 0) || (instr.compare("SUB") == 0)) return true;
	if((instr.compare("mul") == 0) || (instr.compare("MUL") == 0) || (instr.compare("div") == 0) || (instr.compare("DIV") == 0)) return true;
	if ((instr.compare("cmp") == 0) || (instr.compare("CMP") == 0) || (instr.compare("and") == 0) || (instr.compare("AND") == 0)) return true;
	if ((instr.compare("or") == 0) || (instr.compare("OR") == 0) || (instr.compare("not") == 0) || (instr.compare("NOT") == 0)) return true;
	if ((instr.compare("test") == 0) || (instr.compare("TEST") == 0) || (instr.compare("mov") == 0) || (instr.compare("MOV") == 0)) return true;
	if ((instr.compare("shl") == 0) || (instr.compare("SHL") == 0) || (instr.compare("shr") == 0) || (instr.compare("SHR") == 0)) return true;
	return false;
}

bool InstructionHandler::isSrcOpInstr(string instr) {
	if ((instr.compare("push") == 0) || (instr.compare("PUSH") == 0) || (instr.compare("call") == 0) || (instr.compare("CALL") == 0))
		return true;
	return false;
}

bool InstructionHandler::isDstOpInstr(string instr) {
	if ((instr.compare("pop") == 0) || (instr.compare("POP") == 0)) return true;
	return false;
}

bool InstructionHandler::isNoOpInstr(string instr) {
	if ((instr.compare("iret") == 0) || (instr.compare("IRET") == 0)) return true;
	return false;
}


int InstructionHandler::handle(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!assembler->isInstruction(*it)) it++;
	//Extract the instruction and condition from the token
	string instr = (*it).substr(0, (*it).size() - 2);
	if (isTwoOpInstr(instr)) return twoOpHandler(firstCall, tokens, assembler);
	if (isSrcOpInstr(instr)) return srcOpHandler(firstCall, tokens, assembler);
	if (isDstOpInstr(instr)) return dstOpHandler(firstCall, tokens, assembler);
	if (isNoOpInstr(instr)) return iretHandler(firstCall, tokens, assembler);
	if (instr.compare("ret") == 0 || instr.compare("RET") == 0) return retHandler(firstCall, tokens, assembler);
	if (instr.compare("jmp") == 0 || instr.compare("JMP") == 0) return jmpHandler(firstCall, tokens, assembler);
	return -1;  //Never going to happen
}


Condition InstructionHandler::getCondition(string condStr) {
	Condition cond;
	if ((condStr.compare("eq") == 0) || (condStr.compare("EQ") == 0)) cond = EQ;
	else if ((condStr.compare("ne") == 0) || (condStr.compare("NE") == 0)) cond = NE;
	else if ((condStr.compare("gt") == 0) || (condStr.compare("GT") == 0)) cond = GT;
	else if ((condStr.compare("al") == 0) || (condStr.compare("AL") == 0)) cond = AL;
	return cond;
}

int InstructionHandler::getInstrLength(AddressingMode dstAddrMode, AddressingMode srcAddrMode) {
	//For srcOpHandler
	if (dstAddrMode == ERR) {
		if ((srcAddrMode == IMM) || (srcAddrMode == AMPERSAND) || (srcAddrMode == MEMDIR) || (srcAddrMode == ASTERISK) ||
			(srcAddrMode == REGIND_POM) || (srcAddrMode == REGIND_SYMB) || (srcAddrMode == PCREL))
			return 4;
		else return 2;
	}
	//For dstOpHandler
	if (srcAddrMode == ERR) {
		if ((dstAddrMode == MEMDIR) || (dstAddrMode == ASTERISK) || (dstAddrMode == REGIND_POM) || (dstAddrMode == REGIND_SYMB))
			return 4;
		else return 2;
	}
	//For twoOpHandler
	if ((dstAddrMode = MEMDIR) || (dstAddrMode == ASTERISK) || (dstAddrMode == REGIND_POM) || (dstAddrMode == REGIND_SYMB)
		|| (srcAddrMode == IMM) || (srcAddrMode == AMPERSAND) || (srcAddrMode == MEMDIR) || (srcAddrMode == ASTERISK) ||
		(srcAddrMode == REGIND_POM) || (srcAddrMode == REGIND_SYMB) || (srcAddrMode == PCREL)) return 4;
	else return 2;
}

int InstructionHandler::extractRegNum(string operand) {
	if (((operand.at(0) == 'p') && (operand.at(1) == 'c')) || ((operand.at(0) == 'P') && (operand.at(1) == 'C'))) 
		return 7;
	if (((operand.at(0) == 's') && (operand.at(1) == 'p')) || ((operand.at(0) == 'S') && (operand.at(1) == 'P'))) 
		return 6;
	return (int)operand.at(1) - '0';
}

unsigned int InstructionHandler::getOperandBits(string operand, AddressingMode addrMode) {
	unsigned int retVal = 0;
	int regNum = 0;
	switch (addrMode) {
	case IMM:
		retVal |= IMM_PSW << 3;
		break;
	case AMPERSAND:
		retVal |= IMM_PSW << 3;
		break;
	case PSW:
		retVal |= IMM_PSW << 3;
		retVal |= 7;
		break;
	case REGDIR_MODE:
		retVal |= REGDIR << 3;
		regNum = InstructionHandler::extractRegNum(operand);
		retVal |= regNum;
		break;
	case MEMDIR:
		retVal |= MEM << 3;
		break;
	case ASTERISK:
		retVal |= MEM << 3;
		break;
	case REGIND_POM:
		/*retVal |= REGIND << 3;
		int regNum = InstructionHandler::extractRegNum(operand);
		retVal |= regNum;
		break;*/
	case REGIND_SYMB:
		retVal |= REGIND << 3;
		regNum = InstructionHandler::extractRegNum(operand);
		retVal |= regNum;
		break;
	case PCREL:
		retVal |= IMM_PSW << 3;
		break;
	}
	return retVal;
}

unsigned int InstructionHandler::processAdditionalBytes(string operand, AddressingMode addrMode, Assembler* assembler) {
	unsigned int additionalBytes = 0;
	unordered_map<string, SymTabEntry>::const_iterator symtabIt;
	switch (addrMode) {
	case IMM:
		if (Assembler::is_DecNumber(operand))
			additionalBytes = Assembler::littleEndian(atoi((operand).c_str()), 2);
		else if(Assembler::is_HexNumber(operand))
			additionalBytes = Assembler::littleEndian(Assembler::convertToDec(operand), 2);
		break;
	case AMPERSAND:
		operand = operand.substr(1, operand.size());   //Strips the & before the symbol
		symtabIt = assembler->symtab.find(operand);
		if ((symtabIt == assembler->symtab.end()) || (symtabIt->second.binding == 'g')) {
			if (symtabIt == assembler->symtab.end()) {
				assembler->symtab.insert({ operand, SymTabEntry(0, 'g', "UND") });
				symtabIt = assembler->symtab.find(operand);
			}
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "absolute_16", symtabIt->second.id));
		}
		else {   //The symbol is local
			if (!Assembler::isSection(symtabIt->first)) {
				additionalBytes = Assembler::littleEndian(symtabIt->second.value, 2);
			}
			//Value of the symbol depends on the value of the section it is defined in so a relocation is needed
			int refferedSecId = assembler->symtab.find("." + symtabIt->second.section)->second.id;
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "absolute_16", refferedSecId));
		}
		break;
	case MEMDIR:
		symtabIt = assembler->symtab.find(operand);
		if ((symtabIt == assembler->symtab.end()) || (symtabIt->second.binding == 'g')) {
			if (symtabIt == assembler->symtab.end()) {
				assembler->symtab.insert({ operand, SymTabEntry(0, 'g', "UND") });
				symtabIt = assembler->symtab.find(operand);
			}
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "absolute_16", symtabIt->second.id));
		}
		else {   //The symbol is local
			if (!Assembler::isSection(symtabIt->first)) {
				additionalBytes = Assembler::littleEndian(symtabIt->second.value, 2);
			}
			//Value of the symbol depends on the value of the section it is defined in so a relocation is needed
			int refferedSecId = assembler->symtab.find("." + symtabIt->second.section)->second.id;
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "absolute_16", refferedSecId));
		}
		break;
	case ASTERISK:
		operand = operand.substr(1, operand.size()); //Strips the * from the beginning of the operand string, leaving just the number
		if (Assembler::is_DecNumber(operand))
			additionalBytes = Assembler::littleEndian(atoi((operand).c_str()), 2);
		else if (Assembler::is_HexNumber(operand)) {
			additionalBytes = Assembler::littleEndian(Assembler::convertToDec(operand), 2);
		}
		break;
	case REGIND_POM:
		operand = operand.substr(3, operand.size() - 4);
		if (Assembler::is_DecNumber(operand))
			additionalBytes = Assembler::littleEndian(atoi(operand.c_str()), 2);
		else if (Assembler::is_HexNumber(operand))
			additionalBytes = Assembler::littleEndian(Assembler::convertToDec(operand), 2);
		break;
	case REGIND_SYMB:
		operand = operand.substr(3, operand.size() - 4);
		symtabIt = assembler->symtab.find(operand);
		if ((symtabIt == assembler->symtab.end()) || (symtabIt->second.binding == 'g')) {
			if (symtabIt == assembler->symtab.end()) {
				assembler->symtab.insert({ operand, SymTabEntry(0, 'g', "UND") });
				symtabIt = assembler->symtab.find(operand);
			}
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "absolute_16", symtabIt->second.id));
		}
		else {   //The symbol is local
			if (!Assembler::isSection(symtabIt->first)) {
				additionalBytes = Assembler::littleEndian(symtabIt->second.value, 2);
			}
			//Value of the symbol depends on the value of the section it is defined in so a relocation is needed
			int refferedSecId = assembler->symtab.find("." + symtabIt->second.section)->second.id;
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "absolute_16", refferedSecId));
		}
		break;
	case PCREL:
		operand = operand.substr(1, operand.size());   //Strips the $ before the symbol
		symtabIt = assembler->symtab.find(operand);
		if ((symtabIt == assembler->symtab.end()) || (symtabIt->second.binding == 'g')) {
			if (symtabIt == assembler->symtab.end()) {
				assembler->symtab.insert({ operand, SymTabEntry(0, 'g', "UND") });
				symtabIt = assembler->symtab.find(operand);
			}
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "pc_relative_16", symtabIt->second.id));
			additionalBytes = Assembler::littleEndian(-2, 2);
		}
		else {   //The symbol is local
			if (!Assembler::isSection(symtabIt->first)) {
				additionalBytes = Assembler::littleEndian(symtabIt->second.value - 2, 2);
			}
			else additionalBytes = Assembler::littleEndian(-2, 2);
			//Value of the symbol depends on the value of the section it is defined in so a relocation is needed
			int refferedSecId = assembler->symtab.find("." + symtabIt->second.section)->second.id;
			assembler->relocations.find("." + assembler->currentSection)->second.push_back(RelocationEntry(assembler->currentSectionOffset + 2, "pc_relative_16", refferedSecId));
		}
		break;
	}
	return additionalBytes;
}


int InstructionHandler::twoOpHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!assembler->isInstruction(*it)) it++;
	//Extract the instruction and condition from the token
	string instr = (*it).substr(0, (*it).size() - 2);
	string condStr = (*it).substr((*it).size() - 2, (*it).size());
	Condition cond = InstructionHandler::getCondition(condStr);

	it++;
	if (firstCall)
		if (it == tokens.end()) throw runtime_error("Operand expected after instruction " + instr);
	AddressingMode dstAddrMode = Assembler::getAddrMode(*it);
	string dst = *it;
	if (firstCall) {
		if (dstAddrMode == ERR)
			throw runtime_error("Unsupported operand format for destination operand in instruction " + instr);
		if ((dstAddrMode == IMM) || (dstAddrMode == AMPERSAND) || (dstAddrMode == PCREL))
			throw runtime_error("Immediate addressing is not allowed for destination operand");
	}
	it++;
	if (firstCall)
		if (it == tokens.end()) throw runtime_error("Second operand expected after instruction " + instr);
	AddressingMode srcAddrMode = Assembler::getAddrMode(*it);
	string src = *it;
	if (firstCall) {
		if (srcAddrMode == ERR)
			throw runtime_error("Unsupported operand format for source operand in instruction " + instr);
		if (!((dstAddrMode == PSW) || (dstAddrMode == REGDIR_MODE)) && !((srcAddrMode == PSW) || (srcAddrMode == REGDIR_MODE)))
			throw runtime_error("Error: both operands in instruction " + instr + " need the extra 2 bytes of the instruction");
		it++;
		if (it != tokens.end()) throw runtime_error("Error: more than two operands found after instruction " + instr);
	}
	int instrLength = InstructionHandler::getInstrLength(dstAddrMode, srcAddrMode);

	if (!firstCall) {     //Second call
		unsigned int instrBytes = 0;
		instrBytes |= cond << 14;
		//Insert the opCode in the instruction
		instrBytes |= (Assembler::instructions.find(instr)->second) << 10;
		//Insert the destination operand in the instruction
		instrBytes |= (InstructionHandler::getOperandBits(dst, dstAddrMode)) << 5;
		//Insert the source operand in the instruction
		instrBytes |= (InstructionHandler::getOperandBits(src, srcAddrMode));
		//If dst operand requires the extra two bytes
		if ((dstAddrMode != PSW) && (dstAddrMode != REGDIR_MODE)) {
			instrBytes <<= 16;
			instrBytes |= InstructionHandler::processAdditionalBytes(dst, dstAddrMode, assembler);
		}
		//If src operand requires the extra two bytes
		else if ((srcAddrMode != PSW) && (srcAddrMode != REGDIR_MODE)) {
			instrBytes <<= 16;
			instrBytes |= InstructionHandler::processAdditionalBytes(src, srcAddrMode, assembler);
		}
		assembler->sections.find("." + assembler->currentSection)->second.write(instrLength * 2, instrBytes);
	}

	return instrLength;
}

int InstructionHandler::srcOpHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!assembler->isInstruction(*it)) it++;
	//Extract the instruction and condition from the token
	string instr = (*it).substr(0, (*it).size() - 2);
	string condStr = (*it).substr((*it).size() - 2, (*it).size());
	Condition cond = InstructionHandler::getCondition(condStr);

	it++;
	if (firstCall)
		if (it == tokens.end()) throw runtime_error("Operand expected after instruction " + instr);
	AddressingMode srcAddrMode = Assembler::getAddrMode(*it);
	string src = *it;
	if (firstCall) {
		if (srcAddrMode == ERR)
			throw runtime_error("Unsupported operand format for source operand in instruction " + instr);
		it++;
		if (it != tokens.end()) throw runtime_error("Error: more than one operand found after instruction " + instr);
	}
	int instrLength = InstructionHandler::getInstrLength(ERR, srcAddrMode);

	if (!firstCall) {     //Second call
		unsigned int instrBytes = 0;
		instrBytes |= cond << 14;
		//Insert the opCode in the instruction
		instrBytes |= (Assembler::instructions.find(instr)->second) << 10;
		//Insert the source operand in the instruction
		instrBytes |= (InstructionHandler::getOperandBits(src, srcAddrMode));
		//If src operand requires the extra two bytes
		if ((srcAddrMode != PSW) && (srcAddrMode != REGDIR_MODE)) {
			instrBytes <<= 16;
			instrBytes |= InstructionHandler::processAdditionalBytes(src, srcAddrMode, assembler);
		}
		assembler->sections.find("." + assembler->currentSection)->second.write(instrLength * 2, instrBytes);
	}

	return instrLength;
}

int InstructionHandler::dstOpHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!assembler->isInstruction(*it)) it++;
	string instr = (*it).substr(0, (*it).size() - 2);
	string condStr = (*it).substr((*it).size() - 2, (*it).size());
	Condition cond = InstructionHandler::getCondition(condStr);

	it++;
	if (firstCall)
		if (it == tokens.end()) throw runtime_error("Operand expected after instruction " + instr);
	AddressingMode dstAddrMode = Assembler::getAddrMode(*it);
	string dst = *it;
	if (firstCall) {
		if (dstAddrMode == ERR)
			throw runtime_error("Unsupported operand format for destination operand in instruction " + instr);
		if ((dstAddrMode == IMM) || (dstAddrMode == AMPERSAND) || (dstAddrMode == PCREL))
			throw runtime_error("Immediate addressing is not allowed for destination operand");
		it++;
		if (it != tokens.end()) throw runtime_error("Error: more than one operand found after instruction " + instr);
	}
	int instrLength = InstructionHandler::getInstrLength(dstAddrMode, ERR);

	if (!firstCall) {     //Second call
		unsigned int instrBytes = 0;
		instrBytes |= cond << 14;
		//Insert the opCode in the instruction
		instrBytes |= (Assembler::instructions.find(instr)->second) << 10;
		//Insert the destination operand in the instruction
		instrBytes |= (InstructionHandler::getOperandBits(dst, dstAddrMode)) << 5;
		//If dst operand requires the extra two bytes
		if ((dstAddrMode != PSW) && (dstAddrMode != REGDIR_MODE)) {
			instrBytes <<= 16;
			instrBytes |= InstructionHandler::processAdditionalBytes(dst, dstAddrMode, assembler);
		}
		assembler->sections.find("." + assembler->currentSection)->second.write(instrLength * 2, instrBytes);
	}

	return instrLength;
}

int InstructionHandler::iretHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!assembler->isInstruction(*it)) it++;
	string instr = (*it).substr(0, (*it).size() - 2);
	string condStr = (*it).substr((*it).size() - 2, (*it).size());
	Condition cond = InstructionHandler::getCondition(condStr);
	int instrLength = 2;

	if (firstCall) {
		it++;
		if (it != tokens.end()) throw runtime_error("Error: instruction " + instr + " should not have any operands.");
	}

	else {     //Second call
		unsigned int instrBytes = 0;
		instrBytes |= cond << 14;
		//Insert the opCode in the instruction
		instrBytes |= (Assembler::instructions.find(instr)->second) << 10;
		assembler->sections.find("." + assembler->currentSection)->second.write(instrLength * 2, instrBytes);
	}

	return instrLength;  // ret instruction does not have any operands and does not need the extra 2 bytes
}

int InstructionHandler::retHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!assembler->isInstruction(*it)) it++;
	string instr = (*it).substr(0, (*it).size() - 2);
	string condStr = (*it).substr((*it).size() - 2, (*it).size());
	Condition cond = InstructionHandler::getCondition(condStr);
	int instrLength = 2;

	if (firstCall) {
		it++;
		if (it != tokens.end()) throw runtime_error("Error: instruction " + instr + " should not have any operands.");

	}

	else {     //Second call
		unsigned int instrBytes = 0;
		instrBytes |= cond << 14;
		//Insert the opCode in the instruction
		instrBytes |= (Assembler::instructions.find(instr)->second) << 10;
		//Insert dst in the instruction
		instrBytes |= REGDIR << 8;
		instrBytes |= 7 << 5;
		assembler->sections.find("." + assembler->currentSection)->second.write(instrLength * 2, instrBytes);
	}

	return instrLength;  // ret is always interpreted like pop pc which is always 2 bytes long (regdir addressing mode)
}

int InstructionHandler::jmpHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!assembler->isInstruction(*it)) it++;
	string instr = (*it).substr(0, (*it).size() - 2);
	string condStr = (*it).substr((*it).size() - 2, (*it).size());
	Condition cond = InstructionHandler::getCondition(condStr);

	it++;
	if (firstCall)
		if (it == tokens.end()) throw runtime_error("Operand expected after instruction " + instr);
	AddressingMode srcAddrMode = Assembler::getAddrMode(*it);
	string src = *it;
	if (firstCall) {
		if (srcAddrMode == ERR)
			throw runtime_error("Unsupported operand format for source operand in instruction " + instr);
		it++;
		if (it != tokens.end()) throw runtime_error("Error: more than one operand found after instruction " + instr);
	}
	int instrLength = InstructionHandler::getInstrLength(ERR, srcAddrMode);

	if (!firstCall) {     //Second call
		string src = *it;
		unsigned int instrBytes = 0;
		unsigned int additionalBytes = 0;
		instrBytes |= cond << 14;
		//The dst operand is always regdir pc 
		instrBytes |= REGDIR << 8;
		instrBytes |= 7 << 5;
		if (srcAddrMode == PCREL) {
			//In this case the instruction is interpreted as add pc, offset
			instrBytes |= ADD << 10;
		}
		else {
			//In this case the instruction is interpreted as mov pc, operand
			instrBytes |= MOV << 10;
		}
		//Insert the source operand in the instruction
		instrBytes |= (InstructionHandler::getOperandBits(src, srcAddrMode));
		//If src operand requires the extra two bytes
		if ((srcAddrMode != PSW) && (srcAddrMode != REGDIR_MODE)) {
			instrBytes <<= 16;
			instrBytes |= InstructionHandler::processAdditionalBytes(src, srcAddrMode, assembler);
		}
		assembler->sections.find("." + assembler->currentSection)->second.write(instrLength * 2, instrBytes);
	}

	return instrLength;
}
