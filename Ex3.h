#pragma once
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

#include "OpenClContainer.h"
using namespace std;

class Ex3
{
public:
	static int Ex3_main(std::vector<cl_int> unmodifiedInput, int amountOfWorkGroups, int predicateType, int predicateValue, int showOutput, int showOutputScan);
	static void PrintInputVsOutput(vector<cl_int> input, vector<cl_int> output);
	static void PrintInputVsOutput2(vector<cl_int>& input, vector<cl_int>& output);
	static int InitOpenCL(OpenClContainer& container);
	static int Ex3::PerformFilter(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, int predicateType, int predicateValue);
	static int Ex3::PerformScatter(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, vector<cl_int> indexData);
	static int Ex3::PerformScan(int retVal, OpenClContainer container, int amountOfWorkGroups, int sizeOfInput, vector<cl_int> outputFromFilter, vector<cl_int>& outputAgg, int showOutput);
};