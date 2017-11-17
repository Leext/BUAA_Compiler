#pragma once
#include <string>
#include <iostream>
#include <cctype>
#include <unordered_map>
#include <fstream>
enum Symbol
{
	PLUS, MINUS, MULT, DIV,
	LESS, LEQ, GRT, GEQ, NEQ, EQU,
	BECOME,
	ALPHA, NUM, STR,
	CONST, INTSYM, CHARSYM, VOID,
	COMMA, SEMICOLON, COLON,
	lPARE, rPARE,
	lBRACK, rBRACK,
	lBRACE, rBRACE,
	IF, ELSE,
	WHILE,
	SWITCH, CASE, DEFAULT,
	RTN,
	SCANF, PRINTF,
	MAIN,
	IDENT
};
using std::string;
using std::fstream;
using std::unordered_map;
class Tokenizer
{
public:
	Tokenizer(fstream& inputStream) : inputStream(inputStream)
	{
		//initialize map from single char to symbol
		char2sym['+'] = PLUS;
		char2sym['-'] = MINUS;
		char2sym['*'] = MULT;
		char2sym['/'] = DIV;
		char2sym['='] = BECOME;
		char2sym[','] = COMMA;
		char2sym[':'] = COLON;
		char2sym[';'] = SEMICOLON;
		char2sym['{'] = lBRACE;
		char2sym['}'] = rBRACE;
		char2sym['['] = lBRACK;
		char2sym[']'] = rBRACK;
		char2sym['('] = lPARE;
		char2sym[')'] = rPARE;
		//initialize map from string to symbol
		str2sym["const"] = CONST;
		str2sym["int"] = INTSYM;
		str2sym["char"] = CHARSYM;
		str2sym["void"] = VOID;
		str2sym["if"] = IF;
		str2sym["else"] = ELSE;
		str2sym["while"] = WHILE;
		str2sym["switch"] = SWITCH;
		str2sym["case"] = CASE;
		str2sym["default"] = DEFAULT;
		str2sym["return"] = RTN;
		str2sym["main"] = MAIN;
		lineCount = 1;
	};
	Symbol nextSym();
	const string& getIdent()
	{
		return ident;
	}
	int getNum()
	{
		return num;
	}
	const string& getStr()
	{
		return str;
	}
protected:
	Symbol sym;
	string ident;
	int num;
	int c;
	string str;
	fstream& inputStream;
	string line;
	unordered_map<char, Symbol> char2sym;
	unordered_map<string, Symbol> str2sym;
	int lineCount;
	int nextChar()
	{
		if ((c = inputStream.get()) != EOF)
		{
			if (c == '\n')
			{
				lineCount++;
				line.clear();
			}
			else
				line.push_back(c);
			return c;
		}
		throw std::exception("eof");
	}
	void backChar()
	{
		if (c != '\n')
		{
			line.pop_back();
			inputStream.unget();
		}
	}
	void error()
	{
		std::cout << "error at line " << lineCount << " : " << line << std::endl;
	}
	bool isStrChar(char c)
	{
		return (c >= 35 && c <= 126) || c == 32 || c == 33;
	}
	bool isChar(char c)
	{
		return (c >= 65 && c <= 90 || c >= 97 && c <= 122) || c == '_' || isdigit(c) || c == '+' || c == '-' || c == '*' || c == '/';
	}
	bool isAlpha(char c)
	{
		return (c >= 65 && c <= 90 || c >= 97 && c <= 122) || c == '_';
	}
};

