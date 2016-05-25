
/*
* a kernel that simply prints the (2-dimensional) thread id
* processes no data
*/
__kernel void hello(void)
{
	int tidX = get_global_id(0);
	int tidY = get_global_id(1);
	printf("hi from thread (%d,%d) \n", tidX, tidY);
}

/*
* a kernel that add the elements of two vectors pairwise
*/
__kernel void vector_add(
	__global const int *A,
	__global const int *B,
	__global int *C)
{
	int i = get_global_id(0);
	C[i] = A[i] + B[i];
}

__kernel void image_rotate( 
	__global unsigned char * src_data, __global unsigned char * dest_data, 	//Data in global memory
	int W, int H,  							//Image Dimensions 
	float sinTheta, float cosTheta ) 				//Rotation Parameters 
	{ 
	
		//Thread gets its index within index space 
		const int ix = get_global_id(0); 
		const int iy = get_global_id(1); 
		const int centerX = (int) (W/2);
		const int centerY = (int) (H/2);
		
		//Calculate location of data to move into ix and iy– Output decomposition as mentioned 
		float xpos = (( ((float) ix - (float)centerX)*cosTheta + ((float)iy - (float) centerY)*sinTheta) + centerX); 
		float ypos = (( ((float) ix - (float)centerX)*sinTheta + ((float)iy - (float) centerY)*cosTheta) + centerY);

		xpos = floor(xpos);
		ypos = floor(ypos);

		if ((((int)xpos>=0) && ((int)xpos< W)) && ( ((int)ypos>=0) && ((int)ypos< H)) )
		{
			//Read (xpos,ypos) src_data and store at (ix,iy) in dest_data 
			int index = (iy*W+ix)*3; 
			int sourceIndex = (int)(floor((ypos*W+xpos)*3));
			dest_data[index+0]= src_data[sourceIndex+0]; 
			dest_data[index+1]= src_data[sourceIndex+1]; 
			dest_data[index+2]= src_data[sourceIndex+2]; 
		}
		else
		{
			dest_data[(iy*W+ix)*3+0]= 255; 
			dest_data[(iy*W+ix)*3+1]= 0;
			dest_data[(iy*W+ix)*3+2]= 0;
		}	
		
	} 

