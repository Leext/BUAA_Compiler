#pragma once
#include "stdafx.h"
#include "SymbolTable.h"
#include "BasicBlock.h"
#include "Quad.h"

class IRBuilder
{
public:
	IRBuilder();
	~IRBuilder();
	bool createFunc();
	bool createGlobalVarDeclare(Type type, string& name, int size = 0);
	bool createGlobalConstDecalre();
	bool createVarDeclare();
	bool createConstDeclare();
	void addStatement(Quad * quad);
	int addString(const string& str);
	const TableElement* lookupGlobal(const string& name);
	const TableElement* lookupLocal(const string& name);
	const Function* lookupFunc(const string& name);
	const string& lookupStr(int index);


protected:
	SymbolTable globalSymbolTable;
	SymbolTable* localSymbolTable;
	vector<Function> functions;
	vector<string> strTable;

};

