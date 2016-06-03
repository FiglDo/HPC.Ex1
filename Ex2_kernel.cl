
	//http://stackoverflow.com/questions/6890302/barriers-in-opencl
	//https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/barrier.html
__kernel void scan_local1(int *g_odata, int *g_idata, __local int * temp) 
{ 

	int thid = get_global_id(0);
	int groupid = get_group_id(0);
	int localid = get_local_id(0);
	int n = get_global_size(0);
	int l = get_local_size(0);
	
	//printf("thid: %d, n:%d  ",thid,n);
	//printf("lid: %d, l:%d\n",localid,l);
	
	printf("thid: %d, n:%d | lId: %d, lsize:%d | groupID: %d \n",thid,n,localid,l, groupid);
	
	int pout = 0, pin = 1;  
	// Load input into shared memory.  
	 // This is exclusive scan, so shift right by one  
	 // and set first element to 0  
	
	//temp[thid] = 0;
	//temp[n+thid] = 0;
	
	temp[pout*n+thid] = (localid > 0) ? g_idata[thid-1] : 0;  
		
	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE );


	for (int offset = 1; offset < n; offset *= 2)  
	{  
		pout = 1 - pout; // swap double buffer indices  
		pin = 1 - pout;  
	  
		if (thid >= offset)  
			temp[pout*n+thid] = temp[pin*n+thid] + temp[pin*n+thid - offset];  
		else  
			temp[pout*n+thid] = temp[pin*n+thid];  
		
		barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE );
		/*
		if(thid == 5)
		{ 
			for(int i = 0; i < 16; i++)
			{
				printf("%d: %d | ",i,temp[i]);
			}
			printf("\n\n");
		}
		*/
	}

	g_odata[thid] = temp[pout*n+thid]; // write output 

}

	//http://stackoverflow.com/questions/6890302/barriers-in-opencl
	//https://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/barrier.html
__kernel void scan_local(int *g_odata, int *g_idata, __local int * temp, int * bsum) 
{ 
	int thid = get_global_id(0);
	int groupid = get_group_id(0);
	int localid = get_local_id(0);
	int n = get_global_size(0);
	int wgsize = get_local_size(0);
	
	//printf("thid: %d, n:%d  ",thid,n);
	//printf("lid: %d, l:%d\n",localid,l);
	
	//printf("thid: %d, n:%d | lId: %d, lsize:%d | groupID: %d \n",thid,n,localid,l, groupid);
	
	int pout = 0, pin = 1;  
	// Load input into shared memory.  
	 // This is exclusive scan, so shift right by one  
	 // and set first element to 0  
	
	//temp[thid] = 0;
	//temp[n+thid] = 0;
	
	temp[pout*wgsize + localid] = (localid > 0) ? g_idata[thid - 1] : 0;
	
	barrier(CLK_LOCAL_MEM_FENCE );


		if(thid == 5)
		{
		for(int i = 0; i < 16; i++)
		{
		printf("%d: %d | ",i,temp[i]);
		}
		printf("\n\n");
		}
	


		for (int offset = 1; offset < wgsize; offset *= 2)
	{  
		pout = 1 - pout; // swap double buffer indices  
		pin = 1 - pout; 

		if (localid >= offset)
			temp[pout*wgsize + localid] = temp[pin*wgsize + localid] + temp[pin*wgsize + localid - offset];
		else  
			temp[pout*wgsize + localid] = temp[pin*wgsize + localid];
		
		barrier(CLK_LOCAL_MEM_FENCE );
		
		/*if(thid == 5)
		{ 
			for(int i = 0; i < 16; i++)
			{
				printf("%d: %d | ",i,temp[i]);
			}
			printf("\n\n");
		}*/
		
	}

	g_odata[thid] = temp[pout*wgsize + localid]; // write output 

	if (localid == (wgsize - 1))
	{
		//printf("\n\tgroupid: %d, value: %d\n",groupid,temp[pout*n+thid]);

		bsum[groupid] = temp[pout*wgsize + localid] + g_idata[thid];
	}
}

__kernel void scan_agg(int *g_odata, int *g_idata, int * sums)
{
	int thid = get_global_id(0);
	int n = get_global_size(0);
	int sos = sizeof(sums);// / sizeof(sums[0]);
	int index = (int)( thid / (n/sos));

	printf("thid: %d, index: %d, sos: %d, calc: %d\n", thid, index, sos, sizeof(sums) / sizeof(sums[0]));


	g_odata[thid] = g_idata[thid] + sums[index];
}