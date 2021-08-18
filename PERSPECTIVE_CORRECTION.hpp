#include "IMG_TF.hpp"
#include "REGISTER_ENUMS.hpp"
//  auto display_3 = vs_gui_add_display("display 3",2,1,1);

namespace perspective_correction
{
	const int row_insertion_count = 128;
	static signed char row_insertion_jump_locations[row_insertion_count];

	void pre_compute_row_insertions(double angular_res)
	{
		vs_post_text("precomputing row insertion locations\n");

		double sin_pixang = sin(angular_res);
		double cos_pixang = cos(angular_res);

		double tmp_val = cos(-angular_res * 0.5);
		double tmp_val2 = cos(-angular_res * 0.5 + angular_res);
		double default_area = sin_pixang / (tmp_val * tmp_val2);

		double counter = 0;
		int index = 0;

		bool found_first_insertion = false;
		bool found_first_double_insertion = false;
		int previous_jump_location = 0;

		int max_range = 512;
		for (int n = 0; n < max_range; n++)
		{
			tmp_val = cos(n * angular_res);
			tmp_val2 = cos(n * angular_res + angular_res);
			double tmp_val3 = sin_pixang / (tmp_val * tmp_val2);
			counter += tmp_val3;
			counter -= default_area;

			int rows_inserted = 0;
			while (counter > default_area)
			{
				rows_inserted++;
				if (rows_inserted > 1)
				{
					found_first_double_insertion = true;
				}
				counter -= default_area;
			}

			if(rows_inserted > 0)
			{
				if(found_first_insertion)
				{
					if(!found_first_double_insertion)
					{
						row_insertion_jump_locations[index] = n - previous_jump_location;
					}
					else
					{
						row_insertion_jump_locations[index] = -rows_inserted;
					}
				}
				else
				{
					row_insertion_jump_locations[index] = n;
					found_first_insertion = true;
				}
				index++;
				previous_jump_location = n;

				if (index >= row_insertion_count)
				{
					vs_post_text("hit index limit for row insertion locations\n");
					break;
				}
				vs_post_text(" %d/%d :", n, (int)row_insertion_jump_locations[index]);
				if (index % 20 == 0)
				{
					vs_post_text("\n");
				}
			}
		}
		vs_post_text("precomputed %d row insertion locations\n", index);
	}

	const int row_scaling_count = 64;
	static char row_scaling_locations[row_scaling_count];

	void pre_compute_row_scalings(double angular_res)
	{
		vs_post_text("precomputing row scales\n");

		double default_area = 1;

		int index = 0;
		int last_scaling_mag = 0;
		int range = 512;
		int accuracy = 10;

		bool found_first_jump_location = false;
		int last_jump_location = 0;

		for (int n = 0; n < range; n++)
		{
			double total_area = 0;
			double cos_pixang1 = cos(angular_res * n);
			for (int m = 0; m + accuracy < 128; m += accuracy)
			{
				double cos_pixang2 = cos(angular_res * m);
				double cos_pixang3 = cos(angular_res * m + angular_res * 0.5);
				total_area += accuracy * 1.0
						/ (cos_pixang1 * cos_pixang2 * cos_pixang3);
			}
			total_area = total_area * 2;

			if (n == 0)
			{
				default_area = total_area;
			}
			double area_ratio = total_area / default_area;

			int scaling_mag = round((area_ratio - 1.0) * 255.0);
			scaling_mag -= scaling_mag % 2;
			scaling_mag = scaling_mag / 2;

			if (scaling_mag > last_scaling_mag)
			{
				int scaling_steps = scaling_mag - last_scaling_mag;

				while (scaling_steps > 0)
				{
					if (found_first_jump_location)
					{
						row_scaling_locations[index] = n;
					}
					else
					{
						row_scaling_locations[index] = n - last_jump_location;
					}
					last_jump_location = n;
					vs_post_text(" %d/%d :", n, (int)row_scaling_locations[index]);
					if (index % 20 == 0)
					{
						vs_post_text("\n");
					}

					index++;
					if (index >= row_scaling_count)
					{
						vs_post_text("hit index limit for row scalings\n");
						n = range;
						break;
					}

					scaling_steps--;
				}

				last_scaling_mag = scaling_mag;
			}

	//		vs_post_text("%d of %d \n",n,range);
		}

		vs_post_text("precomputed %d row scales\n", index);
	}

	void adjust_row_scalings_positions()
	{
		vs_post_text("adjusting precomputed row scales\n");

		int verticle_location_of_row_scaling = 0;

		int vert_index = 0;
		int verticle_location_of_row_insertion =
				row_insertion_jump_locations[vert_index];

		for (int n = 0; n < row_scaling_count; n++)
		{
			int inserted_rows = 0;
			verticle_location_of_row_scaling += row_scaling_locations[n];

			//COUNT HOW MANY ROWS ARE INSERTED BELOW THE POSTION OF THIS ROW SCALING
			//THEN ADD THIS NUMBER TO THE ROW SCALINGS LOCATION TO SHIFT IT VERTICALLY TO ITS CORRECT POSITION
			while (verticle_location_of_row_scaling
					> verticle_location_of_row_insertion)
			{
				if(row_insertion_jump_locations[vert_index] > 0)
				{
					verticle_location_of_row_insertion += row_insertion_jump_locations[vert_index];
					inserted_rows++;
					row_scaling_locations[n]++;
				}
				else
				{
					verticle_location_of_row_insertion++;
					inserted_rows+= -row_insertion_jump_locations[vert_index];
					row_scaling_locations[n]+= -row_insertion_jump_locations[vert_index];
				}
				vert_index++;

				if (vert_index > row_insertion_count)
				{
					vs_post_text(
							"hit index limit for verticle jumps while adjusting row scales\n");
					vs_post_text("adjusted %d row scales\n", n);
					n = row_scaling_count;
					break;
				}
			}
			vs_post_text(" %d/%d/%d :", verticle_location_of_row_scaling, (int)row_scaling_locations[n],inserted_rows);
			if (n % 20 == 0)
			{
				vs_post_text("\n");
			}
			verticle_location_of_row_scaling += inserted_rows;
		}
	}

	void perspective_correct_rows_of_B(double camera_angle, double angle_per_pixel,
			bool switch_rows_with_cols)
	{

		int start_pix = floor(-camera_angle / angle_per_pixel);
		int index = 0;
		int scaling_step = 0;

		int location = row_scaling_locations[index];
		int magnitude = 1;

		scamp5_kernel_begin();
		mov(F, B);
		scamp5_kernel_end()
		;

		while (start_pix + location < -127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPX_F(scaling_step);
				}
				else
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPY_F(scaling_step);
				}
				scaling_step++;
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}

		scamp5_kernel_begin();
		mov(B, F);
		mov(A, F);
		scamp5_kernel_end()
		;

		while (start_pix + location < 127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPX_F(scaling_step);
				}
				else
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPY_F(scaling_step);
				}
				scaling_step++;
			}

			if (!switch_rows_with_cols)
			{
				scamp5_load_rect(R1, 0, 0, 127 - start_pix - location, 255);
				scamp5_kernel_begin();
				WHERE (R1);
				mov(B, F);
				all();
				scamp5_kernel_end();
			}
			else
			{
				scamp5_load_rect(R1, 0, 0, 255, 127 - start_pix - location);
				scamp5_kernel_begin();
				WHERE (R1);
				mov(B, F);
				all();
				scamp5_kernel_end();
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}
	//	 vs_post_text("%d %d: \n", index, start_pix);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		index = 0;
		scaling_step = 0;

		location = row_scaling_locations[index];
		magnitude = 1;

		scamp5_kernel_begin();
		mov(F, B);
		scamp5_kernel_end()
		;

		while (start_pix - location > 127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPX_F(scaling_step);
				}
				else
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPY_F(scaling_step);
				}
				scaling_step++;
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}

		scamp5_kernel_begin();
		mov(B,F);
		scamp5_kernel_end()
		;

		while (start_pix - location > -127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPX_F(scaling_step);
				}
				else
				{
					IMGTF::SCALING::ANALOG::STEP_SCALE_UPX_F(scaling_step);
				}
				scaling_step++;
			}

			if (!switch_rows_with_cols)
			{
				scamp5_load_rect(R1, 127 - start_pix + location, 0, 255, 255);
				scamp5_kernel_begin();
				WHERE (R1);
				mov(B, F);
				all();
				scamp5_kernel_end()
				;
			}
			else
			{
				scamp5_load_rect(R1, 0, 127 - start_pix + location, 255, 255);
				scamp5_kernel_begin();
				WHERE (R1);
				mov(B, F);
				all();
				scamp5_kernel_end()
				;
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}
	//	 vs_post_text("%d %d: \n", index, start_pix);
	}

	void perspective_correct_cols_of_B(double camera_angle, double angle_per_pixel,
			bool switch_cols_with_rows, bool binary_image)
	{
		int start_pix = floor(-camera_angle / angle_per_pixel);

		int index = 0;
		int cols_inserted = 0;

		int location = row_insertion_jump_locations[index];
		int magnitude = 1;

		if (binary_image)
		{
			scamp5_in(E, -127);
		}

		while (-start_pix - location - cols_inserted > 127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_cols_with_rows)
				{
					scamp5_kernel_begin();
					bus(NEWS, B);
					bus(B, NORTH);
					scamp5_kernel_end()
					;
				}
				else
				{
					scamp5_kernel_begin();
					bus(NEWS, B);
					bus(B, WEST);
					scamp5_kernel_end()
					;
				}
			}
			cols_inserted += magnitude;

			if (binary_image)
			{
				scamp5_kernel_begin();
				mov(A, B);
				mov(B, E);
				where (A);
				sub(B, B, E);
				sub(B, B, E);
				all();
				scamp5_kernel_end()
				;
			}

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
		}

		while (-start_pix - location - cols_inserted - magnitude > -127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (-start_pix - location - m - cols_inserted < -127)
				{
					break;
				}
				if (!switch_cols_with_rows)
				{
					scamp5_load_rect(R1, 0, 0,
							127 - start_pix - location - m - cols_inserted, 255);
					scamp5_kernel_begin();
					bus(NEWS, B);
					WHERE (R1);
					bus(B, NORTH);
					all();
					scamp5_kernel_end()
					;
				}
				else
				{
					scamp5_load_rect(R1, 0, 0, 255,
							127 - start_pix - location - m - cols_inserted);
					scamp5_kernel_begin();
					bus(NEWS, B);
					WHERE (R1);
					bus(B, WEST);
					all();
					scamp5_kernel_end()
					;
				}
			}
			cols_inserted += magnitude;

			if (binary_image)
			{
				scamp5_kernel_begin();
				mov(A, B);
				mov(B, E);
				where (A);
				sub(B, B, E);
				sub(B, B, E);
				all();
				scamp5_kernel_end()
				;
			}

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
	//		vs_post_text("%d : ",row_insetion_jump_locations[index]);
	//		if (index % 20 == 0)
	//		{
	//			vs_post_text("\n");
	//		}
		}
	//	 vs_post_text("%d %d: \n", index, start_pix);

		//////////////////////////////////////////

		index = 0;
		cols_inserted = 0;

		location = row_insertion_jump_locations[index];
		magnitude = 1;

		while (-start_pix + location + cols_inserted < -127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_cols_with_rows)
				{
					scamp5_kernel_begin();
					bus(NEWS, B);
					bus(B, SOUTH);
					scamp5_kernel_end()
					;
				}
				else
				{
					scamp5_kernel_begin();
					bus(NEWS, B);
					bus(B, EAST);
					scamp5_kernel_end()
					;
				}
			}
			cols_inserted += magnitude;

			if (binary_image)
			{
				scamp5_kernel_begin();
				mov(A, B);
				mov(B, E);
				where (A);
				sub(B, B, E);
				sub(B, B, E);
				all();
				scamp5_kernel_end()
				;
			}

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
		}

		while (-start_pix + location + cols_inserted + magnitude < 127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (-start_pix + location + m + cols_inserted > 127)
				{
					break;
				}

				if (!switch_cols_with_rows)
				{
					scamp5_load_rect(R1,
							127 - start_pix + location + m + cols_inserted, 0, 255,
							255);
					scamp5_kernel_begin();
					bus(NEWS, B);
					WHERE (R1);
					bus(B, SOUTH);
					all();
					scamp5_kernel_end()
					;
				}
				else
				{
					scamp5_load_rect(R1, 0,
							127 - start_pix + location + m + cols_inserted, 255,
							255);
					scamp5_kernel_begin();
					bus(NEWS, B);
					WHERE (R1);
					bus(B, EAST);
					all();
					scamp5_kernel_end()
					;
				}
			}
			cols_inserted += magnitude;

			if (binary_image)
			{
				scamp5_kernel_begin();
				mov(A, B);
				mov(B, E);
				where (A);
				sub(B, B, E);
				sub(B, B, E);
				all();
				scamp5_kernel_end()
				;
			}

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
		}
	//	 vs_post_text("%d %d: \n", index, start_pix);
	}

	void apply_perspective_correction_to_B(double facing_ang, double correction_ang,
			double angle_per_pix)
	{
		scamp5_kernel_begin();
		mov(A, B);
		scamp5_kernel_end()
		;

		bool switch_rows_with_cols = false;

		if (facing_ang > M_PI * 0.5)
		{
			facing_ang -= M_PI;
			correction_ang = -correction_ang;
		}
		else
		{
			if (facing_ang < -M_PI * 0.5)
			{
				facing_ang += M_PI;
				correction_ang = -correction_ang;
			}
		}

		if (facing_ang > M_PI * 0.25)
		{
			facing_ang = -M_PI * 0.5 + facing_ang;
			correction_ang = -correction_ang;
			switch_rows_with_cols = true;
		}
		else
		{
			if (facing_ang < -M_PI * 0.25)
			{
				facing_ang = M_PI * 0.5 + facing_ang;
				correction_ang = correction_ang;
				switch_rows_with_cols = true;
			}
		}

		IMGTF::ROTATION::ANALOG::ROT_3SKEWS_AREG(AENUM::A, -facing_ang);

		scamp5_kernel_begin();
		mov(B, A);
		scamp5_kernel_end()

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		perspective_correct_rows_of_B(correction_ang, angle_per_pix,
				switch_rows_with_cols);

		perspective_correct_cols_of_B(correction_ang, angle_per_pix,
				switch_rows_with_cols, false);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		scamp5_kernel_begin();
		mov(A, B);
		scamp5_kernel_end();

		IMGTF::ROTATION::ANALOG::ROT_3SKEWS_AREG(AENUM::A, facing_ang);

		scamp5_kernel_begin();
		mov(B, A);
		scamp5_kernel_end();
	}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





	void perspective_correct_cols_of_R11(double camera_angle, double angle_per_pixel,bool switch_cols_with_rows)
	{
		int start_pix = floor(-camera_angle / angle_per_pixel);

		int index = 0;
		int cols_inserted = 0;

		int location = row_insertion_jump_locations[index];
		int magnitude = 1;

		while (-start_pix - location - cols_inserted > 127)
		{
			if (!switch_cols_with_rows)
			{
				scamp5_kernel_begin()
					CLR(R3,R2,R4);
					SET(R1); //setup north flag for DNEWS on all pixels
				scamp5_kernel_end();
			}
			else
			{
				scamp5_kernel_begin()
					CLR(R1,R3,R4);
					SET(R2); //setup west flag for DNEWS on all pixels
				scamp5_kernel_end();
			}

			scamp5_kernel_begin()
				SET(R0);
				MOV(R12,R11);
			scamp5_kernel_end();
			for (int m = 0; m < magnitude; m++)
			{
				scamp5_kernel_begin();
					DNEWS0(R11,R12);
					MOV(R12,R11);
				scamp5_kernel_end();
			}
			cols_inserted += magnitude;
			scamp5_kernel_begin()
				MOV(R11,R12);
			scamp5_kernel_end();

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
		}

		while (-start_pix - location - cols_inserted - magnitude > -127)
		{
			scamp5_kernel_begin()
				CLR(R1,R2,R3,R4);
				SET(R0);
				MOV(R12,R11);
			scamp5_kernel_end();
			for (int m = 0; m < magnitude; m++)
			{
				if (-start_pix - location - m - cols_inserted < -127)
				{
					break;
				}
				if (!switch_cols_with_rows)
				{
					scamp5_load_rect(R1, 0, 0, 127 - start_pix - location - m - cols_inserted, 255);
					scamp5_kernel_begin();
						DNEWS0(R11,R12);
						MOV(R0,R1);
						MOV(R12,R11);
					scamp5_kernel_end();
				}
				else
				{
					scamp5_load_rect(R2, 0, 0, 255, 127 - start_pix - location - m - cols_inserted);
					scamp5_kernel_begin();
						DNEWS0(R11,R12);
						MOV(R0,R2);
						MOV(R12,R11);
					scamp5_kernel_end();
				}
			}
			cols_inserted += magnitude;

			scamp5_kernel_begin()
				MOV(R11,R12);
			scamp5_kernel_end();

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
	//		vs_post_text("%d : ",row_insetion_jump_locations[index]);
	//		if (index % 20 == 0)
	//		{
	//			vs_post_text("\n");
	//		}
		}
	//	 vs_post_text("%d %d: \n", index, start_pix);

		//////////////////////////////////////////

		index = 0;
		cols_inserted = 0;

		location = row_insertion_jump_locations[index];
		magnitude = 1;

		while (-start_pix + location + cols_inserted < -127)
		{

			if (!switch_cols_with_rows)
			{
				scamp5_kernel_begin()
					CLR(R1,R2,R4);
					SET(R3); //setup north flag for DNEWS on all pixels
				scamp5_kernel_end();
			}
			else
			{
				scamp5_kernel_begin()
					CLR(R2,R3,R4);
					SET(R4); //setup west flag for DNEWS on all pixels
				scamp5_kernel_end();
			}

			scamp5_kernel_begin()
				SET(R0);
				MOV(R12,R11);
			scamp5_kernel_end();
			for (int m = 0; m < magnitude; m++)
			{
				scamp5_kernel_begin();
					DNEWS0(R11,R12);
					MOV(R12,R11);
				scamp5_kernel_end();
			}
			cols_inserted += magnitude;
			scamp5_kernel_begin()
				MOV(R11,R12);
			scamp5_kernel_end();

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
		}

		while (-start_pix + location + cols_inserted + magnitude < 127)
		{
			scamp5_kernel_begin()
				CLR(R1,R2,R3,R4);
				SET(R0);
				MOV(R12,R11);
			scamp5_kernel_end();
			for (int m = 0; m < magnitude; m++)
			{
				if (-start_pix + location + m + cols_inserted > 127)
				{
					break;
				}

				if (!switch_cols_with_rows)
				{
					scamp5_load_rect(R3,127 - start_pix + location + m + cols_inserted, 0, 255,255);
					scamp5_kernel_begin();
						DNEWS0(R11,R12);
						MOV(R0,R3);
						MOV(R12,R11);
					scamp5_kernel_end();

				}
				else
				{
					scamp5_load_rect(R4, 0,127 - start_pix + location + m + cols_inserted, 255,255);
					scamp5_kernel_begin();
						DNEWS0(R11,R12);
						MOV(R0,R4);
						MOV(R12,R11);
					scamp5_kernel_end();
				}
			}
			cols_inserted += magnitude;
			scamp5_kernel_begin()
				MOV(R11,R12);
			scamp5_kernel_end();

			index++;
			if(row_insertion_jump_locations[index] > 0)
			{
				location += row_insertion_jump_locations[index];
				magnitude = 1;
			}
			else
			{
				location += magnitude;
				magnitude = -row_insertion_jump_locations[index];
			}
		}
	//	 vs_post_text("%d %d: \n", index, start_pix);
	}



	void perspective_correct_rows_of_R11(double camera_angle, double angle_per_pixel,
			bool switch_rows_with_cols)
	{

		int start_pix = floor(-camera_angle / angle_per_pixel);
		int index = 0;
		int scaling_step = 0;

		int location = row_scaling_locations[index];
		int magnitude = 1;

		while (start_pix + location < -127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPX_R11(scaling_step);
				}
				else
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPY_R11(scaling_step);
				}
				scaling_step++;
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}

		scamp5_kernel_begin();
			MOV(R10,R11);
		scamp5_kernel_end();
		while (start_pix + location < 127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPX_R11(scaling_step);
				}
				else
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPY_R11(scaling_step);
				}
				scaling_step++;
			}

			if (!switch_rows_with_cols)
			{
				scamp5_load_rect(R1, 0, 0, 127 - start_pix - location, 255);
				scamp5_kernel_begin();
					SET(R0);
					MOV(R12,R10);
					MOV(R0,R1);
					MOV(R12,R11);
					MOV(R10,R12);
				scamp5_kernel_end();
			}
			else
			{
				scamp5_load_rect(R1, 0, 0, 255, 127 - start_pix - location);
				scamp5_kernel_begin();
					SET(R0);
					MOV(R12,R10);
					MOV(R0,R1);
					MOV(R12,R11);
					MOV(R10,R12);
				scamp5_kernel_end();
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}
		scamp5_kernel_begin();
			MOV(R11,R10);
		scamp5_kernel_end();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		index = 0;
		scaling_step = 0;

		location = row_scaling_locations[index];
		magnitude = 1;

		while (start_pix - location > 127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPX_R11(scaling_step);
				}
				else
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPY_R11(scaling_step);
				}
				scaling_step++;
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}

		scamp5_kernel_begin();
			MOV(R10,R11);
		scamp5_kernel_end();
		while (start_pix - location > -127)
		{
			for (int m = 0; m < magnitude; m++)
			{
				if (!switch_rows_with_cols)
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPX_R11(scaling_step);
				}
				else
				{
					IMGTF::SCALING::DIGITAL::STEP_SCALE_UPY_R11(scaling_step);
				}
				scaling_step++;
			}

			if (!switch_rows_with_cols)
			{
				scamp5_load_rect(R1, 127 - start_pix + location, 0, 255, 255);
				scamp5_kernel_begin();
					SET(R0);
					MOV(R12,R10);
					MOV(R0,R1);
					MOV(R12,R11);
					MOV(R10,R12);
				scamp5_kernel_end();
			}
			else
			{
				scamp5_load_rect(R1, 0, 127 - start_pix + location, 255, 255);
				scamp5_kernel_begin();
					SET(R0);
					MOV(R12,R10);
					MOV(R0,R1);
					MOV(R12,R11);
					MOV(R10,R12);
				scamp5_kernel_end();
			}

			index++;
			location += row_scaling_locations[index];
			magnitude = 1;
		}
		scamp5_kernel_begin();
			MOV(R11,R10);
		scamp5_kernel_end();
//		vs_post_text("%d %d: \n", index, start_pix);
	}




	void apply_perspective_correction_to_R11(double facing_ang, double correction_ang,
				double angle_per_pix)
	{
		bool switch_rows_with_cols = false;

		if (facing_ang > M_PI * 0.5)
		{
			facing_ang -= M_PI;
			correction_ang = -correction_ang;
		}
		else
		{
			if (facing_ang < -M_PI * 0.5)
			{
				facing_ang += M_PI;
				correction_ang = -correction_ang;
			}
		}

		if (facing_ang > M_PI * 0.25)
		{
			facing_ang = -M_PI * 0.5 + facing_ang;
			correction_ang = -correction_ang;
			switch_rows_with_cols = true;
		}
		else
		{
			if (facing_ang < -M_PI * 0.25)
			{
				facing_ang = M_PI * 0.5 + facing_ang;
				correction_ang = correction_ang;
				switch_rows_with_cols = true;
			}
		}

		IMGTF::ROTATION::DIGITAL::ROT_3SKEWS_R11(-facing_ang);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		perspective_correct_rows_of_R11(correction_ang, angle_per_pix,switch_rows_with_cols);

		perspective_correct_cols_of_R11(correction_ang, angle_per_pix,switch_rows_with_cols);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		IMGTF::ROTATION::DIGITAL::ROT_3SKEWS_R11(facing_ang);
	}

}
