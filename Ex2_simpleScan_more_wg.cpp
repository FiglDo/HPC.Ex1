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

using namespace std;

class OpenClContainer
{
public:
	cl::Program program;
	cl::Context context;
	cl::CommandQueue queue;
	cl::Device device;
};



int InitOpenCL(string kernelName, OpenClContainer& container)
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

		//create kernels
		cl::Kernel kernel(program, "scan_local", &err);
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


int PerformScan(string kernelName, OpenClContainer container, int workGroupSplit, vector<cl_int> input, vector<cl_int>& output, vector<cl_int>& sum)
{
	cl_int err = CL_SUCCESS;

	try
	{
	

	int sizeOfInput = input.size();

	

	//cl_int input[] = { 3, 1, 7, 0, 4, 1, 6, 3 };
	//const int sizeOfInput = sizeof(input) / sizeof(input[0]);
	//cl_int output[sizeOfInput] = {};

	int _size = sizeOfInput * sizeof(cl_int);
	int _size_temp = sizeOfInput * 20 * sizeof(cl_int);

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
	cl::Buffer bufferTemp = cl::Buffer(container.context, CL_MEM_READ_WRITE, _size_temp);  //fuer global
	cl::Buffer bufferBSum = cl::Buffer(container.context, CL_MEM_WRITE_ONLY, _size_sum);

	


	// fill buffers
	container.queue.enqueueWriteBuffer(
		bufferSource, // which buffer to write to
		CL_TRUE, // block until command is complete
		0, // offset
		_size, // size of write 
		&(input[0])); // pointer to input

	cl::Kernel scanKernel(container.program, "scan_local", &err);
	scanKernel.setArg(0, bufferDest);
	scanKernel.setArg(1, bufferSource);
	//scanKernel.setArg(2, bufferTemp);
	scanKernel.setArg(2, _size_temp, NULL);
	//rotateKernel.setArg(3, sizeOfInput);
	scanKernel.setArg(3, bufferBSum);


	std::cout << "call 'scan' kernel" << std::endl;
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


int Ex2_simpleScan_more_wg::Ex2_main()
{

	//create input and output data

	vector<cl_int> input = vector<cl_int>();
	input.push_back(1);
	input.push_back(1);
	input.push_back(1);
	input.push_back(1);
	input.push_back(1);
	input.push_back(1);
	input.push_back(1);
	input.push_back(1);

	//input.push_back(3);
	//input.push_back(1);
	//input.push_back(7);
	//input.push_back(0);
	//input.push_back(4);
	//input.push_back(1);
	//input.push_back(6);
	//input.push_back(3);

	int sizeOfInput = input.size();
	vector<cl_int> output = vector<cl_int>(sizeOfInput);
	int workGroupSplit = 4;

	int sizeOfSum = (sizeOfInput / workGroupSplit);
	int _size_sum = sizeOfSum * sizeof(cl_int);
	vector<cl_int> sum = vector<cl_int>(sizeOfSum);

	string kernelName = "scan_local";

	OpenClContainer container;

	int retVal;

	//init OPEN CL
	retVal = InitOpenCL(kernelName,container);

	//Scan 1
	retVal = PerformScan(kernelName, container, workGroupSplit, input, output, sum);
	
	//Scan 2


	std::cout << std::endl << "RESULT:" << std::endl;
	std::cout << "INPUT vs. OUTPUT" << std::endl;

	for (size_t i = 0; i < sizeOfInput
		; i++)
	{
		std::cout << "    " << input[i] << "     " << output[i] << std::endl;
	}

	std::cout << std::endl << std::endl << "Block Sums:" << endl;

	for (size_t i = 0; i < sizeOfSum
		; i++)
	{
		std::cout << "    " << sum[i] << std::endl;
	}

	cin.get();

	return retVal;
}