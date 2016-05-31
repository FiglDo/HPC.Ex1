
__kernel void image_rotate( 
	__global unsigned char * src_data, __global unsigned char * dest_data, 	//Data in global memory
	int W, int H,  							//Image Dimensions 
	float sinTheta, float cosTheta ) 				//Rotation Parameters 
	{ 
	
		
		
	} 

__kernel void scan(int *g_odata, int *g_idata, __global int * temp)  
{ 
	//extern __shared float temp[]; // allocated on invocation  

	int thid = get_global_id(0);
	int n = get_global_size(0);

	printf("(thid: %d, n:%d)",thid,n);


	int pout = 0, pin = 1;  
	// Load input into shared memory.  
	 // This is exclusive scan, so shift right by one  
	 // and set first element to 0  
	temp[thid] = (thid > 0) ? g_idata[thid-1] : 0;  

	for (int offset = 1; offset < n; offset *= 2)  
	{  
	  pout = 1 - pout; // swap double buffer indices  
	  pin = 1 - pout;  
	  if (thid >= offset)  
		temp[pout*n+thid] = temp[pin*n+thid] + temp[pin*n+thid - offset];  
	  else  
		temp[pout*n+thid] = temp[pin*n+thid];  

	}
	g_odata[thid] = temp[pout*n+thid]; // write output 
}