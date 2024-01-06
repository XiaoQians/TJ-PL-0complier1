#ifndef CCOMPLIER_LEXER_H
#define CCOMPLIER_LEXER_H

#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <string>
#include <queue>
#include <stack>
#include<unordered_map>
using namespace std;
//产生式

//约定了算法和界符，对于关键字，直接取值即可，使用hash增加查找效率
const unordered_map<string, string> OPERATORWORD = {
        {"+",           "PLUS"},
        {"-",           "MINUS"},
        {"*",           "MULTIPLY"},
        {"/",           "DIVIDE"},
        {":=",          "ASSIGN"},
        {"=",           "EQUAL"},
        {"<>",          "NOT_EQUAL"},
        {">",           "GREATER"},
        {">=",          "GREATER_EQUAL"},
        {"<",           "LESS"},
        {"<=",          "LESS_EQUAL"},
        {"(",           "LEFT_PAREN"},
        {")",           "RIGHT_PAREN"},
        {";",           "SEMICOLON"},
        {",",           "COMMA"},
};

typedef struct prod {
    string left;//产生式左部
    vector<string> right;//产生式右部，含多个符号

} prod;

//文法
typedef struct gra {
    set<string> vn;//非终结符集
    set<string> vt;//终结符集合
    string start;//开始符号
    vector<prod> pro;//产生式序列
} gra;

//自动机
typedef struct fa {
    set<string> state;//状态集
    set<string> word;//字符集
    string start;//开始符号
    set<string> end;//终态集
    map<string, set<string>> fun;//状态转换，当前状态和输入字符
} fa;

//子集法
typedef struct clo {
    set<string> s;//子集法产生的子集
    string sign = "n";//自己是否被标记
    string n;//重命名时用到

    //<重载，不然set无法对数据结构建立红黑树
    bool operator<(const clo& x) const { return s < x.s; }
} clo;

//token表，行号、类型、内容
typedef struct tok {
    int linen{};//行号
    string type{};//类别
    string cont;//内容
} tok;

//通用异常类
class graexcep : public exception {
private:
    string msg;//错误内容
public:
    explicit graexcep(string s) {
        msg = std::move(s);
    }

    const char* what() const throw() {
        return msg.c_str();
    }
};


vector<string> readfile(int type);
vector<prod> StrToProd(const vector<string>& prods);
set<string> ProdToKl(const vector<prod>& v);
gra ProdToGra(const vector<prod>& p);
fa GraToNFA(const gra& g);
set<string> closure(const set<string>& sta, const string& w, const map<string, set<string>>& fun);
set<string> move(const set<string>& sta, const string& w, const map<string, set<string>>& fun);
fa NFAToDFA(const fa& n);
vector<tok> CodeToToken();
vector<tok> test();

#endif //CCOMPLIER_LEXER_H
