#include "StdAfx.h"
#include "Quad.h"
string Operator::toString()
{
	string r;
	r += "(";
	r += s1->toString();
	switch (opcode)
	{
	case Op_ADD:
		r += "+";
		break;
	case Op_SUB:
		r += "-";
		break;
	case Op_DIV:
		r += "/";
		break;
	case Op_MULT:
		r += "*";
		break;
	}
	r += s2->toString();
	return std::move(r);
}

string Constant::toString()
{
	std::stringstream ss;
	if (type == T_INT)
		ss << val;
	else ss << (char)val;
	return std::move(ss.str());
}

string Var::toString()
{
	return name;
}

string Array::toString()
{
	return name + "[" + offset->toString() + "]";
}
