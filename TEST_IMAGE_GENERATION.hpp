#include "IMG_TF.hpp"

void draw_grid(int offset_x, int offset_y, int line_spacing, int line_width)
{
	scamp5_kernel_begin();
		CLR(R5);
	scamp5_kernel_end();

	scamp5_draw_begin(R5);

	int ylocation = offset_x%line_spacing;
	while(ylocation < 255)
	{
		scamp5_draw_line(0,ylocation,255,ylocation);
		ylocation+=line_spacing;
	}

	int xlocation = offset_y%line_spacing;
	while(xlocation < 255)
	{
		scamp5_draw_line(xlocation,0,xlocation,255);
		xlocation+=line_spacing;
	}

	for(int n = 0 ; n < line_width ; n++)
	{
		scamp5_kernel_begin();
			DNEWS(R11,R5,east | west | south | north);
			MOV(R1,R5);
			OR(R5,R1,R11);
		scamp5_kernel_end();
	}

	scamp5_draw_end();
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void generate_test_image()
{
	int spacing = 80;
	int width = 3;
	double speed = 0.02;

	if(false)
	{
		int offsetx = round(cos(vs_loop_counter_get()*speed)*0);
		int offsety = round(cos(vs_loop_counter_get()*speed)*0);
		draw_grid(50+offsetx,50+offsety,spacing,width);

		scamp5_kernel_begin();
			MOV(R11,R5);
		scamp5_kernel_end();

		double double_offset_ang = cos(vs_loop_counter_get()*speed*2)*1;
		IMGTF::ROTATION::DIGITAL::ROT_3SKEWS(R11,double_offset_ang);

		scamp5_kernel_begin();
			MOV(R5,R11);
		scamp5_kernel_end();
	}
	else
	{
		double segement_length = 300;
		double segment_duration = 500;
		int duration = segment_duration*4;
		double segment_progress = (int)(segement_length*sin(M_PI*0.5*(vs_loop_counter_get()%(int)segment_duration)/segment_duration));
//			segment_progress = (int)(segement_length*(vs_loop_counter_get()%(int)segment_duration)/segment_duration);
//		if(vs_loop_counter_get()%duration < segment_duration)
//		{
//			int offsetx = segment_progress;
//			int offsety = 0;
//			draw_grid(offsetx,offsety,spacing,width);
//		}
//		else
//		{
//			if(vs_loop_counter_get()%duration < segment_duration*2)
//			{
//				int offsetx = segement_length;
//				int offsety = segment_progress;
//				draw_grid(offsetx,offsety,spacing,width);
//			}
//			else
//			{
//				if(vs_loop_counter_get()%duration < segment_duration*3)
//				{
//					int offsetx = 500-segment_progress;
//					int offsety = 500;
//					draw_grid(offsetx,offsety,spacing,width);
//				}
//				else
//				{
//					int offsetx = 0;
//					int offsety = 500-segment_progress;
//					draw_grid(offsetx,offsety,spacing,width);
//				}
//			}
//		}

		draw_grid(0,0,spacing,width);

		scamp5_kernel_begin();
			MOV(R11,R5);
		scamp5_kernel_end()

		double double_offset_ang = cos(vs_loop_counter_get()*speed*2)*0.5;
//		IMGTF::ROTATION::DIGITAL::ROT_3SKEWS(R11,double_offset_ang);
		IMGTF::SCALING::DIGITAL::SCALE(R11,(int)(fabs( cos(vs_loop_counter_get()*speed))*40),true);

		scamp5_kernel_begin();
			MOV(R5,R11);
		scamp5_kernel_end();
	}
}
