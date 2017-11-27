#pragma once
#include "stdafx.h"
#include "BasicBlock.h"
using std::unordered_map;
using std::vector;
using std::string;
enum Type
{
	T_INT,
	T_CHAR,
	T_VOID,
	T_STRING
};
class TableElement
{
public:


	enum Kind
	{
		VAR,
		CONST,
		PARA,
		ARRAY
	};


	string name;
	Type type;
	Kind kind;
	int value;


};

class SymbolTable
{
public:
	unordered_map<string, int> ident2index;
	vector<TableElement> symbols;
	SymbolTable();
	~SymbolTable();
	void insert(TableElement& tableElement);
	const TableElement* lookup(const string& name);
};

class Function
{
public:
	const string name;
	SymbolTable symbolTable;
	Type type;
	list<BasicBlock*> body;
	void addStatement(Quad *quad);
	void insert(TableElement& tableElement);
	const TableElement* lookup(const string& name);
};



