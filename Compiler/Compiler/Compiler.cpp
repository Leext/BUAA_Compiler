// Compiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "test.h"
#include "Parser.h"
bool compile(string &file)
{
	std::fstream is = std::fstream(file, std::fstream::in);
	Tokenizer tokenizer = Tokenizer(is);
	IRBuilder builder = IRBuilder();
	auto errorHandler = ErrorHandler();
	Parser parser = Parser(tokenizer, builder, errorHandler);
	parser.startParse();
	if (!parser.haveError())
	{
		Optimizer opt;
		opt.optimize(builder);
		Generator generator = Generator(&builder);
		generator.generate();
		auto out = std::fstream("../output.txt", std::fstream::out);
		generator.print(out);
		return true;
	}
	return false;
}
void loop()
{
	while (1)
	{
		try
		{
			std::cout << "file to compile :  ";
			string input;
			std::getline(std::cin, input);
			if (compile(input))
				std::cout << "\ncompile successfully!\n";
		}
		catch (...)
		{
		}
	}
}
int main(int argc, char *argv[])
{
	//tester::testParseVarAndFunc();
	//tester::testParser();
	tester::test();
	//loop();

	system("pause");
	return 0;
}
