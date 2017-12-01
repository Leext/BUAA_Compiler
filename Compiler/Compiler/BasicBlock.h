#pragma once
#ifndef __BASICBLOCK_H_
#define __BASICBLOCK_H_

#include "stdafx.h"
#include "Quad.h"
using std::list;
class BasicBlock
{
  public:
	BasicBlock()
	{
		next = nullptr;
	}
	~BasicBlock()
	{
		delete next;
	}
	BasicBlock *next;
	list<Quad *> quads;
	void add(Quad *quad);
};

#endif // !__BASICBLOCK_H_
