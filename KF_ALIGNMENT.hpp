#include <scamp5.hpp>

#include "IMG_TF.hpp"
#include "GLOBALS.hpp"
#include "sandcastle_sum.hpp"
#include <stdlib.h>

using namespace SCAMP5_PE;

int min_search_iterations = 1;
int max_search_iterations = 1;
int search_iteration_min_improvement = 0;

bool converged = false;
int previous_score = -1;

int WEST_shift_score = 0;
int EAST_shift_score = 0;
int EW_search_dir = 0;

int SOUTH_shift_score = 0;
int NORTH_shift_score = 0;
int SN_search_dir = 0;

int CW_shift_score = 0;
int ACW_shift_score = 0;
int CW_ACW_search_dir = 0;

int UP_shift_score = 0;
int DOWN_shift_score = 0;
int UP_DOWN_scale_search_dir = 0;

int use_dreg_sum = 1;

extern int gsum_sparsity;

int perform_alignment_gsum()
{
	if(use_dreg_sum)
	{
		scamp5_load_in(100);
		scamp5_kernel_begin();
			bus(F,IN);
			WHERE(R10);
				mov(F,IN);
			ALL();
		scamp5_kernel_end();
		//MISC_FUNCTIONS::load_DREG_into_F(DENUM::R10,100,-100);

		scamp5_kernel_begin();
			mov(E,F);
			MOV(R10,R1);
		scamp5_kernel_end();
		int ret = SC_SUM_S5D_OPTIMIZED(R10,gsum_sparsity);
		scamp5_kernel_begin();
			where(E);
			MOV(R10,FLAG);
			all();
		scamp5_kernel_end();
		return ret;
	}
	else
	{
		return scamp5_global_count(R1,F,0);
	}
}

void evaluate_east_shift()
{
	scamp5_kernel_begin();
		DNEWS(R11,R9,east,0);
		AND(R1,R11,R10);
	scamp5_kernel_end();

	EAST_shift_score = perform_alignment_gsum();
}

void evaluate_west_shift()
{
	scamp5_kernel_begin();
		DNEWS(R11,R9,west,0);
		AND(R1,R11,R10);
	scamp5_kernel_end();

	WEST_shift_score = perform_alignment_gsum();
}

void evaluate_north_shift()
{
	scamp5_kernel_begin();
		DNEWS(R11,R9,north,0);
		AND(R1,R11,R10);
	scamp5_kernel_end();

	NORTH_shift_score = perform_alignment_gsum();
}

void evaluate_south_shift()
{
	scamp5_kernel_begin();
		DNEWS(R11,R9,south,0);
		AND(R1,R11,R10);
	scamp5_kernel_end();

	SOUTH_shift_score = perform_alignment_gsum();
}

void evaluate_CW_shift()
{
	scamp5_kernel_begin();
		MOV(R11,R10);
	scamp5_kernel_end();
	IMGTF::ROTATION::DIGITAL::STEP_ROT_2SKEWS(R11,roll.relative_value,false);
	scamp5_kernel_begin();
		MOV(R7,R11);
		AND(R1,R11,R9);
	scamp5_kernel_end();
//	scamp5_kernel_begin();
//		MOV(R8,R11);
//	scamp5_kernel_end();

	CW_shift_score = perform_alignment_gsum();
}

void evaluate_ACW_shift()
{
	scamp5_kernel_begin();
		MOV(R11,R10);
	scamp5_kernel_end();
	IMGTF::ROTATION::DIGITAL::STEP_ROT_2SKEWS(R11,roll.relative_value,true);
	scamp5_kernel_begin();
		MOV(R8,R11);
		AND(R1,R11,R9);
	scamp5_kernel_end();

	ACW_shift_score = perform_alignment_gsum();
}

void evaluate_DOWN_shift()
{
	scamp5_kernel_begin();
		MOV(R11,R10);
	scamp5_kernel_end();
	IMGTF::SCALING::DIGITAL::SCALE(R11,1,true);
	scamp5_kernel_begin();
		MOV(R7,R11);
		AND(R1,R11,R9);
	scamp5_kernel_end();

	DOWN_shift_score = perform_alignment_gsum();
}

void evaluate_UP_shift()
{
	scamp5_kernel_begin();
		MOV(R11,R10);
	scamp5_kernel_end();
	IMGTF::SCALING::DIGITAL::SCALE(R11,1,false);
	scamp5_kernel_begin();
		AND(R1,R11,R9);
		MOV(R8,R11);
	scamp5_kernel_end();

//	scamp5_kernel_begin();
//		MOV(R3,R11);
//	scamp5_kernel_end();

	UP_shift_score = perform_alignment_gsum();
}

void shifted_E()
{
	scamp5_kernel_begin();
		DNEWS(R0,R9,east,0);
		MOV(R9,R0);
	scamp5_kernel_end();

	yaw.change++;

	previous_score = EAST_shift_score;

	converged = false;
}

void shifted_W()
{
	scamp5_kernel_begin();
		DNEWS(R0,R9,west,0);
		MOV(R9,R0);
	scamp5_kernel_end();

	yaw.change--;

	previous_score = WEST_shift_score;

	converged = false;
}

void shifted_S()
{
	scamp5_kernel_begin();
		DNEWS(R0,R9,south,0);
		MOV(R9,R0);
	scamp5_kernel_end();

	pitch.change++;

	previous_score = SOUTH_shift_score;

	converged = false;
}

void shifted_N()
{
	scamp5_kernel_begin();
		DNEWS(R0,R9,north,0);
		MOV(R9,R0);
	scamp5_kernel_end();

	pitch.change--;

	previous_score = NORTH_shift_score;

	converged = false;
}

void shifted_CW()
{
//	scamp5_kernel_begin();
//		MOV(R10,R8);
//	scamp5_kernel_end();
	scamp5_kernel_begin();
		MOV(R10,R7);
	scamp5_kernel_end();

	roll.change++;

	previous_score = CW_shift_score;

	converged = false;
}

void shifted_ACW()
{
	scamp5_kernel_begin();
		MOV(R10,R8);
	scamp5_kernel_end();

	roll.change--;

	previous_score = ACW_shift_score;

	converged = false;
}

void shifted_DOWN()
{
	scamp5_kernel_begin();
		MOV(R10,R7);
	scamp5_kernel_end();

	scale.change--;

	previous_score = DOWN_shift_score;

	converged = false;
}

void shifted_UP()
{
	scamp5_kernel_begin();
		MOV(R10,R8);
	scamp5_kernel_end();

	scale.change++;

	previous_score = UP_shift_score;

	converged = false;
}



int image_alignment()
{
	//R5 CURRENT EDGE IMG
	//R6 CURRENT EDGE KF
	//R9 CURRENT BEST SHIFTED EDGE KF
	//R10 CURRENT BEST ROTATED EDGE IMAGE

	if(use_rates_as_prior)
	{
		yaw.change = (int)filtered_yaw.rate;
		pitch.change = (int)filtered_pitch.rate;
		roll.change = (int)filtered_roll.rate;
	}
	else
	{
		yaw.change = 0;
		pitch.change = 0;
		roll.change = 0;
	}

	scale.change = 0;

	previous_score = -1;

	EW_search_dir = 0;
	SN_search_dir = 0;
	CW_ACW_search_dir = 0;

	converged = false;

	for(int n = 0 ;  ; n++)
	{
		if((n >= max_search_iterations) && n >= min_search_iterations)
		{
			break;
		}

//		if(enabled_perspective_correction || true)
//		{
//			vs_wait(0);
//		}

		//HORIZONTAL TRANSLATION SEARCH
		EW_search_dir = 0;

		evaluate_east_shift();
		evaluate_west_shift();

		if(EAST_shift_score > WEST_shift_score)
		{
			shifted_E();
			EW_search_dir = 1;
		}
		else
		{
			if(WEST_shift_score > EAST_shift_score)
			{
				shifted_W();
				EW_search_dir = -1;
			}
		}


		//////////////////////////////////////////////////////////////////////////////////////////////////


		//VERTICLE TRANSLATION SEARCH
		SN_search_dir = 0;
		evaluate_south_shift();
		evaluate_north_shift();

		if(SOUTH_shift_score > NORTH_shift_score)
		{
			shifted_S();
			SN_search_dir = 1;
		}
		else
		{
			if(NORTH_shift_score > SOUTH_shift_score)
			{
				shifted_N();
				SN_search_dir = -1;
			}
		}


		//////////////////////////////////////////////////////////////////////////////////////////////////


		if (enable_roll_tracking)
		{
			evaluate_CW_shift();
			evaluate_ACW_shift();

			if(CW_shift_score > ACW_shift_score)
			{
				shifted_CW();
				CW_ACW_search_dir = 1;
			}
			else
			{
				if(ACW_shift_score > CW_shift_score)
				{
					shifted_ACW();
					CW_ACW_search_dir = -1;
				}
			}
		}
	}

	if (enable_scale_tracking)
	{
		evaluate_UP_shift();
		evaluate_DOWN_shift();

		if(UP_shift_score > DOWN_shift_score)
		{
			shifted_UP();
			UP_DOWN_scale_search_dir = 1;
		}
		else
		{
			if(DOWN_shift_score > UP_shift_score)
			{
				shifted_DOWN();
				UP_DOWN_scale_search_dir = -1;
			}
		}
	}

	return previous_score;
}

void apply_prior_transformations_to_current_image_and_keyframe()
{
	//R5 CURRENT EDGE IMG
	//R6 CURRENT EDGE KF
	//R9 CURRENT BEST SHIFTED EDGE KF
	//R10 CURRENT BEST ROTATED EDGE IMAGE

	if(use_rates_as_prior && !paused)
	{
		//UPDATE CURRENT ESTIMATED RELATIVE YPR BY ADDING RATES
		//HELPS TO MAINTAIN TRACKING UNDER SUDDEN MOTIONS
		yaw.relative_value = yaw.relative_value + (int)filtered_yaw.rate;
		pitch.relative_value = pitch.relative_value + (int)filtered_pitch.rate;
		roll.relative_value = roll.relative_value + roll.change;
	}

	scamp5_kernel_begin();
		MOV(R10,R5);
	scamp5_kernel_end();

	//ROTATE CURRENT EDGE IMAGE BASED UPON PRIOR RELATIVE ROLL
	if(enable_roll_tracking)
	{
		scamp5_kernel_begin();
			MOV(R11,R10);
		scamp5_kernel_end();

		double rot_angle_in_rad = -roll.relative_value*0.55555*M_PI/180.0;
		IMGTF::ROTATION::DIGITAL::ROT_3SKEWS(R11,rot_angle_in_rad);

		scamp5_kernel_begin();
			MOV(R10,R11);
		scamp5_kernel_end();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//SCALE CURRENT EDGE IMAGE BASED UPON PRIOR RELATIVE SCALE
	if(enable_scale_tracking)
	{

		scamp5_kernel_begin();
			MOV(R11,R10);
		scamp5_kernel_end();
		int current_scale_value = scale.relative_value;

		IMGTF::SCALING::DIGITAL::SCALE(R11,abs(scale.relative_value),scale.relative_value > 0 ? 0 : 1);
//		for(int n = 0 ; n < abs(scale.relative_value) ; n++)
//		{
//			if(scale.relative_value > 0)
//			{
//				IMGTF::SCALING::DIGITAL::SCALE(R11,current_scale_value,false);
//			}
//			else
//			{
//				IMGTF::SCALING::DIGITAL::SCALE(R11,current_scale_value,true);
//			}
//			vs_post_text("wtf %d \n",current_scale_value );
//		}
		scamp5_kernel_begin();
			MOV(R10,R11);
		scamp5_kernel_end();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//SHIFT KEYFRAME COPY BASED UPON PRIOR RELATIVE YAW / PITCH
	scamp5_kernel_begin();
		MOV(R9,R6);
	scamp5_kernel_end();


	if(yaw.relative_value > 0)
	{
		for(int n = 0 ; n < yaw.relative_value ; n++)
		{
			scamp5_kernel_begin();
				MOV(R11,R9);
				DNEWS(R9,R11,east,0);
			scamp5_kernel_end();
		}
	}
	else
	{
		for(int n = yaw.relative_value ; n < 0 ; n++)
		{
			scamp5_kernel_begin();
				MOV(R11,R9);
				DNEWS(R9,R11,west,0);
			scamp5_kernel_end();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	if(pitch.relative_value > 0)
	{
		for(int n = 0 ; n < pitch.relative_value ; n++)
		{
			scamp5_kernel_begin();
				MOV(R11,R9);
				DNEWS(R9,R11,south,0);
			scamp5_kernel_end();
		}
	}
	else
	{
		for(int n = pitch.relative_value ; n < 0 ; n++)
		{
			scamp5_kernel_begin();
				MOV(R11,R9);
				DNEWS(R9,R11,north,0);
			scamp5_kernel_end();
		}
	}
}
