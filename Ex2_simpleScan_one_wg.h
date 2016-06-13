#pragma once
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

using namespace std;

class Ex2_simpleScan_one_wg
{
public:
	static int Ex2_main(vector<cl_int> input);
};

