#include "StdAfx.h"
#include "BitVector.h"


BitVector::BitVector(int size)
{
	_size = size;
	_length = size / intWidth + 1;
	_bits = new int[_length];
	memset(_bits, 0, sizeof(int) * _length);
}

BitVector::BitVector(BitVector && v)
{
	_size = v._size;
	_length = v._length;
	_bits = v._bits;
	v._bits = nullptr;
}


BitVector::~BitVector()
{
	delete[] _bits;
}

int BitVector::operator[](int i)
{
	// TODO: insert return statement here
	return (_bits[i / intWidth] >> (i % intWidth)) & 1;
}

void BitVector::set(int i)
{
	int idx = i / intWidth;
	int offset = i % intWidth;
	_bits[idx] |= 1 << offset;
}

BitVector BitVector::operator+(const BitVector &v)
{
	BitVector ret(_size);
	for (int i = 0; i < _length; i++)
		ret._bits[i] = _bits[i] | v._bits[i];
	return ret;
}

BitVector BitVector::operator-(const BitVector &v)
{
	BitVector ret(_size);
	for (int i = 0; i < _length; i++)
		ret._bits[i] = _bits[i] & (~v._bits[i]);
	return ret;
}

BitVector & BitVector::operator=(BitVector && v)
{
	if (this != &v)
	{
		delete[]_bits;
		_size = v._size;
		_length = v._length;
		_bits = v._bits;
		v._bits = nullptr;
	}
	return *this;
}

string BitVector::toString()
{
	std::stringstream stream;
	for (int i = 0, j = 0; i < _length; i++)
		for (int k = 0; k < intWidth && j < _size; k++, j++)
			stream << ((_bits[i] >> k) & 1);
	return stream.str();
}
