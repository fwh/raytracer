#include "matrix.h"

Matrix::Matrix(const Vector &r0, const Vector &r1, const Vector &r2)
:
	r0(r0),
	r1(r1),
	r2(r2)
{}

Matrix::Matrix(Vector const &axis, double angle)
{
	// According to wikipedia, this should we use

	Vector u = axis.normalized();
	double ct = cos(angle);
	double st = sin(angle);

	r0 = Vector(
		ct + u.x * u.x * (1 - ct),
		u.x * u.y * (1 - ct) - u.z * st,
		u.x * u.z * (1 - ct) + u.y * st);

	r1 = Vector(
		u.y * u.x * (1 - ct) + u.z * st,
		ct + u.y * u.y * (1 - ct),
		u.y * u.z * (1 - ct) - u.x * st);

	r2 = Vector(
		u.z * u.x * (1 - ct) - u.y * st,
		u.z * u.y * (1 - ct) + u.x * st,
		ct + u.z * u.z * (1 - ct));
}

Vector Matrix::operator*(Vector const &f) const
{
	return Vector(
		r0.dot(f),
		r1.dot(f),
		r2.dot(f));
}
