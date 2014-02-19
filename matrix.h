#ifndef MATRIX_H
#define MATRIX_H

#include "triple.h"

class Matrix
{
private:
	Vector r0, r1, r2;

public:
	Matrix(Vector const &r1, Vector const &r2, Vector const &r3);
	Matrix(Vector const &axis, double angle);

	Vector operator*(Vector const &) const;

};

#endif

