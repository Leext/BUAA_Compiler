#include "stdafx.h"
#include "Tokenizer.h"
Token Tokenizer::nextToken()
{
	while (isspace(lastChar))
		nextChar();
	switch (lastChar)
	{
	case '+':
	case '-':
	case '*':
	case ',':
	case ':':
	case ';':
	case '{':
	case '}':
	case '[':
	case ']':
	case '(':
	case ')':
		token = char2token[lastChar];
		nextChar();
		return token;
	case '!':
		if (nextChar() == '=')
		{
			token = NEQ;
			nextChar();
		}
		else
			token = ERROR;
		return token;
	case '=':
		if (nextChar() == '=')
		{
			token = EQU;
			nextChar();
		}
		else
			token = BECOME;
		return token;
	case '<':
		if (nextChar() == '=')
		{
			token = LEQ;
			nextChar();
		}
		else
			token = LESS;
		return token;
	case '>':
		if (nextChar() == '=')
		{
			token = GEQ;
			nextChar();
		}
		else
			token = GRT;
		return token;
	case '\'':		//read char
		nextChar(); // eat '
		if (isChar(lastChar))
		{
			numVal = lastChar;
			if (nextChar() == '\'')
			{
				nextChar(); // eat '
				return token = ALPHA;
			}
		}
		// invalid char
		error();
		skipToChar(';'); // skip to ;
		return token = ERROR;
	case '\"': //read string
		strConst.clear();
		for (nextChar(); isStrChar(lastChar); nextChar())
			strConst.push_back(lastChar);
		if (lastChar != '\"')
		{
			if (lastChar != '\n')
				skipToChar('\n');
			// expect right "
			error();
			return token = ERROR;
		}
		nextChar(); // eat "
		return token = STR;
	case '/':
		nextChar();
		if (lastChar == '/') //one line comment
		{
			skipToChar('\n');
			return nextToken();
		}
		else
			return token = DIV;
	case EOF:
		return token = TK_EOF;
	default:
		if (isdigit(lastChar)) // is non-zero digit
		{
			numVal = lastChar - '0';
			if (lastChar == '0')
			{
				if (isdigit(nextChar()))
				{
					// number with leading zero
					error();
					return token = ERROR;
				}
			}
			else
			{
				while (isdigit(nextChar()))
					numVal = numVal * 10 + lastChar - '0';
			}
			return token = NUM;
		}
		else if (isAlpha(lastChar)) // read keys and identifiers
		{
			identifierStr.clear();
			identifierStr.push_back(tolower(lastChar));
			for (nextChar(); isAlpha(lastChar) || isdigit(lastChar); nextChar())
				identifierStr.push_back(tolower(lastChar));
			if (str2token.find(identifierStr) != str2token.end())
				token = str2token[identifierStr];
			else
				token = IDENT;
			return token;
		}
		else
		{
			// unknown char
			error();
			skipToChar('\n');
			return token = ERROR;
		}
	}
	return token;
}