#include "StdAfx.h"
#include "Parser.h"
#include "Tokenizer.h"

void Parser::skipToToken(Token tk)
{
	while (token != tk)
		token = tokenizer.nextToken();
}

void Parser::skipToToken(Token t1, Token t2)
{
	while (token != t1 && token != t2)
		token = tokenizer.nextToken();
}

void Parser::skipToToken(unordered_set<Token> &skipSet)
{
	while (skipSet.find(tokenizer.nextToken()) != skipSet.end())
		;
	token = tokenizer.getToken();
}

Value *Parser::parseExpression()
{
	Opcode op = Op_ADD;
	if (token == PLUS || token == MINUS)
	{
		if (token == MINUS)
			op = Op_SUB;
		token = tokenizer.nextToken();
	}
	auto term = parseTerm();
	if (term == nullptr)
		return nullptr;
	if (op == Op_SUB)
		term = new Operator(Op_SUB, T_INT, new Constant(0), term);
	while (token == PLUS || token == MINUS)
	{
		op = (token == PLUS ? Op_ADD : Op_SUB);
		token = tokenizer.nextToken();
		auto term2 = parseTerm();
		if (term2 == nullptr)
			return nullptr;
		term = new Operator(op, T_INT, term, term2);
	}
	return term;
}

Value *Parser::parseTerm()
{
	Opcode op;
	auto factor = parseFactor();
	if (factor == nullptr)
		return nullptr;
	while (token == MULT || token == DIV)
	{
		op = (token == MULT ? Op_MULT : Op_DIV);
		token = tokenizer.nextToken();
		auto factor2 = parseFactor();
		if (factor2 == nullptr)
			return nullptr;
		factor = new Operator(op, T_INT, factor, factor2);
	}
	return factor;
}

Value *Parser::parseFactor()
{
	const TableElement *te;
	const Function *func;
	int sign = 1;
	int n;
	char c;
	Value *ret;
	switch (token)
	{
	case IDENT:
		identifier = tokenizer.getIdent();
		if ((te = builder.lookup(identifier)) != nullptr)
		{
			if (te->kind == K_ARRAY)
			{
				if (!match(lBRACK))
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_BRACKET_EXPECTED);
					return nullptr;
				}
				token = tokenizer.nextToken();
				auto offset = parseExpression();
				if (offset == nullptr)
					return nullptr;
				if (token != rBRACK)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_BRACKET_EXPECTED);
					return nullptr;
				}
				token = tokenizer.nextToken();
				return new Array(offset, te->name, te->type);
			}
			else
			{
				token = tokenizer.nextToken();
				return new Var(te->name, te->type);
			}
		}
		else if ((func = builder.lookupFunc(identifier)) != nullptr)
		{
			if (func->type == T_VOID)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_VOID_FUNCTION_CALL);
				return nullptr;
			}
			vector<Value *> args;
			if (func->haveArgs) // function with args
			{
				// parse args
				if (!match(lPARE))
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
					return nullptr;
				}
				do
				{
					token = tokenizer.nextToken();
					auto e = parseExpression();
					if (e == nullptr)
						return nullptr;
					args.push_back(e);
				} while (token == COMMA);
				if (token != rPARE)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
					return nullptr;
				}
			}
			token = tokenizer.nextToken();
			ret = new FunctCall(func->name, func->type, args);
			return ret;
		}
		break;
	case lPARE:
		token = tokenizer.nextToken();
		ret = parseExpression();
		if (token != rPARE)
			return nullptr;
		else
		{
			token = tokenizer.nextToken();
			return ret;
		}
	case MINUS: //int
		sign = -1;
	case PLUS:
		if (!match(NUM))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), NUM_EXPECTED);
			return nullptr;
		}
	case NUM:
		n = tokenizer.getNum();
		token = tokenizer.nextToken();
		return new Constant(sign * n);
	case ALPHA:
		c = tokenizer.getNum();
		token = tokenizer.nextToken();
		return new Constant(c);
	}
	return nullptr;
}

void Parser::parseStatement()
{
	switch (token)
	{
	case IF:
		parseIf();
		return;
	case WHILE:
		parseWhile();
		return;
	case IDENT:
		// assign,  function call, void function call
		identifier = tokenizer.getIdent();
		const TableElement *te;
		const Function *function;
		if ((te = builder.lookup(identifier)) != nullptr) // assign
		{
			Value *variable;
			if (te->kind == K_ARRAY)
			{
				if (!match(lBRACK))
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_BRACKET_EXPECTED);
					/// todo skip
					goto error;
				}
				token = tokenizer.nextToken();
				auto offset = parseExpression();
				if (offset == nullptr)
					goto error;
				std::cout << "expression " << offset->toString() << std::endl;
				if (token != rBRACK)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_BRACKET_EXPECTED);
					/// todo skip
					goto error;
				}
				variable = new Array(offset, te->name, te->type);
			}
			else
			{
				variable = new Var(te->name, te->type);
			}
			if (!match(BECOME))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), ASSIGN_EXPECTED);
				/// todo skip
				goto error;
			}
			if (te->kind == K_CONST)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), CHANGE_CONST_VALUE);
			}
			std::cout << "this is an assign statement\n";
			token = tokenizer.nextToken();
			auto val = parseExpression();
			if (val == nullptr)
				goto error;
			std::cout << "expression " << val->toString() << std::endl;
			builder.addStatement(new Assign(variable, val));
		}
		else if ((function = builder.lookupFunc(identifier)) != nullptr)
		{
			vector<Value *> args;
			if (function->haveArgs) // function with args
			{
				// parse args
				if (!match(lPARE))
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
					goto error;
				}
				do
				{
					token = tokenizer.nextToken();
					auto e = parseExpression();
					if (e == nullptr)
						goto error;
					args.push_back(e);
				} while (token == COMMA);
				if (token != rPARE)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
					goto error;
				}
			}
			/// todo check args
			if (args.size() != function->args.size())
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNMATCHED_ARGUMENT_LIST);
				goto error;
			}
			for (int i = 0; i < args.size(); i++)
			{
				if (args[i]->type == T_INT && function->args[i]->type == T_CHAR)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNMATCHED_ARGUMENT_LIST);
					goto error;
				}
			}
			if (function->type == T_VOID)
				builder.addStatement(new VoidCall(function->name, args));
			else
				builder.addStatement(new FunctCall(function->name, function->type, args));
			token = tokenizer.nextToken();
		}
		else
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNDEFINED_IDENTIFIER, identifier);
			goto error;
		}
		break;
	case SCANF:
		parseScanf();
		break;
	case PRINTF:
		parsePrintf();
		break;
	case SWITCH:
		parseSwitch();
		return;
	case RTN:
		parseReturn();
		break;
	case lBRACE:
		token = tokenizer.nextToken();
		parseStatements();
		if (token != rBRACE)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_BRACE_EXPECTED);
		}
		token = tokenizer.nextToken();
		return;
	}
	if (token != SEMICOLON)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), SEMICOLON_EXPECTED);
	}
error:
	skipToToken(SEMICOLON);
	token = tokenizer.nextToken();
}

void Parser::parseStatements()
{
	do
	{
		parseStatement();
	} while (stmtBeginSet.find(token) != stmtBeginSet.end());
}

void Parser::parseCompStatement()
{
	while (token == CONST)
		parseConstDeclare();
	while (token == INTSYM || token == CHARSYM)
	{
		type = (token == INTSYM) ? T_INT : T_CHAR;
		if ((token = tokenizer.nextToken()) != IDENT)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
			///TODO skip
			skipToToken(SEMICOLON);
			token = tokenizer.nextToken();
			continue;
		}
		identifier = tokenizer.getIdent();
		token = tokenizer.nextToken();
		parseVarDeclare();
	}
	if (token == rBRACE)
		return;
	if (stmtBeginSet.find(token) == stmtBeginSet.end())
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
		///todo skip
		skipToToken(stmtBeginSet);
	}
	parseStatements();
}

/**
	before: caller must guarentee that a { or ( has been readin
*/
void Parser::parseFunction()
{
	haveReadFunction = true;
	if (token == lBRACE)
	{
		if (!builder.createFunc(type, identifier))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
			///todo skip
		}
		token = tokenizer.nextToken();
		parseCompStatement();
		if (token != rBRACE)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_BRACE_EXPECTED);
			///TODO skip
		}
		token = tokenizer.nextToken();
	}
	else if (token == lPARE)
	{
		if (!builder.createFunc(type, identifier))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
			///todo skip
		}
		// main function
		if (identifier == "main")
		{
			haveReadMain = true;
			if ((token = tokenizer.nextToken()) != rPARE)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
				///todo skip
			}
			token = tokenizer.nextToken();
			goto parse_body;
		}
		// read args here
		token = tokenizer.nextToken();
		if (token == rPARE)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
			///TODO skip
		}
		else
		{
			while (1)
			{
				if (token == rPARE)
					break;
				if (token != INTSYM && token != CHARSYM)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
					skipToToken(rPARE);
					continue;
				}
				type = (token == INTSYM) ? T_INT : T_CHAR;
				if ((token = tokenizer.nextToken()) != IDENT)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
					skipToToken(rPARE);
				}
				identifier = tokenizer.getIdent();
				token = tokenizer.nextToken();
				if (token == rPARE || token == COMMA)
				{
					if (!builder.createArg(type, identifier))
					{
						error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
						///todo skip
					}
					if (token == rPARE)
					{
						token = tokenizer.nextToken();
						break;
					}
					token = tokenizer.nextToken();
				}
				else //error
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
					skipToToken(rPARE);
				}
			} // while
		}
	parse_body:
		if (token != lBRACE)
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);

		token = tokenizer.nextToken();
		parseCompStatement();
		if (token != rBRACE)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_BRACE_EXPECTED);
			///TODO skip
		}
		token = tokenizer.nextToken();
		if (haveReadMain)
			throw 1;

	}
	else //error
	{
		skipToToken(typeSet);
	}
}

void Parser::parseConstDeclare()
{
	if (token != CONST)
	{
		std::cout << "???\n";
		return;
	}
	token = tokenizer.nextToken();
	if (token != INTSYM && token != CHARSYM)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), TYPE_IDENTIFIER_EXPECTED);
		///todo skip
		goto end;
	}
	type = (token == INTSYM ? T_INT : T_CHAR);
	do
	{
		if ((token = tokenizer.nextToken()) != IDENT)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
			///TODO skip
			goto end;
		}
		identifier = tokenizer.getIdent();
		if (!match(BECOME))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), EQUAL_EXPECTED);
			///TODO skip
			goto end;
		}
		if (type == T_INT)
		{
			int sign = 1;
			bool readSign = false;
			token = tokenizer.nextToken();
			if (token == PLUS || token == MINUS)
			{
				readSign = true;
				if (token == MINUS)
					sign = -1;
				token = tokenizer.nextToken();
			}
			if (token != NUM)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), NUM_EXPECTED);
				///TODO skip
				goto end;
			}
			if (readSign && tokenizer.getNum() == 0)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_SIGN);
				///TODO skip
				goto end;
			}

			if (!builder.createConstDecalre(type, identifier, sign * tokenizer.getNum()))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
				///todo skip
			}
		}
		else
		{
			if (!match(ALPHA))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), ALPHA_EXPECTED);
				///TODO skip
				goto end;
			}
			if (!builder.createConstDecalre(type, identifier, tokenizer.getNum()))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
				///todo skip
			}
		}
	} while ((token = tokenizer.nextToken()) == COMMA);
	if (token != SEMICOLON)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), SEMICOLON_EXPECTED);
		///TODO skip
	}
end:
	skipToToken(SEMICOLON);
	token = tokenizer.nextToken();
}

void Parser::parseVarDeclare()
{
	// caller must guarentee that an identifier has been readin
	if (token == COMMA)
	{
		if (!builder.createVarDeclare(type, identifier))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
			///TODO skip
		}
		if ((token = tokenizer.nextToken()) != IDENT)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
			///TODO skip
			goto end;
		}
		identifier = tokenizer.getIdent();
		token = tokenizer.nextToken();
		parseVarDeclare();
	}
	else if (token == lBRACK)
	{
		// parse array
		token = tokenizer.nextToken();
		arraySize = tokenizer.getNum();
		if (token != NUM || arraySize <= 0)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), POSITIVE_INT_EXPECTED);
			///TODO skip
			goto end;
		}
		if ((token = tokenizer.nextToken()) != rBRACK)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
			///TODO skip
			goto end;
		}
		token = tokenizer.nextToken();
		if (token == COMMA || token == SEMICOLON)
		{
			if (!builder.createVarDeclare(type, identifier, arraySize))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
				///TODO skip
			}
			if (token == COMMA)
			{
				if ((token = tokenizer.nextToken()) != IDENT)
				{
					error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
					///TODO skip
					goto end;
				}
				identifier = tokenizer.getIdent();
				token = tokenizer.nextToken();
				parseVarDeclare();
			}
			else
				token = tokenizer.nextToken();
		}
	}
	else if (token == SEMICOLON)
	{
		if (!builder.createVarDeclare(type, identifier))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
			///TODO skip
			goto end;
		}
		token = tokenizer.nextToken();
	}
	else //error
	{
	end:
		skipToToken(SEMICOLON);
		token = tokenizer.nextToken();
	}
}

void Parser::parseVarAndFuncDeclare()
{
	if (token == INTSYM || token == CHARSYM)
	{
		type = (token == INTSYM) ? T_INT : T_CHAR;
		if ((token = tokenizer.nextToken()) != IDENT)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
			///TODO skip
			goto error;
		}
		identifier = tokenizer.getIdent();
		token = tokenizer.nextToken();
		if (token == lPARE || token == lBRACE) // function declare
		{
			if (identifier == "main")
			{
				///todo  wrong main function type
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), WRONG_MAIN_TYPE);
			}
			parseFunction();
		}
		else if (token == COMMA || token == SEMICOLON || token == lBRACK)
		{
			if (haveReadFunction)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_VAR_DECLARE);
				///todo skip
			}
			parseVarDeclare();
		}
		else //error
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
			///TODO skip
			goto error;
		}
	}
	else if (token == VOID)
	{
		type = T_VOID;
		token = tokenizer.nextToken();
		if (token != IDENT && token != MAIN)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
			///TODO skip
			goto error;
		}
		identifier = tokenizer.getIdent();
		token = tokenizer.nextToken();
		if (token != lPARE && token != lBRACE)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
			///todo skip
			goto error;
		}
		parseFunction();
	}
	else //error
	{
	error:
		skipToToken(typeSet);
	}
}

void Parser::parseSwitch()
{
	if (token != SWITCH)
		std::cout << "???\n";
	std::cout << "this is a switch statement\n";
	int n;
	char c;
	if (!match(lPARE))
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
		///todo skip
	}
	token = tokenizer.nextToken();
	auto cond = parseExpression();
	if (cond == nullptr)
	{
		///todo skip
		return;
	}
	if (token != rPARE)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
		///todo skip
	}
	if (!match(lBRACE))
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_BRACE_EXPECTED);
		///todo skip
	}
	Type caseType = cond->type;
	if (!match(CASE))
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), CASE_EXPECTED);
		///todo skip
	}
	unordered_set<int> caseValues;
	vector<Label *> caseBodies;
	vector<Label *> caseCmps;
	auto lastBasicBlock = builder.getLastBasicBlock();
	auto nextBasicBlock = builder.createBasicBlock();
	auto nextLabel = new Label(nextBasicBlock);

	do
	{
		token = tokenizer.nextToken();
		int sign = 1;
		Constant *cmpConst;
		switch (token)
		{
		case MINUS: //int
			sign = -1;
		case PLUS:
			if (!match(NUM))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), NUM_EXPECTED);
			}
		case NUM:
			if (caseType == T_CHAR)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), ALPHA_EXPECTED);
			}
			n = sign * tokenizer.getNum();
			if (caseValues.find(n) != caseValues.end())
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), CASE_VALUE_ALREADY_EXISTS);
			}
			caseValues.insert(n);
			cmpConst = new Constant(n);
			break;
		case ALPHA:
			if (caseType == T_INT)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), NUM_EXPECTED);
			}
			c = tokenizer.getNum();
			if (caseValues.find(c) != caseValues.end())
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), CASE_VALUE_ALREADY_EXISTS);
			}
			caseValues.insert(c);
			cmpConst = new Constant(c);
			break;
		default:
			/// error
			break;
		}

		if (!match(COLON))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), COLON_EXPECTED);
		}

		auto caseBody = builder.createBasicBlock();
		auto bodyLabel = new Label(caseBody);
		caseBodies.push_back(bodyLabel);

		auto caseCmp = builder.createBasicBlock();
		auto cmpLabel = new Label(caseCmp);
		caseCmp->addu(new CmpBr(Op_BEQ, cond, cmpConst, bodyLabel)); // branch to case body if equal
		caseCmps.push_back(cmpLabel);

		builder.setInsertPoint(caseBody);
		token = tokenizer.nextToken();
		parseStatement();
		builder.createGoto(nextLabel); // create goto to outside switch

	} while (token == CASE);

	if (token == DEFAULT)
	{
		if (!match(COLON))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), COLON_EXPECTED);
		}

		auto caseBody = builder.createBasicBlock();
		auto bodyLabel = new Label(caseBody);
		caseBodies.push_back(bodyLabel);

		auto caseCmp = builder.createBasicBlock();
		auto cmpLabel = new Label(caseCmp);
		caseCmp->addu(new Goto(bodyLabel)); // goto default body
		caseCmps.push_back(cmpLabel);

		builder.setInsertPoint(caseBody);
		token = tokenizer.nextToken();
		parseStatement();
		builder.createGoto(nextLabel);
	}
	if (token != rBRACE)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_BRACE_EXPECTED);
	}

	for (auto i = caseCmps.begin(); i != caseCmps.end(); i++)
		builder.setInsertPoint((*i)->controller);
	builder.setInsertPoint(nextLabel->controller);

	lastBasicBlock->addu(new Goto(caseCmps.front()));

	token = tokenizer.nextToken();
}

void Parser::parseIf()
{
	if (token != IF)
		std::cout << "???\n";
	std::cout << "this is an if statement\n";
	if (!match(lPARE))
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
		goto error;
	}
	token = tokenizer.nextToken();
	Value *cond1 = parseExpression();
	Value *cond2 = nullptr;

	if (cond1 == nullptr)
		goto error;

	Token cmpToken = TK_NULL;
	if (cmpSet.find(token) != cmpSet.end())
	{
		cmpToken = token;
		token = tokenizer.nextToken();
		cond2 = parseExpression();
		if (cond2 == nullptr)
			goto error;
	}
	if (token != rPARE)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
		///todo skip
	}
	auto Then = builder.createBasicBlock();
	auto Else = builder.createBasicBlock();
	if (cmpToken == TK_NULL)
		builder.createCmpBr(cond1, Then, Else);
	else
		builder.createCmpBr(cmpToken, cond1, cond2, Then, Else);

	// parse then block
	token = tokenizer.nextToken();
	if (stmtBeginSet.find(token) == stmtBeginSet.end())
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
		goto error;
	}
	builder.setInsertPoint(Then);
	parseStatement();

	// parse else block
	if (token != ELSE)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), ELSE_EXPECTED);
		goto error;
	}
	token = tokenizer.nextToken();
	if (stmtBeginSet.find(token) == stmtBeginSet.end())
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
		goto error;
	}
	builder.setInsertPoint(Else);
	parseStatement();
	return;
error:
	token = tokenizer.nextToken();
}

void Parser::parseWhile()
{
	if (token != WHILE)
		std::cout << "???\n";
	std::cout << "this is a while loop\n";
	if (!match(lPARE))
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
		goto error;
	}

	auto cond = builder.createBasicBlock();
	auto body = builder.createBasicBlock();
	auto next = builder.createBasicBlock();

	builder.setInsertPoint(cond);
	token = tokenizer.nextToken();
	Value *cond1 = parseExpression();
	Value *cond2 = nullptr;

	if (cond1 == nullptr)
		goto error;

	Token cmpToken = TK_NULL;
	if (cmpSet.find(token) != cmpSet.end())
	{
		cmpToken = token;
		token = tokenizer.nextToken();
		cond2 = parseExpression();
		if (cond2 == nullptr)
			goto error;
	}
	if (token != rPARE)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
		goto error;
	}


	if (cmpToken == TK_NULL)
		builder.createCmpBr(cond1, body, next);
	else
		builder.createCmpBr(cmpToken, cond1, cond2, body, next);

	//parse body
	token = tokenizer.nextToken();
	if (stmtBeginSet.find(token) == stmtBeginSet.end())
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
		goto error;
	}
	builder.setInsertPoint(body);
	parseStatement();
	builder.createGoto(new Label(cond));

	builder.setInsertPoint(next);
	return;
error:
	token = tokenizer.nextToken();
}

void Parser::parseScanf()
{
	std::cout << "this is a scanf\n";
	if (!match(lPARE))
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
	vector<Var *> args;
	do
	{
		if (!match(IDENT))
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
			skipToToken(COMMA, rPARE);
			continue;
		}
		identifier = tokenizer.getIdent();
		const TableElement *te;
		if ((te = builder.lookup(identifier)) == nullptr)
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNDEFINED_IDENTIFIER, identifier);
		if (te->kind == K_CONST || te->kind == K_ARRAY)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), VARIABLE_EXPECTED);
			skipToToken(COMMA, rPARE);
			continue;
		}
		args.push_back(new Var(identifier, te->type));
		token = tokenizer.nextToken();
	} while (token == COMMA);
	if (token != rPARE)
	{
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
		skipToToken(SEMICOLON);
		return;
	}
	builder.addStatement(new Scanf(args));
	token = tokenizer.nextToken();
}

void Parser::parsePrintf()
{
	std::cout << "this is a printf\n";
	if (!match(lPARE))
		error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
	int strIndex;
	Value *val;
	token = tokenizer.nextToken();
	if (token == STR)
	{
		strIndex = builder.addString(tokenizer.getStr());
		if (match(COMMA))
		{
			token = tokenizer.nextToken();
			val = parseExpression();
			if (val == nullptr)
				skipToToken(rPARE);
			if (token != rPARE)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
				skipToToken(SEMICOLON);
				return;
			}
			builder.addStatement(new Printf(strIndex, val));
		}
		else if (token == rPARE)
		{
			builder.addStatement(new Printf(strIndex));
		}
		else
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
			skipToToken(SEMICOLON);
			return;
		}
	}
	else
	{
		val = parseExpression();
		if (val == nullptr)
		{
			skipToToken(SEMICOLON);
			return;
		}
		if (token != rPARE)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
			skipToToken(SEMICOLON);
			return;
		}
		builder.addStatement(new Printf(val));
	}
	token = tokenizer.nextToken();
}

void Parser::parseProgram()
{
	try
	{
		token = tokenizer.nextToken();
		while (token == CONST)
		{
			parseConstDeclare();
		}
		while (token == INTSYM || token == CHARSYM || token == VOID)
		{
			parseVarAndFuncDeclare();
		}
	}
	catch (string &e)
	{
		if (token != TK_EOF)
			std::cout << "redundant\n";
	}
	catch (int e)
	{
		if (token != TK_EOF)
			std::cout << "redundant\n";
	}
}

void Parser::parseReturn()
{
	std::cout << "this is a return statement\n";
	const Function *func = builder.getCurrentFunction();
	token = tokenizer.nextToken();
	if (func->type == T_VOID)
	{
		if (token != SEMICOLON)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), INVALID_RETURN);
			skipToToken(SEMICOLON);
		}
		builder.addStatement(new Return());
	}
	else
	{
		Value *val;
		if (token == lPARE)
		{
			token = tokenizer.nextToken();
			val = parseExpression();
			if (val == nullptr)
			{
				skipToToken(SEMICOLON);
				return;
			}
			if (token != rPARE)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), RIGHT_PARENTHESES_EXPECTED);
				skipToToken(SEMICOLON);
				return;

			}
			if (func->type == T_CHAR && val->type == T_INT)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNMATCHED_RETURN_TYPE);
				skipToToken(SEMICOLON);
				return;
			}
			builder.addStatement(new Return(val));
		}
		else
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), LEFT_PARENTHESES_EXPECTED);
			skipToToken(SEMICOLON);
			return;
		}
		token = tokenizer.nextToken();
	}
}

void Parser::startParse()
{
}
