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
#include "Ex2.h"
#include "tga/tga.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>


int Ex2::Ex2_main()
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
		//program.build(devices);

		if (program.build({ devices }) != CL_SUCCESS){
			//std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices) << "\n";
			exit(1);
		}

		//create kernels
		cl::Kernel kernel(program, "scan", &err);
		cl::Event event;

		cl::CommandQueue queue(context, devices[0], 0, &err);

		//create input and output data
		int input[] = { 3, 1, 7, 0, 4, 1, 6, 3 };
		const int sizeOfInput = sizeof(input) / sizeof(input[0]);
		int output[sizeOfInput] = {};

		size_t _size = sizeOfInput * sizeof(int);
		size_t _size_temp = sizeOfInput * 2 * sizeof(int);

		// input buffers
		cl::Buffer bufferSource = cl::Buffer(context, CL_MEM_READ_ONLY, _size);
		// output buffers
		cl::Buffer bufferDest = cl::Buffer(context, CL_MEM_WRITE_ONLY, _size);
		cl::Buffer bufferTemp = cl::Buffer(context, CL_MEM_READ_WRITE, _size_temp);

		// fill buffers
		queue.enqueueWriteBuffer(
			bufferSource, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			_size, // size of write 
			&(input[0])); // pointer to input

		cl::Kernel rotateKernel(program, "scan", &err);
		rotateKernel.setArg(0, bufferDest);
		rotateKernel.setArg(1, bufferSource);
		rotateKernel.setArg(2, bufferTemp);
		//rotateKernel.setArg(3, sizeOfInput);

		// launch add kernel
		// Run the kernel on specific ND range
		// todo was machen diese 2???
		cl::NDRange global(sizeOfInput);
		cl::NDRange local(sizeOfInput); //make sure local range is divisible by global range
		cl::NDRange offset(0);

		std::cout << "call 'scan' kernel" << std::endl;
		queue.enqueueNDRangeKernel(rotateKernel, offset, global, local);

		// read back result
		queue.enqueueReadBuffer(bufferDest, CL_TRUE, 0, _size, &(output[0]));


		std::cout << std::endl << "RESULT:" << std::endl;
		std::cout << "INPUT vs. OUTPUT" << std::endl;

		for (size_t i = 0; i < sizeOfInput
			; i++)
		{
			std::cout << "    " << input[i] << "     " << output[i] << std::endl;
		}

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

	std::cin.get();

	return EXIT_SUCCESS;

}


