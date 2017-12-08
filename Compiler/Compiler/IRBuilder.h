#pragma once
#include "stdafx.h"
#include "SymbolTable.h"
#include "BasicBlock.h"
#include "Quad.h"
#include "Tokenizer.h"

class IRBuilder
{
	friend class Generator;
  public:
	IRBuilder();
	~IRBuilder();
	bool createFunc(Type type, string &name);
	bool createArg(Type type, string &name);
	bool createVarDeclare(Type type, string &name, int size = 0);
	bool createConstDecalre(Type type, string &name, int val);
	void createCmpBr(Token tk, Value *cond1, Value *cond2, BasicBlock *Then, BasicBlock *Else);
	void createCmpBr(Value *cond, BasicBlock *Then, BasicBlock *Else);
	void createGoto(Label *label);
	BasicBlock *getLastBasicBlock();
	BasicBlock *createBasicBlock();
	void setInsertPoint(BasicBlock *bb);
	void addStatement(Quad *quad);
	int addString(const string &str);
	const string &lookupStr(int index);
	const Function *lookupFunc(const string &name);
	const TableElement *lookup(const string &name);
	const TableElement *lookupLocal(const string &name);
	const Function *getCurrentFunction()
	{
		return functions.back();
	}

  protected:
	SymbolTable globalSymbolTable;
	SymbolTable *currentSymbolTable;
	unordered_map<string, int> ident2func;
	BasicBlock *insertBlock;
	vector<Function *> functions;
	vector<string> strTable;
};
