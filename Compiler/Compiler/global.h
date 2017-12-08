#pragma once
class Tokenizer;
class Parser;
class SymbolTable;
class TableElement;
class IRBuilder;
class BasicBlock;
class Quad;
class Value;
class Instruction;
class Function;

enum Type
{
	T_INT,
	T_CHAR,
	T_VOID,
	T_STRING
};
#define __TEST