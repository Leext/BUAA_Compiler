#pragma once
#include "stdafx.h"
#include "IRBuilder.h"
class Generator
{
#ifdef __TEST
	friend class tester;
#endif // __TEST
  public:
	Generator(IRBuilder *builder);
	void generate();
	void print(fstream &output);

  protected:
	IRBuilder *builder;
	vector<string> code;
	unordered_map<string, int> localOffset; // relative to $fp
	unordered_map<Quad *, int> tempOffset;  // relative to $fp
	unordered_map<BasicBlock *, int> bb2label;
	int labelCount;
	void generateData();
	void generateText();
	void generateFunction(Function *function);
	void generateFunctionCall();
	void convertQuad(Quad *quad);
	void loadValue(Function *function, Quad *quad, string &reg, int temp = 0);
	void storeValue(Function *function, Quad *quad, string &reg);
	void storeValueArray(Function *function, Quad *quad, string &reg, string &freeReg);
};
