#include "Ex1.h"
#include "Ex2_simpleScan_one_wg.h"
#include "Ex2_simpleScan_more_wg.h"
#include "Ex2_seq.h"
#include "Ex2_WorkEfficient_Scan.h"
#include "Ex3.h"
#include "Ex3_seq.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

void GetInput(vector<cl_int>& input, int inputVectorSize, int predicateType, int predicateValue, int&targetScan, int&targetFilterCount)
{

	int value = 0;


	for (int i = 0; i < inputVectorSize; i++)
	{
		value = rand() % 20;
		input.push_back(value);
		targetScan += value;

		if (predicateType != -1)
		{		
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
	}
	
	targetScan -= value;
}

int main(void)
{
	string selection = "";

	do
	{
		cout << "Please select the implementation you want to run!" << endl;
		cout
			<< "0: EXIT" << endl
			<< "1: Ex1 - Image Rotate" << endl
			<< "2: Ex2 - sequential scan" << endl
			<< "3: Ex2 - simple scan (one workgroup)" << endl
			<< "4: Ex2 - simple scan (4 workgroups; 32 input values)" << endl
			<< "5: Ex2 - work efficient scan (4 workgroups; 32 input values)" << endl
			<< "6: Ex3 - Stream Compaction Sequential" << endl
			<< "7: Ex3 - Stream Compaction Parallel(4 workgroups; 31 input values)" << endl
			<< "8: Ex3 - Stream Compaction Parallel(1024 workgroups; 2097151 input values)" << endl;

		cout << "Insert number: ";

		cin >> selection;


		vector<cl_int> input = vector<cl_int>();
		
		if (selection == "1")
		{
			cout << endl << "Running Image Rotate" << endl;
			cout << "Specify degree (-360 to 360): ";
			double degree = 0;
			cin >> degree;

			Ex1::Ex1_main(degree);
		}
		else if (selection == "2")
		{
			input.push_back(3);
			input.push_back(1);
			input.push_back(7);
			input.push_back(0);
			input.push_back(4);
			input.push_back(1);
			input.push_back(6);
			input.push_back(3);

			cout << "Expected result => 22" << endl;

			Ex2_seq::Ex2_seq_main(input);
		}
		else if (selection == "3")
		{
			input.push_back(3);
			input.push_back(1);
			input.push_back(7);
			input.push_back(0);
			input.push_back(4);
			input.push_back(1);
			input.push_back(6);
			input.push_back(3);

			cout << "Expected result => 22" << endl;

			Ex2_seq::Ex2_seq_main(input);

			Ex2_simpleScan_one_wg::Ex2_main(input);
		}
		else if (selection == "4")
		{
			int showOutput = 0;
			cout << endl << "Ausgabe (0|1): ";
			cin >> showOutput;

			int inputVectorSize = 32;
			int amountOfWorkGroups = 4;

			int targetScan = 0;
			int targetFilterCount = 0;
			srand(time(0));


			GetInput(input, inputVectorSize, -1, 0, targetScan, targetFilterCount);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET SUM SCAN: " << targetScan << endl;
			cout << endl << endl;

			Ex2_seq::Ex2_seq_main(input);

			Ex2_simpleScan_more_wg::Ex2_main(input, amountOfWorkGroups, showOutput);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET SUM SCAN: " << targetScan << endl;
			cout << endl << endl;
		}
		else if (selection == "5")
		{
			int showOutput = 0;
			cout << endl << "Ausgabe (0|1): ";
			cin >> showOutput;

			int inputVectorSize = 32;
			int amountOfWorkGroups = 4;

			int targetScan = 0;
			int targetFilterCount = 0;
			srand(time(0));


			GetInput(input, inputVectorSize, -1, 0, targetScan, targetFilterCount);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET SUM SCAN: " << targetScan << endl;
			cout << endl << endl;

			Ex2_seq::Ex2_seq_main(input);

			Ex2_WorkEfficient_Scan::Ex2_main(input, amountOfWorkGroups, showOutput);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET SUM SCAN: " << targetScan << endl;
			//cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
			cout << endl << endl;
		}
		else if (selection == "6")
		{

			int inputVectorSize = 80;
			int predicateType = 0; // 0 = LT; 1=EQ; 2 = GT
			int predicateValue = 10;

			int targetScan = 0;
			int targetFilterCount = 0;
			srand(time(0));


			GetInput(input, inputVectorSize, predicateType, predicateValue, targetScan, targetFilterCount);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
			cout << endl << endl;

			Ex3_seq::Ex3_seq_main(input, predicateType, predicateValue);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
			cout << endl << endl;
		}
		else if (selection == "7")
		{
			int showOutput = 0;
			int showOutputScan = 0;
			cout << endl << "Ausgabe Scan (0|1): ";
			cin >> showOutputScan;
			cout << "Ausgabe Filter (0|1): ";
			cin >> showOutput;

			int inputVectorSize = 31;
			int amountOfWorkGroups = 4;
			int predicateType = 0; // 0 = LT; 1=EQ; 2 = GT
			cout << endl << "PredicateType(0:LowerThan, 1 : Equal, 2 : GreaterThan): ";
			cin >> predicateType;

			int predicateValue = 10;

			int targetScan = 0;
			int targetFilterCount = 0;
			srand(time(0));


			GetInput(input, inputVectorSize, predicateType, predicateValue, targetScan, targetFilterCount);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
			cout << endl << endl;

			Ex3_seq::Ex3_seq_main(input, predicateType, predicateValue);

			Ex3::Ex3_main(input, amountOfWorkGroups, predicateType, predicateValue, showOutput, showOutputScan);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
			cout << endl << endl;
		}
		else if (selection == "8")
		{
			int showOutput = 0;
			int showOutputScan = 0;
			cout << endl << "Ausgabe Scan (0|1): ";
			cin >> showOutputScan;
			cout << "Ausgabe Filter (0|1): ";
			cin >> showOutput;

			int inputVectorSize = 2097151;
			int amountOfWorkGroups = 1024;
			int predicateType = 0; // 0 = LT; 1=EQ; 2 = GT
			cout << endl << "PredicateType(0:LowerThan, 1 : Equal, 2 : GreaterThan) : ";
			cin >> predicateType;

			int predicateValue = 20;

			int targetScan = 0;
			int targetFilterCount = 0;
			srand(time(0));


			GetInput(input, inputVectorSize, predicateType, predicateValue, targetScan, targetFilterCount);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
			cout << endl << endl;

			Ex3_seq::Ex3_seq_main(input, predicateType, predicateValue);

			Ex3::Ex3_main(input, amountOfWorkGroups, predicateType, predicateValue, showOutput, showOutputScan);

			cout << endl << "Processing " << input.size() << " Elements." << endl;

			cout << "INPUT SIZE: " << input.size() << endl;
			cout << "WORKGROUPS: " << amountOfWorkGroups << endl;
			cout << "TARGET FILTER COUNT : " << targetFilterCount << endl;
			cout << endl << endl;
		}

		cout << endl << "Press any key to continue!" << endl;
		cin.get();
		cin.get();

		std::system("cls");

	} while (selection != "0");

	return EXIT_SUCCESS;
}