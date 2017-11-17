#include "Tokenizer.h"
#include <cctype>
Symbol Tokenizer::nextSym()
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
			sym = char2sym[c];
			return sym;
		case '<':
			if (nextChar() == '=')
				sym = LEQ;
			else
			{
				backChar();
				sym = LESS;
			}
			return sym;
		case '>':
			if (nextChar() == '=')
				sym = GEQ;
			else
			{
				backChar();
				sym = GRT;
			}
			return sym;
		case '\'':  //read char
			nextChar();
			sym = ALPHA;
			if (isChar(c))
			{
				num = c;
				if (nextChar() == '\'')
					return sym;
				else
					error();
			}
			else
				error();
			num = 0;
			return sym;
		case '\"':  //read string
			str.clear();
			for (nextChar(); isStrChar(c); nextChar())
				str.push_back(c);
			sym = STR;
			if (c != '\"')
			{
				error();
				num = 0;
			}
			return sym;
		case '/':
            nextChar();
			if (c == '/')  //one line comment
			{
                try
                {
				while (nextChar() != '\n');
                }catch(std::exception &e)
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
				sym = DIV;
				backChar();
			}
			return sym;
		case '\n':
		case '\r':
			goto start;
		default:
			if (isdigit(c)) // is non-zero digit
			{
				num = c - '0';
				sym = NUM;
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
					ident.push_back(c);
				backChar();
				if (str2sym.find(ident) != str2sym.end())
					sym = str2sym[ident];
				else
					sym = IDENT;
			}
			else
			{
				error();
				printf("%c\n", c);
				goto start;
			}
			return sym;

		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
        system("pause");
	}
}