#include "sandcastle_sum.hpp"

using namespace SCAMP5_PE;

void DREG_STACKING_R10(int steps = 256)
{
	//ROUTINE FOR STACKING SET PIXELS, WITH EACH STEP SET PIXELS "FALL" INTO ANY EMPTY NEIGHBOURS BELOW
	scamp5_select_pattern(255,0,0,255);//SELECT BOTTOM ROW

	scamp5_kernel_begin();
		CLR(R2,R4);//CLEAR UNUSED DNEWS DIRECTIONS
	scamp5_kernel_end();
	for(int m = 0 ; m < steps ; m++)
	{
		scamp5_kernel_begin();
			MOV(R1,R10);// WHITE PIXELS COPY THOSE BELOW THEM
			NOT(R3,R10);//BLACKS PIXELS COPY THOSE ABOVE THEM
			DNEWS0(R11,R10); //FALL PIXELS

			AND(R3,SELECT,R10);//GET BOTTOM ROW OF CURRENT STACKED PIXLE IMAGE WHICH DNEWS HAS MOVEd OFF THE IMAGE PLANE IN R11
			OR(R10,R3,R11);//UPDATE STACKED PIXEL IMAGE, IE R11 + BOTTOM ROW OF PREVIOUS STACKED PIXEL IMAGE
		scamp5_kernel_end();
	}
	scamp5_kernel_begin();
			ALL();
	scamp5_kernel_end();
}

void DREG_STACKING_R10_X256()
{
	//ROUTINE FOR STACKING SET PIXELS, WITH EACH STEP SET PIXELS "FALL" INTO ANY EMPTY NEIGHBOURS BELOW
	//PERFORMS 256 STEPS
	scamp5_select_pattern(255,0,0,255);//SELECT BOTTOM ROW

	scamp5_kernel_begin();
		CLR(R2,R4);//CLEAR UNUSED DNEWS DIRECTIONS
	scamp5_kernel_end();
	for(int m = 0 ; m < 16 ; m++)
	{
		scamp5_kernel_begin();
		//LOOP INSIDE "KERNEL BEGIN" TO PRODUCE KERNEL CONTAINING MANY DUPLICATED INSTRUCTIONS (KERNEL COMPILED ONCE ON FIRST RUN)
		//SIGNIFICANTLY FASTER SENDING 16 LARGE KERNEL WITH DUPLICATED INSTRUCTIONS, THAN SENDING 256 SHORTER KERNELS
		for(int i = 0 ; i < 16 ; i++)
		{
			MOV(R1,R10);// WHITE PIXELS COPY THOSE BELOW THEM
			NOT(R3,R10);//BLACKS PIXELS COPY THOSE ABOVE THEM
			DNEWS0(R11,R10); //FALL PIXELS

			AND(R3,SELECT,R10);//GET BOTTOM ROW OF CURRENT STACKED PIXLE IMAGE WHICH DNEWS HAS MOVEd OFF THE IMAGE PLANE IN R11
			OR(R10,R3,R11);//UPDATE STACKED PIXEL IMAGE, IE R11 + BOTTOM ROW OF PREVIOUS STACKED PIXEL IMAGE
		}
		scamp5_kernel_end();
	}
	scamp5_kernel_begin();
			ALL();
	scamp5_kernel_end();
}


int SC_SUM_STEPWISE(dreg_t reg, int steps)
{
	scamp5_dynamic_kernel_begin();
		MOV(R10,reg);
	scamp5_dynamic_kernel_end();

	//STACK SET PIXELS
	DREG_STACKING_R10(steps);

	//ELIMINATE ALL SET PIXELS EXCEPT THOSE ON TOP OF A STACK
	scamp5_select_pattern(255,255,0,255);
	scamp5_kernel_begin();
		DNEWS(R11,R10,south,0);
		XOR(R1,R11,R10);
		NOT(R3,SELECT);
		AND(R2,R1,R3);
		MOV(R1,R2);
	scamp5_kernel_end();

	//READOUT REMAINING SET PIXELS, POTENTIALLY 256, ONE FOR EACH VERTICAL STACK
	uint8_t event_data [256*2];
	scamp5_scan_events(R1,event_data,256);

	int sum = 0;
	for(int n = 0 ; n < 256 ; n++)
	{
		if(event_data[n*2] == 0 && event_data[n*2+1] == 0)
		{
			break; //BREAK IF NO MORE STACKS REMAINING
		}
		//ADD TOGETHER STACK HEIGHTS TO DETERMINE TOTAL NUMBER OF SET PIXELS
		int stack_height = 255- event_data[n*2+1];
		sum = sum + stack_height;
	}
	return sum;
}

int SC_SUM(dreg_t reg, int sum_sparsity_value)
{
	if(reg.LR_bits != R10.LR_bits || reg.LR_index != R10.LR_index || reg.RR_bits != R10.RR_bits || reg.RR_index  != R10.RR_index)
	{
		scamp5_dynamic_kernel_begin();
			MOV(R10,reg);
		scamp5_dynamic_kernel_end();
	}

	//STACK SET PIXELS
	DREG_STACKING_R10_X256();

	//READOUT REMAINING SET PIXELS, POTENTIALLY 256, ONE FOR EACH VERTICAL STACK
	uint8_t event_data [256*2];
	int sum = 0;
	if(sum_sparsity_value > 0)
	{
		//PERFORM A SPARSE READOUT
		int event_sparsity = 1;
		for(int n = 0 ; n < sum_sparsity_value; n++)
		{
			event_sparsity*=2;
		}

		//LOAD PATTERN OF EQUALLY SPACE HORIZONTAL LINES FOR SPARSE EVENT READOUT AND ELIMINATE ALL SET PIXELS EXCEPT THOSE ON TOP OF A STACK
		scamp5_load_pattern(R8,0,event_sparsity/2,255,256-event_sparsity);
		scamp5_kernel_begin();
				DNEWS(R11,R10,north,0);
				XOR(R1,R10,R11);
				AND(R11,R1,R8);
		scamp5_kernel_end();
		scamp5_scan_events(R11,event_data,256/event_sparsity);

		for(int n = 0 ; n < 256/event_sparsity ; n++)
		{
			if(event_data[n*2] == 0 && event_data[n*2+1] == 0 && n != 0)
			{
				break;
			}
			sum = sum + (255- event_data[n*2+1])*event_sparsity;
		}
	}
	else
	{
		//PERFORM A WHOLE READOUT
		//ELIMINATE ALL SET PIXELS EXCEPT THOSE ON TOP OF A STACK
		scamp5_kernel_begin();
			DNEWS(R11,R10,north,0);
			XOR(R1,R11,R10);
		scamp5_kernel_end();

		scamp5_scan_events(R1,event_data,256);
		for(int n = 0 ; n < 256 ; n++)
		{
			if(n != 0 && event_data[n*2] == 0 && event_data[n*2+1] == 0)
			{
				break;//BREAK IF NO MORE STACKS REMAINING
			}
			//ADD TOGETHER STACK HEIGHTS TO DETERMINE TOTAL NUMBER OF SET PIXELS
			int stack_height = 255- event_data[n*2+1];
			sum = sum + stack_height;
		}
	}
	return sum;
}

void DREG_STACKING_R10_S5D_OPTIMIZED()
{
	//SELECT BOTTOM ROW
	scamp5_select_pattern(255,0,0,255);
	scamp5_in(F,15);
	scamp5_kernel_begin();
		icw(1,{plrb,bit,bitmode,wr,LR4});
		WHERE(R4);
	scamp5_kernel_end();
		scamp5_in(F,-120);
	scamp5_kernel_begin();
		all();
		bus(NEWS, F);

		CLR(R2,R4); //CLEAR UNUSED DNEWS DIRECTIONS
		MOV(R1,R10);//moved this out of loop below so first iteration runs properly
	scamp5_kernel_end();

	for(int m = 0 ; m < 16 ; m++)
	{
		scamp5_kernel_begin();
		for(int i = 0 ; i < 16 ; i++)
		{
			icw(1,{SEL4,seln,sels,plrb,bitmode,plwb,nbit,LR3,RR10});//amalgamating R3=!R10 and preset of boundary bus inputs
			 icw(2,{plrb,bit,s_in,rid,nb,LR11,RR10});//DNEWS(R9,R10) with floating boundary input
			icw(1,{LR1,LR10,plrb,bit,bitmode,RR11,SEL4,seln,sels}); //R1,R10 = R9; floating boundary defined
		}
		scamp5_kernel_end();
	}
}

int SC_SUM_S5D_OPTIMIZED(dreg_t reg, int sum_sparsity_value)
{
	if(reg.LR_bits != R10.LR_bits || reg.LR_index != R10.LR_index || reg.RR_bits != R10.RR_bits || reg.RR_index  != R10.RR_index)
	{
		scamp5_dynamic_kernel_begin();
			MOV(R10,reg);
		scamp5_dynamic_kernel_end();
	}

//	vs_stopwatch timer_stacking;
//	timer_stacking.reset();
	//STEVE VERSION
	DREG_STACKING_R10_S5D_OPTIMIZED();
//	vs_post_text("stacking time %d us\n",timer_stacking.get_usec());

//	vs_stopwatch timer_events;
//	timer_events.reset();

	int sum = 0;
	uint8_t event_data [256*2];
	if(sum_sparsity_value > 0)
	{
		int event_sparsity = 1;
		for(int n = 0 ; n < sum_sparsity_value; n++)
		{
			event_sparsity*=2;
		}

		//LOAD PATTERN OF EQUALLY SPACE HORIZONTAL LINES FOR SPARSE EVENT READOUT AND ELIMINATE ALL SET PIXELS EXCEPT THOSE ON TOP OF A STACK
		scamp5_select_pattern(0,event_sparsity/2,255,256-event_sparsity);
		scamp5_kernel_begin();
				DNEWS(R11,R10,north,0);
				XOR(R1,R10,R11);
				AND(R11,R1,SELECT);
		scamp5_kernel_end();
		scamp5_scan_events(R11,event_data,256/event_sparsity);

		for(int n = 0 ; n < 256/event_sparsity ; n++)
		{
			if(event_data[n*2] == 0 && event_data[n*2+1] == 0 && n != 0)
			{
				break;
			}
			sum = sum + (255- event_data[n*2+1])*event_sparsity;
		}
	}
	else
	{
		//ELIMINATE ALL SET PIXELS EXCEPT THOSE ON TOP OF A STACK
		scamp5_kernel_begin();
			DNEWS(R11,R10,north,0);
			XOR(R1,R11,R10);
		scamp5_kernel_end();

		scamp5_scan_events(R1,event_data,256);

		for(int n = 0 ; n < 256 ; n++)
		{
			if(event_data[n*2] == 0 && event_data[n*2+1] == 0 && n != 0)
			{
				break;
			}
			sum = sum + 255- event_data[n*2+1];
		}
	}

//	vs_post_text("event time %d us\n",timer_events.get_usec());

	return sum;
}


int DOUBLE_SC_APPROX_SUM_S5D_OPTIMIZED(dreg_t reg, int sum_sparsity_value, int sum_stack_tail)
{
	//THIS FUNCTION STACKS BIT VERTICALLY, AND THE HORIZONTALLY RESULTING IN A MONOTONIC "SLOPE" OF SET BITS ALONG THE LEFT SIDE OF THE IMAGE PLACE
	//SPARSE EVENT READOUT IS THEN USED TO FIND THE APPROXIMATE AREA OF THIS SLOPE

	if(reg.LR_bits != R10.LR_bits || reg.LR_index != R10.LR_index || reg.RR_bits != R10.RR_bits || reg.RR_index  != R10.RR_index)
	{
		scamp5_dynamic_kernel_begin();
			MOV(R10,reg);
		scamp5_dynamic_kernel_end();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//STACKING X

	scamp5_in(F,-120);//EXTRA_SJC

	//ROUTINE FOR STACKING SET PIXELS, WITH EACH STEP SET PIXELS "FALL" INTO ANY EMPTY NEIGHBOURS BELOW
	scamp5_select_pattern(0,255,255,0);//SELECT LEFT COLUMN
	scamp5_kernel_begin();
		CLR(R1,R3);//CLEAR UNUSED DNEWS DIRECTIONS
		bus(NEWS, F); //EXTRA_SJC
		MOV(R2,R10);//EXTRA_SJC
	scamp5_kernel_end();

	for(int m = 0 ; m < 8 ; m++)
	{
		scamp5_kernel_begin();
			for(int i = 0 ; i < 32 ; i++)
			{
				icw(1,{SEL4,sele,plrb,bitmode,nbit,LR4,RR10});//amalgamating R3=!R10 and preset of boundary bus inputs
				 icw(2,{plrb,bit,s_in,rid,nb,LR11,RR10});//DNEWS(R9,R10) with floating boundary input
				icw(1,{LR2,LR10,plrb,bit,bitmode,RR11,SEL4,sele}); //R1,R10 = R9; floating boundary defined
			}
		scamp5_kernel_end();
	}

	scamp5_kernel_begin();
			ALL();
	scamp5_kernel_end();


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//STACKING Y

	//SELECT BOTTOM ROW
	scamp5_select_pattern(255,0,0,255);
	scamp5_in(F,15);
	scamp5_kernel_begin();
		icw(1,{plrb,bit,bitmode,wr,LR4});
		WHERE(R4);
	scamp5_kernel_end();

	scamp5_in(F,-120);
	scamp5_kernel_begin();
		all();
		bus(NEWS, F);

		CLR(R2,R4); //CLEAR UNUSED DNEWS DIRECTIONS
		MOV(R1,R10);//moved this out of loop below so first iteration runs properly
	scamp5_kernel_end();

	for(int m = 0 ; m < 8 ; m++)
	{
		scamp5_kernel_begin();
			for(int i = 0 ; i < 32 ; i++)
			{
				icw(1,{SEL4,seln,sels,plrb,bitmode,plwb,nbit,LR3,RR10});//amalgamating R3=!R10 and preset of boundary bus inputs
				 icw(2,{plrb,bit,s_in,rid,nb,LR11,RR10});//DNEWS(R9,R10) with floating boundary input
				icw(1,{LR1,LR10,plrb,bit,bitmode,RR11,SEL4,seln,sels}); //R1,R10 = R9; floating boundary defined
			}
		scamp5_kernel_end();
	}
	scamp5_kernel_begin();
			ALL();
	scamp5_kernel_end();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//		vs_post_text("test_timer %d us \n",stacking_timer.get_usec());

	int event_sparsity = 1;
	for(int n = 0 ; n < sum_sparsity_value; n++)
	{
		event_sparsity*=2;
	}

	uint8_t event_data [256*2];

	//LOAD PATTERN OF EQUALLY SPACE HORIZONTAL LINES FOR SPARSE EVENT READOUT
	scamp5_select_pattern(255,255,256-event_sparsity,255);
	scamp5_kernel_begin();
			DNEWS(R11,R10,east);
			XOR(R1,R10,R11);
			AND(R11,R1,SELECT);
	scamp5_kernel_end();
	scamp5_scan_events(R11,event_data,256/event_sparsity);

	int approx_sum_value = 0;
	int last_value = 0;
	for(int n = 1 ; n < 256/event_sparsity ; n++)
	{
		if(event_data[n*2] == 0 && event_data[n*2+1] == 0)
		{
			break;
		}
		last_value = event_data[n*2];
		approx_sum_value += (255- (event_data[n*2]+event_data[n*2-2])/2)*event_sparsity;
	}

	if(sum_stack_tail)
	{
		scamp5_scan_events(R10,event_data,1,1,0);
		approx_sum_value += (255- (event_data[0]+last_value)/2)*event_sparsity;

//		scamp5_load_point(R9,event_data[1],event_data[0]);
//		scamp5_kernel_begin();
//			OR(R1,R9,R11);
//			MOV(R11,R1);
//		scamp5_kernel_end();
	}

	return approx_sum_value;
}

