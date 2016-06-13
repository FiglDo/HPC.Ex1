#define __CL_ENABLE_EXCEPTIONS

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif
#include "Ex3.h"
#include "tga/tga.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>

#include <iomanip>
#include <chrono>

#include "Ex2_simpleScan_more_wg.h"


using namespace std;

int Ex3::InitOpenCL(OpenClContainer& container)
{
#if defined(__APPLE__) || defined(__MACOSX)
	const std::string KERNEL_FILE = "./Ex3_kernel.cl";
	int platformNr = 0;
#else
	const std::string KERNEL_FILE = "Ex3_kernel.cl";
	int platformNr = 1;
#endif

	cl_int err = CL_SUCCESS;
	cl::Program program;
	std::vector<cl::Device> devices;


	try {

		// get available platforms ( NVIDIA, Intel, AMD,...)
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0) {
			std::cout << "No OpenCL platforms available!\n";
			return 1;
		}

		// create a context and get available devices
		cl::Platform platform = platforms[platformNr]; // on a different machine, you may have to select a different platform
		cl_context_properties properties[] =
		{ CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[platformNr])(), 0 };
		//cl::Context context(CL_DEVICE_TYPE_GPU, properties);
		cl::Context context(CL_DEVICE_TYPE_CPU, properties);

		container.context = context;

		devices = context.getInfo<CL_CONTEXT_DEVICES>();

		// load and build the kernel
		std::ifstream sourceFile(KERNEL_FILE);
		if (!sourceFile)
		{
			std::cout << "kernel source file " << KERNEL_FILE << " not found!" << std::endl;
			return 1;
		}
		std::string sourceCode(
			std::istreambuf_iterator<char>(sourceFile),
			(std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		program = cl::Program(context, source);

		container.program = program;

		//program.build(devices);

		if (program.build({ devices }) != CL_SUCCESS){
			//std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices) << "\n";
			exit(1);
		}

		cl::Event event;

		cl::CommandQueue queue(context, devices[0], 0, &err);

		container.device = devices[0];
		container.queue = queue;

	}
	catch (cl::Error err) {
		// error handling
		// if the kernel has failed to compile, print the error log
		std::string s;
		program.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &s);
		std::cout << s << std::endl;
		program.getBuildInfo(devices[0], CL_PROGRAM_BUILD_OPTIONS, &s);
		std::cout << s << std::endl;

		std::cerr
			<< "ERROR: "
			<< err.what()
			<< "("
			<< err.err()
			<< ")"
			<< std::endl;
	}

	return EXIT_SUCCESS;

}



int Ex3::PerformFilter(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, int predicateType, int predicateValue)
{
	cl_int err = CL_SUCCESS;

	try
	{
		int sizeOfInput = input.size();
		int _size = sizeOfInput * sizeof(cl_int);
		int workGroupSplit = sizeOfInput / amountOfWorkGroups;

		int _size_temp = workGroupSplit * 2 * sizeof(cl_int);

		// launch add kernel
		// Run the kernel on specific ND range
		cl::NDRange global(sizeOfInput); //global => von bis ueber die ganze Range des Arrays
		cl::NDRange local(workGroupSplit); //unterteilung des global in workgroups => make sure local range is divisible by global range
		cl::NDRange offset(0); //todo: offset auf workgroup Ebene?


		int sizeOfSum = (sizeOfInput / workGroupSplit);
		int _size_sum = sizeOfSum * sizeof(cl_int);

		// input buffers
		cl::Buffer bufferSource = cl::Buffer(container.context, CL_MEM_READ_ONLY, _size);
		// output buffers
		cl::Buffer bufferDest = cl::Buffer(container.context, CL_MEM_WRITE_ONLY, _size);
		
		// fill buffers
		container.queue.enqueueWriteBuffer(
			bufferSource, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			_size, // size of write 
			&(input[0])); // pointer to input


		cl::Kernel scanKernel(container.program, "filter", &err);
		scanKernel.setArg(0, bufferDest);
		scanKernel.setArg(1, bufferSource);
		scanKernel.setArg(2, predicateType);
		scanKernel.setArg(3, predicateValue);


		//std::cout << "call 'filter' kernel" << std::endl;
		container.queue.enqueueNDRangeKernel(scanKernel, offset, global, local);

		// read back result
		container.queue.enqueueReadBuffer(bufferDest, CL_TRUE, 0, _size, &(output[0]));

	}
	catch (cl::Error err) {
		// error handling
		// if the kernel has failed to compile, print the error log
		std::string s;
		container.program.getBuildInfo(container.device, CL_PROGRAM_BUILD_LOG, &s);
		std::cout << s << std::endl;
		container.program.getBuildInfo(container.device, CL_PROGRAM_BUILD_OPTIONS, &s);
		std::cout << s << std::endl;

		std::cerr
			<< "ERROR: "
			<< err.what()
			<< "("
			<< err.err()
			<< ")"
			<< std::endl;
	}

	return EXIT_SUCCESS;
}

int Ex3::PerformScatter(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, vector<cl_int> indexData)
{
	cl_int err = CL_SUCCESS;

	try
	{
		int outputSize = output.size();
		int _sizeOutput = outputSize * sizeof(cl_int);
		int sizeOfInput = input.size();
		int _size = sizeOfInput * sizeof(cl_int);
		int workGroupSplit = sizeOfInput / amountOfWorkGroups;

		int _size_temp = workGroupSplit * 2 * sizeof(cl_int);

		// launch add kernel
		// Run the kernel on specific ND range
		cl::NDRange global(sizeOfInput); //global => von bis ueber die ganze Range des Arrays
		cl::NDRange local(workGroupSplit); //unterteilung des global in workgroups => make sure local range is divisible by global range
		cl::NDRange offset(0); //todo: offset auf workgroup Ebene?


		int sizeOfSum = (sizeOfInput / workGroupSplit);
		int _size_sum = sizeOfSum * sizeof(cl_int);

		// input buffers
		cl::Buffer bufferSource = cl::Buffer(container.context, CL_MEM_READ_ONLY, _size);
		cl::Buffer bufferSourceIndex = cl::Buffer(container.context, CL_MEM_READ_ONLY, _size);
		// output buffers
		cl::Buffer bufferDest = cl::Buffer(container.context, CL_MEM_WRITE_ONLY, _sizeOutput);

		// fill buffers
		container.queue.enqueueWriteBuffer(
			bufferSource, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			_size, // size of write 
			&(input[0])); // pointer to input

		container.queue.enqueueWriteBuffer(
			bufferSourceIndex, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			_size, // size of write 
			&(indexData[0])); // pointer to input

		cl::Kernel scanKernel(container.program, "scatter", &err);
		scanKernel.setArg(0, bufferDest);
		scanKernel.setArg(1, bufferSource);
		scanKernel.setArg(2, bufferSourceIndex);

		//std::cout << "call 'filter' kernel" << std::endl;
		container.queue.enqueueNDRangeKernel(scanKernel, offset, global, local);

		// read back result
		container.queue.enqueueReadBuffer(bufferDest, CL_TRUE, 0, _sizeOutput, &(output[0]));

	}
	catch (cl::Error err) {
		// error handling
		// if the kernel has failed to compile, print the error log
		std::string s;
		container.program.getBuildInfo(container.device, CL_PROGRAM_BUILD_LOG, &s);
		std::cout << s << std::endl;
		container.program.getBuildInfo(container.device, CL_PROGRAM_BUILD_OPTIONS, &s);
		std::cout << s << std::endl;

		std::cerr
			<< "ERROR: "
			<< err.what()
			<< "("
			<< err.err()
			<< ")"
			<< std::endl;
	}

	return EXIT_SUCCESS;
}

void Ex3::PrintInputVsOutput(vector<cl_int> input, vector<cl_int> output)
{
	std::cout << endl << "INPUT vs. OUTPUT" << std::endl;

	for (size_t i = 0; i < input.size(); i++)
	{
		std::cout << "    " << input[i] << "     " << output[i] << std::endl;
	}
}

void Ex3::PrintInputVsOutput2(vector<cl_int>& input, vector<cl_int>& output)
{
	std::cout << endl << "INPUT: " << std::endl;

	for (size_t i = 0; i < input.size(); i++)
	{
		std::cout << "INDEX: " << i << "     " << input[i] << std::endl;
	}

	std::cout << endl << endl << "Output: " << std::endl;

	for (size_t i = 0; i < output.size(); i++)
	{
		std::cout << "INDEX: " << i << "     " << output[i] << std::endl;
	}
}


int Ex3::PerformScan(int retVal, OpenClContainer container, int amountOfWorkGroups, int sizeOfInput, vector<cl_int> outputFromFilter, vector<cl_int>& outputAgg, int showOutput)
{
	int sizeOfSum = amountOfWorkGroups;
	int _size_sum = sizeOfSum * sizeof(cl_int);
	vector<cl_int> sum = vector<cl_int>(sizeOfSum);

	vector<cl_int> inputForScan = outputFromFilter;
	vector<cl_int> outputForScan = vector<cl_int>(sizeOfInput);


	retVal = Ex2_simpleScan_more_wg::PerformScan("scan_local", container, amountOfWorkGroups, inputForScan, outputForScan, sum);

	if (showOutput)
		PrintInputVsOutput(inputForScan, outputForScan);

	vector<cl_int> inputForScanSum = sum;
	vector<cl_int> outputForScanSum = vector<cl_int>(sum.size());
	vector<cl_int> unnecessarySums = vector<cl_int>(sum.size());

	retVal = Ex2_simpleScan_more_wg::PerformScan("scan_local", container, 1, inputForScanSum, outputForScanSum, unnecessarySums);
	
	if (showOutput)
		PrintInputVsOutput(inputForScanSum, outputForScanSum);

	vector<cl_int> inputForAgg = outputForScan;
	outputAgg = vector<cl_int>(inputForAgg.size());

	retVal = Ex2_simpleScan_more_wg::PerformAgg("scan_agg", container, amountOfWorkGroups, inputForAgg, outputAgg, outputForScanSum);

	if (showOutput)
		PrintInputVsOutput(inputForScan, outputAgg);

	return retVal;
}


int Ex3::Ex3_main(std::vector<cl_int> unmodifiedInput, int amountOfWorkGroups, int predicateType, int predicateValue, int showOutput, int showOutputScan)
{
	vector<int> input(unmodifiedInput);
	input.push_back(0); //push pack help value

	cout << endl << "Starting Stream Compaction" << endl;

	cout << endl << "PredicateType (0:LowerThan, 1 : Equal, 2 : GreaterThan): " << predicateType << endl;
	cout << endl << "PredicateValue: " << predicateValue << endl;
	std::clock_t c_start = std::clock();
	auto t_start = std::chrono::high_resolution_clock::now();

	int sizeOfInput = input.size();
	vector<cl_int> outputForFilter = vector<cl_int>(sizeOfInput);



	string kernelName = "filter";

	OpenClContainer container;

	int retVal;

	//init OPEN CL
	retVal = InitOpenCL(container);

	//Filter
	retVal = PerformFilter(kernelName, container, amountOfWorkGroups, input, outputForFilter, predicateType, predicateValue);

	if (showOutput)
		PrintInputVsOutput(input, outputForFilter);

	/////////////////// <scan>
	
	vector<cl_int> outputAgg;
	retVal = PerformScan(retVal, container, amountOfWorkGroups, sizeOfInput, outputForFilter, outputAgg, showOutputScan);

	/////////////////// </scan>
	
	int sizeForOutputScatter = outputAgg[(outputAgg.size() - 1)]; //get the last element and reduce by one;

	vector<cl_int> outputScatter;

	if (sizeForOutputScatter > 0)
	{
		outputScatter = vector<cl_int>(sizeForOutputScatter);

		retVal = PerformScatter("scatter", container, amountOfWorkGroups, input, outputScatter, outputAgg);

	}
	else
	{
		outputScatter = vector<cl_int>(0);
	}

	if (showOutput)
		PrintInputVsOutput2(unmodifiedInput, outputScatter);

	std::cout << endl << endl << "Elements in Output: " << outputScatter.size() << std::endl;

	std::clock_t c_end = std::clock();
	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << std::fixed << std::setprecision(2) << "CPU time used: "
		<< 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC << " ms\n"
		<< "Wall clock time passed: "
		<< std::chrono::duration<double, std::milli>(t_end - t_start).count()
		<< " ms\n";

	return retVal;
};