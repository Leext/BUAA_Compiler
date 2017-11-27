#pragma once
#include "stdafx.h"
enum ErrorType
{
	IDENTIFIER_EXPECTED,UNEXPECTED_TOKEN,POSITIVE_INT_EXPECTED,IDENTIFIER_ALREADY_DEFINED,
};
class ErrorHandler
{
protected:
	std::ostream& err;
	std::unordered_map<ErrorType, std::string> errorInfo;
public:
	ErrorHandler(std::ostream& err = std::cerr) : err(err)
	{
		errorInfo[IDENTIFIER_EXPECTED] = "identifier expected here";
		errorInfo[UNEXPECTED_TOKEN] = "unexpected token here";
		errorInfo[POSITIVE_INT_EXPECTED] = "expect a positive integer here";
		errorInfo[IDENTIFIER_ALREADY_DEFINED] = "already defined identifier : ";
	}
	void report(int lineNum, const std::string& line, ErrorType type)
	{
		err << "error at line " << lineNum << " : " << line << "  " << errorInfo[type] << std::endl;
	}
	void report(int lineNum, const std::string& line, ErrorType type, std::string& ident)
	{
		err << "error at line " << lineNum << " : " << line << "  " << errorInfo[type] << ident << std::endl;
	}
};

