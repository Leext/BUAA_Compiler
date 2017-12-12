// Compiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "test.h"
#include "Parser.h"
int main(int argc, char *argv[])
{
    //tester::testParseVarAndFunc();
    //tester::testParser();
	//tester::test();
	tester::optimizeTest();
    system("pause");
    return 0;
}
