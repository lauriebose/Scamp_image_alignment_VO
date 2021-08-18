#include <scamp5.hpp>
#include <math.h>
#include "GLOBALS.hpp"
#include "KF_ALIGNMENT.hpp"
#include "EDGE_IMAGE_GENERATION.hpp"
#include "VECTOR_OPS.hpp"
#include "IMG_TF.hpp"

#include "display_drawing.hpp"
#include "spi_coms.hpp"
#include "TEST_IMAGE_GENERATION.hpp"

const int locked_frame_rate = -1; //-1 = use slider

const bool send_usb_images = true;

uint8_t image_requested = 0; // User requested image over SPI when > 0, number determines which type of image
bool user_forced_take_keyframe = false;

using namespace SCAMP5_PE;

static bool take_keyframe = true;
extern bool send_keyframe_to_host;
extern int gsum_sparsity;

static int debug_display = 1;

vs_stopwatch timer_total;
vs_stopwatch timer_test;
vs_stopwatch stats_trigger_timer;

const int display_size = 2;
vs_handle display00,display_00_red,display_00_green;
vs_handle display01;
vs_handle display02;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//EXTERNAL FUNCTIONS

void apply_prior_transformations_to_current_image_and_keyframe();

int image_alignment();

void make_debug_display();

extern int use_dreg_sum;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


const int graph_range = 400;
const int graph_time_frame = 1000;

int yaw_scalar = 0;

extern int min_search_iterations;
extern int max_search_iterations;

int use_generated_test_image = 0;

int output_pose = 0;
int output_stats = 0;
int image_boarder_edges;

void setup_host_gui()
{
	auto slider_vxc = vs_gui_add_slider("vxc: ",0,4095,2900);
	  vs_on_gui_update(slider_vxc,[&](int32_t new_value){
			vs_scamp5_configure_voltage(3,new_value);
			});

	auto slider_debug_display =  vs_gui_add_slider("debug display: ",0,4,0,&debug_display);
	vs_on_gui_update(slider_debug_display,[&](int32_t new_value){
			scamp5_kernel_begin();
				CLR(R1);
			scamp5_kernel_end();
			scamp5_output_image(R1,display00);
			scamp5_output_image(R1,display_00_red);
			scamp5_output_image(R1,display_00_green);
	    });

	vs_gui_add_switch("generated_image: ",false,&use_generated_test_image);


	auto button_force_take_keyframe = vs_gui_add_button("force_take_keyframe:");
	vs_on_gui_update(button_force_take_keyframe,[&](int32_t new_value){
	    	vs_post_text("user forced new key-frame\n");
	    	user_forced_take_keyframe = true;
	    	take_keyframe = true;
	    });

	vs_gui_add_slider("gsum_sparsity: ",0,5,1,&gsum_sparsity);

	vs_gui_add_slider("edge_threshold: ",0,100,15,&edge_threshold);
//	vs_gui_add_slider("edge expansion: ",0,10,0,&edge_expansion);
	vs_gui_add_slider("HDR iteration: ",0,50,1,&HDR_iterations);


	auto slider_kf_threshold =  vs_gui_add_slider("key-frame threshold: ",10,128,64);
	vs_on_gui_update(slider_kf_threshold,[&](int32_t new_value){
	    	kf_yaw_pitch_trigger = new_value;
	    	kf_roll_trigger = new_value;

	    });

	vs_gui_add_switch("roll tracking: ",true,&enable_roll_tracking);
	vs_gui_add_switch("scale tracking: ",false,&enable_scale_tracking);
	vs_gui_add_switch("motion prior: ",true,&use_rates_as_prior);

	vs_gui_add_slider("yaw_scalar: ",0,100,43,&yaw_scalar);

//	vs_gui_add_switch("use_dreg_sum: ",true,&use_dreg_sum);

    vs_gui_add_slider("min_search iterations: ",0,10,1,&min_search_iterations);
    vs_gui_add_slider("max_search iterations: ",0,10,1,&max_search_iterations);

	 vs_on_gui_update(VS_GUI_FRAME_RATE,[&](int32_t new_value){
	        uint32_t new_framerate = new_value;
	        if(locked_frame_rate == -1)
			{
				if(new_framerate > 0){
					vs_frame_trigger_set(1,new_framerate);
					vs_enable_frame_trigger();
					vs_post_text("frame trigger: 1/%d\n",(int)new_framerate);
				}else{
					vs_disable_frame_trigger();
					vs_post_text("frame trigger disabled\n");
				}
			}
	    });

	 vs_on_gui_update(VS_GUI_FRAME_GAIN,[&](int32_t new_value){
		 	 gain_levels = new_value;
		    });

	 vs_gui_add_switch("output_pose: ",false,&output_pose);
	 vs_gui_add_switch("output_stats: ",false,&output_stats);

	 vs_gui_add_slider("image_boarder_edges: ",0,32,4,&image_boarder_edges);


	display00 = vs_gui_add_display("00",0,0,display_size);
    display_00_red = vs_gui_add_display("",0,0,display_size,"overlay_red");
    display_00_green = vs_gui_add_display("",0,0,display_size,"overlay_green");

	VS_GUI_DISPLAY_STYLE(style_plot,R"JSON(
		{
			"plot_palette": "plot_cmyw",
			"plot_palette_groups": 4
		}
		)JSON");
	display01 = vs_gui_add_display("01",0,display_size,display_size,style_plot);
	vs_gui_set_scope(display01,0,graph_range,graph_time_frame);

	display02 = vs_gui_add_display("02",0,display_size*2,display_size,style_plot);
	vs_gui_set_scope(display02,0,graph_range,graph_time_frame);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void initial_setup()
{
	vs_on_host_connect([&](){
		scamp5_kernel::print_debug_info();
		vs_led_on(VS_LED_2);
	});

	vs_on_host_disconnect([&](){
		vs_enable_frame_trigger();
		vs_led_off(VS_LED_2);
		scamp5_kernel::print_debug_info();
	});
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void scamp5_main(){

//SPI INPUT
//	 vs_on_user_value([&](uint8_t id,int32_t value) {
////	   vs_post_text("\nuser val %d\n",value);
//	   if (id == 0) {
//		 // User requested a new image
//		   image_requested = value;
//		 }
//
//	   if (id == 2) {
//		 // User forced keyframe
//		   vs_post_text("user forced keyframe");
//		   user_forced_take_keyframe = true;
//		 }
//	});

	initial_setup();

	setup_host_gui();

	//set inital frame rate
    vs_frame_trigger_set(1,locked_frame_rate);
    vs_enable_frame_trigger();

	static struct time_stats{
		float time_edge_image_creation ;
		float time_apply_prior_transform;
		float time_alignment_search;
		float time_debug_output;
		float time_total;}time_stats;

	debug_display_timer.reset();

	user_forced_take_keyframe = true;

	while(1){

		timer_total.reset();


		vs_wait_frame_trigger();
		vs_process_message();

		//REFRESH STORED DREG DATA
		scamp5_kernel_begin();
			REFRESH(R6);
		scamp5_kernel_end();


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//GET INPUT IMAGE

		timer_test.reset();
		if(!use_generated_test_image)
		{
			acquire_edge_image_R5(gain_levels,edge_threshold,edge_expansion,HDR_iterations,200,image_boarder_edges );
		}
		else
		{
			generate_test_image();
		}
		time_stats.time_edge_image_creation = timer_test.get_usec();


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//TRANSFORM CURRENT EDGE IMAGE ACCORDING TO CURRENT POSE RELATIVE TO KEYFRAME


		timer_test.reset();
		apply_prior_transformations_to_current_image_and_keyframe();
		time_stats.time_apply_prior_transform = timer_test.get_usec();


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



		if(!paused)
		{
			//R5 CURRENT EDGE IMG
			//R6 CURRENT EDGE KF
			//R9 CURRENT BEST SHIFTED EDGE KF
			//R10 CURRENT BEST ROTATED EDGE IMAGE

			//ALIGN CURRENT IMAGE WITH KEYFRAME
				timer_test.reset();
				image_alignment();
				time_stats.time_alignment_search = timer_test.get_usec();

			/////////////////////////
			//UPDATE POSE

				filtered_yaw.update(yaw.change,false);
				filtered_pitch.update(pitch.change,false);
				filtered_roll.update(roll.change,true);
				filtered_scale.update(scale.change,true);

				yaw.relative_value += yaw.change;
				pitch.relative_value += pitch.change;
				roll.relative_value += roll.change;
				scale.relative_value += scale.change;

			/////////////////////////
			//OUTPUT POSE CHANGES

//				vs_post_text("%d,%d,%d,%d\n",yaw.change,pitch.change,roll.change,scale.change);

			/////////////////////
			//DETERMINE IF NEW KEYFRAME SHOULD BE TAKEN

				if(abs(yaw.relative_value) > kf_yaw_pitch_trigger ||
					abs(pitch.relative_value) > kf_yaw_pitch_trigger ||
					abs(roll.relative_value) > kf_roll_trigger ||
					abs(scale.relative_value) > kf_scale_trigger)
				{
					take_keyframe = true;
				}


			/////////////////////////////////
			//UPDATE TRACE GRAPHS

				int16_t plotted_value[4];
				plotted_value[0] = ((int16_t)filtered_yaw.value+graph_range/2)%graph_range;
				plotted_value[1] = ((int16_t)filtered_pitch.value+graph_range/2)%graph_range;
				plotted_value[2] = ((int16_t)filtered_roll.value+graph_range/2)%graph_range;
				plotted_value[3] = ((int16_t)filtered_scale.value+graph_range/2)%graph_range;
				for(int n = 0 ; n < 4 ; n++)
				{
					if(plotted_value[n] < 0)
					{
						plotted_value[n] += graph_range;
					}
				}

				vs_post_set_channel(display01);
				vs_post_int16(plotted_value,1,4);

				plotted_value[0] = ((int16_t)(filtered_yaw.change_log_sum*10)+graph_range/2)%graph_range;
				plotted_value[1] = ((int16_t)(filtered_pitch.change_log_sum*10)+graph_range/2)%graph_range;
				plotted_value[2] = ((int16_t)(filtered_roll.change_log_sum*10)+graph_range/2)%graph_range;
				plotted_value[3] = ((int16_t)(filtered_scale.change_log_sum*10)+graph_range/2)%graph_range;
				for(int n = 0 ; n < 4 ; n++)
				{
					if(plotted_value[n] < 0)
					{
						plotted_value[n] += graph_range;
					}
				}

				vs_post_set_channel(display02);
				vs_post_int16(plotted_value,1,4);
		}


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//HANDLE TAKING NEW KEYFRAME

		if(take_keyframe || user_forced_take_keyframe)
		{
			send_keyframe_to_host = true;
			take_keyframe = false;

			vs_led_toggle(VS_LED_1);

//			double rot_angle_in_rad = -roll.KF_value*yaw_scalar*M_PI/(180.0*100.0);
//			double xmag = cos(rot_angle_in_rad);
//			double ymag = sin(rot_angle_in_rad);

			yaw.KF_value = yaw.KF_value + yaw.relative_value;
			yaw.relative_value = 0;

			pitch.KF_value = pitch.KF_value + pitch.relative_value;
			pitch.relative_value = 0;

			roll.KF_value = roll.KF_value + roll.relative_value;
			roll.relative_value = 0;

			scale.KF_value = scale.KF_value + scale.relative_value;
			scale.relative_value = 0;

			//UPDATE KEY-FRAME IMAGE
			scamp5_kernel_begin();
				MOV(R6,R5);
			scamp5_kernel_end();


			if(user_forced_take_keyframe)
			{
				user_forced_take_keyframe = false;

				yaw.reset();
				pitch.reset();
				roll.reset();
				scale.reset();

				filtered_yaw.reset();
				filtered_pitch.reset();
				filtered_roll.reset();
				filtered_scale.reset();

				paused = 0;
			}
		}


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			timer_test.reset();

			if(send_usb_images)
			{
				update_display(display00,display_00_red,display_00_green,debug_display);
			}

			time_stats.time_debug_output = timer_test.get_usec();


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//SPI OUTPUT

//		spi_coms(display00,image_requested,send_usb_images);



		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//OUTPUT FOR PIPE

		if(output_pose)
		{
			int Y = yaw.KF_value + yaw.relative_value;
			int P = pitch.KF_value + pitch.relative_value;
			int R = roll.KF_value + roll.relative_value;

			int S = scale.KF_value + scale.relative_value;

			std::string test = ",Y" +  std::to_string(Y) + ",P" +  std::to_string(P)
			 + ",R" +  std::to_string(R) + ",S" +  std::to_string(S);
			vs_post_text(test.c_str());
			vs_post_text("\n");
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		time_stats.time_total = timer_total.get_usec();


//		vs_post_text("%s m0\n", vs_get_version(0));
//		vs_post_text("%s m4\n", vs_get_version(1));

		//POST TIMNING STATS
		if(output_stats && stats_trigger_timer.get_usec() > 1000000)
		{
//			vs_post_text("FPS:%.0f %.0fus edge:%.0f%% PC:%.0f%% priorTF:%.0f%% align:%.0f%% display:%.0f%% \n",
//					1000000/time_stats.time_total,
//					100*time_stats.time_total,
//					100*time_stats.time_edge_image_creation/time_stats.time_total,
//					100*time_stats.time_perspective_correction/time_stats.time_total,
//					100*time_stats.time_apply_prior_transform/time_stats.time_total,
//					100*time_stats.time_alignment_search/time_stats.time_total,
//					100*time_stats.time_debug_output/time_stats.time_total);

			vs_post_text("FPS:%.0f %.0fus edge:%.0f%%  priorTF:%.0f%% align:%.0f%% display:%.0f%% \n",
								1000000/time_stats.time_total,
								100*time_stats.time_total,
								100*time_stats.time_edge_image_creation/time_stats.time_total,

								100*time_stats.time_apply_prior_transform/time_stats.time_total,
								100*time_stats.time_alignment_search/time_stats.time_total,
								100*time_stats.time_debug_output/time_stats.time_total);

			stats_trigger_timer.reset();
		}

		vs_loop_counter_inc();
	}
}


int main(){

	vs_init();
	vs_post_bind_io_agent(vs_usb);
	scamp5_bind_io_agent(vs_usb);

	scamp5_main();

	return 0;
}

