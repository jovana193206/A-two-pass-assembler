#ifndef DIRECTIVE_H
#define DIRECTIVE_H

#include <vector>
#include <string>

using namespace std;

class Assembler;


class DirectiveHandler {
public:
	static int handle(bool firstCall, vector<string> tokens, Assembler* assembler);

	static int globalHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
	static int charWordLongHandler(bool firstCall, vector<string> tokens, Assembler* assembler, int size);
	static int alignHandler(bool firstCall, vector<string> tokens, Assembler* assembler);
	static int skipHandler(bool firstCall, vector<string> tokens, Assembler* assembler);

};



#endif

