#pragma once
#include "stdafx.h"
#include "BasicBlock.h"
using std::unordered_map;
using std::vector;
using std::string;
class TableElement
{
public:
	enum Type
	{
		VARINT,
		CHAR,
		VOID,
		STRING
	};

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
	TableElement::Type type;
	list<BasicBlock*> body;
	void addStatement(Quad *quad);
	void insert(TableElement& tableElement);
	const TableElement* lookup(const string& name);
};



