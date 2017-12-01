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
	r += ")";
	return std::move(r);
}

string Constant::toString()
{
	std::stringstream ss;
	if (type == T_INT)
		ss << val;
	else
		ss << "'" << (char)val << "'";
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

string FunctCall::toString()
{
	string ret = name;
	if (args.size())
	{
		ret += "(";
		for (auto i = args.begin(); i != args.end(); i++)
			ret += (*i)->toString() + ", ";
		ret.pop_back();
		ret.pop_back();
		ret += ")";
	}
	return ret;
}

string Label::toString()
{
	return string();
}

string CmpBr::toString()
{
	return string();
}

string Goto::toString()
{
	return string();
}

string VoidCall::toString()
{
	return string();
}

string Assign::toString()
{
	return string();
}

string Return::toString()
{
	return string();
}

string Scanf::toString()
{
	return string();
}

string Printf::toString()
{
	return string();
}
