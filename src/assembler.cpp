#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <iomanip>
#include "assembler.h"
#include "symtab.h"
#include "instruction.h"
#include "directive.h"
#include "section.h"
#include "relocation.h"

using namespace std;



unordered_map<string, Opcode> Assembler::instructions = {
	{ "add", ADD },
	{ "ADD", ADD },
	{ "sub", SUB },
	{ "SUB", SUB },
	{ "mul", MUL },
	{ "MUL", MUL },
	{ "div", DIV },
	{ "DIV", DIV },
	{ "cmp", CMP },
	{ "CMP", CMP },
	{ "and", AND },
	{ "AND", AND },
	{ "or", OR },
	{ "OR", OR },
	{ "not", NOT },
	{ "NOT", NOT },
	{ "test", TEST },
	{ "TEST", TEST },
	{ "push", PUSH },
	{ "PUSH", PUSH },
	{ "pop", POP },
	{ "POP", POP },
	{ "call", CALL },
	{ "CALL", CALL },
	{ "iret", IRET },
	{ "IRET", IRET },
	{ "mov", MOV },
	{ "MOV", MOV },
	{ "shl", SHL },
	{ "SHL", SHL },
	{ "shr", SHR },
	{ "SHR", SHR },
	{ "ret", POP },
	{ "RET", POP },
	{ "jmp", UNDEF },
	{ "JMP", UNDEF }
};

unordered_map<char, int> Assembler::hexToDec = {
	{ '0', 0 },
	{ '1', 1 },
	{ '2', 2 },
	{ '3', 3 },
	{ '4', 4 },
	{ '5', 5 },
	{ '6', 6 },
	{ '7', 7 },
	{ '8', 8 },
	{ '9', 9 },
	{ 'a', 10 },
	{ 'A', 10 },
	{ 'b', 11 },
	{ 'B', 11 },
	{ 'c', 12 },
	{ 'C', 12 },
	{ 'd', 13 },
	{ 'D', 13 },
	{ 'e', 14 },
	{ 'E', 14 },
	{ 'f', 15 },
	{ 'F', 15 }
};

Assembler::Assembler() {
	startAddr = 0;
	locationCounter = 0;
	currentSection = "UND";
	currentSectionOffset = 0;
	bssExists = false;

	symtab.insert({ " ", SymTabEntry(0, 'l', "UND") });
	symtab.find(" ")->second.setSize(0);
}


int Assembler::assemble(ifstream& inputFile, int start, ofstream& outputFile) {
	startAddr = start;
	try {
		firstCall(inputFile);

		secondCall();

		writeOutputFile(outputFile);
	}
	catch (exception& e) {
		cout << e.what() << endl;
		return 1;
	}
	return 0;
}

vector<string> Assembler::stringTokenizer(string s) {
	vector<string> tokens;
	if (s.empty()) return tokens;
	const char *str = s.c_str();
	while (((*str == ' ') || (*str == '\t') || (*str == '\r')) && (*str != '\0')) str++;
	if (*str == '\0') return tokens;
	str = s.c_str();
	while (0 != *str)
	{
		const char *begin = str;
		while (((*str == ' ') || (*str == '\t') || (*str == '\r')) && (*str != '\0')) {
			str++;   //skip all blancs before the start of a token
			begin++;
		}
		while ((*str != ' ') && (*str != ',') && (*str != '\t') && (*str != '\r') && (*str != '\0')) str++;
		if (*begin != 0) {
			string token = string(begin, str);
			tokens.push_back(token);
		}
		str++;
	}

	return tokens;
}

bool Assembler::isLabel(string token) {
	if(token.back() != ':') return false;
	string label = (token).substr(0, (token).size() - 1);
	return isSymbol(label);
}

bool Assembler::isSection(string token) {
	if (token.compare(".text") == 0) return true;
	if (token.compare(".data") == 0) return true;
	if (token.compare(".rodata") == 0) return true;
	if (token.compare(".bss") == 0) return true;
	return false;
}

bool Assembler::isInstruction(string token) {
	string instr = token.substr(0, token.size() - 2); //Strips the 2 last letters of an instruction - condition
	string cond = token.substr(token.size() - 2, token.size()); //Takes the condition out of the token
	if ((cond.compare("eq") != 0) && (cond.compare("EQ") != 0) && (cond.compare("ne") != 0) && (cond.compare("NE") != 0)
		&& (cond.compare("gt") != 0) && (cond.compare("GT") != 0) && (cond.compare("al") != 0)
		&& (cond.compare("AL") != 0)) return false;
	if ((instr.compare("add") == 0) || (instr.compare("ADD") == 0) || (instr.compare("sub") == 0) || (instr.compare("SUB") == 0)) return true;
	if ((instr.compare("mul") == 0) || (instr.compare("MUL") == 0) || (instr.compare("div") == 0) || (instr.compare("DIV") == 0)) return true;
	if ((instr.compare("cmp") == 0) || (instr.compare("CMP") == 0) || (instr.compare("and") == 0) || (instr.compare("AND") == 0)) return true;
	if ((instr.compare("or") == 0) || (instr.compare("OR") == 0) || (instr.compare("not") == 0) || (instr.compare("NOT") == 0)) return true;
	if ((instr.compare("test") == 0) || (instr.compare("TEST") == 0) || (instr.compare("mov") == 0) || (instr.compare("MOV") == 0)) return true;
	if ((instr.compare("shl") == 0) || (instr.compare("SHL") == 0) || (instr.compare("shr") == 0) || (instr.compare("SHR") == 0)) return true;
	if ((instr.compare("push") == 0) || (instr.compare("PUSH") == 0) || (instr.compare("call") == 0) || (instr.compare("CALL") == 0))
		return true;
	if ((instr.compare("pop") == 0) || (instr.compare("POP") == 0)) return true;
	if ((instr.compare("iret") == 0) || (instr.compare("IRET") == 0)) return true;
	if (instr.compare("ret") == 0 || instr.compare("RET") == 0) return true;
	if (instr.compare("jmp") == 0 || instr.compare("JMP") == 0) return true;
	return false;
}

bool Assembler::isDirective(string token) {
	if (token.compare(".global") == 0) return true;
	if (token.compare(".char") == 0) return true;
	if (token.compare(".word") == 0) return true;
	if (token.compare(".long") == 0) return true;
	if (token.compare(".align") == 0) return true;
	if (token.compare(".skip") == 0) return true;
	return false;
}

bool Assembler::is_DecNumber(const std::string& s) {
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

bool Assembler::is_HexNumber(const std::string& s) {
	std::string::const_iterator it = s.begin();
	if (!((*it == '0') && (*(it + 1) == 'x'))) return false;
	it += 2;
	while (it != s.end() && isxdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

bool Assembler::isSymbol(const std::string& s) {
	std::string::const_iterator it = s.begin();
	if (!(isalpha(*it) || (*it == '_'))) return false;
	it++;
	while (it != s.end() && isalnum(*it)) ++it;
	return !s.empty() && it == s.end();
}

bool Assembler::isReg(const std::string& s) {
	if (s.size() < 2) return false;
	if ((s.compare("pc") == 0) || (s.compare("PC") == 0) || (s.compare("sp") == 0) || (s.compare("SP") == 0))
		return true;
	std::string::const_iterator it = s.begin();
	if (((*it != 'r') || (*it != 'R')) && isdigit(*(++it))) {
		int i = (int)(*it) - '0';
		it++;
		return (i >= 0) && (i <= 7) && (it == s.end());
	}
	return false;
}

bool  Assembler::isRegIndPom(const std::string& s) {
	std::string::const_iterator it = s.begin();
	unsigned int pos1 = 0;
	while (it != s.end() && *it != '[') {
		it++;
		pos1++;
	}
	if (it == s.end()) return false;
	unsigned int pos2 = pos1;
	if (!Assembler::isReg(s.substr(0, pos1))) return false;
	while (it != s.end() && *it != ']') {
		it++;
		pos2++;
	}
	if (it == s.end()) return false;
	string pom = s.substr(pos1 + 1, pos2 - 3);
	return Assembler::is_DecNumber(pom) || Assembler::is_HexNumber(pom);
}

bool Assembler::isRegIndSymb(const std::string& s) {
	std::string::const_iterator it = s.begin();
	unsigned int pos1 = 0;
	while (it != s.end() && *it != '[') {
		it++;
		pos1++;
	}
	if (it == s.end()) return false;
	unsigned int pos2 = pos1;
	if (!Assembler::isReg(s.substr(0, pos1))) return false;
	while (it != s.end() && *it != ']') {
		it++;
		pos2++;
	}
	if (it == s.end()) return false;
	string pom = s.substr(pos1 + 1, pos2 - 3);
	return Assembler::isSymbol(pom);
}

AddressingMode Assembler::getAddrMode(string token) {
	if (token.empty()) return ERR;

	if (Assembler::isReg(token)) return REGDIR_MODE;

	if (Assembler::is_DecNumber(token) || Assembler::is_HexNumber(token)) return IMM;
	if ((token.compare("psw") == 0) || (token.compare("PSW") == 0)) return PSW;
	if (Assembler::isSymbol(token)) return MEMDIR;

	if ((token.at(0) == '&') && (Assembler::isSymbol(token.substr(1, token.size())))) return AMPERSAND;
	if ((token.at(0) == '$') && (Assembler::isSymbol(token.substr(1, token.size())))) return PCREL;
	if ((token.at(0) == '*') && 
		(Assembler::is_DecNumber(token.substr(1, token.size())) || Assembler::is_HexNumber(token.substr(1, token.size()))))
		return ASTERISK;

	if (Assembler::isRegIndPom(token)) return REGIND_POM;
	if (Assembler::isRegIndSymb(token)) return REGIND_SYMB;
	
	return ERR;
}

unsigned int Assembler::littleEndian(unsigned int value, int width) {
	unsigned int byte0 = 0;
	unsigned int byte1 = 0;
	unsigned int byte2 = 0;
	unsigned int byte3 = 0;
	unsigned int retVal = 0;
	switch (width) {
	case 1:
		retVal = value & 0xff;
		break;
	case 2:
		byte0 = value & 0xff;
		byte1 = (value & 0xff00) >> 8;
		retVal |= byte0 << 8;
		retVal |= byte1;
		break;
	case 4:
		byte0 = value & 0xff;
		byte1 = (value & 0xff00) >> 8;
		byte2 = (value & 0xff0000) >> 16;
		byte3 = value >> 24;
		retVal = byte0 << 24;
		retVal |= byte1 << 16;
		retVal |= byte2 << 8;
		retVal |= byte3;
	}
	
	return retVal;
}

int Assembler::convertToDec(string hex) {
	int bytesNum = 0;
	int mul = 1;
	for (int pos = (hex).size() - 1; pos >= 2; pos--) {
		bytesNum += mul * Assembler::hexToDec.find((hex).at(pos))->second;
		mul *= 16;
	}
	return bytesNum;
}

void Assembler::firstCall(ifstream& inputFile) {
	locationCounter = startAddr;
	currentSectionOffset = 0;
	string line;
	while (getline(inputFile, line)) {

		vector<string> tokens;
		tokens = stringTokenizer(line);
		if (tokens.size() == 0) continue;
		if (tokens[0].compare(".end") == 0) {
			if (currentSection.compare("UND") != 0) {
				string currentSectionToken = "." + currentSection;
				symtab.find(currentSectionToken)->second.setSize(currentSectionOffset);
			}
			break;
		}
		inputTokenizedLines.push_back(tokens);

		vector<string>::iterator it = tokens.begin();
		if (isLabel(*it)) {
			if (currentSection.compare("UND") == 0)
				throw runtime_error("Label must be in a section");
			string label = (*it).substr(0, (*it).size() - 1);
			symtab.insert({label, SymTabEntry(currentSectionOffset, 'l', currentSection)});
			it++;
			if (it != tokens.end()) {
				if (isInstruction(*it)) {
					if (currentSection.compare("text") != 0) throw runtime_error("Instruction found out of text section in line " + (locationCounter - startAddr));
					string instr = (*it).substr(0, (*it).size() - 2);
					
					int instrLength = InstructionHandler::handle(true, tokens, this);
					locationCounter += instrLength;
					currentSectionOffset += instrLength;
				}
				else if (isDirective(*it)) {
					int directiveLength = DirectiveHandler::handle(true, tokens, this);
					locationCounter += directiveLength;
					currentSectionOffset += directiveLength;
					}
				else throw runtime_error("Unsupported line format in line " + (locationCounter - startAddr));
			}
			continue;
		}
		if (isSection(*it)) {
			string sectionToken = *it;
			it++;
			if (it != tokens.end()) throw runtime_error("New line expected after section start in line" + (locationCounter - startAddr));
			//Set the size of the previous section
			if (currentSection.compare("UND") != 0) {
				string currentSectionToken = "." + currentSection;
				symtab.find(currentSectionToken)->second.setSize(currentSectionOffset);
			}
			//Check if the section already exists
			if ((sections.find(sectionToken) != sections.end()) || ((sectionToken.compare(".bss") == 0) && bssExists))
				throw runtime_error("Error: secion " + sectionToken + " found more than once");
			//Register new section
			currentSection = sectionToken.substr(1, sectionToken.size());  //Strips the . from the start of a sectionToken
			currentSectionOffset = 0;
			symtab.insert({sectionToken, SymTabEntry(locationCounter, 'l', currentSection) });
			if (sectionToken.compare(".bss") != 0) {
				sectionOrder.push(sectionToken);
				sections.insert({ sectionToken, Section(sectionToken) });
				//For every section (except .bss) that firstCall runs into an empty relocation section will be created
				relocations.insert({ { sectionToken, vector<RelocationEntry>() } });
			}
			else bssExists = true;
			continue;
		}
		if (isInstruction(*it)) {
			if (currentSection.compare("text") != 0) throw runtime_error("Instruction found out of text section in line " + (locationCounter - startAddr));
			int instrLength = InstructionHandler::handle(true, tokens, this);
			locationCounter += instrLength;
			currentSectionOffset += instrLength;
			continue;
		}
		if (isDirective(*it)) {
			int directiveLength = DirectiveHandler::handle(true, tokens, this);
			locationCounter += directiveLength;
			currentSectionOffset += directiveLength;
			continue;
		}
		throw runtime_error("Unsupported line format in line " + (locationCounter - startAddr));
	}
}

void Assembler::secondCall() {
	currentSection = "UND";
	locationCounter = startAddr;
	currentSectionOffset = 0;
	vector<vector<string> >::const_iterator linesIt = inputTokenizedLines.begin();
	//Iterates through inputTokenizedLines one line at the time
	for (; linesIt != inputTokenizedLines.end(); linesIt++) {
		vector<string> tokens = *linesIt;
		if (tokens.size() == 0) continue;
		if (tokens[0].compare(".end") == 0) break;
		vector<string>::iterator it = tokens.begin();
		if (isLabel(*it)) {
			it++;
			if (it == tokens.end()) continue;
		}
		if (isSection(*it)) {
			string sectionToken = *it;
			currentSection = sectionToken.substr(1, sectionToken.size());  //Strips the . from the start of a sectionToken
			currentSectionOffset = 0;
			continue;
		}
		if (isInstruction(*it)) {
			int instrLength = InstructionHandler::handle(false, tokens, this);
			locationCounter += instrLength;
			currentSectionOffset += instrLength;
			continue;
		}
		if (isDirective(*it)) {
			int directiveLength = DirectiveHandler::handle(false, tokens, this);
			locationCounter += directiveLength;
			currentSectionOffset += directiveLength;
			continue;
		}
	}
}

void Assembler::writeOutputFile(ofstream& outputFile) {
	//Write relocation sections in defined order
	for (unsigned int i = 0; i < sectionOrder.size(); i++) {
		string sectionName = sectionOrder.front();
		sectionOrder.pop();
		sectionOrder.push(sectionName);
		unordered_map<string, vector<RelocationEntry> >::const_iterator relocation = relocations.find(sectionName);
		if ((relocation != relocations.end()) && (!relocation->second.empty())) {
			outputFile << "#.rel" << sectionName << endl;
			outputFile << "#Ofset \t tip \t\t vr [" << sectionName << "]:" << endl;
			vector<RelocationEntry>::const_iterator it = relocation->second.begin();
			while (it != relocation->second.end()) {
				outputFile << *it << endl;
				it++;
			}
		}
	}
	//Write sections in defined order
	for (unsigned int i = 0; i < sectionOrder.size(); i++) {
		string sectionName = sectionOrder.front();
		sectionOrder.pop();
		sectionOrder.push(sectionName);
		unordered_map<string, Section>::const_iterator section = sections.find(sectionName);
		if (section != sections.end()) {
			outputFile << section->second << endl;
		}
	}
	//Write symbol table
	outputFile << "#tabela simbola" << endl;
	outputFile << "#ime \t\t sek \t\t vr. \t\t vid. \t\t r.b. \t\t size" << endl;
	unsigned int id = 0;
	for (unsigned int i = 0; i < symtab.size(); i++) {
		unordered_map<string, SymTabEntry>::const_iterator it = symtab.begin();
		while (it != symtab.end()) {
			if (it->second.id == id) {
				outputFile << it->first << " \t";
				if (it->first.compare(".rodata") != 0) outputFile << " \t";
				outputFile << it->second << endl;
			}
			it++;
		}
		id++;
	}
}
