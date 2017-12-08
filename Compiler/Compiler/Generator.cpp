#include "StdAfx.h"
#include "Generator.h"
using std::to_string;
Generator::Generator(IRBuilder * builder) : builder(builder)
{
	labelCount = 0;
}

void Generator::generateData()
{
	code.push_back(".data");
	auto & symbols = builder->globalSymbolTable.symbols;
	for (auto i = symbols.begin(); i != symbols.end(); i++)
	{
		switch ((*i)->kind)
		{
		case K_VAR:
		case K_CONST:
			code.push_back("g_" + (*i)->name + ": .word 0");
			break;
		case K_ARRAY:
			code.push_back("g_" + (*i)->name + ": .space " + std::to_string((long long)(4 * (*i)->value)));
			break;
		}
	}
	auto & strs = builder->strTable;
	for (int i = 0; i < strs.size(); i++)
		code.push_back("str_" + std::to_string((long long)i) + ": .asciiz \"" + strs[i] + "\"");
}

void Generator::generateFunction(Function * function)
{
	// function label
	code.push_back("f_" + function->name + ':');
	// get stack slot count
	localOffset.clear();
	int stackSlotCount = 2 + 8;   // for save $ra + $fp + $s0~$s7
	auto symbolTable = function->symbolTable->symbols;
	int offset = -(stackSlotCount << 2);
	int localCount = 0;
	for (auto i = symbolTable.begin(); i != symbolTable.end(); i++)
	{
		switch ((*i)->kind)
		{
		case K_CONST:
		case K_VAR:
			localCount++;
			break;
		case K_ARRAY:
			localCount += (*i)->value;
			break;
		}
	}
	localCount = localCount << 2;
	offset -= localCount;
	for (auto i = symbolTable.begin(); i != symbolTable.end(); i++)
	{
		switch ((*i)->kind)
		{
		case K_CONST:
		case K_VAR:
			localCount -= 4;
			localOffset["l_" + (*i)->name] = localCount;
			break;
		case K_ARRAY:
			localCount -= 4 + ((*i)->value << 2);
			localOffset["l_" + (*i)->name] = localCount;
			break;
		}
	}
	code.push_back("addi $sp $sp " + to_string((long long)offset));
	code.push_back("sw $ra " + to_string((long long)(-4 - offset)) + "($sp)");
	code.push_back("sw $fp " + to_string((long long)(-8 - offset)) + "($sp)");
	code.push_back("move $fp $sp");
	// save $s0~$s7
	for (int i = 0; i < 8; i++)
		code.push_back("sw $s" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($fp)");
	// store $a0~a3
	for (int i = 0; i < function->args.size(); i++)
	{
		int tmp = (i << 2) - offset;
		if (i < 4)
			code.push_back("sw $a" + (i + '0') + ' ' + to_string((long long)tmp) + "($fp)");
		localOffset["l_" + function->args[i]->name] = tmp;
	}
	// initialize const
	for (auto i = symbolTable.begin(); i != symbolTable.end(); i++)
		if ((*i)->kind == K_CONST)
		{
			code.push_back("li $t0 " + to_string((long long)(*i)->value));
			code.push_back("sw $t0 " + to_string((long long)localOffset["l_" + (*i)->name]) + "($fp)");
		}
	for (auto bb = function->head; bb != nullptr; bb = bb->next)
	{
		tempOffset.clear();
		if (bb2label.find(bb) != bb2label.end())
			code.push_back("label_" + to_string((long long)bb2label[bb]) + ":");
		auto& quads = bb->quads;
		int tempVarOffset = offset;
		for (auto quad = quads.begin(); quad != quads.end(); quad++)
		{
			switch ((*quad)->opcode)
			{
			case Op_ADD:
			case Op_SUB:
			case Op_MULT:
			case Op_DIV:
			case Op_FUNCALL:
				tempVarOffset -= 4;
				tempOffset[*quad] = tempVarOffset;
				break;
			}
		}
		code.push_back("addi $sp $sp " + to_string((long long)(tempVarOffset - offset)));
		for (auto quad = quads.begin(); quad != quads.end(); quad++)
		{
			switch ((*quad)->opcode)
			{
			case Op_ADD:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("add $t0 $t1 $t1"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_SUB:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("sub $t0 $t1 $t1"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_MULT:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("mul $t0 $t1 $t1"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_DIV:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("div $t0 $t1 $t1"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_FUNCALL:
			{
				auto op = static_cast<FunctCall*>(*quad);
				// pass args
				code.push_back("addi $sp $sp -" + to_string((long long)(op->args.size() << 2)));
				int i;
				for (i = 0; i < 4 && i < op->args.size(); i++)
					loadValue(function, op->args[i], string("$a" + to_string((long long)i)));
				for (; i < op->args.size(); i++)
				{
					loadValue(function, op->args[i], string("$t0"));
					code.push_back("sw $t0 " + to_string((long long)(i << 2)) + "($sp)");
				}

				// save $t0~$t7
				code.push_back("addi $sp $sp -32");
				for (int i = 0; i < 8; i++)
					code.push_back("sw $t" + ('0' + i) + ' ' + to_string((long long)(i << 2)) + "($sp)");

				// call
				code.push_back("jal f_" + op->name);
				code.push_back("nop");

				// restore $t0~$t7
				for (int i = 0; i < 8; i++)
					code.push_back("lw $t" + ('0' + i) + ' ' + to_string((long long)(i << 2)) + "($sp)");
				code.push_back("addi $sp $sp 32");
				code.push_back("addi $sp $sp " + to_string((long long)(op->args.size() << 2)));

				storeValue(function, *quad, string("$v0"));
				break;
			}
			case Op_VOIDCALL:
			{
				auto op = static_cast<FunctCall*>(*quad);
				// pass args
				code.push_back("addi $sp $sp -" + to_string((long long)(op->args.size() << 2)));
				int i;
				for (i = 0; i < 4 && i < op->args.size(); i++)
					loadValue(function, op->args[i], string("$a" + to_string((long long)i)));
				for (; i < op->args.size(); i++)
				{
					loadValue(function, op->args[i], string("$t0"));
					code.push_back("sw $t0 " + to_string((long long)(i << 2)) + "($sp)");
				}

				// save $t0~$t7
				code.push_back("addi $sp $sp -32");
				for (int i = 0; i < 8; i++)
					code.push_back("sw $t" + ('0' + i) + ' ' + to_string((long long)(i << 2)) + "($sp)");

				// call
				code.push_back("jal f_" + op->name);
				code.push_back("nop");

				// restore $t0~$t7
				for (int i = 0; i < 8; i++)
					code.push_back("lw $t" + ('0' + i) + ' ' + to_string((long long)(i << 2)) + "($sp)");
				code.push_back("addi $sp $sp 32");
				code.push_back("addi $sp $sp " + to_string((long long)(op->args.size() << 2)));

				break;
			}
			case Op_ASSIGN:
			{
				auto op = static_cast<Assign*>(*quad);
				loadValue(function, op->s1, string("$t0"));
				if (op->var->opcode == Op_ARRAY)
					storeValueArray(function, op->var, string("$t0"), string("$t1"));
				else
					storeValue(function, op->var, string("$t0"));
				break;
			}
			case Op_BEQ:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				bb2label[op->label->controller] = labelCount;
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				code.push_back("beq $t0 $t1 label_" + to_string((long long)labelCount));
				labelCount++;
				break;
			}
			case Op_BEQZ:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				bb2label[op->label->controller] = labelCount;
				loadValue(function, op->s1, string("$t0"));
				code.push_back("beq $t0 label_" + to_string((long long)labelCount));
				code.push_back("nop");
				labelCount++;
				break;
			}
			case Op_BNE:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				bb2label[op->label->controller] = labelCount;
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				code.push_back("bne $t0 $t1 label_" + to_string((long long)labelCount));
				code.push_back("nop");
				labelCount++;
				break;
			}
			case Op_BLE:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				bb2label[op->label->controller] = labelCount;
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				code.push_back("ble $t0 $t1 label_" + to_string((long long)labelCount));
				code.push_back("nop");
				labelCount++;
				break;
			}
			case Op_BGE:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				bb2label[op->label->controller] = labelCount;
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				code.push_back("bge $t0 $t1 label_" + to_string((long long)labelCount));
				code.push_back("nop");
				labelCount++;
				break;
			}
			case Op_BGT:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				bb2label[op->label->controller] = labelCount;
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				code.push_back("bgt $t0 $t1 label_" + to_string((long long)labelCount));
				code.push_back("nop");
				labelCount++;
				break;
			}
			case Op_BLT:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				bb2label[op->label->controller] = labelCount;
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				code.push_back("blt $t0 $t1 label_" + to_string((long long)labelCount));
				code.push_back("nop");
				labelCount++;
				break;
			}
			case Op_GOTO:
			{
				auto op = static_cast<Goto*>(*quad);
				bb2label[op->label->controller] = labelCount;
				code.push_back("b label_" + to_string((long long)labelCount));
				code.push_back("nop");
				labelCount++;
				break;
			}

			case Op_SCANF:
			{

			}
			case Op_PRINTF:
			{

			}

			}
		}
		code.push_back("addi $sp $sp " + to_string((long long)(offset - tempVarOffset)));


	}
	// load $s0~$s7
	for (int i = 0; i < 8; i++)
		code.push_back("sw $s" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($fp)");

	code.push_back("lw $ra " + to_string((long long)(4 - offset)) + "($fp)");
	code.push_back("lw $fp " + to_string((long long)(8 - offset)) + "($fp)");
	code.push_back("addi $sp $sp " + to_string((long long)(-offset)));

	// return 

	code.push_back("j $ra");
}

void Generator::convertQuad(Quad * quad)
{
	switch (quad->opcode)
	{
	case Op_ASSIGN:
	default:
		break;
	}
}

void Generator::loadValue(Function* function, Quad * quad, string & reg)
{
	if (reg.back() != ' ')
		reg += ' ';
	if (quad->opcode == Op_CONST)
		code.push_back("li " + reg + to_string((long long)static_cast<Constant*>(quad)->val));
	else if (quad->opcode == Op_VAR)
	{
		auto name = static_cast<Var*>(quad)->name;
		if (function->lookup(name) != nullptr)// local var
			code.push_back("lw " + reg + to_string((long long)localOffset["l_" + name]) + "($fp)");
		else //global var
			code.push_back("lw " + reg + "g_" + name);
	}
	else if (quad->opcode == Op_ARRAY)
	{
		auto name = static_cast<Array*>(quad)->name;
		auto offset = static_cast<Array*>(quad)->offset;
		if (function->lookup(name) != nullptr)
		{
			loadValue(function, offset, reg);
			code.push_back("sll " + reg + reg + to_string(2ll));  // reg is address offset
			code.push_back("add " + reg + reg + "$fp");			// reg = $fp + address offset
			code.push_back("lw " + reg + to_string((long long)localOffset[name]) + "(" + reg + ")");  // reg += base address
		}
		else
		{
			loadValue(function, offset, reg);
			code.push_back("sll " + reg + reg + to_string(2ll));
			code.push_back("lw " + reg + "g_" + name + "(" + reg + ")");
		}
	}
	else
	{
		code.push_back("lw " + reg + to_string((long long)tempOffset[quad]) + "($sp)");
	}
}

void Generator::storeValue(Function * function, Quad * quad, string & reg)
{
	if (reg.back() != ' ')
		reg += ' ';
	if (quad->opcode == Op_VAR)
	{
		auto name = static_cast<Var*>(quad)->name;
		if (function->lookup(name) != nullptr)
			code.push_back("sw " + reg + to_string((long long)localOffset["l_" + name]) + "($fp)");
		else
			code.push_back("sw " + reg + "g_" + name);
	}

	else if (quad->opcode != Op_ARRAY)
	{
		code.push_back("sw " + reg + to_string((long long)tempOffset[quad]) + "($sp)");
	}
}

void Generator::storeValueArray(Function * function, Quad * quad, string & reg, string & freeReg)
{
	if (quad->opcode == Op_ARRAY)
	{
		if (freeReg.back() != ' ')
			freeReg += ' ';
		auto name = static_cast<Array*>(quad)->name;
		auto offset = static_cast<Array*>(quad)->offset;
		if (function->lookup(name) != nullptr)
		{
			loadValue(function, offset, freeReg);
			code.push_back("sll " + freeReg + freeReg + to_string(2ll));  // reg is address offset
			code.push_back("add " + freeReg + freeReg + "$fp");			// reg = $fp + address offset
			code.push_back("sw " + reg + to_string((long long)localOffset[name]) + "(" + freeReg + ")");  // reg += base address
		}
		else
		{
			loadValue(function, offset, freeReg);
			code.push_back("sll " + freeReg + freeReg + to_string(2ll));
			code.push_back("sw " + reg + "g_" + name + "(" + freeReg + ")");
		}
	}
}
