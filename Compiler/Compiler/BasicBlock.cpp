#include "StdAfx.h"
#include "BasicBlock.h"

int BasicBlock::count = 0;

void BasicBlock::addu(Quad *quad)
{
	if (quad->opcode == Op_LABEL)
		static_cast<Label *>(quad)->controller = this;
	quads.push_back(quad);
}
