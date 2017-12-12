#pragma once
#include "IRBuilder.h"
using std::map;
class Optimizer
{
#ifdef __TEST
	friend class tester;
#endif // __TEST
public:
	Optimizer();
	void optimize(IRBuilder builder);

protected:
	void optimizeBB(BasicBlock *bb);
	unordered_map<string, Value *> name2var;
	map<Value *, Value *> var2var;
	Value* simplify(Value *);
};
