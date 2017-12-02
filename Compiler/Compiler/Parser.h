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
	Tokenizer &tokenizer;
	IRBuilder &builder;
	ErrorHandler &error;

	Token token;
	Type type;
	int arraySize;
	string identifier;

	bool haveReadFunction;
	bool haveReadMain;

	unordered_set<Token> stmtBeginSet;
	unordered_set<Token> cmpSet;
	unordered_set<Token> typeSet;

	void skipToToken(Token tk);
	void skipToToken(Token t1, Token t2);
	void skipToToken(unordered_set<Token> &skipSet);
	Value *parseExpression();
	Value *parseTerm();
	Value *parseFactor();
	void parseStatement();
	void parseStatements();
	void parseCompStatement();
	void parseFunction();
	void parseConstDeclare();
	void parseVarDeclare();
	void parseVarAndFuncDeclare();
	void parseSwitch();
	void parseIf();
	void parseWhile();
	void parseScanf();
	void parsePrintf();
	void parseProgram();
	void parseReturn();
	bool match(Token tk)
	{
		return (token = tokenizer.nextToken()) == tk;
	}
	Type inferType(Type a, Type b)
	{
		if (a == b)
			return a;
		else
			return T_INT;
	}

  public:
	Parser(Tokenizer &tokenizer, IRBuilder &builder, ErrorHandler &errorHandler) : tokenizer(tokenizer), builder(builder), error(errorHandler)
	{
		haveReadFunction = haveReadMain = false;

		// initialize statement begin token set
		stmtBeginSet.insert(IF);
		stmtBeginSet.insert(WHILE);
		stmtBeginSet.insert(lBRACE);
		stmtBeginSet.insert(IDENT);
		stmtBeginSet.insert(SCANF);
		stmtBeginSet.insert(PRINTF);
		stmtBeginSet.insert(SEMICOLON);
		stmtBeginSet.insert(SWITCH);
		stmtBeginSet.insert(RTN);

		// initialize compare token set
		cmpSet.insert(EQU);
		cmpSet.insert(GRT);
		cmpSet.insert(GEQ);
		cmpSet.insert(LESS);
		cmpSet.insert(LEQ);
		cmpSet.insert(NEQ);

		// initialize type token set
		typeSet.insert(INTSYM);
		typeSet.insert(CHARSYM);
		typeSet.insert(VOID);
	}
	void startParse();
};
