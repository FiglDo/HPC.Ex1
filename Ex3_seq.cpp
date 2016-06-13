#include "Ex3_seq.h"

bool Ex3_seq::CheckLT(int value, int predicateValue)
{
	if (value < predicateValue)
		return true;
	return false;
}
bool Ex3_seq::CheckEQ(int value, int predicateValue)
{
	if (value == predicateValue)
		return true;
	return false;
}
bool Ex3_seq::CheckGT(int value, int predicateValue)
{
	if (value > predicateValue)
		return true;
	return false;
}
bool Ex3_seq::Checker(int value, int predicateType, int predicateValue)
{
	if (predicateType == 0)
		return CheckLT(value, predicateValue);
	else if (predicateType == 1)
		return CheckEQ(value, predicateValue);
	else if (predicateType == 2)
		return CheckGT(value, predicateValue);
}

int Ex3_seq::Ex3_seq_main(vector<int> input, int predicateType, int predicateValue)
{
	cout << endl << "Starting sequential Filter " << endl;

	std::clock_t c_start = std::clock();
	auto t_start = std::chrono::high_resolution_clock::now();

	vector<int> output;

	for(vector<int>::iterator it = input.begin(); it != input.end(); ++it) {
		if (Checker(*it, predicateType, predicateValue))
		{
			output.push_back(*it);
		}
	}

	cout << "Size of Output: " << output.size() << endl;

	std::clock_t c_end = std::clock();
	auto t_end = std::chrono::high_resolution_clock::now();

	std::cout << std::fixed << std::setprecision(2) << "CPU time used: "
		<< 1000.0 * (c_end - c_start) / CLOCKS_PER_SEC << " ms\n"
		<< "Wall clock time passed: "
		<< std::chrono::duration<double, std::milli>(t_end - t_start).count()
		<< " ms\n";

	return 0;
}
