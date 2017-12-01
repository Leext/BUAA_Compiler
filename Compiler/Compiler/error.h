#pragma once
#include "stdafx.h"
enum ErrorType
{
	IDENTIFIER_EXPECTED,
	UNEXPECTED_TOKEN,
	POSITIVE_INT_EXPECTED,
	IDENTIFIER_ALREADY_DEFINED,
	UNEXPECTED_VAR_DECLARE,
	RIGHT_PARENTHESES_EXPECTED,
	RIGHT_BRACE_EXPECTED,
	TYPE_IDENTIFIER_EXPECTED,
	EQUAL_EXPECTED,
	NUM_EXPECTED,
	ALPHA_EXPECTED,
	SEMICOLON_EXPECTED,
	LEFT_PARENTHESES_EXPECTED,
	UNEXPECTED_SIGN,
	ELSE_EXPECTED,
	LEFT_BRACKET_EXPECTED,
	RIGHT_BRACKET_EXPECTED,
	UNEXPECTED_VOID_FUNCTION_CALL,
	LEFT_BRACE_EXPECTED,
	CASE_EXPECTED,
	CASE_VALUE_ALREADY_EXISTS,
	COLON_EXPECTED,
	ASSIGN_EXPECTED,
	UNDEFINED_IDENTIFIER,
	VARIABLE_EXPECTED,
	INVALID_RETURN,
	UNMATCHED_RETURN_TYPE,
	UNMATCHED_ARGUMENT_LIST
};
class ErrorHandler
{
  protected:
	std::ostream &err;
	std::unordered_map<ErrorType, std::string> errorInfo;

  public:
	ErrorHandler(std::ostream &err = std::cerr) : err(err)
	{
		errorInfo[IDENTIFIER_EXPECTED] = "identifier expected here";
		errorInfo[UNEXPECTED_TOKEN] = "unexpected token here";
		errorInfo[POSITIVE_INT_EXPECTED] = "expect a positive integer here";
		errorInfo[IDENTIFIER_ALREADY_DEFINED] = "already defined identifier : ";
		errorInfo[UNEXPECTED_VAR_DECLARE] = "unexpected var declaration here";
		errorInfo[RIGHT_PARENTHESES_EXPECTED] = "expect right parentheses here";
		errorInfo[RIGHT_BRACE_EXPECTED] = "expect right brace here";
		errorInfo[TYPE_IDENTIFIER_EXPECTED] = "expect type identifier";
		errorInfo[EQUAL_EXPECTED] = "expect equal";
		errorInfo[NUM_EXPECTED] = "expect number here";
		errorInfo[ALPHA_EXPECTED] = "expect alphabet here";
		errorInfo[SEMICOLON_EXPECTED] = "expect semicolon here";
		errorInfo[LEFT_PARENTHESES_EXPECTED] = "expect left parentheses";
		errorInfo[UNEXPECTED_SIGN] = "unexpected sign";
		errorInfo[ELSE_EXPECTED] = "expect else here";
		errorInfo[LEFT_BRACKET_EXPECTED] = "expect left bracket here";
		errorInfo[RIGHT_BRACKET_EXPECTED] = "expect right bracket here";
		errorInfo[UNEXPECTED_VOID_FUNCTION_CALL] = "unexpected void function call here";
		errorInfo[LEFT_BRACE_EXPECTED] = "expect left brace here";
		errorInfo[CASE_EXPECTED] = "expect case here";
		errorInfo[CASE_VALUE_ALREADY_EXISTS] = "case value already exists";
		errorInfo[COLON_EXPECTED] = "expect colon here";
		errorInfo[ASSIGN_EXPECTED] = "expect assign symbol here";
		errorInfo[UNDEFINED_IDENTIFIER] = "undefined identifier : ";
		errorInfo[VARIABLE_EXPECTED] = "expect a variable here";
		errorInfo[INVALID_RETURN] = "invalid return statement";
		errorInfo[UNMATCHED_RETURN_TYPE] = "return type does not match";
		errorInfo[UNMATCHED_ARGUMENT_LIST] = "argument list dose not match";
	}

	void report(int lineNum, const std::string &line, ErrorType type)
	{
		err << "error at line " << lineNum << " : " << line << "  " << errorInfo[type] << std::endl;
	}
	void report(int lineNum, const std::string &line, ErrorType type, std::string &ident)
	{
		err << "error at line " << lineNum << " : " << line << "  " << errorInfo[type] << ident << std::endl;
	}
};
