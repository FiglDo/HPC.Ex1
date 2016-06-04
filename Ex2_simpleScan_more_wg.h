#pragma once
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

#include "OpenClContainer.h"
using namespace std;

class Ex2_simpleScan_more_wg
{
public:
	static int Ex2_main(std::vector<cl_int> input);
	static int PerformScan(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, vector<cl_int>& sum);
	static int PerformAgg(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, vector<cl_int> sum);
	static int InitOpenCL(OpenClContainer& container);
};


