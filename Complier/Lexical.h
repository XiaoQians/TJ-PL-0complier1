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
//����ʽ

//Լ�����㷨�ͽ�������ڹؼ��֣�ֱ��ȡֵ���ɣ�ʹ��hash���Ӳ���Ч��
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
    string left;//����ʽ��
    vector<string> right;//����ʽ�Ҳ������������

} prod;

//�ķ�
typedef struct gra {
    set<string> vn;//���ս����
    set<string> vt;//�ս������
    string start;//��ʼ����
    vector<prod> pro;//����ʽ����
} gra;

//�Զ���
typedef struct fa {
    set<string> state;//״̬��
    set<string> word;//�ַ���
    string start;//��ʼ����
    set<string> end;//��̬��
    map<string, set<string>> fun;//״̬ת������ǰ״̬�������ַ�
} fa;

//�Ӽ���
typedef struct clo {
    set<string> s;//�Ӽ����������Ӽ�
    string sign = "n";//�Լ��Ƿ񱻱��
    string n;//������ʱ�õ�

    //<���أ���Ȼset�޷������ݽṹ���������
    bool operator<(const clo& x) const { return s < x.s; }
} clo;

//token���кš����͡�����
typedef struct tok {
    int linen{};//�к�
    string type{};//���
    string cont;//����
} tok;

//ͨ���쳣��
class graexcep : public exception {
private:
    string msg;//��������
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
