//http://www.nehalemlabs.net/prototype/blog/2014/06/23/parallel-programming-with-opencl-and-python-parallel-scan/


__kernel void scan_WE(__global int *output,
					__global int *input,
                   __local int *temp,
                   uint n_items)
{
    uint gid = get_global_id(0);
    uint lid = get_local_id(0);
    uint dp = 1;

    temp[2*lid] = input[2*gid];
    temp[2*lid+1] = input[2*gid+1];

    for(uint s = n_items>>1; s > 0; s >>= 1) {
        barrier(CLK_LOCAL_MEM_FENCE);
        if(lid < s) {
            uint i = dp*(2*lid+1)-1;
            uint j = dp*(2*lid+2)-1;
            temp[j] += temp[i];
        }

        dp <<= 1;
    }

    if(lid == 0) temp[n_items - 1] = 0;

    for(uint s = 1; s < n_items; s <<= 1) {
        dp >>= 1;
        barrier(CLK_LOCAL_MEM_FENCE);

        if(lid < s) {
            uint i = dp*(2*lid+1)-1;
            uint j = dp*(2*lid+2)-1;

            int t = temp[j];
            temp[j] += temp[i];
            temp[i] = t;
        }
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    output[2*gid] = temp[2*lid];
    output[2*gid+1] = temp[2*lid+1];
}