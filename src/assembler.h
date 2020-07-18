#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include "declarations.h"
#include "section.h"
#include "instruction.h"
#include "relocation.h"
#include "symtab.h"
#include "directive.h"

using namespace std;


class Assembler {


public:
	Assembler();

	int assemble(ifstream&, int, ofstream&);
	void firstCall(ifstream&);
	void secondCall();
	void writeOutputFile(ofstream&);

	static vector<string> stringTokenizer(string str);
	static bool isLabel(string);
	static bool isSection(string);
	static bool isInstruction(string);
	static bool isDirective(string);
	static bool is_DecNumber(const std::string& s);
	static bool is_HexNumber(const std::string& s);
	static bool isSymbol(const std::string& s);
	static bool isReg(const std::string& s);
	static bool isRegIndPom(const std::string& s);
	static bool isRegIndSymb(const std::string& s);
	static AddressingMode getAddrMode(string token);
	static unsigned int littleEndian(unsigned int value, int width);
	static int convertToDec(string hex);

	int startAddr;
	int locationCounter;
	string currentSection;
	int currentSectionOffset;
	bool bssExists;

	unordered_map<string, SymTabEntry> symtab;  
	vector<vector<string> > inputTokenizedLines;
	unordered_map<string, vector<RelocationEntry> > relocations;  // .rel.text, .rel.data, ...
	unordered_map<string, DirectiveHandler*> directives;
	queue<string> sectionOrder;
	unordered_map<string, Section> sections;

	static unordered_map<string, Opcode> instructions;
	static unordered_map<char, int> hexToDec;

	
};




#endif
