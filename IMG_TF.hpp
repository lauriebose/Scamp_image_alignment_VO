#include <scamp5.hpp>
#include <math.h>

using namespace SCAMP5_PE;

#include "IMG_ROTATION.hpp"
#include "IMG_ROTATION_DIGITAL.hpp"
#include "IMG_SCALING.hpp"
#include "IMG_SCALING_DIGITAL.hpp"
#include "IMG_SKEW.hpp"
#include "IMG_SKEW_DIGITAL.hpp"

#ifndef IMG_TF
#define IMG_TF
namespace IMGTF
{
	unsigned char reverse_byte(unsigned char x);
	double sin_approx3(double angle);
	double cos_approx3(double angle);
	double acos_approx3(double value);
	double tan_approx3(double angle);
}
#endif



