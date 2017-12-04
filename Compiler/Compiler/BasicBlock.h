#pragma once
#ifndef __BASICBLOCK_H_
#define __BASICBLOCK_H_

#include "stdafx.h"
#include "Quad.h"
using std::list;
class BasicBlock
{
  public:
	static int count;
	const int id;
	BasicBlock() : id(count++)
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
