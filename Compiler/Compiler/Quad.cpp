#include "StdAfx.h"
#include "BasicBlock.h"
#include "Quad.h"
Operator::~Operator()
{
}
int Quad::count = 0;
string Operator::toString() const
{
	string r;
	switch (opcode)
	{
	case Op_ADD:
		r += " + ";
		break;
	case Op_SUB:
		r += " - ";
		break;
	case Op_DIV:
		r += " / ";
		break;
	case Op_MULT:
		r += " * ";
		break;
	}
	return id + " = " + s1->id + r + s2->id;
}

bool Operator::operator==(const Value &q)
{
	if (opcode != q.opcode)
		return false;
	auto o = static_cast<const Operator &>(q);
	if (s1->opcode == Op_CONST)
	{
		if (!((*s1) == (*o.s1)))
			return false;
	}
	else if (s1 != o.s1)
		return false;
	if (s2->opcode == Op_CONST)
	{
		if (!((*s2) == (*o.s2)))
			return false;
	}
	else if (s2 != o.s2)
		return false;
	return true;
}

string Constant::toString() const
{
	std::stringstream ss;
	if (type == T_INT)
		ss << val;
	else
		ss << "'" << (char)val << "'";
	return std::move(ss.str());
}

bool Constant::operator==(const Value &q)
{
	if (opcode != q.opcode)
		return false;
	auto o = static_cast<const Constant &>(q);
	return type == o.type && val == o.val;
}

string Var::toString() const
{
	string r = name;
	if (value != nullptr)
		r += " = " + value->id;
	return r;
}

bool Var::operator==(const Value &q)
{
	return opcode == q.opcode && name == static_cast<const Var &>(q).name;
}

string Array::toString() const
{
	return name + "[" + offset->id + "]";
}

bool Array::operator==(const Value &q)
{
	return false;
}

string FunctCall::toString() const
{
	string ret = name;
	if (args.size())
	{
		ret += '(';
		for (auto i = args.begin(); i != args.end(); i++)
			ret += (*i)->id + ", ";
		ret.pop_back();
		ret.pop_back();
		ret += ')';
	}
	return ret;
}

string Label::toString() const
{
	return string("block " + std::to_string((long long)(controller->id)));
}

string CmpBr::toString() const
{
	string ret;
	return ret;
}

string Goto::toString() const
{
	return string("goto " + label->toString());
}

string VoidCall::toString() const
{
	string ret = name;
	ret += '(';
	if (args.size())
	{
		for (auto i = args.begin(); i != args.end(); i++)
			ret += (*i)->id + ", ";
		ret.pop_back();
		ret.back() = ')';
	}
	return ret;
}

//string Assign::toString() const
//{
//	return string(var->toString() + " = " + s1->toString());
//}

string Return::toString() const
{
	return string("return" + (ret == nullptr ? "" : ret->toString()));
}

string Scanf::toString() const
{
	string ret = "scanf(";
	for (auto i = args.begin(); i != args.end(); i++)
		ret += (*i)->toString() + ", ";
	ret.pop_back();
	ret.back() = ')';
	return ret;
}

string Printf::toString() const
{
	string ret = "printf(";
	if (strIndex >= 0)
		ret += "str " + std::to_string((long long)strIndex) + " , ";
	if (value != nullptr)
		ret += value->toString();
	ret += ')';
	return ret;
}

bool Value::operator==(const Value &q)
{
	return opcode == q.opcode;
}
