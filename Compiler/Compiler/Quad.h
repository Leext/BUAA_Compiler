#pragma once
#ifndef __QUAD_H_
#define __QUAD_H_

#include "stdafx.h"
using std::string;
using std::vector;

enum Opcode
{
	Op_CONST,
	Op_VAR,
	Op_LABEL,
	Op_GOTO,
	Op_FUNCALL,
	Op_VOIDCALL,
	Op_ADD,
	Op_SUB,
	Op_DIV,
	Op_MULT,
	Op_ARRAY,
	Op_RETURN,
	Op_SCANF,
	Op_PRINTF,
	Op_BEQ,
	Op_BEQZ,
	Op_BNE,
	Op_BLE,
	Op_BGE,
	Op_BGT,
	Op_BLT,
};
class Quad
{
public:
	const Opcode opcode;
	string id;
	Quad(Opcode opcode) : opcode(opcode)
	{
		id = "";
		count++;
	}
	virtual string toString() const = 0;
protected:
	static int count;
};

class Value : public Quad
{
public:
	const Type type;
	Value(Opcode opcode, Type type) : Quad(opcode), type(type) {}
	virtual bool operator==(const Value &q);
};

class Instruction : public Quad
{
public:
	Instruction(Opcode opcode) : Quad(opcode) {}
};

class Label : public Instruction
{
public:
	Label() : Instruction(Op_LABEL) {}
	Label(BasicBlock *c) : Instruction(Op_LABEL), controller(c) {}
	BasicBlock *controller;
	virtual string toString() const;
};

class Constant : public Value
{
public:
	Constant(int intVal) : Value(Op_CONST, T_INT), val(intVal)
	{
		id = "int_" + std::to_string((long long)count);
	}
	Constant(char charVal) : Value(Op_CONST, T_CHAR), val(charVal)
	{
		id = "char_" + std::to_string((long long)count);
	}
	const int val;
	virtual string toString() const;
	virtual bool operator==(const Value &q);
};

class Var : public Value
{
public:
	const string name;
	Value *value;
	Var(const string &name, Type type, Value *value = nullptr) : Value(Op_VAR, type), value(value), name(name)
	{
		id = name;
	}
	virtual string toString() const;
	virtual bool operator==(const Value &q);
};

class Operator : public Value
{
public:
	Value * s1;
	Value *s2;
	Operator(Opcode opcode, Type type, Value *const s1, Value *const s2) : Value(opcode, type), s1(s1), s2(s2)
	{
		switch (opcode)
		{
		case Op_ADD:
			id = "add_" + std::to_string((long long)count);
			break;
		case Op_SUB:
			id = "sub_" + std::to_string((long long)count);
			break;
		case Op_DIV:
			id = "div_" + std::to_string((long long)count);
			break;
		case Op_MULT:
			id = "mult_" + std::to_string((long long)count);
			break;
		}
	}
	~Operator();
	virtual string toString() const;
	virtual bool operator==(const Value &q);
};

class Goto : public Instruction
{
public:
	const Label *label;
	Goto(Label *const label) : Instruction(Op_GOTO), label(label) {}
	virtual string toString() const;
};

class CmpBr : public Instruction
{
public:
	const Label *label;
	Value *s1;
	Value *s2;
	CmpBr(Opcode opcode, Value *const s1, Value *const s2, Label *const label) : Instruction(opcode), label(label), s1(s1), s2(s2) {}
	CmpBr(Opcode opcode, Value *const s1, Label *const label) : Instruction(opcode), label(label), s1(s1), s2(nullptr) {}
	virtual string toString() const;
};

class FunctCall : public Value
{
public:
	vector<Value *> args;
	string name;
	FunctCall(const string &name, Type type, vector<Value *> args) : name(name), args(std::move(args)), Value(Op_FUNCALL, type)
	{
		id = "fcall_" + std::to_string((long long)count);
	}
	virtual string toString() const;
};

class VoidCall : public Instruction
{
public:
	vector<Value *> args;
	string name;
	VoidCall(const string &name, vector<Value *> args) : name(name), args(std::move(args)), Instruction(Op_VOIDCALL) {}
	virtual string toString() const;
};

class Array : public Value
{
public:
	Value * offset;
	Value *value;
	const string name;
	Array(Value *const offset, const string &name, Type type, Value *value = nullptr) : offset(offset), name(name), value(value), Value(Op_ARRAY, type)
	{
		id = "array_" + std::to_string((long long)count);
	}
	virtual string toString() const;
	virtual bool operator==(const Value& q);
};

//class Assign : public Instruction
//{
//public:
//	Value * var;
//	Value *s1;
//	Assign(Var *const var, Value *const s1) : var(var), s1(s1), Instruction(Op_ASSIGN) {}
//	Assign(Array *const var, Value *const s1) : var(var), s1(s1), Instruction(Op_ASSIGN) {}
//	Assign(Value *const var, Value *const s1) : var(var), s1(s1), Instruction(Op_ASSIGN) {}
//	virtual string toString() const;
//};

class Return : public Instruction
{
public:
	Value * ret;
	Return(Value *const var) : ret(var), Instruction(Op_RETURN) {}
	Return() : ret(nullptr), Instruction(Op_RETURN) {}
	virtual string toString() const;
};

class Scanf : public Instruction
{
public:
	vector<Var *> args;
	Scanf(vector<Var *> args) : args(std::move(args)), Instruction(Op_SCANF) {}
	virtual string toString() const;
};

class Printf : public Instruction
{
public:
	Value * value;
	int strIndex;
	Printf(int strIndex, Value *value = nullptr) : strIndex(strIndex), value(value), Instruction(Op_PRINTF) {}
	Printf(Value *value) : strIndex(-1), value(value), Instruction(Op_PRINTF) {}
	virtual string toString() const;
};
#endif // !__QUAD_H_