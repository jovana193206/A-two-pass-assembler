#ifndef RELOCATION_H
#define RELOCATION_H

#include <string>

using namespace std;

class RelocationEntry {

public:

	RelocationEntry(unsigned int off, string t, unsigned int val);

	friend ostream &operator<<(ostream &output, const RelocationEntry &entry);

	unsigned int offset;    //from the beginning of the section where linker should make corrections
	string type;   //absolute_32, absolute_16, absolute_8 or pc_relative_16
	unsigned int value;     //the cardinal number of the symbol in symtab to which this relocation refers
};


#endif
