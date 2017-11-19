#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include "Tokenizer.h"
using std::string;
using std::unordered_map;
void test()
{
	unordered_map<Symbol, string> sym2str;
	sym2str[PLUS] = "+";
	sym2str[MINUS] = "-";
	sym2str[MULT] = "*";
	sym2str[DIV] = "/";
	sym2str[BECOME] = "=";
	sym2str[COMMA] = ",";
	sym2str[COLON] = ":";
	sym2str[SEMICOLON] = ";";
	sym2str[lBRACE] = "{";
	sym2str[rBRACE] = "}";
	sym2str[lBRACK] = "[";
	sym2str[rBRACK] = "]";
	sym2str[lPARE] = "(";
	sym2str[rPARE] = ")";
	sym2str[LEQ] = "<=";
	sym2str[LESS] = "<";
	sym2str[GEQ] = ">=";
	sym2str[GRT] = ">";
	sym2str[CONST] = "const";
	sym2str[INTSYM] = "int";
	sym2str[CHARSYM] = "char";
	sym2str[VOID] = "void";
	sym2str[IF] = "if";
	sym2str[ELSE] = "else";
	sym2str[WHILE] = "while";
	sym2str[SWITCH] = "switch";
	sym2str[CASE] = "case";
	sym2str[DEFAULT] = "default";
	sym2str[RTN] = "return";
	sym2str[MAIN] = "main";
	std::fstream is = std::fstream("test.txt", std::fstream::in);
	Tokenizer tokenizer = Tokenizer(is);
	Symbol sym;
	int num;
	string ident;
	while (1)
	{
		sym = tokenizer.nextSym();
		std::cout << "sym " << sym << "  ";
		switch (sym)
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
			std::cout << sym2str[sym] << std::endl;
		}
	}
}