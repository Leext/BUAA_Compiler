#include "stdafx.h"
#include "Tokenizer.h"
Token Tokenizer::nextToken()
{
	try
	{
	start:
		nextChar();
		while (c == ' ' || c == '\t')
			nextChar();
		switch (c)
		{
		case '+':
		case '-':
		case '*':
		case '=':
		case ',':
		case ':':
		case ';':
		case '{':
		case '}':
		case '[':
		case ']':
		case '(':
		case ')':
			return token = char2token[c];
		case '<':
			if (nextChar() == '=')
				token = LEQ;
			else
			{
				backChar();
				token = LESS;
			}
			return token;
		case '>':
			if (nextChar() == '=')
				token = GEQ;
			else
			{
				backChar();
				token = GRT;
			}
			return token;
		case '\'':  //read char
			nextChar();
			token = ALPHA;
			if (isChar(c))
			{
				num = c;
				if (nextChar() == '\'')
					return token;
				else
					error();
			}
			else
				error();
			num = 0;
			return token;
		case '\"':  //read string
			str.clear();
			for (nextChar(); isStrChar(c); nextChar())
				str.push_back(c);
			token = STR;
			if (c != '\"')
			{
				error();
				num = 0;
			}
			return token;
		case '/':
			nextChar();
			if (c == '/')  //one line comment
			{
				try
				{
					while (nextChar() != '\n');
				}
				catch (std::exception &e)
				{
				}
				goto start;
			}
			else if (c == '*')  //multi lines comment
			{
				do
				{
					while (nextChar() != '*');
				} while (nextChar() != '/');
				goto start;
			}
			else
			{
				token = DIV;
				backChar();
			}
			return token;
		case '\n':
		case '\r':
			goto start;
		default:
			if (isdigit(c)) // is non-zero digit
			{
				num = c - '0';
				token = NUM;
				if (c == '0')
				{
					if (isdigit(nextChar()))
						error();
				}
				else
				{
					while (isdigit(nextChar()))
						num = num * 10 + c - '0';
				}
				backChar();
			}
			else if (isAlpha(c))     // read keys and identifiers
			{
				ident.clear();
				ident.push_back(c);
				for (nextChar(); isAlpha(c) || isdigit(c); nextChar())
					ident.push_back(tolower(c));
				backChar();
				if (str2token.find(ident) != str2token.end())
					token = str2token[ident];
				else
					token = IDENT;
			}
			else
			{
				error();
				printf("%c\n", c);
				goto start;
			}
			return token;

		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
		system("pause");
	}
    return token;
}