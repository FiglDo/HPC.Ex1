#include "Ex1.h"
#include "Ex2_simpleScan_one_wg.h"
#include "Ex2_simpleScan_more_wg.h"
#include "Ex2_seq.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

int main(void)
{
	vector<cl_int> input = vector<cl_int>();

	//input.push_back(1);
	//input.push_back(1);
	//input.push_back(1);
	//input.push_back(1);
	//input.push_back(1);
	//input.push_back(1);
	//input.push_back(1);
	//input.push_back(1);

	//input.push_back(3);
	//input.push_back(1);
	//input.push_back(7);
	//input.push_back(0);
	//input.push_back(4);
	//input.push_back(1);
	//input.push_back(6);
	//input.push_back(3);


	int target = 0;
	int value = 0;
	
	for (int i = 0; i < (1024*16); i++)
	{
		value = rand() % 50;
		input.push_back(value);
		target += value;
	}

	target -= value;
	
	cout << "Processing " << input.size() << " Elements." << endl;

	cout << "TARGET: " << target << endl;
	cout << endl << endl;


	Ex2_seq::Ex2_seq_main(input);


	Ex2_simpleScan_more_wg::Ex2_main(input);


	

	cin.get();

	
}