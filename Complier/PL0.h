#pragma once
#include<exception>
#include<iostream>
#include<string>
using namespace std;
class PL0 :public exception
{
public:
	PL0(string msg) :mMsg(msg) {}
	PL0(string msg, int line) :mMsg(msg), line(line) {}
	virtual ~PL0()noexcept {}
	virtual const char* what() const noexcept override
	{
		return mMsg.c_str();
	}
	string msg() {
		string x = to_string(line);
		string linestr = " at line " + x;
		string output = mMsg + linestr;
		return output;
	}
private:
	string mMsg;
	int line = 0;
	int col = 0;
};
