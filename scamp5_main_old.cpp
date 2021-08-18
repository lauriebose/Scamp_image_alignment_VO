//#include <scamp5.hpp>
//#include <math.h>
//#include "GLOBALS.hpp"
//#include "KF_ALIGNMENT.hpp"
//#include "EDGE_IMAGE_GENERATION.hpp"
////#include "PERSPECTIVE_CORRECTION.hpp"
//#include "VECTOR_OPS.hpp"
//#include "IMG_TF.hpp"
//
//#include "display_drawing.hpp"
//#include "spi_coms.hpp"
//#include "TEST_IMAGE_GENERATION.hpp"
//
//const int locked_frame_rate = -1; //-1 = use slider
//
//const bool send_usb_images = true;
//int send_birds_eye_map = 0;
//
//
//uint8_t image_requested = 0; // User requested image over SPI when > 0, number determines which type of image
//bool user_forced_take_keyframe = false;
//
//using namespace SCAMP5_PE;
//
//static bool take_keyframe = true;
//extern bool send_keyframe_to_host;
//
//static int debug_display = 1;
//
//vs_stopwatch timer_total;
//vs_stopwatch timer_test;
//vs_stopwatch stats_trigger_timer;
//
//const int display_size = 2;
//vs_handle display00,display01,display10,display11;
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////EXTERNAL FUNCTIONS
//
//void apply_prior_transformations_to_current_image_and_keyframe();
//
//int image_alignment();
//
//void make_debug_display();
//
//extern int use_dreg_sum;
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////EXTERNAL FUNCTIONS
//
//#define MAX_BIRDS_EYE_PATH_LENGTH	500
//int prev_birds_eye_path_index = 0;
//int birds_eye_path_index = 0;
//int birds_eye_path_drawing_index = 0;
//int birds_eye_path_length = 0;
//int birds_eye_path_spacing = 2;
//uint8_t *birds_eye_path_data;
//int birds_eye_path_offx = 0, birds_eye_path_offy = 0;
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//const int graph_range = 400;
//const int graph_time_frame = 1000;
//
//int birds_eye_plot_scale = 20;
//int birds_eye_offx = 0, birds_eye_offy = 0;
//int birds_eye_x = 0, birds_eye_y = 0;
//int prev_birds_eye_x = 0, prev_birds_eye_y = 0;
//int yaw_scalar = 0;
//
//int slider_min_loop,slider_ext_loop;
//
//int use_generated_test_image = 0;
//
//void setup_host_gui()
//{
//	auto slider_vxc = vs_gui_add_slider("vxc: ",0,4095,2900);
//	  vs_on_gui_update(slider_vxc,[&](int32_t new_value){
//			vs_scamp5_configure_voltage(3,new_value);
//			});
//
//	auto slider_debug_display =  vs_gui_add_slider("debug display: ",0,3,0,&debug_display);
//	vs_gui_add_switch("birds eye view: ",false,&send_birds_eye_map);
//	vs_gui_add_switch("generated_image: ",false,&use_generated_test_image);
//
//
//	auto button_force_take_keyframe = vs_gui_add_button("force_take_keyframe:");
//	vs_on_gui_update(button_force_take_keyframe,[&](int32_t new_value){
//	    	vs_post_text("user forced new key-frame\n");
//	    	user_forced_take_keyframe = true;
//	    	take_keyframe = true;
//	    });
//
//	vs_gui_add_slider("edge_threshold: ",0,100,15,&edge_threshold);
////	vs_gui_add_slider("edge expansion: ",0,10,0,&edge_expansion);
//	vs_gui_add_slider("HDR iteration: ",0,50,1,&HDR_iterations);
//
//
//	auto slider_kf_threshold =  vs_gui_add_slider("key-frame threshold: ",10,120,20);
//	vs_on_gui_update(slider_kf_threshold,[&](int32_t new_value){
//	    	kf_yaw_pitch_trigger = new_value;
//	    	kf_roll_trigger = new_value;
//
//	    });
//
//	vs_gui_add_switch("roll tracking: ",true,&enable_roll_tracking);
//	vs_gui_add_switch("scale tracking: ",false,&enable_scale_tracking);
//	vs_gui_add_switch("motion prior: ",true,&use_rates_as_prior);
//
//	vs_gui_add_slider("birds_eye_plot_scale: ",0,40,20,&birds_eye_plot_scale);
//
//	vs_gui_add_slider("yaw_scalar: ",0,100,43,&yaw_scalar);
//
////	vs_gui_add_switch("use_dreg_sum: ",true,&use_dreg_sum);
//
////	vs_gui_add_switch("perspective correction: ",true,&enabled_perspective_correction);
//
//
//
////	auto slider_spi_scaling = vs_gui_add_slider("spi_scaling: / 1000: ",0,10000,8,&spi_scaling);
////	 vs_on_gui_update(slider_spi_scaling,[&](int32_t new_value){
////				spi_roll_angle = spi_roll*0.001*new_value;
////				spi_pitch_angle = spi_pitch*0.001*new_value;
////				 });
//
//    vs_gui_add_slider("base iterations: ",0,10,5,&slider_min_loop);
//    vs_gui_add_slider("over iterations: ",0,10,5,&slider_ext_loop);
//
//	 vs_on_gui_update(VS_GUI_FRAME_RATE,[&](int32_t new_value){
//	        uint32_t new_framerate = new_value;
//	        if(locked_frame_rate == -1)
//			{
//				if(new_framerate > 0){
//					vs_frame_trigger_set(1,new_framerate);
//					vs_enable_frame_trigger();
//					vs_post_text("frame trigger: 1/%d\n",(int)new_framerate);
//				}else{
//					vs_disable_frame_trigger();
//					vs_post_text("frame trigger disabled\n");
//				}
//			}
//	    });
//
//	 vs_on_gui_update(VS_GUI_FRAME_GAIN,[&](int32_t new_value){
//		 	 gain_levels = new_value;
//		    });
//
//
//
//	display00 = vs_gui_add_display("00",0,0,display_size,"s1");
//	display10 = vs_gui_add_display("10",display_size,0,display_size,"s2");
//	VS_GUI_DISPLAY_STYLE(style_plot,R"JSON(
//		{
//			"plot_palette": "plot_cmyw",
//			"plot_palette_groups": 4
//		}
//		)JSON");
//	display01 = vs_gui_add_display("01",0,display_size,display_size,style_plot);
//	vs_gui_set_scope(display01,0,graph_range,graph_time_frame);
//	display11 = vs_gui_add_display("11",display_size,display_size,display_size);
//}
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//void reset_features()
//{
//	prev_birds_eye_path_index = 0;
//	birds_eye_path_index = 0;
//	birds_eye_path_drawing_index = 0;
//	birds_eye_path_length = 0;
//
//	for(int n = 0 ; n < MAX_BIRDS_EYE_PATH_LENGTH*2 ; n++)
//	{
//		birds_eye_path_data[n] = 0;
//	}
//}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//void initial_setup()
//{
//	birds_eye_path_data = (uint8_t*)vs_prm->malloc(MAX_BIRDS_EYE_PATH_LENGTH*2);// each feature is 2 bytes
//	reset_features();
//
//	vs_on_shutdown([&](){
//		vs_prm->free(birds_eye_path_data);// deallocate when M0 is restarted
//	});
//
//	vs_on_host_connect([&](){
//		scamp5_kernel::print_debug_info();
//		vs_led_on(VS_LED_2);
//	});
//
//	vs_on_host_disconnect([&](){
//		vs_enable_frame_trigger();
//		vs_led_off(VS_LED_2);
//		scamp5_kernel::print_debug_info();
//	});
//}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//void scamp5_main(){
//
////SPI INPUT
////	 vs_on_user_value([&](uint8_t id,int32_t value) {
//////	   vs_post_text("\nuser val %d\n",value);
////	   if (id == 0) {
////		 // User requested a new image
////		   image_requested = value;
////		 }
////
////	   if (id == 2) {
////		 // User forced keyframe
////		   vs_post_text("user forced keyframe");
////		   user_forced_take_keyframe = true;
////		 }
////
////	    //receive IMU data for perspectiv correction
////		if (id == 4)
////		{
////			spi_pitch = value;
////			spi_pitch_angle = spi_pitch*0.001*spi_scaling;
////	    }
////		if (id == 5)
////		{
////			spi_roll = -value;
////			spi_roll_angle = spi_roll*0.001*spi_scaling;
////	    }
////	});
//
//	initial_setup();
//
//	setup_host_gui();
//
//	//set inital frame rate
//    vs_frame_trigger_set(1,locked_frame_rate);
//    vs_enable_frame_trigger();
//
//	static struct time_stats{
//		float time_edge_image_creation ;
//		float time_perspective_correction;
//		float time_apply_prior_transform;
//		float time_alignment_search;
//		float time_debug_output;
//		float time_total;}time_stats;
//
//	debug_display_timer.reset();
//
//	user_forced_take_keyframe = true;
////	perspective_correction::pre_compute_row_insertions(angle_per_pixel);
////	perspective_correction::pre_compute_row_scalings(angle_per_pixel);
////	perspective_correction::adjust_row_scalings_positions();
//
//	while(1){
//
//		timer_total.reset();
//
//
//		vs_wait_frame_trigger();
//		vs_process_message();
//
//		//REFRESH STORED DREG DATA
//		scamp5_kernel_begin();
//			REFRESH(R6);
//		scamp5_kernel_end();
//
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//GET INPUT IMAGE
//
//		timer_test.reset();
//		if(!use_generated_test_image)
//		{
//			acquire_edge_image_R5(gain_levels,edge_threshold,edge_expansion,HDR_iterations,200 );
//		}
//		else
//		{
//			generate_test_image();
//		}
//		time_stats.time_edge_image_creation = timer_test.get_usec();
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//APPLY PC
//
//		timer_test.reset();
////		if(enabled_perspective_correction)
////		{
////			vector<double> facing = {0,0,1};
////
////			facing = vector_rot_Y(facing,spi_pitch_angle);
////	//		vs_post_text("facing 1 %f %f %f \n",facing[0],facing[1],facing[2]);
////
////			facing = vector_rot_X(facing,spi_roll_angle);
////	//		vs_post_text("facing 2 %f %f %f \n",facing[0],facing[1],facing[2]);
////
////			double mag_angle = acos(vector_dot(facing,{0,0,1}));
//////			double mag_angle = acos_approx3(vector_dot(facing,{0,0,1}));
////
////			double facing_angle = 0;
////			if(mag_angle > perspective_correction_trigger_angle)
////			{
////				facing_angle = acos(vector_dot({facing[0],facing[1],0},{1,0,0})/vector_mag({facing[0],facing[1],0}));
//////				facing_angle = acos_approx3(vector_dot({facing[0],facing[1],0},{1,0,0})/vector_mag({facing[0],facing[1],0}));
////				if(facing[1] > 0)
////				{
////					facing_angle *= -1;
////				}
////				scamp5_kernel_begin();
////					MOV(R11,R5);
////				scamp5_kernel_end();
////				perspective_correction::apply_perspective_correction_to_R11(facing_angle,mag_angle,angle_per_pixel);
////				scamp5_kernel_begin();
////					MOV(R5,R11);
////				scamp5_kernel_end();
////			}
////		}
//		time_stats.time_perspective_correction = timer_test.get_usec();
//
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//TRANSFORM CURRENT EDGE IMAGE ACCORDING TO CURRENT POSE RELATIVE TO KEYFRAME
//
//
//		timer_test.reset();
//		apply_prior_transformations_to_current_image_and_keyframe();
//		time_stats.time_apply_prior_transform = timer_test.get_usec();
//
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//		if(!paused)
//		{
//			//R5 CURRENT EDGE IMG
//			//R6 CURRENT EDGE KF
//			//R9 CURRENT BEST SHIFTED EDGE KF
//			//R10 CURRENT BEST ROTATED EDGE IMAGE
//
//			//ALIGN CURRENT IMAGE WITH KEYFRAME
//				timer_test.reset();
//				image_alignment();
//				time_stats.time_alignment_search = timer_test.get_usec();
//
//			/////////////////////////
//			//UPDATE POSE
//
//				filtered_yaw.update(yaw.change,false);
//				filtered_pitch.update(pitch.change,false);
//				filtered_roll.update(roll.change,true);
//				filtered_scale.update(scale.change,true);
//
//				yaw.relative_value += yaw.change;
//				pitch.relative_value += pitch.change;
//				roll.relative_value += roll.change;
//				scale.relative_value += scale.change;
//
//			/////////////////////////
//			//OUTPUT POSE CHANGES
//
//				vs_post_text("%d,%d,%d,%d\n",yaw.change,pitch.change,roll.change,scale.change);
//
//			/////////////////////
//			//DETERMINE IF NEW KEYFRAME SHOULD BE TAKEN
//
//				if(abs(yaw.relative_value) > kf_yaw_pitch_trigger ||
//					abs(pitch.relative_value) > kf_yaw_pitch_trigger ||
//					abs(roll.relative_value) > kf_roll_trigger ||
//					abs(scale.relative_value) > kf_scale_trigger)
//				{
//					take_keyframe = true;
//				}
//
//
//			/////////////////////////////////
//			//UPDATE TRACE GRAPHS
//
//				int16_t plotted_value[3];
//				plotted_value[0] = ((int16_t)filtered_yaw.value+graph_range/2)%graph_range;
//				plotted_value[1] = ((int16_t)filtered_pitch.value+graph_range/2)%graph_range;
//				plotted_value[2] = ((int16_t)filtered_roll.value+graph_range/2)%graph_range;
//				for(int n = 0 ; n < 3 ; n++)
//				{
//					if(plotted_value[n] < 0)
//					{
//						plotted_value[n] += graph_range;
//					}
//				}
//
//				vs_post_set_channel(display01);
//				vs_post_int16(plotted_value,1,3);
//		}
//
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//HANDLE TAKING NEW KEYFRAME
//
//		if(take_keyframe || user_forced_take_keyframe)
//		{
//			send_keyframe_to_host = true;
//			take_keyframe = false;
//
//			vs_led_toggle(VS_LED_1);
//
//			double rot_angle_in_rad = -roll.KF_value*yaw_scalar*M_PI/(180.0*100.0);
//			double xmag = cos(rot_angle_in_rad);
//			double ymag = sin(rot_angle_in_rad);
//
//			birds_eye_offx += (int)(xmag*yaw.relative_value+ymag*pitch.relative_value);
//			birds_eye_offy += (int)(xmag*pitch.relative_value-ymag*yaw.relative_value);
//
//			yaw.KF_value = yaw.KF_value + yaw.relative_value;
//			yaw.relative_value = 0;
//
//			pitch.KF_value = pitch.KF_value + pitch.relative_value;
//			pitch.relative_value = 0;
//
//			roll.KF_value = roll.KF_value + roll.relative_value;
//			roll.relative_value = 0;
//
//			scale.KF_value = scale.KF_value + scale.relative_value;
//			scale.relative_value = 0;
//
//			//UPDATE KEY-FRAME IMAGE
//			scamp5_kernel_begin();
//				MOV(R6,R5);
//			scamp5_kernel_end();
//
//
//			if(user_forced_take_keyframe)
//			{
//				user_forced_take_keyframe = false;
//
//				reset_features();
//
//				birds_eye_x =0;
//				birds_eye_y =0;
//				birds_eye_offx =0;
//				birds_eye_offy =0;
//
//				yaw.reset();
//				pitch.reset();
//				roll.reset();
//				scale.reset();
//
//				filtered_yaw.reset();
//				filtered_pitch.reset();
//				filtered_roll.reset();
//				filtered_scale.reset();
//
//				paused = 0;
//			}
//		}
//
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//			timer_test.reset();
//
//			if(send_usb_images)
//			{
//				update_display(display00,display10,debug_display);
//			}
//
//			if(send_birds_eye_map)
//			{
//				uint8_t events_pos[2];
//				//send replay path pos
//				events_pos[0] = birds_eye_path_data[birds_eye_path_drawing_index*2]+birds_eye_path_offx;
//				events_pos[1] = birds_eye_path_data[birds_eye_path_drawing_index*2+1]+birds_eye_path_offy;
//				birds_eye_path_drawing_index++;
//				if(birds_eye_path_drawing_index > birds_eye_path_length)
//				{
//					birds_eye_path_drawing_index = 0;
//				}
//	//			//send latest path pos
//	//			events_pos[2] = birds_eye_path_data[prev_birds_eye_path_index*2]+birds_eye_path_offx;
//	//			events_pos[3] = birds_eye_path_data[prev_birds_eye_path_index*2+1]+birds_eye_path_offy;
//
//				scamp5_display_events(display11,events_pos,1);
//			}
//
//			time_stats.time_debug_output = timer_test.get_usec();
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			//CALCULATE CURRENT 2D BIRDS EYE POSE
//
//			if(send_birds_eye_map)
//			{
//				prev_birds_eye_x = birds_eye_x;
//				prev_birds_eye_y = birds_eye_y;
//
//				double kf_rot_angle_in_rad = -roll.KF_value*0.4*M_PI/180.0;
//				double xmag = cos(kf_rot_angle_in_rad);
//				double ymag = sin(kf_rot_angle_in_rad);
//				birds_eye_x = (int)(birds_eye_offx+xmag*yaw.relative_value+ymag*pitch.relative_value);
//				birds_eye_y = (int)(birds_eye_offy+xmag*pitch.relative_value-ymag*yaw.relative_value);
//
//	//OUTPUT BIRDS EYE POSE
//	//		double rot_angle_in_rad = -(roll.KF_value+roll.relative_value)*yaw_scalar*M_PI/(180.0*100.0);
//	//		vs_post_text("%d, %d, %f \n",birds_eye_x,birds_eye_y,rot_angle_in_rad);
//
//				//UPDATE STORED BIRDS EYE PATH
//				uint8_t new_birds_eye_plot_x = -birds_eye_x/birds_eye_plot_scale+127;
//				uint8_t new_birds_eye_plot_y = birds_eye_y/birds_eye_plot_scale+127;
//				if(abs(new_birds_eye_plot_x - birds_eye_path_data[prev_birds_eye_path_index*2]) > birds_eye_path_spacing
//						|| abs(new_birds_eye_plot_y - birds_eye_path_data[prev_birds_eye_path_index*2+1]) > birds_eye_path_spacing)
//				{
//					birds_eye_path_data[birds_eye_path_index*2] = new_birds_eye_plot_x;
//					birds_eye_path_data[birds_eye_path_index*2+1]= new_birds_eye_plot_y;
//					prev_birds_eye_path_index = birds_eye_path_index;
//					birds_eye_path_index++;
//					if(birds_eye_path_index > MAX_BIRDS_EYE_PATH_LENGTH)
//					{
//						birds_eye_path_index = 0;
//					}
//
//					birds_eye_path_length++;
//					if(birds_eye_path_length > MAX_BIRDS_EYE_PATH_LENGTH)
//					{
//						birds_eye_path_length = MAX_BIRDS_EYE_PATH_LENGTH;
//					}
//				}
//			}
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		//SPI OUTPUT
//
////		spi_coms(display00,image_requested,send_usb_images);
//
//
//		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		time_stats.time_total = timer_total.get_usec();
//
//
////		vs_post_text("%s m0\n", vs_get_version(0));
////		vs_post_text("%s m4\n", vs_get_version(1));
//
//		//POST TIMNING STATS
////		if(stats_trigger_timer.get_usec() > 1000000)
////		{
////			vs_post_text("FPS:%.0f %.0fus edge:%.0f%% PC:%.0f%% priorTF:%.0f%% align:%.0f%% display:%.0f%% \n",
////					1000000/time_stats.time_total,
////					100*time_stats.time_total,
////					100*time_stats.time_edge_image_creation/time_stats.time_total,
////					100*time_stats.time_perspective_correction/time_stats.time_total,
////					100*time_stats.time_apply_prior_transform/time_stats.time_total,
////					100*time_stats.time_alignment_search/time_stats.time_total,
////					100*time_stats.time_debug_output/time_stats.time_total);
////
////			stats_trigger_timer.reset();
////		}
//
//		vs_loop_counter_inc();
//	}
//}
//
//
//int main(){
//
//	vs_init();
//	vs_post_bind_io_agent(vs_usb);
//	scamp5_bind_io_agent(vs_usb);
//
//	scamp5_main();
//
//	return 0;
//}
//
