#pragma once
#ifndef __SYMBOLTABLE_H_
#define __SYMBOLTABLE_H_

#include "stdafx.h"
#include "BasicBlock.h"
using std::unordered_map;
using std::vector;
using std::string;

enum Kind
{
	K_VAR,
	K_CONST,
	K_PARA,
	K_ARRAY
};

class TableElement
{
  public:
	string name;
	Type type;
	Kind kind;
	int value;
	TableElement(string &name, Type type, Kind kind, int value = 0) : name(name), type(type), kind(kind), value(value) {}
};

class SymbolTable
{
  public:
	unordered_map<string, int> ident2index;
	vector<TableElement *> symbols;
	SymbolTable();
	~SymbolTable();
	void insert(TableElement *tableElement);
	const TableElement *lookup(const string &name);
};

class Function
{
	friend class IRBuilder;
	friend class Parser;
	friend class Generator;
	friend class Optimizer;

protected:
	const string name;
	SymbolTable *symbolTable;
	vector<TableElement *> args;
	Type type;
	BasicBlock *head;
	BasicBlock *current;
	bool haveArgs;

  public:
	Function(string &name, Type type) : name(name), type(type), haveArgs(false)
	{
		symbolTable = new SymbolTable();
		head = new BasicBlock();
		current = head;
	}
	~Function()
	{
		delete symbolTable;
		delete head;
	}
	void addBlock(BasicBlock *bb);
	void addStatement(Quad *quad);
	void insert(TableElement *tableElement);
	const TableElement *lookup(const string &name);
};
#endif // !__SYMBOLTABLE_H_