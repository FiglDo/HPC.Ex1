// opencltest.cpp : an example for using OpenCL with C++
// requires cl.hpp, the C++ bindings for OpenCL v 1.2
// https://www.khronos.org/registry/cl/api/1.2/cl.hpp
// author: Eugen Jiresch
//

#define __CL_ENABLE_EXCEPTIONS

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif
#include "Ex2_simpleScan_more_wg.h"
#include "tga/tga.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>

#include <iomanip>
#include <chrono>

using namespace std;

int Ex2_simpleScan_more_wg::InitOpenCL(OpenClContainer& container)
{
#if defined(__APPLE__) || defined(__MACOSX)
	const std::string KERNEL_FILE = "./Ex2_kernel.cl";
	int platformNr = 0;
#else
	const std::string KERNEL_FILE = "Ex2_kernel.cl";
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


int Ex2_simpleScan_more_wg::PerformScan(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, vector<cl_int>& sum)
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
	cl::Buffer bufferBSum = cl::Buffer(container.context, CL_MEM_READ_WRITE, _size_sum);

	


	// fill buffers
	container.queue.enqueueWriteBuffer(
		bufferSource, // which buffer to write to
		CL_TRUE, // block until command is complete
		0, // offset
		_size, // size of write 
		&(input[0])); // pointer to input


	// fill buffers
	container.queue.enqueueWriteBuffer(
		bufferBSum, // which buffer to write to
		CL_TRUE, // block until command is complete
		0, // offset
		_size_sum, // size of write 
		&(sum[0])); // pointer to input

	cl::Kernel scanKernel(container.program, "scan_local", &err);
	scanKernel.setArg(0, bufferDest);
	scanKernel.setArg(1, bufferSource);
	scanKernel.setArg(2, _size_temp, NULL);
	scanKernel.setArg(3, bufferBSum);


	//std::cout << "call 'scan_local' kernel" << std::endl;
	container.queue.enqueueNDRangeKernel(scanKernel, offset, global, local);

	// read back result
	container.queue.enqueueReadBuffer(bufferDest, CL_TRUE, 0, _size, &(output[0]));
	container.queue.enqueueReadBuffer(bufferBSum, CL_TRUE, 0, _size_sum, &(sum[0]));

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


int Ex2_simpleScan_more_wg::PerformAgg(string kernelName, OpenClContainer container, int amountOfWorkGroups, vector<cl_int> input, vector<cl_int>& output, vector<cl_int> sum)
{
	cl_int err = CL_SUCCESS;

	try
	{
		int sizeOfInput = input.size();
		int workGroupSplit = sizeOfInput / amountOfWorkGroups;
		int _size = sizeOfInput * sizeof(cl_int);

		// launch add kernel
		// Run the kernel on specific ND range
		cl::NDRange global(sizeOfInput); //global => von bis ueber die ganze Range des Arrays
		cl::NDRange local(workGroupSplit); //unterteilung des global in workgroups => make sure local range is divisible by global range
		cl::NDRange offset(0); //todo: offset auf workgroup Ebene?


		int sizeOfSum = sum.size();
		int _size_sum = sizeOfSum * sizeof(cl_int);

		// input buffers
		cl::Buffer bufferSource = cl::Buffer(container.context, CL_MEM_READ_ONLY, _size);
		// output buffers
		cl::Buffer bufferDest = cl::Buffer(container.context, CL_MEM_WRITE_ONLY, _size);
		cl::Buffer bufferBSum = cl::Buffer(container.context, CL_MEM_READ_ONLY, _size_sum);

		// fill buffers
		container.queue.enqueueWriteBuffer(
			bufferSource, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			_size, // size of write 
			&(input[0])); // pointer to input

		container.queue.enqueueWriteBuffer(
			bufferBSum, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			_size_sum, // size of write 
			&(sum[0])); // pointer to input


		cl::Kernel scanKernel(container.program, "scan_agg", &err);
		scanKernel.setArg(0, bufferDest);
		scanKernel.setArg(1, bufferSource);
		scanKernel.setArg(2, bufferBSum);
		scanKernel.setArg(3, sum.size());


		//std::cout << "call 'scan_agg' kernel" << std::endl;
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

void PrintInputVsOutput(vector<cl_int> input, vector<cl_int> output)
{
	std::cout << endl << "INPUT vs. OUTPUT" << std::endl;

	for (size_t i = 0; i < input.size(); i++)
	{
		std::cout << "    " << input[i] << "     " << output[i] << std::endl;
	}
}

void PrintBSum(vector<cl_int> sum)
{
	std::cout << std::endl << std::endl << "Block Sums:" << endl;

	for (size_t i = 0; i < sum.size(); i++)
	{
		std::cout << "    " << sum[i] << std::endl;
	}
}


int Ex2_simpleScan_more_wg::Ex2_main(vector<cl_int> input)
{
	cout << endl << "Starting SimpleScan using more workgroups" << endl;
	std::clock_t c_start = std::clock();
	auto t_start = std::chrono::high_resolution_clock::now();

	int sizeOfInput = input.size();
	vector<cl_int> output = vector<cl_int>(sizeOfInput);
	int amountOfWorkGroups = 256;

	int sizeOfSum = amountOfWorkGroups;
	int _size_sum = sizeOfSum * sizeof(cl_int);
	vector<cl_int> sum = vector<cl_int>(sizeOfSum);
		
	string kernelName = "scan_local";

	OpenClContainer container;

	int retVal;

	//init OPEN CL
	retVal = InitOpenCL(container);

	//Scan 1
	retVal = PerformScan(kernelName, container, amountOfWorkGroups, input, output, sum);

	//PrintInputVsOutput(input, output);
	//PrintBSum(sum);

	vector<cl_int> sum2 = vector<cl_int>(sum.size());
	vector<cl_int> output2 = vector<cl_int>(sum.size());
	
	//Scan 2
	retVal = PerformScan(kernelName, container, 1, sum, output2, sum2);

	//PrintInputVsOutput(sum, output2);
	//PrintBSum(sum2);

	vector<cl_int> output3 = vector<cl_int>(sizeOfInput);
	retVal = PerformAgg("scan_agg", container, amountOfWorkGroups, output, output3, output2);
	
	//PrintInputVsOutput(input, output3);
	
	cout << "LAST: " << output3[sizeOfInput-1] << endl;

	std::clock_t c_end = std::clock();
	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << std::fixed << std::setprecision(2) << "CPU time used: "
		<< 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC << " ms\n"
		<< "Wall clock time passed: "
		<< std::chrono::duration<double, std::milli>(t_end - t_start).count()
		<< " ms\n";

	return retVal;
}
