#include "StdAfx.h"
#include "IRBuilder.h"


IRBuilder::IRBuilder()
{
}


IRBuilder::~IRBuilder()
{
}

/**
	behavior: first check if identifier exists in symbol table,
			  if already exists, return false
			  create symbol record in symbol table
*/
bool IRBuilder::createGlobalVarDeclare(Type type, string & name, int size)
{
	std::cout << "create var " << name << ((size == 0) ? "" : "[]") << " of type " << ((type == T_INT) ? "int" : "char") << std::endl;
	return true;
}
