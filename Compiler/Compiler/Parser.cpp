#include "StdAfx.h"
#include "Parser.h"
#include "Tokenizer.h"


void Parser::skipToToken(Token tk)
{
	while (tokenizer.nextToken() != tk);
	token = tokenizer.getToken();
}

void Parser::skipToToken(unordered_set<Token>& skipSet)
{
	while (skipSet.find(tokenizer.nextToken()) != skipSet.end());
	token = tokenizer.getToken();
}

Value * Parser::parseExpression()
{
	return nullptr;
}

Value * Parser::parseTerm()
{
	return nullptr;
}

Value * Parser::parseFactor()
{
	return nullptr;
}



void Parser::parseStatement()
{
}

void Parser::parseFunction()
{
}

void Parser::parseConstDeclare()
{
}

void Parser::parseVarDeclare(bool global)
{
	if (global)  // global var declare
	{
		// caller must guarentee that an identifier has been readin
		if (token == COMMA)
		{
			if (!builder.createGlobalVarDeclare(type, identifier))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
				///TODO skip
			}
			if ((token = tokenizer.nextToken()) != IDENT)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
				///TODO skip
			}
			identifier = tokenizer.getIdent();
			token = tokenizer.nextToken();
			parseVarDeclare(true);
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
			}
			if ((token = tokenizer.nextToken()) != rBRACK)
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
				///TODO skip
			}
			token = tokenizer.nextToken();
			if (token == COMMA || token == SEMICOLON)
			{
				if (!builder.createGlobalVarDeclare(type, identifier, arraySize))
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
					}
					token = tokenizer.nextToken();
					parseVarDeclare(true);
				}
				else
					token = tokenizer.nextToken();
			}
		}
		else if (token == SEMICOLON)
		{
			if (!builder.createGlobalVarDeclare(type, identifier))
			{
				error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_ALREADY_DEFINED, identifier);
				///TODO skip
			}
			token = tokenizer.nextToken();
		}
		else //error
		{

		}
	}
	else  // local var declare
	{

	}

	//skip:
	//	auto& skipSet = unordered_set<Token>();
	//	skipSet.insert(INTSYM);
	//	skipSet.insert(CHARSYM);
	//	skipSet.insert(VOID);
	//	skipToToken(skipSet);
}



void Parser::parseVarAndFuncDeclare()
{
start:
	if (token == INTSYM || token == CHARSYM)
	{
		type = (token == INTSYM) ? T_INT : T_CHAR;
		if ((token = tokenizer.nextToken()) != IDENT)
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), IDENTIFIER_EXPECTED);
			auto& skipSet = unordered_set<Token>();
			skipSet.insert(INTSYM);
			skipSet.insert(CHARSYM);
			skipSet.insert(VOID);
			skipToToken(skipSet);
			goto start;
		}
		identifier = tokenizer.getIdent();
		token = tokenizer.nextToken();
		if (token == lPARE || token == lBRACE)  // function declare
		{

		}
		else if (token == COMMA || token == SEMICOLON || token == lBRACK)
			parseVarDeclare(true);
		else  //error
		{
			error.report(tokenizer.getLineCount(), tokenizer.getLine(), UNEXPECTED_TOKEN);
			///TODO skip
		}
	}
	else if (token == VOID)
	{

	}
	else  //error
	{

	}

}

void Parser::parseSwitch()
{
}

void Parser::parseIf()
{
}

void Parser::parseWhile()
{
}

void Parser::parseScanf()
{
}

void Parser::parsePrintf()
{
}

void Parser::parseProgram()
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

void Parser::startParse()
{
}
