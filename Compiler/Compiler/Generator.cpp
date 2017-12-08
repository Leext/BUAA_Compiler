#include "StdAfx.h"
#include "Generator.h"
using std::to_string;
Generator::Generator(IRBuilder * builder) : builder(builder)
{
	labelCount = 0;
}

void Generator::generate()
{
	generateData();
	code.push_back(".text");
	code.push_back("b f_main");
	code.push_back("nop");
	for (auto i = builder->functions.begin(); i != builder->functions.end(); i++)
		generateFunction(*i);
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
			code.push_back("g_" + (*i)->name + ": .word 0");
			break;
		case K_CONST:
			code.push_back("g_" + (*i)->name + ": .word " + to_string((long long)(*i)->value));
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
	int localVarOffset = 0;
	for (auto i = symbolTable.begin(); i != symbolTable.end(); i++)
	{
		switch ((*i)->kind)
		{
		case K_CONST:
		case K_VAR:
			localOffset["l_" + (*i)->name] = localVarOffset;
			localVarOffset += 4;
			break;
		case K_ARRAY:
			localOffset["l_" + (*i)->name] = localVarOffset;
			localVarOffset += (*i)->value << 2;
			break;
		}
	}
	offset -= localVarOffset;

	code.push_back("addiu $sp $sp " + to_string((long long)offset));
	code.push_back("sw $ra " + to_string((long long)(-4 - offset)) + "($sp)");
	code.push_back("sw $fp " + to_string((long long)(-8 - offset)) + "($sp)");
	code.push_back("move $fp $sp");
	// save $s0~$s7
	for (int i = 0; i < 8; i++)
		code.push_back("sw $s" + to_string((long long)i) + " " + to_string((long long)(-12 - (i << 2) - offset)) + "($fp)");
	// store $a0~a3
	for (int i = 0; i < function->args.size(); i++)
	{
		int tmp = (i << 2) - offset;
		if (i < 4)
			code.push_back("sw $a" + to_string((long long)i) + " " + to_string((long long)tmp) + "($fp)");
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
		if (bb2label.find(bb) == bb2label.end())
			bb2label[bb] = labelCount++;
		code.push_back("label_" + to_string((long long)bb2label[bb]) + ":");
		auto& quads = bb->quads;
		int tempVarOffset = 0;
		for (auto quad = quads.begin(); quad != quads.end(); quad++)
		{
			switch ((*quad)->opcode)
			{
			case Op_ADD:
			case Op_SUB:
			case Op_MULT:
			case Op_DIV:
			case Op_FUNCALL:
				tempOffset[*quad] = tempVarOffset;
				tempVarOffset += 4;
				break;
			}
		}
		code.push_back("addiu $sp $sp -" + to_string((long long)tempVarOffset));
		for (auto quad = quads.begin(); quad != quads.end(); quad++)
		{
			switch ((*quad)->opcode)
			{
			case Op_ADD:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("addu $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_SUB:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("subu $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_MULT:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("mul $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_DIV:
			{
				auto op = static_cast<Operator*>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("div $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_FUNCALL:
			{
				auto op = static_cast<FunctCall*>(*quad);
				// pass args
				int temp = (op->args.size() << 2);
				code.push_back("addiu $sp $sp -" + to_string((long long)temp));
				int i;
				for (i = 0; i < 4 && i < op->args.size(); i++)
					loadValue(function, op->args[i], string("$a" + to_string((long long)i)), temp);
				for (; i < op->args.size(); i++)
				{
					loadValue(function, op->args[i], string("$t0"), temp);
					code.push_back("sw $t0 " + to_string((long long)(i << 2)) + "($sp)");
				}

				// save $t0~$t7
				code.push_back("addiu $sp $sp -32");
				for (int i = 0; i < 8; i++)
					code.push_back("sw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");

				// call
				code.push_back("jal f_" + op->name);
				code.push_back("nop");

				// restore $t0~$t7
				for (int i = 0; i < 8; i++)
					code.push_back("lw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");
				code.push_back("addiu $sp $sp 32");
				code.push_back("addiu $sp $sp " + to_string((long long)(op->args.size() << 2)));

				// restore $a0~a3
				for (int i = 0; i < function->args.size() && i < 4; i++)
				{
					int tmp = (i << 2) - offset;
					code.push_back("lw $a" + to_string((long long)i) + " " + to_string((long long)tmp) + "($fp)");
				}

				storeValue(function, *quad, string("$v0"));
				break;
			}
			case Op_VOIDCALL:
			{
				auto op = static_cast<VoidCall*>(*quad);
				// pass args
				int temp = (op->args.size() << 2);
				code.push_back("addiu $sp $sp -" + to_string((long long)temp));
				int i;
				for (i = 0; i < 4 && i < op->args.size(); i++)
					loadValue(function, op->args[i], string("$a" + to_string((long long)i)), temp);
				for (; i < op->args.size(); i++)
				{
					loadValue(function, op->args[i], string("$t0"), temp);
					code.push_back("sw $t0 " + to_string((long long)(i << 2)) + "($sp)");
				}

				// save $t0~$t7
				code.push_back("addiu $sp $sp -32");
				for (int i = 0; i < 8; i++)
					code.push_back("sw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");

				// call
				code.push_back("jal f_" + op->name);
				code.push_back("nop");

				// restore $t0~$t7
				for (int i = 0; i < 8; i++)
					code.push_back("lw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");
				code.push_back("addiu $sp $sp 32");
				code.push_back("addiu $sp $sp " + to_string((long long)(op->args.size() << 2)));

				// restore $a0~a3
				for (int i = 0; i < function->args.size() && i < 4; i++)
				{
					int tmp = (i << 2) - offset;
					code.push_back("lw $a" + to_string((long long)i) + " " + to_string((long long)tmp) + "($fp)");
				}
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
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("beq $t0 $t1 label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("beq $t0 $t1 label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BEQZ:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				loadValue(function, op->s1, string("$t0"));
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("beqz $t0 label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("beqz $t0 label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BNE:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("bne $t0 $t1 label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("bne $t0 $t1 label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BLE:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("ble $t0 $t1 label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("ble $t0 $t1 label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BGE:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("bge $t0 $t1 label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("bge $t0 $t1 label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BGT:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("bgt $t0 $t1 label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("bgt $t0 $t1 label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BLT:
			{
				CmpBr* op = static_cast<CmpBr*>(*quad);
				loadValue(function, op->s1, string("$t0"));
				loadValue(function, op->s2, string("$t1"));
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("blt $t0 $t1 label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("blt $t0 $t1 label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_GOTO:
			{
				auto op = static_cast<Goto*>(*quad);
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("b label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("b label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}

			case Op_SCANF:
			{
				auto op = static_cast<Scanf*>(*quad);
				for (auto i = op->args.begin(); i != op->args.end(); i++)
				{
					if ((*i)->type == T_INT)
					{
						code.push_back("li $v0 5");
						code.push_back("syscall");
						storeValue(function, *i, string("$v0"));

					}
					else if ((*i)->type == T_CHAR)
					{
						code.push_back("li $v0 12");
						code.push_back("syscall");
						storeValue(function, *i, string("$v0"));
					}
				}
				break;
			}
			case Op_PRINTF:
			{
				auto op = static_cast<Printf*>(*quad);
				if (op->strIndex >= 0)
				{
					code.push_back("li $v0 4");
					code.push_back("la $a0 str_" + to_string((long long)op->strIndex));
					code.push_back("syscall");
				}
				if (op->value != nullptr)
				{
					if (op->value->type == T_INT)
					{
						code.push_back("li $v0 1");
						loadValue(function, op->value, string("$a0"));
						code.push_back("syscall");
					}
					else if (op->value->type == T_CHAR)
					{
						code.push_back("li $v0 11");
						loadValue(function, op->value, string("$a0"));
						code.push_back("syscall");
					}
				}
				break;
			}

			case Op_RETURN:
			{
				auto op = static_cast<Return*>(*quad);
				if (op->ret != nullptr)
					loadValue(function, op->ret, string("$v0"));
				code.push_back("b f_" + function->name + "_return");
				goto out;
			}
			} // switch
		} // for
	out:
		code.push_back("addiu $sp $sp " + to_string((long long)tempVarOffset));


	} // for
	code.push_back("f_" + function->name + "_return:");
	// load $s0~$s7
	for (int i = 0; i < 8; i++)
		code.push_back("sw $s" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($fp)");

	code.push_back("lw $ra " + to_string((long long)(-4 - offset)) + "($fp)");
	code.push_back("lw $fp " + to_string((long long)(-8 - offset)) + "($fp)");
	code.push_back("addiu $sp $sp " + to_string((long long)(-offset)));

	// return 

	code.push_back("jr $ra");
	code.push_back("nop");
}

void Generator::loadValue(Function* function, Quad * quad, string & reg, int temp)
{
	if (reg.back() != ' ')
		reg += ' ';
	string instr = (static_cast<Value*>(quad)->type == T_INT) ? "lw " : "lb ";
	if (quad->opcode == Op_CONST)
		code.push_back("li " + reg + to_string((long long)static_cast<Constant*>(quad)->val));
	else if (quad->opcode == Op_VAR)
	{
		auto name = static_cast<Var*>(quad)->name;
		if (function->lookup(name) != nullptr)// local var
			code.push_back(instr + reg + to_string((long long)localOffset["l_" + name]) + "($fp)");
		else //global var
			code.push_back(instr + reg + "g_" + name);
	}
	else if (quad->opcode == Op_ARRAY)
	{
		auto name = static_cast<Array*>(quad)->name;
		auto offset = static_cast<Array*>(quad)->offset;
		if (function->lookup(name) != nullptr)
		{
			loadValue(function, offset, reg);
			code.push_back("sll " + reg + reg + to_string(2ll));  // reg is address offset
			code.push_back("addu " + reg + reg + "$fp");			// reg = $fp + address offset
			code.push_back(instr + reg + to_string((long long)localOffset[name]) + "(" + reg + ")");  // reg += base address
		}
		else
		{
			loadValue(function, offset, reg);
			code.push_back("sll " + reg + reg + to_string(2ll));
			code.push_back(instr + reg + "g_" + name + "(" + reg + ")");
		}
	}
	else
	{
		code.push_back(instr + reg + to_string((long long)(tempOffset[quad] + temp)) + "($sp)");
	}
}

void Generator::storeValue(Function * function, Quad * quad, string & reg)
{
	if (reg.back() != ' ')
		reg += ' ';
	string instr = (static_cast<Value*>(quad)->type == T_INT) ? "sw " : "sb ";
	if (quad->opcode == Op_VAR)
	{
		auto name = static_cast<Var*>(quad)->name;
		if (function->lookup(name) != nullptr)
			code.push_back(instr + reg + to_string((long long)localOffset["l_" + name]) + "($fp)");
		else
			code.push_back(instr + reg + " g_" + name);
	}

	else if (quad->opcode != Op_ARRAY)
	{
		code.push_back(instr + reg + to_string((long long)tempOffset[quad]) + "($sp)");
	}
}

void Generator::storeValueArray(Function * function, Quad * quad, string & reg, string & freeReg)
{
	if (quad->opcode == Op_ARRAY)
	{
		string instr = (static_cast<Value*>(quad)->type == T_INT) ? "sw " : "sb ";
		if (freeReg.back() != ' ')
			freeReg += ' ';
		auto name = static_cast<Array*>(quad)->name;
		auto offset = static_cast<Array*>(quad)->offset;
		if (function->lookup(name) != nullptr)
		{
			loadValue(function, offset, freeReg);
			code.push_back("sll " + freeReg + freeReg + to_string(2ll));  // reg is address offset
			code.push_back("addu " + freeReg + freeReg + "$fp");			// reg = $fp + address offset
			code.push_back(instr + reg + to_string((long long)localOffset[name]) + "(" + freeReg + ")");  // reg += base address
		}
		else
		{
			loadValue(function, offset, freeReg);
			code.push_back("sll " + freeReg + freeReg + to_string(2ll));
			code.push_back(instr + reg + " g_" + name + "(" + freeReg + ")");
		}
	}
}

void Generator::print(fstream & output)
{
	for (auto i = code.begin(); i != code.end(); i++)
		output << *i << std::endl;
}
