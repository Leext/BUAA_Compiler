#pragma once
#include "stdafx.h"
#include "IRBuilder.h"
using std::unordered_set;
struct Reg
{
	string name;
	Value*value;
	bool free;
	bool fix;
	Reg(string& n) :name(n), free(true),fix(false) {}
};
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
	IRBuilder * builder;
	vector<string> initCode;
	vector<string> code;
	vector<string> finalCode;
	unordered_map<string, int> localOffset; // relative to $fp
	unordered_map<Quad *, int> tempOffset;  // relative to $fp
	unordered_map<BasicBlock *, int> bb2label;
	unordered_map<string, int> GlobalReg;
	unordered_set<string> loadedToGloabal;
	unordered_map<string, Reg*> TempReg;
	vector<Reg> tempRegs;
	unordered_set<string> inStack;
	unordered_map<string, int> refCount;
	unordered_map<string, int> stackOffset;

	Function* function;

	vector<string>::iterator globalInitInsertPoint;

	int labelCount;
	void generateData();
	void generateFunction(Function *function);
	void init();
	void generateFunctionOpt(Function *function);
	void loadValue(Function *function, Quad *quad, string &reg, int temp = 0);
	void loadValueG(Function *function, Quad *quad, string &reg, int temp = 0);
	void storeValue(Function *function, Quad *quad, string &reg);
	void storeValueArray(Function *function, Quad *quad, string &reg, string &freeReg);
	string getReg(Value& value, bool write = false, int temp = 0);
	void moveToReg(Value &value, string& reg, int temp = 0);
	void decreaseRef(Value *value);
	Reg& spill();
	void writeBack();
	int countVar(Function *function);
	void allocateGloabal(Function* function);
};
