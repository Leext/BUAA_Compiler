#pragma once
#include "stdafx.h"
#include "Quad.h"
using std::list;
class BasicBlock
{
public:
	BasicBlock();
	list<Quad*> quads;
};

