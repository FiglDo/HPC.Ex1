
	//http://stackoverflow.com/questions/6890302/barriers-in-opencl
	//https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/barrier.html
__kernel void scan(int *g_odata, int *g_idata, __global int * temp)  //warum hier nicht local?  
{ 
	//extern __shared float temp[]; // allocated on invocation  

	int thid = get_global_id(0);
	int n = get_global_size(0);
	printf("thid: %d, n:%d\n",thid,n);	//Warum ist das geordnet?? 


	int pout = 0, pin = 1;  
	// Load input into shared memory.  
	 // This is exclusive scan, so shift right by one  
	 // and set first element to 0  
	temp[thid] = (thid > 0) ? g_idata[thid-1] : 0;  
		
	barrier(CLK_GLOBAL_MEM_FENCE);
	for (int offset = 1; offset < n; offset *= 2)  
	{  
		pout = 1 - pout; // swap double buffer indices  
		pin = 1 - pout;  
	  
		if (thid >= offset)  
			temp[pout*n+thid] = temp[pin*n+thid] + temp[pin*n+thid - offset];  
		else  
			temp[pout*n+thid] = temp[pin*n+thid];  
		
		barrier(CLK_GLOBAL_MEM_FENCE);

		if(thid == 5)
		{ 
			for(int i = 0; i < 16; i++)
			{
				printf("%d: %d | ",i,temp[i]);
			}
			printf("\n\n");
		}
	}

	g_odata[thid] = temp[pout*n+thid]; // write output 
}