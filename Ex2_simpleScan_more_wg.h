#pragma once
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

class Ex2_simpleScan_more_wg
{
public:
	static int Ex2_main(std::vector<cl_int> input);
};


