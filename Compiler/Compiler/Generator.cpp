#include "StdAfx.h"
#include "Generator.h"
#include <cassert>
using std::to_string;
Generator::Generator(IRBuilder *builder) : builder(builder)
{
	labelCount = 0;
}

void Generator::generate()
{
	init();
	generateData();
	finalCode.push_back(".text");
	finalCode.push_back("b f_main");
	finalCode.push_back("nop");
	for (auto i = builder->functions.begin(); i != builder->functions.end(); i++)
	{
		initCode.clear();
		code.clear();
		loadedToGloabal.clear();
		generateFunctionOpt(*i);
		finalCode.insert(finalCode.end(), initCode.begin(), initCode.end());
		finalCode.insert(finalCode.end(), code.begin(), code.end());
	}
}

void Generator::generateData()
{
	finalCode.push_back(".data");
	auto &symbols = builder->globalSymbolTable.symbols;
	for (auto i = symbols.begin(); i != symbols.end(); i++)
	{
		switch ((*i)->kind)
		{
		case K_VAR:
			finalCode.push_back("g_" + (*i)->name + ": .word 0");
			break;
		case K_CONST:
			finalCode.push_back("g_" + (*i)->name + ": .word " + to_string((long long)(*i)->value));
			break;
		case K_ARRAY:
			finalCode.push_back("g_" + (*i)->name + ": .space " + std::to_string((long long)(4 * (*i)->value)));
			break;
		}
	}
	auto &strs = builder->strTable;
	for (int i = 0; i < strs.size(); i++)
		finalCode.push_back("str_" + std::to_string((long long)i) + ": .asciiz \"" + strs[i] + "\"");
}

void Generator::generateFunction(Function *function)
{
	// function label
	code.push_back("f_" + function->name + ':');
	// get stack slot count
	localOffset.clear();
	int stackSlotCount = 2 + 8; // for save $ra + $fp + $s0~$s7
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
		auto &quads = bb->quads;
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
				auto op = static_cast<Operator *>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("addu $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_SUB:
			{
				auto op = static_cast<Operator *>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("subu $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_MULT:
			{
				auto op = static_cast<Operator *>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("mul $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_DIV:
			{
				auto op = static_cast<Operator *>(*quad);
				loadValue(function, op->s1, string("$t1"));
				loadValue(function, op->s2, string("$t2"));
				code.push_back(string("div $t0 $t1 $t2"));
				storeValue(function, *quad, string("$t0"));
				break;
			}
			case Op_FUNCALL:
			{
				auto op = static_cast<FunctCall *>(*quad);
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
				auto op = static_cast<VoidCall *>(*quad);
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
			case Op_VAR:
			{
				auto op = static_cast<Var *>(*quad);
				if (op->value != nullptr)
				{
					loadValue(function, op->value, string("$t0"));
					storeValue(function, op, string("$t0"));
				}
				break;
			}
			case Op_ARRAY:
			{
				auto op = static_cast<Array *>(*quad);
				if (op->value != nullptr)
				{
					loadValue(function, op->value, string("$t0"));
					storeValueArray(function, op, string("$t0"), string("$t1"));
				}
				break;
			}
			//case Op_ASSIGN:
			//{
			//	auto op = static_cast<Assign*>(*quad);
			//	loadValue(function, op->s1, string("$t0"));
			//	if (op->var->opcode == Op_ARRAY)
			//		storeValueArray(function, op->var, string("$t0"), string("$t1"));
			//	else
			//		storeValue(function, op->var, string("$t0"));
			//	break;
			//}
			case Op_BEQ:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
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
				CmpBr *op = static_cast<CmpBr *>(*quad);
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
				CmpBr *op = static_cast<CmpBr *>(*quad);
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
				CmpBr *op = static_cast<CmpBr *>(*quad);
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
				CmpBr *op = static_cast<CmpBr *>(*quad);
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
				CmpBr *op = static_cast<CmpBr *>(*quad);
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
				CmpBr *op = static_cast<CmpBr *>(*quad);
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
				auto op = static_cast<Goto *>(*quad);
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
				auto op = static_cast<Scanf *>(*quad);
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
				auto op = static_cast<Printf *>(*quad);
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
				auto op = static_cast<Return *>(*quad);
				if (op->ret != nullptr)
					loadValue(function, op->ret, string("$v0"));
				code.push_back("b f_" + function->name + "_return");
				goto out;
			}
			} // switch
		}	 // for
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
	if (function->name != "main")
	{
		code.push_back("jr $ra");
		code.push_back("nop");
	}
}

void Generator::init()
{
	int tempRegNum = 8;
	for (int i = 0; i < tempRegNum; i++)
		tempRegs.emplace_back(Reg("$t" + std::to_string((long long)i)));
}

void Generator::generateFunctionOpt(Function * function)
{
	this->function = function;
	int offset = countVar(function);
	allocateGloabal(function);
	TempReg.clear();

	initCode.push_back("\nf_" + function->name + ":");
	auto symbolTable = function->symbolTable->symbols;

	initCode.push_back("addiu $sp $sp -" + to_string((long long)offset));
	initCode.push_back("sw $ra " + to_string((long long)(-4 + offset)) + "($sp)");
	initCode.push_back("sw $fp " + to_string((long long)(-8 + offset)) + "($sp)");
	// save $s0~$s7
	for (int i = 0; i < 8; i++)
		initCode.push_back("sw $s" + to_string((long long)i) + " " + to_string((long long)(-12 - (i << 2) + offset)) + "($sp)");
	// store $a0~a3
	for (int i = 0; i < function->args.size(); i++)
	{
		int tmp = (i << 2) + offset + (8 << 2);
		if (i < 4)
			initCode.push_back("sw $a" + to_string((long long)i) + " " + to_string((long long)tmp) + "($sp)");
		stackOffset[function->args[i]->name] = tmp;
	}
	// initialize const
	for (auto i = symbolTable.begin(); i != symbolTable.end(); i++)
		if ((*i)->kind == K_CONST)
		{
			if (GlobalReg.find((*i)->name) != GlobalReg.end())
			{
				loadedToGloabal.insert((*i)->name);
				initCode.push_back("li $s" + to_string((long long)GlobalReg[(*i)->name]) + " " + to_string((long long)(*i)->value));
			}
			else
			{
				initCode.push_back("li $t0 " + to_string((long long)(*i)->value));
				initCode.push_back("sw $t0 " + to_string((long long)stackOffset[(*i)->name]) + "($sp)");
			}
		}
	// init global


	for (auto bb = function->head; bb != nullptr; bb = bb->next)
	{
		if (bb2label.find(bb) == bb2label.end())
			bb2label[bb] = labelCount++;
		code.push_back("label_" + to_string((long long)bb2label[bb]) + ":");
		auto &quads = bb->quads;
		for (auto quad = quads.begin(); quad != quads.end(); quad++)
		{
			switch ((*quad)->opcode)
			{
			case Op_ADD:
			{
				auto op = static_cast<Operator *>(*quad);
				if (op->s2->opcode == Op_CONST)
				{
					auto s2 = static_cast<Constant*>(op->s2);
					if (op->s1->opcode == Op_CONST)
					{
						auto s1 = static_cast<Constant*>(op->s1);
						auto s0 = getReg(*op, true) + " ";
						code.push_back("li " + s0 + to_string((long long)(s1->val + s2->val)) + "\t#" + op->toString());
						break;
					}
					auto s1 = getReg(*op->s1) + " ";
					auto s0 = getReg(*op, true) + " ";
					code.push_back("addiu " + s0 + s1 + " " + to_string((long long)s2->val) + "\t#" + op->toString());
				}
				else
				{
					auto s1 = getReg(*op->s1) + " ";
					tempRegs[s1[2] - '0'].fix = true;
					auto s2 = getReg(*op->s2) + " ";
					tempRegs[s1[2] - '0'].fix = false;
					decreaseRef(op->s1);
					decreaseRef(op->s2);
					auto s0 = getReg(*op, true) + " ";
					code.push_back("addu " + s0 + s1 + s2 + "#\t" + op->toString());
				}
				break;
			}
			case Op_SUB:
			{

				auto op = static_cast<Operator *>(*quad);
				if (op->s2->opcode == Op_CONST)
				{
					auto s2 = static_cast<Constant*>(op->s2);
					if (op->s1->opcode == Op_CONST)
					{
						auto s1 = static_cast<Constant*>(op->s1);
						auto s0 = getReg(*op, true) + " ";
						code.push_back("li " + s0 + to_string((long long)(s1->val - s2->val)) + "\t#" + op->toString());
						break;
					}
					auto s1 = getReg(*op->s1) + " ";
					auto s0 = getReg(*op, true) + " ";
					code.push_back("subiu " + s0 + s1 + " " + to_string((long long)s2->val) + "\t#" + op->toString());
				}
				else
				{
					auto s1 = getReg(*op->s1) + " ";
					tempRegs[s1[2] - '0'].fix = true;
					auto s2 = getReg(*op->s2) + " ";
					tempRegs[s1[2] - '0'].fix = false;
					decreaseRef(op->s1);
					decreaseRef(op->s2);
					auto s0 = getReg(*op, true) + " ";;
					code.push_back("subu " + s0 + s1 + s2 + "#\t" + op->toString());
				}
				break;;
			}
			case Op_MULT:
			{
				auto op = static_cast<Operator *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				auto s0 = getReg(*op, true) + " ";;
				code.push_back("mul " + s0 + s1 + s2 + "#\t" + op->toString());
				break;
			}
			case Op_DIV:
			{
				auto op = static_cast<Operator *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				auto s0 = getReg(*op, true) + " ";;
				code.push_back("div " + s0 + s1 + s2 + "#\t" + op->toString());
				break;
			}
			case Op_FUNCALL:
			{
				auto op = static_cast<FunctCall *>(*quad);
				// pass args
				int temp = (op->args.size() << 2);
				code.push_back("addiu $sp $sp -" + to_string((long long)temp));
				int i;
				for (i = 0; i < 4 && i < op->args.size(); i++)
				{
					moveToReg(*op->args[i], string("$a" + to_string((long long)i)), temp);
					decreaseRef(op->args[i]);
				}
				for (; i < op->args.size(); i++)
				{
					moveToReg(*op->args[i], string("$t8"), temp);
					decreaseRef(op->args[i]);
					code.push_back("sw $t8 " + to_string((long long)(i << 2)) + "($sp)");
				}

				// save $t0~$t7
				code.push_back("addiu $sp $sp -32");
				for (int i = 0; i < 8; i++)
					code.push_back("sw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");

				// call
				code.push_back("jal f_" + op->name + "#\t" + op->toString());
				code.push_back("nop");

				// restore $t0~$t7
				for (int i = 0; i < 8; i++)
					code.push_back("lw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");
				code.push_back("addiu $sp $sp 32");
				code.push_back("addiu $sp $sp " + to_string((long long)(op->args.size() << 2)));

				// restore $a0~a3
				for (int i = 0; i < function->args.size() && i < 4; i++)
				{
					int tmp = (i << 2) + offset + (8 << 2);
					code.push_back("lw $a" + to_string((long long)i) + " " + to_string((long long)tmp) + "($sp)");
				}
				auto reg = getReg(*op, true);
				code.push_back("move " + reg + " $v0");

				break;
			}
			case Op_VOIDCALL:
			{
				auto op = static_cast<VoidCall *>(*quad);
				// pass args
				int temp = (op->args.size() << 2);
				code.push_back("addiu $sp $sp -" + to_string((long long)temp));
				int i;
				for (i = 0; i < 4 && i < op->args.size(); i++)
				{
					moveToReg(*op->args[i], string("$a" + to_string((long long)i)), temp);
					decreaseRef(op->args[i]);
				}
				for (; i < op->args.size(); i++)
				{
					moveToReg(*op->args[i], string("$t8"), temp);
					decreaseRef(op->args[i]);
					code.push_back("sw $t8 " + to_string((long long)(i << 2)) + "($sp)");
				}

				// save $t0~$t7
				code.push_back("addiu $sp $sp -32");
				for (int i = 0; i < 8; i++)
					code.push_back("sw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");

				// call
				code.push_back("jal f_" + op->name + "#\t" + op->toString());
				code.push_back("nop");

				// restore $t0~$t7
				for (int i = 0; i < 8; i++)
					code.push_back("lw $t" + to_string((long long)i) + " " + to_string((long long)(i << 2)) + "($sp)");
				code.push_back("addiu $sp $sp 32");
				code.push_back("addiu $sp $sp " + to_string((long long)(op->args.size() << 2)));

				// restore $a0~a3
				for (int i = 0; i < function->args.size() && i < 4; i++)
				{
					int tmp = (i << 2) + offset + (8 << 2);
					code.push_back("lw $a" + to_string((long long)i) + " " + to_string((long long)tmp) + "($sp)");
				}
				break;
			}
			case Op_VAR:
			{
				auto op = static_cast<Var *>(*quad);
				if (op->value != nullptr)
				{
					if (op->value->opcode == Op_CONST && function->lookup(op->name) != nullptr)
					{

						auto lReg = getReg(*op, true);
						auto val = static_cast<Constant*>(op->value);
						code.push_back("li " + lReg + " " + to_string((long long)val->val) + "\t#" + op->toString());
						break;
					}
					auto rReg = getReg(*op->value);
					decreaseRef(op->value);
					if (function->lookup(op->name) != nullptr)
					{
						auto lReg = getReg(*op, true);
						decreaseRef(op);
						if (rReg != lReg)
							code.push_back("move " + lReg + " " + rReg + "\t#" + op->toString());
					}
					else
					{
						code.push_back("\t#" + op->toString());
						storeValue(function, op, rReg);
					}
				}
				break;
			}
			case Op_ARRAY:
			{
				auto op = static_cast<Array *>(*quad);
				if (op->value != nullptr)
				{
					//moveToReg(*op->value, string("$t8"));
					auto reg = getReg(*op->value);
					decreaseRef(op->value);
					code.push_back("#\t" + op->toString());
					storeValueArray(function, op, reg, string("$t1"));
				}
				break;
			}
			//case Op_ASSIGN:
			//{
			//	auto op = static_cast<Assign*>(*quad);
			//	loadValue(function, op->s1, string("$t0"));
			//	if (op->var->opcode == Op_ARRAY)
			//		storeValueArray(function, op->var, string("$t0"), string("$t1"));
			//	else
			//		storeValue(function, op->var, string("$t0"));
			//	break;
			//}
			case Op_BEQ:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				writeBack();
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("beq " + s1 + s2 + "label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("beq " + s1 + s2 + "label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BEQZ:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				decreaseRef(op->s1);
				writeBack();
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("beqz " + s1 + " label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("beqz " + s1 + " label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BNE:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				writeBack();
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("bne " + s1 + s2 + "label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("bne " + s1 + s2 + "label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BLE:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				writeBack();
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("ble " + s1 + s2 + "label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("ble " + s1 + s2 + "label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BGE:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				writeBack();
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("bge " + s1 + s2 + "label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("bge " + s1 + s2 + "label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BGT:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				writeBack();
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("bgt " + s1 + s2 + "label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("bgt " + s1 + s2 + "label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_BLT:
			{
				CmpBr *op = static_cast<CmpBr *>(*quad);
				auto s1 = getReg(*op->s1) + " ";
				tempRegs[s1[2] - '0'].fix = true;
				auto s2 = getReg(*op->s2) + " ";
				tempRegs[s1[2] - '0'].fix = false;
				decreaseRef(op->s1);
				decreaseRef(op->s2);
				writeBack();
				if (bb2label.find(op->label->controller) != bb2label.end())
				{
					code.push_back("blt " + s1 + s2 + "label_" + to_string((long long)bb2label[op->label->controller]));
				}
				else
				{
					bb2label[op->label->controller] = labelCount;
					code.push_back("blt " + s1 + s2 + "label_" + to_string((long long)labelCount));
					labelCount++;
				}
				break;
			}
			case Op_GOTO:
			{
				auto op = static_cast<Goto *>(*quad);
				writeBack();
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
				auto op = static_cast<Scanf *>(*quad);
				for (auto i = op->args.begin(); i != op->args.end(); i++)
				{
					if ((*i)->type == T_INT)
					{
						code.push_back("li $v0 5");
						code.push_back("syscall");
						if (function->lookup((*i)->name) != nullptr)
						{
							auto reg = getReg(**i, true);
							code.push_back("move " + reg + " $v0");
						}
						else
						{
							storeValue(function, *i, string("$v0"));
						}
					}
					else if ((*i)->type == T_CHAR)
					{
						code.push_back("li $v0 12");
						code.push_back("syscall");
						if (function->lookup((*i)->name) != nullptr)
						{
							auto reg = getReg(**i, true);
							code.push_back("move " + reg + " $v0");
						}
						else
						{
							storeValue(function, *i, string("$v0"));
						}
					}
				}
				break;
			}
			case Op_PRINTF:
			{
				auto op = static_cast<Printf *>(*quad);
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
						moveToReg(*op->value, string("$a0"));
						decreaseRef(op->value);
						code.push_back("syscall");
					}
					else if (op->value->type == T_CHAR)
					{
						code.push_back("li $v0 11");
						moveToReg(*op->value, string("$a0"));
						decreaseRef(op->value);
						code.push_back("syscall");
					}
				}
				break;
			}

			case Op_RETURN:
			{
				auto op = static_cast<Return *>(*quad);
				if (op->ret != nullptr)
				{
					moveToReg(*op->ret, string("$v0"));
					decreaseRef(op->ret);
				}
				code.push_back("b f_" + function->name + "_return");
				goto out;
			}
			} // switch

		}	 // for
	out:
		code.push_back("nop");
		writeBack();

	} // for
	code.push_back("f_" + function->name + "_return:");
	// load $s0~$s7
	for (int i = 0; i < 8; i++)
		code.push_back("lw $s" + to_string((long long)i) + " " + to_string((long long)(-12 - (i << 2)) + offset) + "($sp)");

	code.push_back("lw $ra " + to_string((long long)(-4 + offset)) + "($sp)");
	code.push_back("lw $fp " + to_string((long long)(-8 + offset)) + "($sp)");
	code.push_back("addiu $sp $sp " + to_string((long long)(offset)));

	// return
	if (function->name != "main")
	{
		code.push_back("jr $ra");
		code.push_back("nop");
	}
}

void Generator::loadValue(Function *function, Quad *quad, string &reg, int temp)
{
	if (reg.back() != ' ')
		reg += ' ';
	string instr = (static_cast<Value *>(quad)->type == T_INT) ? "lw " : "lb ";
	if (quad->opcode == Op_CONST)
		code.push_back("li " + reg + to_string((long long)static_cast<Constant *>(quad)->val) + "\t#" + quad->id);
	else if (quad->opcode == Op_VAR)
	{
		auto name = static_cast<Var *>(quad)->name;
		if (function->lookup(name) != nullptr) // local var
			code.push_back(instr + reg + to_string((long long)stackOffset[name] + temp) + "($sp)" + "\t#" + name);
		else //global var
			code.push_back(instr + reg + "g_" + name + "\t#" + name);
	}
	else if (quad->opcode == Op_ARRAY)
	{
		auto name = static_cast<Array *>(quad)->name;
		auto offset = static_cast<Array *>(quad)->offset;
		if (offset->opcode != Op_ARRAY)
		{
			auto offReg = getReg(*offset, false, temp);
			decreaseRef(offset);
			moveToReg(*offset, reg);
		}
		else
			loadValue(function, offset, reg, temp);
		if (function->lookup(name) != nullptr)
		{
			code.push_back("sll " + reg + reg + to_string(2ll));											// reg is address offset
			code.push_back("addu " + reg + reg + "$sp");													// reg = $fp + address offset
			code.push_back(instr + reg + to_string((long long)stackOffset[name] + temp) + "(" + reg + ")"); // reg += base address
		}
		else
		{
			code.push_back("sll " + reg + reg + to_string(2ll));
			code.push_back(instr + reg + "g_" + name + "(" + reg + ")");
		}
	}
	else
	{
		code.push_back(instr + reg + to_string((long long)(stackOffset[quad->id] + temp)) + "($sp)" + "\t#" + quad->id);
	}
}
void Generator::loadValueG(Function *function, Quad *quad, string &reg, int temp)
{
	if (reg.back() != ' ')
		reg += ' ';
	string instr = (static_cast<Value *>(quad)->type == T_INT) ? "lw " : "lb ";
	if (quad->opcode == Op_CONST)
		initCode.push_back("li " + reg + to_string((long long)static_cast<Constant *>(quad)->val) + "\t#" + quad->id);
	else if (quad->opcode == Op_VAR)
	{
		auto name = static_cast<Var *>(quad)->name;
		if (function->lookup(name) != nullptr) // local var
			initCode.push_back(instr + reg + to_string((long long)stackOffset[name] + temp) + "($sp)" + "\t#" + name);
		else //global var
			initCode.push_back(instr + reg + "g_" + name + "\t#" + name);
	}
	else if (quad->opcode == Op_ARRAY)
	{
		auto name = static_cast<Array *>(quad)->name;
		auto offset = static_cast<Array *>(quad)->offset;
		if (offset->opcode != Op_ARRAY)
		{
			auto offReg = getReg(*offset, false, temp);
			decreaseRef(offset);
			moveToReg(*offset, reg);
		}
		else
			loadValue(function, offset, reg, temp);
		if (function->lookup(name) != nullptr)
		{
			initCode.push_back("sll " + reg + reg + to_string(2ll));											// reg is address offset
			initCode.push_back("addu " + reg + reg + "$sp");													// reg = $fp + address offset
			initCode.push_back(instr + reg + to_string((long long)stackOffset[name] + temp) + "(" + reg + ")"); // reg += base address
		}
		else
		{
			initCode.push_back("sll " + reg + reg + to_string(2ll));
			initCode.push_back(instr + reg + "g_" + name + "(" + reg + ")");
		}
	}
	else
	{
		initCode.push_back(instr + reg + to_string((long long)(stackOffset[quad->id] + temp)) + "($sp)" + "\t#" + quad->id);
	}
}
void Generator::storeValue(Function *function, Quad *quad, string &reg)
{
	if (reg.back() != ' ')
		reg += ' ';
	string instr = (static_cast<Value *>(quad)->type == T_INT) ? "sw " : "sb ";
	if (quad->opcode == Op_VAR)
	{
		auto name = static_cast<Var *>(quad)->name;
		if (function->lookup(name) != nullptr)
			code.push_back(instr + reg + to_string((long long)stackOffset[name]) + "($sp)" + "\t#" + name);
		else
			code.push_back(instr + reg + " g_" + name + "\t#" + name);
	}
	else if (quad->opcode != Op_ARRAY && quad->opcode != Op_CONST)
	{
		code.push_back(instr + reg + to_string((long long)stackOffset[quad->id]) + "($sp)" + "\t#" + quad->id);
	}
}

void Generator::storeValueArray(Function *function, Quad *quad, string &reg, string &freeReg)
{
	if (quad->opcode == Op_ARRAY)
	{
		string instr = (static_cast<Value *>(quad)->type == T_INT) ? "sw " : "sb ";
		auto name = static_cast<Array *>(quad)->name;
		auto offset = static_cast<Array *>(quad)->offset;
		if (function->lookup(name) != nullptr)
		{
			string offReg = "$t8 ";
			moveToReg(*offset, offReg);
			decreaseRef(offset);
			code.push_back("sll " + offReg + offReg + to_string(2ll));											  // reg is address offset
			code.push_back("addu " + offReg + offReg + "$sp");													  // reg = $sp + address offset
			code.push_back(instr + reg + " " + to_string((long long)stackOffset[name]) + "(" + offReg + ")"); // reg += base address
		}
		else
		{
			string offReg = "$t8 ";
			moveToReg(*offset, offReg);
			decreaseRef(offset);
			code.push_back("sll " + offReg + offReg + to_string(2ll));
			code.push_back(instr + reg + " g_" + name + "(" + offReg + ")");
		}
	}
}

string Generator::getReg(Value & value, bool write, int temp)
{
	auto&id = value.id;
	if (GlobalReg.find(id) != GlobalReg.end())   // 分配全局寄存器的变量
	{
		string reg = "$s" + to_string((long long)GlobalReg[id]);
		if (loadedToGloabal.find(id) == loadedToGloabal.end())
		{
			loadedToGloabal.insert(id);
			if (!write)
			{
				loadValueG(function, &value, reg, temp);
			}
		}
		return reg;
	}
	else if (TempReg.find(id) != TempReg.end())  // 在临时寄存器中的变量
		return TempReg[id]->name;
	auto& reg = spill();
	reg.value = &value;
	reg.free = false;
	TempReg[id] = &reg;
	if (!write)
		loadValue(function, &value, reg.name, temp);
	return reg.name;
}

void Generator::moveToReg(Value & value, string&reg, int temp)
{
	auto&id = value.id;
	if (GlobalReg.find(id) != GlobalReg.end())   // 分配全局寄存器的变量
	{
		string greg = " $s" + to_string((long long)GlobalReg[id]);
		if (loadedToGloabal.find(id) == loadedToGloabal.end())
		{
			loadedToGloabal.insert(id);
			loadValueG(function, &value, greg, temp);
		}
		code.push_back("move " + reg + greg);
	}
	else if (TempReg.find(id) != TempReg.end())  // 在临时寄存器中的变量
		code.push_back("move " + reg + " " + TempReg[id]->name);
	else
		loadValue(function, &value, reg, temp);
}

void Generator::decreaseRef(Value * value)
{
	//std::cout << "decrease " << value->id << "  " << refCount[value->id] - 1 << std::endl;
	refCount[value->id];
}

Reg& Generator::spill()
{
	int max = 0;
	Value* maxValue;
	decltype(tempRegs.begin()) maxIter;
	for (auto i = tempRegs.begin(); i != tempRegs.end(); i++)
	{
		if ((*i).free)
		{
			(*i).free = false;
			return (*i);
		}
		auto pre_value = (*i).value;
		int ref = refCount[pre_value->id];
		if (ref == 0)
		{
			TempReg.erase(pre_value->id);
			(*i).free = true;
			return (*i);
		}
		if (!(*i).fix&&ref > max)
		{
			max = ref;
			maxValue = pre_value;
			maxIter = i;
		}
	}
	storeValue(function, maxValue, (*maxIter).name);
	TempReg.erase(maxValue->id);
	return (*maxIter);
}

void Generator::writeBack()
{
	for (auto i = tempRegs.begin(); i != tempRegs.end(); i++)
	{
		auto& reg = *i;
		if (!reg.free&&refCount[reg.value->id] > 0)
		{
			TempReg.erase(reg.value->id);
			storeValue(function, reg.value, reg.name);
			reg.free = true;
		}
	}
}

int Generator::countVar(Function * function)
{
	stackOffset.clear();
	refCount.clear();
	int count = 0;
	int offset = ((2 + 8) << 2);
	auto symbolTable = function->symbolTable->symbols;
	for (auto i = symbolTable.begin(); i != symbolTable.end(); i++)
	{
		switch ((*i)->kind)
		{
		case K_CONST:
		case K_VAR:
		case K_PARA:
			stackOffset[(*i)->name] = offset;
			offset += 4;
			break;
		case K_ARRAY:
			stackOffset[(*i)->name] = offset + ((*i)->value << 2) - 4;
			offset += (*i)->value << 2;
			break;
		}
	}

	for (auto bb = function->head; bb != nullptr; bb = bb->next)
	{
		for (auto quad = bb->quads.begin(); quad != bb->quads.end(); quad++)
		{
			switch ((*quad)->opcode)
			{
			case Op_ADD:
			case Op_SUB:
			case Op_MULT:
			case Op_DIV:
			{
				auto op = static_cast<Operator*>(*quad);
				refCount[op->s1->id]++;
				refCount[op->s2->id]++;
				stackOffset[(*quad)->id] = offset;
				offset += 4;
				break;
			}
			case Op_FUNCALL:
			{
				auto func = static_cast<FunctCall*>(*quad);
				for (auto i = func->args.begin(); i != func->args.end(); i++)
					refCount[(*i)->id]++;
				stackOffset[(*quad)->id] = offset;
				offset += 4;
				break;
			}
			case Op_VOIDCALL:
			{
				auto func = static_cast<VoidCall*>(*quad);
				for (auto i = func->args.begin(); i != func->args.end(); i++)
					refCount[(*i)->id]++;
				break;
			}
			case Op_CONST:
			{
				auto op = static_cast<Constant*>(*quad);
				refCount[op->id]++;
				break;
			}
			case Op_VAR:
			{
				auto op = static_cast<Var*>(*quad);
				refCount[op->id]++;
				if (op->value != nullptr)
					refCount[op->value->id]++;
				break;
			}
			case Op_ARRAY:
			{
				auto op = static_cast<Array*>(*quad);
				refCount[op->offset->id]++;
				if (op->value != nullptr)
					refCount[op->value->id]++;
				break;
			}
			case Op_BEQ:
			case Op_BEQZ:
			case Op_BNE:
			case Op_BLE:
			case Op_BGE:
			case Op_BGT:
			case Op_BLT:
			{
				auto op = static_cast<CmpBr*>(*quad);
				if (op->s1 != nullptr)
					refCount[op->s1->id]++;
				if (op->s2 != nullptr)
					refCount[op->s2->id]++;
				break;
			}
			case Op_PRINTF:
			{
				auto op = static_cast<Printf*>(*quad);
				if (op->value != nullptr)
					refCount[op->value->id]++;
				break;
			}
			case Op_RETURN:
			{
				auto op = static_cast<Return*>(*quad);
				if (op->ret != nullptr)
					refCount[op->ret->id]++;
				break;
			}
			}
		}
	}
	offset -= 4;
	for (auto i = stackOffset.begin(); i != stackOffset.end(); i++)
		(*i).second = offset - (*i).second;
	return offset + 4;
}
typedef std::pair<string, int> V;
bool cmp(V&a, V&b)
{
	return a.second > b.second;
}
void Generator::allocateGloabal(Function * function)
{
	int globalRegNum = 8;
	vector<V> v;
	for (auto var = refCount.begin(); var != refCount.end(); var++)
		v.emplace_back(std::make_pair(var->first, var->second));
	std::sort(v.begin(), v.end(), cmp);
	GlobalReg.clear();
	for (int i = 0, rest = globalRegNum; rest > 0 && i < v.size(); i++)
	{
		if (function->lookup(v[i].first) == nullptr)
			continue;
		GlobalReg[v[i].first] = 8 - rest;
		rest--;
	}
}

void Generator::print(fstream &output)
{
	for (auto i = finalCode.begin(); i != finalCode.end(); i++)
		output << *i << std::endl;
}
