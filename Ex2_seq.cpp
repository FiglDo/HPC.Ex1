#include "Ex2_seq.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

int Ex2_seq::Ex2_seq_main(vector<cl_int> input)
{
	cout << endl << "Starting sequential Scan" << endl;

	std::clock_t c_start = std::clock();
	auto t_start = std::chrono::high_resolution_clock::now();

	vector<cl_int> output = vector<cl_int>(input.size());
	output[0] = 0;

	int j = 0;
	for (j = 1; j < input.size(); j++)
	{
		output[j] = output[j - 1] + input[j - 1];
	}

	j--;
	
	cout << "LAST:" << output[j] << endl;

	std::clock_t c_end = std::clock();
	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << std::fixed << std::setprecision(2) << "CPU time used: "
		<< 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC << " ms\n"
		<< "Wall clock time passed: "
		<< std::chrono::duration<double, std::milli>(t_end - t_start).count()
		<< " ms\n";

	return 0;

}