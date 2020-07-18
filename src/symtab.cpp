#include <iostream>
#include <fstream>
#include"symtab.h"


unsigned int SymTabEntry::lastId = 0;

ostream &operator<<(ostream &output, const SymTabEntry &entry) {
	output << entry.section << " \t\t ";
	//if (entry.section.compare("rodata") != 0) output << "\t";
	output << hex << entry.value;
	output << " \t\t " << entry.binding << " \t\t " << dec << entry.id << " \t\t " << entry.size;
	return output;
}

SymTabEntry::SymTabEntry(unsigned int val, char bind, string sec) {
	id = lastId++;
	value = val;
	binding = bind;
	section = sec;
	size = 2;  //default for symbols that are not sections
}

void SymTabEntry::setBinding(char b) {
	binding = b;
}

void SymTabEntry::setSize(unsigned int s) {
	size = s;
}
