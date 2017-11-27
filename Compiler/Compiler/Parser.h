#pragma once
#include "stdafx.h"
#include "Tokenizer.h"
#include "IRBuilder.h"
#include "SymbolTable.h"
#include "Quad.h"
#include "error.h"
#define __TEST
using std::shared_ptr;
using std::unordered_set;
class Parser
{
#ifdef __TEST
	friend class tester;
#endif // __TEST

protected:
	Tokenizer& tokenizer;
	IRBuilder& builder;
	ErrorHandler& error;
	Token token;
	Type type;
	int arraySize;
	string identifier;
	void skipToToken(Token tk);
	void skipToToken(unordered_set<Token>& skipSet);
	Value* parseExpression();
	Value* parseTerm();
	Value* parseFactor();
	void parseStatement();
	void parseFunction();
	void parseConstDeclare();
	void parseVarDeclare(bool global);
	void parseVarAndFuncDeclare();
	void parseSwitch();
	void parseIf();
	void parseWhile();
	void parseScanf();
	void parsePrintf();
	void parseProgram();

public:
	Parser(Tokenizer& tokenizer, IRBuilder& builder, ErrorHandler& errorHandler) : tokenizer(tokenizer), builder(builder), error(errorHandler) {}
	void startParse();
};

