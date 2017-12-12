#include "StdAfx.h"
#include "Optimizer.h"
using std::map;
Optimizer::Optimizer()
{
}

void Optimizer::optimize(IRBuilder builder)
{
}

void Optimizer::optimizeBB(BasicBlock *bb)
{
	name2var.clear();
	var2var.clear();
	for (auto quad = bb->quads.begin(); quad != bb->quads.end(); quad++)
	{
		switch ((*quad)->opcode)
		{
		case Op_VAR:
		{
			auto var = static_cast<Var *>(*quad);
			//var2var 中是一定能找到value的，因为operator必定先于var生成
			name2var[var->name] = var->value = simplify(var->value);
			break;
		}
		case Op_ARRAY:
		{
			auto var = static_cast<Array *>(*quad);
			var->value = simplify(var->value);
			var->offset = simplify(var->offset);
			break;
		}
		case Op_ADD:
		case Op_SUB:
		case Op_MULT:
		case Op_DIV:
		{
			auto op = static_cast<Operator *>(*quad);
			op->s1 = simplify(op->s1);
			op->s2 = simplify(op->s2);
			bool haveSame = false;
			for (auto i = var2var.begin(); i != var2var.end(); i++)
			{
				if ((*i).first->opcode == op->opcode)
				{
					auto target = static_cast<Operator *>((*i).first);
					if (target->s1 == op->s1 && target->s2 == op->s2)
					{
						var2var[op] = target;
						haveSame = true;
						break;
					}
				}
			}
			if (!haveSame)
				var2var[op] = op;
			break;
		}
		case Op_FUNCALL:
		{
			auto fcall = static_cast<FunctCall *>(*quad);
			var2var[fcall] = fcall;
			for (auto arg = fcall->args.begin(); arg != fcall->args.end(); arg++)
				*arg = simplify(*arg);
			break;
		}
		case Op_VOIDCALL:
		{
			auto fcall = static_cast<VoidCall *>(*quad);
			for (auto arg = fcall->args.begin(); arg != fcall->args.end(); arg++)
				*arg = simplify(*arg);
			break;
		}
		case Op_BEQ:
		case Op_BEQZ:
		case Op_BGE:
		case Op_BGT:
		case Op_BLE:
		case Op_BLT:
		case Op_BNE:
		{
			auto b = static_cast<CmpBr *>(*quad);
			if (b->s1 != nullptr)
				b->s1 = simplify(b->s1);
			if (b->s2 != nullptr)
				b->s2 = simplify(b->s2);
			break;
		}
		case Op_RETURN:
		{
			auto r = static_cast<Return *>(*quad);
			if (r->ret != nullptr)
				r->ret = simplify(r->ret);
			break;
		}
		}
	}
	for (auto quad = bb->quads.begin(); quad != bb->quads.end();)
	{
		switch ((*quad)->opcode)
		{
		case Op_ADD:
		case Op_SUB:
		case Op_MULT:
		case Op_DIV:
			if (var2var[static_cast<Value*>(*quad)] != *quad)
			{
				delete *quad;
				quad = bb->quads.erase(quad);
				continue;
			}
			break;
		}
		quad++;
	}
}

Value *Optimizer::simplify(Value *value)
{
	switch (value->opcode)
	{
	case Op_VAR:
	{
		auto val = static_cast<Var *>(value);
		if (name2var.find(val->name) == name2var.end())
			name2var[val->name] = val;
		return name2var[val->name];
	}
	case Op_ADD:
	case Op_SUB:
	case Op_MULT:
	case Op_DIV:
		return var2var[value];
	case Op_FUNCALL:
		return value;
	case Op_ARRAY:
	{
		auto a = static_cast<Array *>(value);
		a->offset = simplify(a->offset);
		return value;
	}
	default:
		return value;
	};
}
