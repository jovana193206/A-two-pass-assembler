#include <stdexcept>
#include "directive.h"
#include "assembler.h"
#include "declarations.h"
#include "symtab.h"
#include "section.h"
#include "relocation.h"


int DirectiveHandler::handle(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!Assembler::isDirective(*it)) it++;
	string directive = *it;
	if (directive.compare(".global") == 0) return globalHandler(firstCall, tokens, assembler);
	if (directive.compare(".char") == 0) return charWordLongHandler(firstCall, tokens, assembler, 1);
	if (directive.compare(".word") == 0) return charWordLongHandler(firstCall, tokens, assembler, 2);
	if (directive.compare(".long") == 0) return charWordLongHandler(firstCall, tokens, assembler, 4);
	if (directive.compare(".align") == 0) return alignHandler(firstCall, tokens, assembler);
	if (directive.compare(".skip") == 0) return skipHandler(firstCall, tokens, assembler);
	return -1;   //Never going to happen
}

int DirectiveHandler::globalHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	if (firstCall) return 0;
	vector<string>::iterator it = tokens.begin();
	while (!Assembler::isDirective(*it)) it++;
	it++;
	while (it != tokens.end()) {
		if (!Assembler::isSymbol(*it))
			throw runtime_error("Unsupported symbol format " + *it);
		unordered_map<string, SymTabEntry>::iterator symtabIt = assembler->symtab.find(*it);
		if (symtabIt == assembler->symtab.end()) {
			//extern simbol koji se uvozi, nije definisan u ovom fajlu
			assembler->symtab.insert({ *it, SymTabEntry(0, 'g', "UND") });
		}
		else symtabIt->second.setBinding('g');
		it++;
	}
	return 0;
}

int DirectiveHandler::charWordLongHandler(bool firstCall, vector<string> tokens, Assembler* assembler, int size) {
	vector<string>::iterator it = tokens.begin();
	while (!Assembler::isDirective(*it)) it++;
	string directive = *it;
	int symNum = 0;  //Number of tokens after directive .char / .word / .long
	if (firstCall) {
		if (assembler->currentSection.compare("UND") == 0)
			throw runtime_error("Directive " + *it + " must be in a section");
		it++; //Move the iterator to the first token after directive .char / .word / .long
		while (it != tokens.end()) {
			//Only constants and symbols are supported as an initializator
			if (!(Assembler::is_DecNumber(*it) || Assembler::is_HexNumber(*it) || Assembler::isSymbol(*it)))
				throw runtime_error("Unsupported initializator format in directive " + directive);
			symNum++;
			it++;
		}
		return symNum * size;
	}
	//Second call
	string sectionName = "." + assembler->currentSection;
	it++;
	if ((sectionName.compare(".bss") == 0)) {
		if (it != tokens.end()) throw runtime_error("Initializators not allowed in .bss section");
		return size;
	}
	int offset = 0;
	while (it != tokens.end()) {
		int initializator = 0;
		symNum++;
		//If the initializator is a constant
		if (Assembler::is_DecNumber(*it)) {
			initializator = atoi((*it).c_str());
			initializator = Assembler::littleEndian(initializator, size);
			assembler->sections.find(sectionName)->second.write(size * 2, initializator);
			it++;
			offset++;
			continue;
		}
		if (Assembler::is_HexNumber(*it)) {
			initializator = Assembler::convertToDec(*it);
			initializator = Assembler::littleEndian(initializator, size);
			assembler->sections.find(sectionName)->second.write(size * 2, initializator);
			it++;
			offset++;
			continue;
		}
		//Else the initializator must be a symbol
		if (Assembler::isSymbol(*it)) {
			string relocationType;
			if (size == 1) relocationType = "absolute_8";
			else if (size == 2) relocationType = "absolute_16";
			else relocationType = "absolute_32";
			unordered_map<string, SymTabEntry>::const_iterator symtabIt = assembler->symtab.find(*it);
			if ((symtabIt == assembler->symtab.end()) || (symtabIt->second.binding == 'g')) {
				if (symtabIt == assembler->symtab.end()) {
					assembler->symtab.insert({ *it, SymTabEntry(0, 'g', "UND") });
					symtabIt = assembler->symtab.find(*it);
				}
				assembler->sections.find(sectionName)->second.write(size * 2);
				assembler->relocations.find(sectionName)->second.push_back(RelocationEntry(assembler->currentSectionOffset + offset * size, relocationType, symtabIt->second.id));
			}
			else {   //The symbol is local
				if (Assembler::isSection(symtabIt->first))
					assembler->sections.find(sectionName)->second.write(size * 2);
				else {
					unsigned int initializator = Assembler::littleEndian(symtabIt->second.value, size);
					assembler->sections.find(sectionName)->second.write(size * 2, initializator);
				}
				//Value of the symbol depends on the value of the section it is defined in so a relocation is needed
				int refferedSecId = assembler->symtab.find("." + symtabIt->second.section)->second.id;
				assembler->relocations.find(sectionName)->second.push_back(RelocationEntry(assembler->currentSectionOffset, relocationType, refferedSecId));
			}
		}
		it++;
		offset++;
	}
	return symNum * size;
}

int DirectiveHandler::alignHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!Assembler::isDirective(*it)) it++;
	if (firstCall && (assembler->currentSection.compare("UND") == 0))
		throw runtime_error("Directive " + *it + " must be in a section");
	int bytesNum = 0;
	it++;
	if (it != tokens.end()) {
		int param = 1;
		if (Assembler::is_DecNumber(*it)) param = atoi((*it).c_str());
		else if (Assembler::is_HexNumber(*it)) {
			param = Assembler::convertToDec(*it);
		}
		if ((assembler->currentSectionOffset % param) == 0) return 0;
		bytesNum = assembler->currentSectionOffset % param;
		if (!firstCall && (assembler->currentSection.compare("bss") != 0)) {  //Second call
			string sectionName = "." + assembler->currentSection;
			assembler->sections.find(sectionName)->second.write(bytesNum * 2);
		}
		it++;
		if (it != tokens.end()) throw runtime_error("Too many parameters found after .align");
	}

	return bytesNum;
}

int DirectiveHandler::skipHandler(bool firstCall, vector<string> tokens, Assembler* assembler) {
	vector<string>::iterator it = tokens.begin();
	while (!Assembler::isDirective(*it)) it++;
	string directive = *it;
	int bytesNum = 0;
	it++;
	if (it != tokens.end()) {
		if (Assembler::is_DecNumber(*it)) bytesNum = atoi((*it).c_str());
		else if (Assembler::is_HexNumber(*it)) {
			bytesNum = Assembler::convertToDec(*it);
		}
		//Report errors in first call
		else if (firstCall) throw runtime_error("Unsupported format in  " + directive);
		if (firstCall && bytesNum < 0) throw runtime_error("Negative parameter not allowed in  " + directive);
		it++;
		if (firstCall && (it != tokens.end())) throw runtime_error("Too many parameters found in directive " + directive);
		//Second call
		if (!firstCall && (assembler->currentSection.compare("bss") != 0)) {
			string sectionName = "." + assembler->currentSection;
			assembler->sections.find(sectionName)->second.write(bytesNum * 2);
		}
	}
	//If there is no parameter 0 is default
	return bytesNum;
}
