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
#include "tga/tga.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>


int mainASDF(void)
{

#if defined(__APPLE__) || defined(__MACOSX)
	const std::string KERNEL_FILE = "./kernel.cl";
	int platformNr = 0;
#else
	const std::string KERNEL_FILE = "kernel.cl";
	int platformNr = 1;
#endif


	//http://simpleopencl.blogspot.co.at/2013/06/tutorial-simple-start-with-opencl-and-c.html
	//get all platforms (drivers)
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0){
		std::cout << " No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Platform default_platform = all_platforms[0];
	std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << "\n";

	//get default device of the default platform
	std::vector<cl::Device> all_devices;
	default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if (all_devices.size() == 0){
		std::cout << " No devices found. Check OpenCL installation!\n";
		exit(1);
	}
	cl::Device default_device = all_devices[platformNr];
	std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";

	cl::Context context({ default_device });

	cl::Program::Sources sources;

	// load and build the kernel
	std::ifstream sourceFile(KERNEL_FILE);
	std::string sourceCode(
		std::istreambuf_iterator<char>(sourceFile),
		(std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
	cl::Program program(context, sources);

	if (program.build({ default_device }) != CL_SUCCESS){
		std::cout << " Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
		exit(1);
	}

	tga::TGAImage* img = new tga::TGAImage();
	tga::LoadTGA(img, "lenna.tga");

	size_t _size = img->imageData.size() * sizeof(unsigned char);

	cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, _size);
	cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, _size);

	//cl::make_kernel test(cl::Kernel(program, "simple_add"));
	//cl::EnqueueArgs eargs(queue, cl::NullRange, cl::NDRange(10), cl::NullRange);
	//simple_add(eargs, buffer_A, buffer_B, buffer_C).wait();
}
	

int  main(void)
{

#if defined(__APPLE__) || defined(__MACOSX)
	const std::string KERNEL_FILE = "./kernel.cl";
	int platformNr = 0;
#else
	const std::string KERNEL_FILE = "kernel.cl";
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

		cl_int clWidth = img->width;
		cl_int clHeight = img->height;

		float sintheta = 1;
		float costheta = 0;

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

		////////////////////////////////////////////////////////////////////////////////////////


		////create kernels
		//cl::Kernel kernel(program, "hello", &err);
		//cl::Event event;

		//// launch hello kernel
		//std::cout << "calling 'hello' kernel" << std::endl;
		//cl::CommandQueue queue(context, devices[0], 0, &err);
		//queue.enqueueNDRangeKernel(
		//	kernel,
		//	cl::NullRange,
		//	cl::NDRange(4, 4),
		//	cl::NullRange,
		//	NULL,
		//	&event);
		//event.wait();

		//// create input and output data
		//std::vector<int> a({ 1, 2, 3, 4, 5, 6, 7, 8 });
		//std::vector<int> b({ 1, 2, 3, 4, 5, 6, 7, 8 });
		//std::vector<int> c;
		//c.resize(a.size());
		//// input buffers
		//cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, a.size() * sizeof(int));
		//cl::Buffer bufferB = cl::Buffer(context, CL_MEM_READ_ONLY, b.size() * sizeof(int));
		//// output buffers
		//cl::Buffer bufferC = cl::Buffer(context, CL_MEM_WRITE_ONLY, c.size() * sizeof(int));

		//// fill buffers
		//queue.enqueueWriteBuffer(
		//	bufferA, // which buffer to write to
		//	CL_TRUE, // block until command is complete
		//	0, // offset
		//	a.size() * sizeof(int), // size of write 
		//	&a[0]); // pointer to input
		//queue.enqueueWriteBuffer(bufferB, CL_TRUE, 0, b.size() * sizeof(int), &b[0]);

		//cl::Kernel addKernel(program, "vector_add", &err);
		//addKernel.setArg(0, bufferA);
		//addKernel.setArg(1, bufferB);
		//addKernel.setArg(2, bufferC);

		//// launch add kernel
		//// Run the kernel on specific ND range
		//cl::NDRange global(a.size());
		//cl::NDRange local(1); //make sure local range is divisible by global range
		//cl::NDRange offset(0);
		//std::cout << "call 'vector_add' kernel" << std::endl;
		//queue.enqueueNDRangeKernel(addKernel, offset, global, local);

		//// read back result
		//queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, c.size() * sizeof(int), &c[0]);

		//for (int i : c)
		//{
		//	std::cout << i << std::endl;
		//}

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


