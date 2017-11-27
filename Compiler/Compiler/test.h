#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include "Tokenizer.h"
#include <iomanip>
#include "global.h"
#include "IRBuilder.h"
#include "error.h"
#include "Parser.h"
using std::string;
using std::unordered_map;
class tester
{
public:
	static void test()
	{
		unordered_map<Token, string> token2str;
		token2str[PLUS] = "PLUS";
		token2str[MINUS] = "MINUS";
		token2str[MULT] = "MULT";
		token2str[DIV] = "DIV";
		token2str[BECOME] = "BECOME";
		token2str[COMMA] = "COMMA";
		token2str[COLON] = "COLON";
		token2str[SEMICOLON] = "SEMICOLON";
		token2str[lBRACE] = "lBRACE";
		token2str[rBRACE] = "rBRACE";
		token2str[lBRACK] = "lBRACK";
		token2str[rBRACK] = "rBRACK";
		token2str[lPARE] = "lPARE";
		token2str[rPARE] = "rPARE";
		token2str[LEQ] = "LEQ";
		token2str[LESS] = "LESS";
		token2str[GEQ] = "GEQ";
		token2str[GRT] = "GRT";
		token2str[CONST] = "CONST";
		token2str[INTSYM] = "INTSYM";
		token2str[CHARSYM] = "CHARSYM";
		token2str[VOID] = "VOID";
		token2str[IF] = "IF";
		token2str[ELSE] = "ELSE";
		token2str[WHILE] = "WHILE";
		token2str[SWITCH] = "SWITCH";
		token2str[CASE] = "CASE";
		token2str[DEFAULT] = "DEFAULT";
		token2str[RTN] = "RETURN";
		token2str[MAIN] = "MAIN";
		std::fstream is = std::fstream("test.txt", std::fstream::in);
		Tokenizer tokenizer = Tokenizer(is);
		Token token;
		string ident;
		int count = 0;
		while (1)
		{
			token = tokenizer.nextToken();
			if (token == TK_EOF)
				break;
			if (token == ERROR)
				continue;
			std::cout << std::setw(3) << count++ << "  ";
			switch (token)
			{
			case IDENT:
				std::cout << std::setw(10) << "IDENT" << std::setw(10) << tokenizer.getIdent() << std::endl;
				break;
			case NUM:
				std::cout << std::setw(10) << "NUM" << std::setw(10) << tokenizer.getNum() << std::endl;
				break;
			case ALPHA:
				std::cout << std::setw(10) << "ALPHA" << std::setw(10) << (char)tokenizer.getNum() << std::endl;
				break;
			case STR:
				std::cout << std::setw(10) << "STRING" << std::setw(10) << tokenizer.getStr() << std::endl;
				break;
			default:
				std::cout << std::setw(10) << token2str[token] << std::endl;
			}
		}
		system("pause");
	}

	static void testParseVarAndFunc()
	{
		std::fstream is = std::fstream("../TestCase/testGlobalVarD.txt", std::fstream::in);
		Tokenizer tokenizer = Tokenizer(is);
		IRBuilder builder = IRBuilder();
		auto errorHandler = ErrorHandler();
		Parser parser = Parser(tokenizer, builder, errorHandler);
		parser.parseProgram();

	}
};