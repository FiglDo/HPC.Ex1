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
#include "Ex1.h"
#include "tga/tga.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <math.h>

#define PI 3.14159265


int Ex1::Ex1_main()
{

#if defined(__APPLE__) || defined(__MACOSX)
	const std::string KERNEL_FILE = "./Ex1_kernel.cl";
	int platformNr = 0;
#else
	const std::string KERNEL_FILE = "Ex1_kernel.cl";
	int platformNr = 1;
#endif

	cl_int err = CL_SUCCESS;
	cl::Program program;
	std::vector<cl::Device> devices;

	tga::TGAImage* img = new tga::TGAImage();
	tga::LoadTGA(img, "lenna.tga");


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
		cl::Kernel kernel(program, "image_rotate", &err);
		cl::Event event;

		cl::CommandQueue queue(context, devices[0], 0, &err);

		// create input and output data
		tga::TGAImage* imgDest = new tga::TGAImage();
		imgDest->bpp = img->bpp;
		imgDest->height = img->height;
		imgDest->width= img->width;
		imgDest->type = img->type;
		imgDest->imageData.resize(img->imageData.size());

		size_t _size = img->imageData.size() * sizeof(unsigned char);

		// input buffers
		cl::Buffer bufferSource = cl::Buffer(context, CL_MEM_READ_ONLY, _size);
		// output buffers
		cl::Buffer bufferDest = cl::Buffer(context, CL_MEM_WRITE_ONLY, _size);

		double degree;
		degree = 33.0;

		float sintheta = 0.0;
		sintheta = sin(degree*PI / 180);
		float costheta = 0.0;
		costheta = cos(degree*PI / 180);

		// fill buffers
		queue.enqueueWriteBuffer(
			bufferSource, // which buffer to write to
			CL_TRUE, // block until command is complete
			0, // offset
			_size, // size of write 
			&(img->imageData[0])); // pointer to input

		cl::Kernel rotateKernel(program, "image_rotate", &err);
		rotateKernel.setArg(0, bufferSource);
		rotateKernel.setArg(1, bufferDest);
		rotateKernel.setArg(2, img->width);
		rotateKernel.setArg(3, img->height);
		rotateKernel.setArg(4, sintheta);
		rotateKernel.setArg(5, costheta);
				
		// launch add kernel
		// Run the kernel on specific ND range
		cl::NDRange global(img->width, img->height);
		cl::NDRange local(1,1); //make sure local range is divisible by global range
		cl::NDRange offset(0,0);
		std::cout << "call 'image_rotate' kernel" << std::endl;
		queue.enqueueNDRangeKernel(rotateKernel, offset, global, local);
		//event.wait();

		// read back result
		//queue.enqueueReadBuffer(bufferDest, CL_TRUE, 0, _size, &c[0]);
		queue.enqueueReadBuffer(bufferDest, CL_TRUE, 0, _size, &(imgDest->imageData[0]));

		tga::saveTGA(*imgDest, "test.tga");
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

	//std::cin.get();

	return EXIT_SUCCESS;

}


