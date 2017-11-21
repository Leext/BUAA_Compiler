#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include "Tokenizer.h"
using std::string;
using std::unordered_map;
void test()
{
	unordered_map<Token, string> token2str;
	token2str[PLUS] = "+";
	token2str[MINUS] = "-";
	token2str[MULT] = "*";
	token2str[DIV] = "/";
	token2str[BECOME] = "=";
	token2str[COMMA] = ",";
	token2str[COLON] = ":";
	token2str[SEMICOLON] = ";";
	token2str[lBRACE] = "{";
	token2str[rBRACE] = "}";
	token2str[lBRACK] = "[";
	token2str[rBRACK] = "]";
	token2str[lPARE] = "(";
	token2str[rPARE] = ")";
	token2str[LEQ] = "<=";
	token2str[LESS] = "<";
	token2str[GEQ] = ">=";
	token2str[GRT] = ">";
	token2str[CONST] = "const";
	token2str[INTSYM] = "int";
	token2str[CHARSYM] = "char";
	token2str[VOID] = "void";
	token2str[IF] = "if";
	token2str[ELSE] = "else";
	token2str[WHILE] = "while";
	token2str[SWITCH] = "switch";
	token2str[CASE] = "case";
	token2str[DEFAULT] = "default";
	token2str[RTN] = "return";
	token2str[MAIN] = "main";
	std::fstream is = std::fstream("test.txt", std::fstream::in);
	Tokenizer tokenizer = Tokenizer(is);
	Token token;
	int num;
	string ident;
	while (1)
	{
		token = tokenizer.nextToken();
		if (token == TK_EOF)
			break;
		if (token == ERROR)
			continue;
		std::cout << "token " << token << "  ";
		switch (token)
		{
		case IDENT:
			std::cout << "ident " << tokenizer.getIdent() << std::endl;
			break;
		case NUM:
			std::cout << "num " << tokenizer.getNum() << std::endl;
			break;
		case ALPHA:
			std::cout << "char " << (char)tokenizer.getNum() << std::endl;
			break;
		case STR:
			std::cout << "str " << tokenizer.getStr() << std::endl;
			break;
		default:
			std::cout << token2str[token] << std::endl;
		}
	}
	system("pause");
}