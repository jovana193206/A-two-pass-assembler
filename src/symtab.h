#ifndef SYMBTAB_H
#define SYMBTAB_H

#include <string>

using namespace std;

class SymTabEntry {

public:
	SymTabEntry(unsigned int val, char bind, string sec);

	static unsigned int lastId;

	//string name; the name is set as the key in a hash map that represents symbol table
	unsigned int value;       //Section offset or VM address
	char binding;       //l-local, g-global
	string section;     //text, data, rodata, bss
	unsigned int id;    //redni broj
	unsigned int size;  //in bytes 

	void setBinding(char b);
	void setSize(unsigned int s);

	friend ostream &operator<<(ostream &output, const SymTabEntry &entry); 


};



#endif
