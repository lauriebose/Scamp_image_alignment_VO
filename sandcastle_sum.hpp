

#ifndef SANDCASTLE_SUM
#define SANDCASTLE_SUM

	#include <scamp5.hpp>

	using namespace SCAMP5_PE;
	int SC_SUM_STEPWISE(dreg_t reg, int steps = 256);
	int SC_SUM(dreg_t reg, int sum_sparsity_value = 0);
	int SC_SUM_S5D_OPTIMIZED(dreg_t reg, int sum_sparsity_value = 0);
	int DOUBLE_SC_APPROX_SUM_S5D_OPTIMIZED(dreg_t reg, int sum_sparsity_value, int sum_stack_tail);
#endif
