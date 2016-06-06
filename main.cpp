#include "Ex1.h"
#include "Ex2_simpleScan_one_wg.h"
#include "Ex2_simpleScan_more_wg.h"
#include "Ex2_seq.h"
#include "Ex2_WorkEfficient_Scan.h"
#include "Ex3.h"
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

	int inputVectorSize = 64;// -1;
	int amountOfWorkGroups = 4;
	int predicateType = 0; // 0 = LT; 1=EQ; 2 = GT
	int predicateValue = 20;
	
	int targetScan = 0;
	int targetFilterCount = 0;
	int value = 0;
	srand(time(0));

	
	for (int i = 0; i < inputVectorSize; i++)
	{
		value = rand() % 10;
		input.push_back(value);
		targetScan += value;

		if (predicateType == 0)
		{
			if (value < predicateValue)
			{
				targetFilterCount += 1;
			}
		}
		else if (predicateType == 1)
		{
			if (value == predicateValue)
			{
				targetFilterCount += 1;
			}
		}
		else if (predicateType == 2)
		{
			if (value > predicateValue)
			{
				targetFilterCount += 1;
			}
		}		
	}


	targetScan -= value;
	
	cout << "Processing " << input.size() << " Elements." << endl;

	cout << "INPUT SIZE: " << input.size() << endl;
	cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
	cout << "TARGET SUM SCAN: " << targetScan << endl;
	cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
	cout << endl << endl;

	
	
	//Ex2_seq::Ex2_seq_main(input);
	//Ex2_simpleScan_more_wg::Ex2_main(input, amountOfWorkGroups);


	
	//Ex3::Ex3_main(input, amountOfWorkGroups, predicateType, predicateValue,1,0);


	Ex2_WorkEfficient_Scan::Ex2_main(input, amountOfWorkGroups, 1);


	cout << endl << endl;

	cout << "INPUT SIZE: " << input.size() << endl;
	cout << "TARGET SUM SCAN: " << targetScan << endl;
	cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
	


	cin.get();

	
}