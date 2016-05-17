
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