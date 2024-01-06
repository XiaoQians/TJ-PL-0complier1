#pragma once

#include "Lexical.h"
#include<string>
#include<array>
#include<fstream>

/*
	�﷨������
*/

//typedef struct tok {
//	int linen{};//�к�
//	string type{};//���
//	string cont;//����
//} tok;

class Parser
{
private:
	vector<tok> tokens;
	bool end_flag = 0;
	//ָʾ�������Ƕ�ף����ڸ���������


	void ProgramHead(); // �����ײ�
	void SubProgram(); //�ֳ���
	void ConstDeclaration();// ��������
	void ConstDefinite();//��������
	void VarDeclaration();//��������

	void Statement();//���
	void AssignmentStat();//��ֵ���
	void ConditionStat();//�������
	void LoopStat();//ѭ�����
	void CompoundStat();//�������



	std::string Expression();//���ʽ
	std::string Item();//��
	std::string Factor();//����

	std::array<std::string, 3> Condition();//����
	size_t index = 0;        // Keep track of the current token index

public:
	// Constructor to initialize the Parser with a vector of tokens
	Parser(const vector<tok>& to) : tokens(to), index(0) {}
	~Parser() {}
	bool BeginParse(); // ����  �����﷨����
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


