#include "StdAfx.h"
#include "BasicBlock.h"
#include "Quad.h"
string Operator::toString() const
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

string Constant::toString() const
{
	std::stringstream ss;
	if (type == T_INT)
		ss << val;
	else
		ss << "'" << (char)val << "'";
	return std::move(ss.str());
}

string Var::toString() const
{
	return name;
}

string Array::toString() const
{
	return name + "[" + offset->toString() + "]";
}

string FunctCall::toString() const
{
	string ret = name;
	if (args.size())
	{
		ret += '(';
		for (auto i = args.begin(); i != args.end(); i++)
			ret += (*i)->toString() + ", ";
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
	string ret = "compare&branch";
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
			ret += (*i)->toString() + ", ";
		ret.pop_back();
		ret.back() = ')';
	}
	return ret;
}

string Assign::toString() const
{
	return string(var->toString() + " = " + s1->toString());
}

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
