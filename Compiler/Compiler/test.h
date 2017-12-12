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
#include "Generator.h"
#include "Optimizer.h"
#include <cassert>
using std::string;
using std::unordered_map;
class tester
{
public:
	static void test()
	{
		autoTest(string("test.txt"), string("-16464 0123456789 zyxwvutsrq 0123456789 hello10-cube27 -square-4 self-1 self0 self1 square4 cube27 0123456789 zyxwvutsrq 120"), string("1 -1"));
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
	static void testParser()
	{
		std::fstream is = std::fstream("../TestCase/testZhufeng.txt", std::fstream::in);
		Tokenizer tokenizer = Tokenizer(is);
		IRBuilder builder = IRBuilder();
		auto errorHandler = ErrorHandler();
		Parser parser = Parser(tokenizer, builder, errorHandler);
		parser.parseProgram();
	}
	static void testGen()
	{
		std::fstream is = std::fstream("../TestCase/test.txt", std::fstream::in);
		Tokenizer tokenizer = Tokenizer(is);
		IRBuilder builder = IRBuilder();
		auto errorHandler = ErrorHandler();
		Parser parser = Parser(tokenizer, builder, errorHandler);
		parser.parseProgram();
		if (!parser.haveError())
		{
			Generator generator = Generator(&builder);
			generator.generate();
			auto out = std::fstream("../output.txt", std::fstream::out);
			generator.print(out);
		}
	}
	static void autoTest(string& testFile, string& output, string& input)
	{
		std::fstream is = std::fstream("../TestCase/" + testFile, std::fstream::in);
		Tokenizer tokenizer = Tokenizer(is);
		IRBuilder builder = IRBuilder();
		auto errorHandler = ErrorHandler();
		Parser parser = Parser(tokenizer, builder, errorHandler);
		parser.startParse();
		assert(parser.haveError() == false);
		Generator generator = Generator(&builder);
		generator.generate();
		auto out = std::fstream("../output.txt", std::fstream::out);
		generator.print(out);
		out.close();

		string cmd = "python ../test.py --asm ../output.txt --output \"" + output + "\" --input " + input;
		int rtn = system(cmd.c_str());
		assert(rtn == 0);
	}
	static void optimizeTest()
	{
		Optimizer opt;
		BasicBlock bb;
		Value * x = new Operator(Op_ADD, T_INT, new Var(string("aa"), T_INT), new Var(string("bb"), T_INT));
		Value * y = new Operator(Op_ADD, T_INT, new Var(string("aa"), T_INT), new Var(string("bb"), T_INT));
		Value * z = new Operator(Op_ADD, T_INT, x, y);
		Value * v = new Var(string("cc"), T_INT, z);
		Value * a = new Operator(Op_ADD, T_INT, new Var(string("aa"), T_INT), new Var(string("bb"), T_INT));
		Value * b = new Operator(Op_ADD, T_INT, new Var(string("aa"), T_INT), new Var(string("bb"), T_INT));
		Value * c = new Operator(Op_ADD, T_INT, a, b);
		Value * w = new Var(string("bb"), T_INT, c);
		bb.addu(v);
		bb.addu(w);
		opt.optimizeBB(&bb);

	}
};