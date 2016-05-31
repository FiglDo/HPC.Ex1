#include "Ex2_seq.h"


int Ex2_seq::Ex2_seq_main()
{
	int in[] = { 3, 1, 7, 0, 4, 1, 6, 3 };	

	const int sizeOfIN = sizeof(in) / sizeof(in[0]);

	int out[sizeOfIN] = {};
	out[0] = 0;

	for (int j = 1; j < sizeOfIN; j++)
	{
		out[j] = out[j - 1] + in[j - 1];
	}

	return 0;

}