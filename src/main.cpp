#include <iostream>
#include <fstream>
#include <string>
#include "assembler.h"

using namespace std;

int main(int argc, char ** argv) 
{
	if (argc < 4)
	{
		cout << "This program must have 4 arguments: program name, input file, start address, output file." << endl;
		return 1;
	}

	int startAddr = atoi(argv[2]);
	if (!((startAddr > 0) && (startAddr < 0x10000))) 
	{
		cout << "Invalid start address" << endl;
		return 1;
	}

	ifstream inputFile(argv[1]);
	if (!inputFile.is_open())
	{
		cout << "Error openning input file" << endl;
		return 1;
	}

	ofstream outputFile(argv[3]);
	if (!outputFile.is_open())
	{
		cout << "Error openning output file" << endl;
		return 1;
	}

	Assembler* assembler = new Assembler();

	int retval = assembler->assemble(inputFile, startAddr, outputFile);
	delete assembler;

	if (retval != 0) return 1;

	return 0;
}

