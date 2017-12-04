#include "StdAfx.h"
#include "IRBuilder.h"

IRBuilder::IRBuilder()
{
	currentSymbolTable = &globalSymbolTable;
}

IRBuilder::~IRBuilder()
{
}

bool IRBuilder::createFunc(Type type, string &name)
{
	string t;
	if (type == T_INT)
		t = "int";
	else if (type == T_CHAR)
		t = "char";
	else
		t = "void";
	auto function = new Function(name, type);
	ident2func[name] = functions.size();
	functions.push_back(function);
	currentSymbolTable = function->symbolTable;
	insertBlock = function->head;
	std::cout << "create function " << name << " of type " << t << std::endl;
	return true;
}

bool IRBuilder::createArg(Type type, string &name)
{
	std::cout << "create arg " << name << " of type " << (type == T_INT ? "int" : "char") << std::endl;
	functions.back()->haveArgs = true;
	if (currentSymbolTable->lookup(name) != nullptr)
		return false;
	else
		functions.back()->insert(new TableElement(name, type, K_PARA));
	return true;
}

/**
	behavior: first check if identifier exists in symbol table,
			  if already exists, return false
			  create symbol record in symbol table
*/
bool IRBuilder::createVarDeclare(Type type, string &name, int size)
{
	std::cout << "create var " << name << ((size == 0) ? "" : "[]") << " of type " << ((type == T_INT) ? "int" : "char") << std::endl;
	if (currentSymbolTable->lookup(name) != nullptr)
		return false;
	else
		currentSymbolTable->insert(new TableElement(name, type, (size > 0 ? K_ARRAY : K_VAR), size));
	return true;
}

bool IRBuilder::createConstDecalre(Type type, string &name, int val)
{
	std::cout << "create constant " << name << " of type " << (type == T_INT ? "int" : "char") << "  val : " << (type == T_INT ? val : (char)val) << std::endl;
	if (currentSymbolTable->lookup(name) != nullptr)
		return false;
	else
		currentSymbolTable->insert(new TableElement(name, type, K_CONST, val));
	return true;
}

// jump when not satisfy
void IRBuilder::createCmpBr(Token tk, Value *cond1, Value *cond2, BasicBlock *Then, BasicBlock *Else)
{
	Opcode op;
	switch (tk)
	{
	case EQU:
		op = Op_BNE;
		break;
	case NEQ:
		op = Op_BEQ;
		break;
	case LESS:
		op = Op_BGE;
		break;
	case GEQ:
		op = Op_BLT;
		break;
	case GRT:
		op = Op_BLE;
		break;
	case LEQ:
		op = Op_BGT;
		break;
	}
	insertBlock->add(new CmpBr(op, cond1, cond1, new Label(Else)));
}

// jump when not satisfy
void IRBuilder::createCmpBr(Value *cond, BasicBlock *Then, BasicBlock *Else)
{
	insertBlock->add(new CmpBr(Op_BEQZ, cond, new Label(Else)));
}

void IRBuilder::createGoto(Label *label)
{
	insertBlock->add(new Goto(label));
}

BasicBlock *IRBuilder::getLastBasicBlock()
{
	return insertBlock;
}

BasicBlock *IRBuilder::createBasicBlock()
{
	return new BasicBlock();
}

void IRBuilder::setInsertPoint(BasicBlock *bb)
{
	functions.back()->addBlock(bb);
	insertBlock = bb;
}

void IRBuilder::addStatement(Quad *quad)
{
	insertBlock->add(quad);
}

int IRBuilder::addString(const string &str)
{
	int ret = strTable.size();
	strTable.push_back(str);
	return ret;
}

const string &IRBuilder::lookupStr(int index)
{
	// TODO: insert return statement here
	return strTable[index];
}

const Function *IRBuilder::lookupFunc(const string &name)
{
	if (ident2func.find(name) != ident2func.end())
		return functions[ident2func[name]];
	return nullptr;
}

const TableElement *IRBuilder::lookup(const string &name)
{
	const TableElement *ret;
	if ((ret = currentSymbolTable->lookup(name)) != nullptr)
		return ret;
	else if ((ret = globalSymbolTable.lookup(name)) != nullptr)
		return ret;
	return nullptr;
}

const TableElement *IRBuilder::lookupLocal(const string &name)
{
	const TableElement *ret;
	if ((ret = currentSymbolTable->lookup(name)) != nullptr)
		return ret;
	return nullptr;
}
