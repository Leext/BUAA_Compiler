#pragma once
#include "stdafx.h"
using std::string;
class BitVector
{
public:
	static const int intWidth = 32;
	BitVector(int size);
	BitVector(BitVector &&v);
	~BitVector();
	int operator[](int i);
	void set(int i);
	BitVector operator+(const BitVector&);
	BitVector operator-(const BitVector&);
	BitVector& operator=(BitVector && v);
	string toString();
protected:
	int _size,_length;
	int *_bits;
	
};

