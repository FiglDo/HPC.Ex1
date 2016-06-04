#pragma once

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

class Ex2_seq
{
public:
	static int Ex2_seq_main(std::vector<cl_int> input);
};

