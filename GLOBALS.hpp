/*
 * GLOBALS.hpp
 *
 *  Created on: 9 Feb 2018
 *      Author: agile
 */

#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

#define DEMO_MODE 0

enum class DISPLAY_MODES {ALIGNED_IMAGES,CURRENT_IMG_AND_KF,CURRENT_IMG,CURRENT_KF,GRAPHS,TRACE};
vs_stopwatch debug_display_timer;
static uint32_t debug_display_rate = 16667; //IE 1,000,000/(MONITOR FPS), 60HZ = 16667, 100HZ = 10,000

vs_stopwatch birds_eye_view_display_timer;
static uint32_t birds_eye_view_display_rate = 16667; //IE 1,000,000/(MONITOR FPS), 60HZ = 16667, 100HZ = 10,000

int kf_yaw_pitch_trigger = 30;
int kf_roll_trigger = 30;
int kf_scale_trigger = 16;

int HDR_iterations = 1;
int edge_threshold = 40;
int gain_levels = 1;
int edge_expansion = 0;

int enable_roll_tracking = true;
int enable_scale_tracking = false;
int paused = 0;
int use_rates_as_prior = 1;

int gsum_sparsity = 0;

struct dimension
{
public:
	int KF_value;
	int relative_value;
	int change;
	int prev_change;

	void reset()
	{
		KF_value=0;
		relative_value = 0;
		change = 0;
		prev_change = 0;
	}
};

dimension yaw;
dimension pitch;
dimension roll;
dimension scale;

struct filtered_dimension
{
public:
	int log_length = 64;

	double filter_value = 0.1;

	double value = 0;
	double prev_value = 0;
	double rate = 0;

	int8_t *raw_change_log;
	double change_log_sum = 0;
	int change_log_index = 0;

	filtered_dimension(int filter_length, double filter_value)
	{
		this->log_length = filter_length;
		raw_change_log = new int8_t[filter_length];
		this->filter_value = filter_value;
	}

	void update(int8_t raw_change, bool filter)
	{
		change_log_sum -= raw_change_log[change_log_index];
		change_log_sum += raw_change;
		raw_change_log[change_log_index] = raw_change;
		change_log_index = (change_log_index+1)%log_length;

		prev_value = value;
		double filtered_change = change_log_sum/log_length;
		if(fabs(filtered_change) < filter_value && filter)
		{
			filtered_change = 0;
		}

		value = value + filtered_change;
		rate = value-prev_value;
	}

	void reset()
	{
		value = 0;
		prev_value = 0;
		change_log_sum = 0;
		rate = 0;
		for(int n = 0 ; n < log_length ; n++)
		{
			raw_change_log[n] = 0;
		}
	}
};

filtered_dimension filtered_roll (4,0.02);
filtered_dimension filtered_scale(4,0.0);
filtered_dimension filtered_yaw (4,0.02);
filtered_dimension filtered_pitch (4,0.02);


int enabled_perspective_correction = 0;
const double perspective_correction_trigger_angle = 5.0*M_PI/180.0;

const double angle_per_pixel = 38.0*0.0001;
int spi_pitch,spi_roll;
double spi_pitch_angle, spi_roll_angle;
int spi_scaling = 8;


#endif /* GLOBALS_HPP_ */
