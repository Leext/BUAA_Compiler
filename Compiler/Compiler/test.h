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
#include "BitVector.h"
#include <cassert>
using std::string;
using std::unordered_map;
class tester
{
public:
	static void test()
	{
	/*	autoTest(string("test.txt"), string("-16464 0123456789 zyxwvutsrq 0123456789 hello10-cube27 -square-4 self-1 self0 self1 square4 cube27 0123456789 zyxwvutsrq 120"), string("1 -1"), true);
		autoTest(string("testOp.txt"), string("-16464"), string(""), true);
		autoTest(string("qsort.cpp"), string("0123456789"), string(""), true);
		autoTest(string("testDag.txt"), string("21 | 0 | 10 | 10"), string(""), true);
		optimizeTest();*/
		testBitVector();
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
	static void autoTest(string& testFile, string& output, string& input, bool optimize)
	{
		std::fstream is = std::fstream("../TestCase/" + testFile, std::fstream::in);
		Tokenizer tokenizer = Tokenizer(is);
		IRBuilder builder = IRBuilder();
		auto errorHandler = ErrorHandler();
		Parser parser = Parser(tokenizer, builder, errorHandler);
		parser.startParse();
		assert(parser.haveError() == false);
		if (optimize)
		{
			Optimizer opt;
			opt.optimize(builder);
		}
		Generator generator = Generator(&builder);
		generator.generate();
		auto out = std::fstream("../output.txt", std::fstream::out);
		generator.print(out);
		out.close();
		if (input != "")
			input = " --input " + input;
		string cmd = "python ../test.py --asm ../output.txt --output \"" + output + "\"" + input;
		int rtn = system(cmd.c_str());
		assert(rtn == 0);
	}
	static void optimizeTest()
	{
		Optimizer opt;
		BasicBlock bb;
		auto va = new Var(string("aa"), T_INT);
		auto vb = new Var(string("bb"), T_INT);
		Value * x = new Operator(Op_ADD, T_INT, va, vb);
		auto vvb = new Var(string("bb"), T_INT);
		Value * y = new Operator(Op_ADD, T_INT, new Constant(1), vvb);
		Value * z = new Operator(Op_ADD, T_INT, x, y);
		Value * v = new Var(string("cc"), T_INT, z);
		auto vva = new Var(string("aa"), T_INT);
		auto vvvb = new Var(string("bb"), T_INT);
		Value * a = new Operator(Op_ADD, T_INT, vva, vvvb);
		auto vvvvb = new Var(string("bb"), T_INT);
		Value * b = new Operator(Op_ADD, T_INT, new Constant(1), vvvvb);
		Value * c = new Operator(Op_ADD, T_INT, a, b);
		Value * w = new Var(string("bb"), T_INT, c);
		bb.addu(v);
		bb.addu(w);
		opt.optimizeBB(&bb);

	}
	static void testBitVector()
	{
		BitVector v(36);
		BitVector w(36);
		v.set(32);
		v.set(0);
		v.set(2);
		w.set(2);
		v = v - w;
		std::cout << v.toString() << std::endl;
	}
};