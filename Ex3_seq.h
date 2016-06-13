#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

class Ex3_seq
{
public:
	static bool	CheckLT(int value, int predicateValue);
	static bool	CheckEQ(int value, int predicateValue);
	static bool	CheckGT(int value, int predicateValue);
	static bool	Checker(int value, int predicateType, int predicateValue);
	static int Ex3_seq_main(vector<int> intput, int predicateType, int predicateValue);
};

