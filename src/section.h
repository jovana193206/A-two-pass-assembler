#ifndef SECTION_H
#define SECTION_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;


class SectionEntry {
public:
	SectionEntry(unsigned int val, int width);

	unsigned int value;
	int hexWidth;
};

class Section {
public:
	Section(string sectionName);

	string name;    //.text, .data, .rodata, .bss
	vector<SectionEntry> data;

	void write(int width, unsigned int content = 0);

	friend ostream &operator<<(ostream &output, const Section &entry);

};



#endif
