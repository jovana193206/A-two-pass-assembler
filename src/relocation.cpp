#include <iostream>
#include <fstream>
#include <iomanip>
#include "relocation.h"


using namespace std;


RelocationEntry::RelocationEntry(unsigned int off, string t, unsigned int val) {
	offset = off;
	type = t;
	value = val;
}

ostream &operator<<(ostream &output, const RelocationEntry &entry) {
	output << setfill('0') << setw(4) << hex << entry.offset;
	output << " \t " << entry.type << " \t";
	if (entry.type.compare("pc_relative_16") != 0) output << " \t";
	output << dec << entry.value;
	return output;
}
