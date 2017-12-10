#include "StdAfx.h"
#include "BasicBlock.h"

int BasicBlock::count = 0;

void BasicBlock::addu(Quad *quad)
{
	if (quad == nullptr)
		return;
	if (quad->opcode == Op_LABEL)
		static_cast<Label *>(quad)->controller = this;
	switch (quad->opcode)
	{
	case Op_ADD:
	case Op_SUB:
	case Op_MULT:
	case Op_DIV:
	{
		auto op = static_cast<Operator*>(quad);
		addu(op->s1);
		addu(op->s2);
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
		auto op = static_cast<CmpBr*>(quad);
		addu(op->s1);
		addu(op->s2);
		break;
	}
	case Op_CONST:
	case Op_VAR:
		return;
	case Op_ASSIGN:
	{
		auto op = static_cast<Assign*>(quad);
		addu(op->s1);
		addu(op->var);
		break;
	}
	case Op_ARRAY:
	{
		auto op = static_cast<Array*>(quad);
		addu(op->offset);
		break;
	}
	case Op_FUNCALL:
	{
		auto op = static_cast<FunctCall*>(quad);
		for (auto i = op->args.begin(); i != op->args.end(); i++)
			addu(*i);
		break;
	}
	case Op_VOIDCALL:
	{
		auto op = static_cast<VoidCall*>(quad);
		for (auto i = op->args.begin(); i != op->args.end(); i++)
			addu(*i);
		break;
	}
	case Op_RETURN:
	{
		auto op = static_cast<Return*>(quad);
		addu(op->ret);
		break;
	}
	case Op_PRINTF:
	{
		auto op = static_cast<Printf*>(quad);
		addu(op->value);
		break;
	}
	}
	quads.push_back(quad);
}
