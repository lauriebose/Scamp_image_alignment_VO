#include <scamp5.hpp>
#include "GLOBALS.hpp"

using namespace SCAMP5_PE;

bool send_keyframe_to_host = false;

void update_display(vs_handle display,vs_handle display_red,vs_handle display_green,int display_mode)
{
	//ONLY SEND BACK DEBUG IMAGES AT CERTAIN FPS
	if(debug_display_timer.get_usec() > debug_display_rate)
	{
		debug_display_timer.reset();

		switch ((DISPLAY_MODES)display_mode)
		{
			case DISPLAY_MODES::ALIGNED_IMAGES:
				scamp5_kernel_begin();
					MOV(R8,R10);
				scamp5_kernel_end();

				if(yaw.relative_value < 0)
				{
					for(int n = 0 ; n < abs(yaw.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,east,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}
				else
				{
					for(int n = 0 ; n < abs(yaw.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,west,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}

				if(pitch.relative_value < 0)
				{
					for(int n = 0 ; n < abs(pitch.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,south,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}
				else
				{
					for(int n = 0 ; n < abs(pitch.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,north,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}

				scamp5_kernel_begin();
					MOV(R2,R6);
					XOR(R1,R2,R8);
				scamp5_kernel_end();
				scamp5_output_image(R1,display);
				break;

			case DISPLAY_MODES::CURRENT_IMG_AND_KF:
				scamp5_kernel_begin();
					MOV(R8,R10);
				scamp5_kernel_end();

				if(yaw.relative_value < 0)
				{
					for(int n = 0 ; n < abs(yaw.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,east,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}
				else
				{
					for(int n = 0 ; n < abs(yaw.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,west,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}

				if(pitch.relative_value < 0)
				{
					for(int n = 0 ; n < abs(pitch.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,south,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}
				else
				{
					for(int n = 0 ; n < abs(pitch.relative_value) ; n++)
					{
						scamp5_kernel_begin();
							DNEWS(R11,R8,north,0);
							MOV(R8,R11);
						scamp5_kernel_end();
					}
				}

				scamp5_kernel_begin();
					AND(R1,R8,R6);
					XOR(R2,R8,R6);
				scamp5_kernel_end();
				scamp5_output_image(R2,display_red);
				scamp5_output_image(R1,display_green);
				break;

			case DISPLAY_MODES::CURRENT_IMG:
				scamp5_output_image(R5,display);
				break;

			case DISPLAY_MODES::CURRENT_KF:
				if(send_keyframe_to_host)
				{
					send_keyframe_to_host = false;
					scamp5_output_image(R6,display);
				}
				break;
		}
	}
}

