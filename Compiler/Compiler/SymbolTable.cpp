#include "StdAfx.h"
#include "SymbolTable.h"

SymbolTable::SymbolTable()
{
}

SymbolTable::~SymbolTable()
{
}

void SymbolTable::insert(TableElement *tableElement)
{
	ident2index[tableElement->name] = symbols.size();
	symbols.push_back(tableElement);
}

const TableElement *SymbolTable::lookup(const string &name)
{
	if (ident2index.find(name) != ident2index.end())
		return symbols[ident2index[name]];
	return nullptr;
}

void Function::addBlock(BasicBlock *bb)
{
	current->next = bb;
	current = bb;
}

void Function::addStatement(Quad *quad)
{
	current->addu(quad);
}

void Function::insert(TableElement *tableElement)
{
	if (tableElement->kind == K_PARA)
		args.push_back(tableElement);
	symbolTable->insert(tableElement);
}

const TableElement *Function::lookup(const string &name)
{
	return symbolTable->lookup(name);
}
