// opencltest.cpp : an example for using OpenCL with C++
// requires cl.hpp, the C++ bindings for OpenCL v 1.2
// https://www.khronos.org/registry/cl/api/1.2/cl.hpp
// author: Eugen Jiresch
//

#define __CL_ENABLE_EXCEPTIONS
	
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

int  main(void)
{

const std::string KERNEL_FILE = "kernel.cl";
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
	cl::Platform platform = platforms[1]; // on a different machine, you may have to select a different platform
	cl_context_properties properties[] =
		{ CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[1])(), 0 };
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
	program.build(devices);
	//create kernels
	cl::Kernel kernel(program, "hello", &err);
	cl::Event event;

	// launch hello kernel
	std::cout << "calling 'hello' kernel" << std::endl;
	cl::CommandQueue queue(context, devices[0], 0, &err);
	queue.enqueueNDRangeKernel(
		kernel,
		cl::NullRange,
		cl::NDRange(4, 4),
		cl::NullRange,
		NULL,
		&event);	
	event.wait();

	// create input and output data
	std::vector<int> a({ 1,2,3,4,5,6,7,8 });
	std::vector<int> b({ 1,2,3,4,5,6,7,8 });
	std::vector<int> c;
	c.resize(a.size());
	// input buffers
	cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, a.size() * sizeof(int));
	cl::Buffer bufferB = cl::Buffer(context, CL_MEM_READ_ONLY, b.size() * sizeof(int));
	// output buffers
	cl::Buffer bufferC = cl::Buffer(context, CL_MEM_WRITE_ONLY, c.size() * sizeof(int));

	// fill buffers
	queue.enqueueWriteBuffer(
		bufferA, // which buffer to write to
		CL_TRUE, // block until command is complete
		0, // offset
		a.size() * sizeof(int), // size of write 
		&a[0]); // pointer to input
	queue.enqueueWriteBuffer(bufferB, CL_TRUE, 0, b.size() * sizeof(int), &b[0]);

	cl::Kernel addKernel(program, "vector_add", &err);
	addKernel.setArg(0, bufferA);
	addKernel.setArg(1, bufferB);
	addKernel.setArg(2, bufferC);

	// launch add kernel
	// Run the kernel on specific ND range
	cl::NDRange global(a.size());
	cl::NDRange local(1); //make sure local range is divisible by global range
	cl::NDRange offset(0);
	std::cout << "call 'vector_add' kernel" << std::endl;
	queue.enqueueNDRangeKernel(addKernel, offset, global, local);

	// read back result
	queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, c.size() * sizeof(int), &c[0]);

	for (int i : c)
	{
		std::cout << i << std::endl;
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


