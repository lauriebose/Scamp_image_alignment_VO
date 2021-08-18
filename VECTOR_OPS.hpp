#include <math.h>
#include <vector>
using namespace std;

#ifndef VECTOR_OPS_HPP_
#define VECTOR_OPS_HPP_

vector<double> vector_sub(vector<double> const &a, vector<double> const &b);

vector<double> vector_add(vector<double> const &a, vector<double> const &b);

vector<double> vector_mul(vector<double> const &a, double c);

vector<double> vector_rot_X(vector<double> const &a, double c);

vector<double> vector_rot_Y(vector<double> const &a, double c);

vector<double> vector_rot_Z(vector<double> const &a, double c);

double vector_mag(vector<double> const &a);

double vector_dot(vector<double> const &a, vector<double> const &b);

vector<double> vector_cross(vector<double> const &a, vector<double> const &b);

#endif /* VECTOR_OPS_HPP_ */
