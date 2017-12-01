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
	Op_ASSIGN,
	Op_ARRAY,
	Op_RETURN,
	Op_SCANF,
	Op_PRINTF,
	Op_BEQ,
	Op_BNE,
};
class Quad
{
  public:
	Quad(Opcode opcode) : opcode(opcode) {}
	const Opcode opcode;
	virtual string toString() = 0;
};

class Value : public Quad
{
  public:
	const Type type;
	Value(Opcode opcode, Type type) : Quad(opcode), type(type) {}
};

class Instruction : public Quad
{
  public:
	Instruction(Opcode) : Quad(opcode) {}
};

class Label : public Instruction
{
  public:
	Label() : Instruction(Op_LABEL) {}
	Label(BasicBlock *c) : Instruction(Op_LABEL), controller(c) {}
	BasicBlock *controller;
	virtual string toString();
};

class Constant : public Value
{
  public:
	Constant(int intVal) : Value(Op_CONST, T_INT), val(intVal) {}
	Constant(char charVal) : Value(Op_CONST, T_CHAR), val(charVal) {}
	const int val;
	virtual string toString();
};

class Var : public Value
{
  public:
	const string name;
	Var(const string &name, Type type) : Value(Op_VAR, type), name(name) {}
	virtual string toString();
};

class Operator : public Value
{
  public:
	Value *s1;
	Value *s2;
	Operator(Opcode opcode, Type type, Value *const s1, Value *const s2) : Value(opcode, type), s1(s1), s2(s2) {}
	virtual string toString();
};

class Goto : public Instruction
{
  public:
	const Label *label;
	Goto(Label *const label) : Instruction(Op_GOTO), label(label) {}
	virtual string toString();
};

class CmpBr : public Instruction
{
  public:
	const Label *label;
	Value *s1;
	Value *s2;
	CmpBr(Opcode opcode, Value *const s1, Value *const s2, Label *const label) : Instruction(opcode), label(label), s1(s1), s2(s2) {}
	virtual string toString();
};

class FunctCall : public Value
{
  public:
	vector<Value *> args;
	string name;
	FunctCall(const string &name, Type type, vector<Value *> args) : name(name), args(std::move(args)), Value(Op_FUNCALL, type) {}
	virtual string toString();
};

class VoidCall : public Instruction
{
  public:
	vector<Value *> args;
	string name;
	VoidCall(const string &name, vector<Value *> args) : name(name), args(std::move(args)), Instruction(Op_VOIDCALL) {}
	virtual string toString();
};

class Array : public Value
{
  public:
	Value *offset;
	const string name;
	Array(Value *const offset, const string &name, Type type) : offset(offset), name(name), Value(Op_ARRAY, type) {}
	virtual string toString();
};

class Assign : public Instruction
{
  public:
	Value *var;
	Value *s1;
	Assign(Var *const var, Value *const s1) : var(var), s1(s1), Instruction(Op_ASSIGN) {}
	Assign(Array *const var, Value *const s1) : var(var), s1(s1), Instruction(Op_ASSIGN) {}
	Assign(Value *const var, Value *const s1) : var(var), s1(s1), Instruction(Op_ASSIGN) {}
	virtual string toString();
};

class Return : public Instruction
{
  public:
	Value *ret;
	Return(Value *const var) : ret(var), Instruction(Op_RETURN) {}
	Return() : ret(nullptr), Instruction(Op_RETURN) {}
	virtual string toString();
};

class Scanf : public Instruction
{
  public:
	vector<Var *> args;
	Scanf(vector<Var *> args) : args(std::move(args)), Instruction(Op_SCANF) {}
	virtual string toString();
};

class Printf : public Instruction
{
  public:
	Value *value;
	int strIndex;
	Printf(int strIndex, Value *value = nullptr) : strIndex(strIndex), value(value), Instruction(Op_PRINTF) {}
	Printf(Value *value) : strIndex(-1), value(value), Instruction(Op_PRINTF) {}
	virtual string toString();
};
#endif // !__QUAD_H_