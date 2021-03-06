__kernel void scan_local(int *g_odata, int *g_idata, __local int * temp, int * bsum) 
{ 
	int thid = get_global_id(0);
	int groupid = get_group_id(0);
	int localid = get_local_id(0);
	int n = get_global_size(0);
	int wgsize = get_local_size(0);
	
	int pout = 0, pin = 1;  

	// Load input into shared memory.
	temp[pout*wgsize + localid] = (localid > 0) ? g_idata[thid - 1] : 0;
	
	barrier(CLK_LOCAL_MEM_FENCE );

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
	


	for (int offset = 1; offset < wgsize; offset *= 2)
	{  
		pout = 1 - pout; // swap double buffer indices  
		pin = 1 - pout; 

		if (localid >= offset)
			temp[pout*wgsize + localid] = temp[pin*wgsize + localid] + temp[pin*wgsize + localid - offset];
		else  
			temp[pout*wgsize + localid] = temp[pin*wgsize + localid];
		
		barrier(CLK_LOCAL_MEM_FENCE );	
	}

	g_odata[thid] = temp[pout*wgsize + localid]; // write output 

	//printf("thid: %d, localid: %d, wgsize: %d\n", thid, localid, wgsize);

	if((thid+1)%wgsize == 0)
	{
		bsum[groupid] = g_odata[thid] + g_idata[thid];
	}
}

__kernel void scan_agg(int *g_odata, int *g_idata, int * sums, int sos)
{
	int thid = get_global_id(0);
	int n = get_global_size(0);
	int index = (int)( thid / (n/sos));

	//printf("thid: %d, index: %d, sos: %d, calc: %d\n", thid, index, sos, sizeof(sums) / sizeof(sums[0]));

	g_odata[thid] = g_idata[thid] + sums[index];
}

short checkLower(short value, short compare)
{
    if(value < compare)
		return 1;             

	return 0;
}

short checkEqual(short value, short compare)
{
    if(value == compare)
		return 1;             

	return 0;
}

short checkBigger(short value, short compare)
{
    if(value > compare)
		return 1;             

	return 0;
}


short checker(short type, short value, short compare)
{ 
	if(type == 0)
		return checkLower(value,compare);
	else if(type == 1)
		return checkEqual(value,compare);
	else if(type == 1)
		return checkBigger(value,compare);
}

__kernel void filter(int *g_odata, int *g_idata, int predicateType, int predicateValue)
{
	int thid = get_global_id(0);

	g_odata[thid] = (int)checker(predicateType,g_idata[thid],predicateValue);
}

__kernel void scatter(int *g_odata, int *g_idata, int * g_indexdata)
{
	int thid = get_global_id(0);
	int groupid = get_group_id(0);
	int localid = get_local_id(0);	
	int maxSize = get_global_size(0);
	int wgsize = get_local_size(0);

	//printf("ThreadID: %d, groupId: %d, localId: %d, wgSize: %d maxSize: %d\n",thid, groupid,localid,wgsize,maxSize);
	
	if((thid) < (maxSize-1))
	{
		if(g_indexdata[thid] != g_indexdata[(thid+1)])
		{
			int index=g_indexdata[thid];
			g_odata[index] = g_idata[thid];
		}
	}
}

