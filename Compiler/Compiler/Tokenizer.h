#pragma once
#include "stdafx.h"
enum Token
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
		char2token['+'] = PLUS;
		char2token['-'] = MINUS;
		char2token['*'] = MULT;
		char2token['/'] = DIV;
		char2token['='] = BECOME;
		char2token[','] = COMMA;
		char2token[':'] = COLON;
		char2token[';'] = SEMICOLON;
		char2token['{'] = lBRACE;
		char2token['}'] = rBRACE;
		char2token['['] = lBRACK;
		char2token[']'] = rBRACK;
		char2token['('] = lPARE;
		char2token[')'] = rPARE;
		//initialize map from string to symbol
		str2token["const"] = CONST;
		str2token["int"] = INTSYM;
		str2token["char"] = CHARSYM;
		str2token["void"] = VOID;
		str2token["if"] = IF;
		str2token["else"] = ELSE;
		str2token["while"] = WHILE;
		str2token["switch"] = SWITCH;
		str2token["case"] = CASE;
		str2token["default"] = DEFAULT;
		str2token["return"] = RTN;
		str2token["main"] = MAIN;
		lineCount = 1;
		c = 0;
	};
	Token nextToken();
	const string& getIdent() const
	{
		return ident;
	}
	int getNum() const
	{
		return num;
	}
	const string& getStr() const
	{
		return str;
	}
protected:
	Token token;
	string ident;
	int num;
	int c;
	string str;
	fstream& inputStream;
	string line;
	unordered_map<char, Token> char2token;
	unordered_map<string, Token> str2token;
	int lineCount;
	int nextChar()
	{
		if (c == '\n')
		{
			lineCount++;
			line.clear();
		}
		if ((c = inputStream.get()) != EOF)
		{
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

