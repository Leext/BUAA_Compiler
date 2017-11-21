#pragma once
#include "QuaternaryArray.h"
using std::shared_ptr;
using std::vector;
class ExprAST
{
public:
    ExprAST();
    ~ExprAST();
    virtual shared_ptr<QuaternaryArray> genCode() = 0;
};

