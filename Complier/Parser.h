#pragma once

#include "Lexical.h"
#include<string>
#include<array>
#include<fstream>

/*
	语法分析器
*/

//typedef struct tok {
//	int linen{};//行号
//	string type{};//类别
//	string cont;//内容
//} tok;

class Parser
{
private:
	vector<tok> tokens;
	bool end_flag = 0;
	//指示复合语句嵌套，用于复合语句解析


	void ProgramHead(); // 程序首部
	void SubProgram(); //分程序
	void ConstDeclaration();// 常量声明
	void ConstDefinite();//常量定义
	void VarDeclaration();//变量声明

	void Statement();//语句
	void AssignmentStat();//赋值语句
	void ConditionStat();//条件语句
	void LoopStat();//循环语句
	void CompoundStat();//复合语句



	std::string Expression();//表达式
	std::string Item();//项
	std::string Factor();//因子

	std::array<std::string, 3> Condition();//条件
	size_t index = 0;        // Keep track of the current token index

public:
	// Constructor to initialize the Parser with a vector of tokens
	Parser(const vector<tok>& to) : tokens(to), index(0) {}
	~Parser() {}
	bool BeginParse(); // 程序  启动语法分析
	void Output(std::ofstream& IRout);
	// Function to get the next token
	tok NextToken() {
		// Check if there are more tokens
		if (index < tokens.size()) {
			// Get the current token and increment the index for the next call
			index++;
			return tokens[index];
			cout << index;
		}
		else {
			// If no more tokens, return a dummy token or handle it as needed
			tok dummyToken;
			dummyToken.linen = -1;  // Some value indicating the end of tokens
			return dummyToken;
		}
	}

	// Add other parsing functions as needed for your specific parser logic
};


