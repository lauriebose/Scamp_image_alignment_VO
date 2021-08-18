#include <math.h>
#include <vector>
using namespace std;

#include "VECTOR_OPS.hpp"

vector<double> vector_sub(vector<double> const &a, vector<double> const &b)
{
  vector<double> r(3);
  r[0] = a[0] - b[0];
  r[1] = a[1] - b[1];
  r[2] = a[2] - b[2];
  return r;
}

vector<double> vector_add(vector<double> const &a, vector<double> const &b)
{
  vector<double> r(3);
  r[0] = a[0] + b[0];
  r[1] = a[1] + b[1];
  r[2] = a[2] + b[2];
  return r;
}

vector<double> vector_mul(vector<double> const &a, double c)
{
  vector<double> r(3);
  r[0] = a[0]*c;
  r[1] = a[1]*c;
  r[2] = a[2]*c;
  return r;
}

vector<double> vector_rot_X(vector<double> const &a, double c)
{
  double cos_c = cos(c);
  double sin_c = sin(c);

  vector<double> r(3);
  r[0] = a[0];
  r[1] = cos_c*a[1] - sin_c*a[2];
  r[2] = sin_c*a[1] + cos_c*a[2];
  return r;
}

vector<double> vector_rot_Y(vector<double> const &a, double c)
{
  double cos_c = cos(c);
  double sin_c = sin(c);

  vector<double> r(3);
  r[0] = cos_c*a[0] + sin_c*a[2];
  r[1] = a[1];
  r[2] = -sin_c*a[0] + cos_c*a[2];
  return r;
}

vector<double> vector_rot_Z(vector<double> const &a, double c)
{
  double cos_c = cos(c);
  double sin_c = sin(c);

  vector<double> r(3);
  r[0] = cos_c*a[0] - sin_c*a[1];
  r[2] = sin_c*a[0] + cos_c*a[1];
  r[2] = a[2];
  return r;
}

double vector_mag(vector<double> const &a)
{
  return sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
}

double vector_dot(vector<double> const &a, vector<double> const &b)
{
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

vector<double> vector_cross(vector<double> const &a, vector<double> const &b)
{
	vector<double> r(3);
	r[0] = a[1]*b[2]-a[2]*b[1];
	r[1] = a[2]*b[0]-a[0]*b[2];
	r[2] = a[0]*b[1]-a[1]*b[0];
	return r;
}
