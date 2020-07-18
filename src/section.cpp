#include <iostream>
#include <fstream>
#include <iomanip>
#include "section.h"

using namespace std;


SectionEntry::SectionEntry(unsigned int val, int width) {
	value = val;
	hexWidth = width;
}

Section::Section(string sectionName) {
	name = sectionName;
}

ostream &operator<<(ostream &output, const Section &entry) {
	output << "#" << entry.name << endl;
	vector<SectionEntry>::const_iterator it = entry.data.begin();
	while (it != entry.data.end()) {
		output << setfill('0') << setw(it->hexWidth) << hex << it->value;
		it++;
		if (it != entry.data.end()) output << " ";
	}
	return output;
}

void Section::write(int width, unsigned int content) {
	data.push_back(SectionEntry(content, width));
}
