#pragma once
#include "stdafx.h"
class BasicBlock;
using std::string;
using std::vector;

enum VarType
{
	VARINT, CHAR
};

class Quad
{
public:
	enum Opcode
	{
		CONST, VAR, LABEL, GOTO, FUNCALL, VOIDCALL, ASSIGN, ARRAY,
		RETURN
	};
	Quad(Opcode opcode) : opcode(opcode) {}
	const Opcode opcode;
	virtual string toString() = 0;
};

class Value : public Quad
{
public:
	Value(Opcode opcode) : Quad(opcode) {}
};

class Instruction : public Quad
{
public:
	Instruction(Opcode) : Quad(opcode) {}
};

class Label : public Instruction
{
public:
	Label() : Instruction(LABEL) {}
	BasicBlock* controller;
};

class Constant : public Value
{
public:
	Constant(int intVal) : Value(CONST), val(intVal), type(VARINT) {}
	Constant(char charVal) : Value(CONST), val(charVal), type(CHAR) {}
	const int val;
	const VarType type;
};

class Var : public Value
{
public:
	const string name;
	const VarType type;
	Var(const string& name, VarType type) : Value(VAR), name(name), type(type) {}
};

class Operator : public Value
{
public:
	Value *s1;
	Value *s2;
	Operator(Opcode opcode, Value* const s1, Value* const s2) : Value(opcode), s1(s1), s2(s2) {}

};

class Goto : public Instruction
{
public:
	const Label* label;
	Goto(Label* const label) : Instruction(GOTO), label(label) {}
};

class CmpBr : public Instruction
{
public:
	const Label* label;
	Value *s1;
	Value *s2;
	CmpBr(Opcode opcode, Value* const s1, Value* const s2, Label* const label) : Instruction(opcode), label(label), s1(s1), s2(s2) {}
};

class FunctCall : public Value
{
public:
	vector<Value*> args;
	string name;
	FunctCall(const string& name, vector<Value*> args) :name(name), args(std::move(args)), Value(FUNCALL) {}
};

class VoidCall : public Instruction
{
public:
	vector<Value*> args;
	string name;
	VoidCall(const string& name, vector<Value*> args) :name(name), args(std::move(args)), Instruction(VOIDCALL) {}
};

class Array : public Value
{
public:
	Value* offset;
	const string name;
	const VarType type;
	Array(Value * const offset, const string& name, VarType type) :offset(offset), name(name), type(type), Value(ARRAY) {}
};

class Assign : public Instruction
{
public:
	Value* var;
	Value *s1;
	Assign(Var * const var, Value * const s1) :var(var), s1(s1), Instruction(ASSIGN) {}
	Assign(Array * const var, Value * const s1) :var(var), s1(s1), Instruction(ASSIGN) {}
};

class Return : public Instruction
{
public:
	Value* ret;
	Return(Value * const var) : ret(var), Instruction(RETURN) {}
	Return() :ret(nullptr), Instruction(RETURN) {}
};