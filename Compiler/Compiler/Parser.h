#pragma once
#include "stdafx.h"
#include "Quad.h"
using std::shared_ptr;
class Parser
{
public:
    Parser();
	Value* parseExpression();
	Value* parseTerm();
	Value* parseFactor();
	void parseStatement();
	void parseFunction();
	void parseConstDeclare();
	void parseVarDeclare();
	void parseFuncDeclare();
	void parseSwitch();
	void parseIf();
	void parseWhile();
	void parseScanf();
	void parsePrintf();



};

