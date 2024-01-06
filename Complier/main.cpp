#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include "Lexical.h"
#include "Parser.h"
#include"PL0.h"
using namespace std;


int main() {
    string inputFile = "C:/Users/43411/Desktop/Test/source1.txt";
    string ResultFile = "C:/Users/43411/Desktop/Test/Result/IR.csv";

    int i = 0;
    //词法分析
    vector<tok>to = test();

    //语法分析
    Parser parser(to);
    bool flag = parser.BeginParse();

    //输出结果文件
    ofstream Resultout(ResultFile);

    parser.Output(Resultout);
    Resultout.close();
    system("start C:\\Users\\43411\\Desktop\\Test\\Result\\IR.txt");
    system("start C:\\Users\\43411\\Desktop\\Test\\Result\\token.txt");
    return 0;
}