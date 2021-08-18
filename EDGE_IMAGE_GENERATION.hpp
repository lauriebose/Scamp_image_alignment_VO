#include <scamp5.hpp>

scamp5_kernel sobel_edge_R5([]{

    using namespace scamp5_kernel_api;

    // vertical edge
    mov(D,C,north);
    mov(B,C,south);
    add(D,D,B);
    add(D,D,C);
    add(D,D,C);

    mov(B,D,east);
    mov(D,D,west);

    sub(B,B,D);// B = B - D
    abs(E,B);// D is the vertical edge

    // horizontal edge
    mov(D,C,east);
    mov(B,C,west);
    add(D,D,B);
    add(D,D,C);
    add(D,D,C);

    mov(B,D,south);
    mov(D,D,north);

    sub(B,B,D);// B = B - D
    abs(D,B);// D is the horizontal edge

    add(D,D,E);// merge both result

    // digitize
    sub(D,D,IN);
    where(D);
    	DNEWS(R11,FLAG,east|west|north|south);
    	MOV(R1,FLAG);
    all();

    // filter stand-alone points
    NOT(R2,R11);
    CLR_IF(R1,R2);

    // merge result into R5
    MOV(R2,R5);
	OR(R5,R1,R2);

    res(D);

});


scamp5_kernel gain_x2_C([]{
	using namespace scamp5_kernel_api;
	where(C);
		mov(E,C);
		mov(B,C);
		add(C,E,B);
	all();
});


void acquire_edge_image_R5(int gain,int edge_thresold, int edge_expansion,int HDR_iterations, int HDR_exposure_time, int boarder_edge_removal = 0)
{

	//first exposure
	scamp5_kernel_begin();
		get_image(C);
		CLR(R5);
		respix(F);// store reset level of PIX in F
	scamp5_kernel_end();

	// apply gain and get edge map
	scamp5_load_in(edge_thresold);
	scamp5_launch_kernel(sobel_edge_R5);

	for(int n = 0 ; n < gain ; n++)
	{
		scamp5_launch_kernel(gain_x2_C);
		scamp5_load_in(edge_thresold);
		scamp5_launch_kernel(sobel_edge_R5);
	}

	scamp5_kernel_begin();
		mov(D,C);
		respix(C);// store reset level of PIX in F
	scamp5_kernel_end();

	// short exposure iterations to deal with high light part
	for(int i=0;i<HDR_iterations;i++){
		vs_wait(HDR_exposure_time);

		scamp5_kernel_begin();
			getpix(C,F);
		scamp5_kernel_end();

		scamp5_load_in(edge_thresold);
		scamp5_launch_kernel(sobel_edge_R5);

		scamp5_kernel_begin();
			mov(E,C);
			add(C,C,D);
		scamp5_kernel_end();

		scamp5_load_in(edge_thresold);
		scamp5_launch_kernel(sobel_edge_R5);

		scamp5_kernel_begin();
			mov(C,E);
		scamp5_kernel_end();
	}

	for(int n = 0 ; n < edge_expansion ; n++)
	{
		scamp5_kernel_begin();
			DNEWS(R11,R5,east | west | south | north);
			MOV(R1,R5);
			OR(R5,R1,R11);
		scamp5_kernel_end();
	}

	if(boarder_edge_removal > 0)
	{
		scamp5_load_rect(R1,boarder_edge_removal,boarder_edge_removal,255-boarder_edge_removal,255-boarder_edge_removal);
		scamp5_kernel_begin();
			AND(R2,R1,R5);
			MOV(R5,R2);
		scamp5_kernel_end();
	}
}
