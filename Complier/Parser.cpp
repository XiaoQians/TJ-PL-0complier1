
#include"Parser.h"
#include"PL0.h"
#include<map>
#include<vector>
#include<cstdlib>
#include<string.h>
#include<cstring>
using namespace std;
map<string, string> IdentifierTable;
map<string, string> TempTable;

int tempCount = 0;
vector<array<string, 4>> IR(100);

std::vector<std::string> variableNames;

// Function to check for duplicate variable names
bool isDuplicateVariable(const std::string& name) {
	return std::find(variableNames.begin(), variableNames.end(), name) != variableNames.end();
}

void emit(string op, string arg1, string arg2, string result)
{
	IR.push_back({ op,arg1,arg2,result });
}
string newtemp()
{
	while (IdentifierTable.find("TempVar" + to_string(tempCount)) != IdentifierTable.end()
		   ||
		   TempTable.find("TempVar" + to_string(tempCount)) != TempTable.end())
	{
		tempCount++;
	}
	TempTable["TempVar" + to_string(tempCount)] = "Temp";
	return "TempVar" + to_string(tempCount);
}
/*
	<条件> => <表达式><关系运算符><表达式>

	Condition将指针指向Condition后一项
*/
array<string, 3> Parser::Condition() {
	auto arg1 = Expression();
	if (tokens[index].type != "EQUAL" &&
		tokens[index].type != "NOT_EQUAL" &&
		tokens[index].type != "LESS" &&
		tokens[index].type != "LESS_EQUAL" &&
		tokens[index].type != "GREATER" &&
		tokens[index].type != "GREATER_EQUAL"
		) {
		throw PL0("非法的关系运算符", tokens[index].linen);
	}
	auto op = tokens[index].cont;
	auto arg2 = Expression();
	return { op,arg1,arg2 };
}

/*
	<条件语句> => IF <条件> THEN <语句>
*/
void Parser::ConditionStat() {
	// 此时token已经指向IF
	auto [op, arg1, arg2] = Condition();
	auto to_refill1 = IR.size();
	emit("j" + op, arg1, arg2, "M");
	auto to_refill2 = IR.size();
	emit("j", "_", "_", "M");
	if (tokens[index].type != "THEN") {
		throw PL0("条件语句格式错误", tokens[index].linen);
	}
	NextToken();
	IR[to_refill1][3] = to_string(IR.size());
	Statement();
	IR[to_refill2][3] = to_string(IR.size());
}

/*
	<循环语句> => WHILE <条件> DO <语句>
*/
void Parser::LoopStat() {
	auto x = IR.size();
	auto [op, arg1, arg2] = Condition();
	auto to_refill1 = IR.size();
	emit("j" + op, arg1, arg2, "M");
	auto to_refill2 = IR.size();
	emit("j", "_", "_", "M");
	if (tokens[index].type != "DO") {
		throw PL0("循环语句格式错误", tokens[index].linen);
	}
	NextToken();
	IR[to_refill1][3] = to_string(IR.size());
	Statement();
	emit("j", "_", "_", to_string(x));
	IR[to_refill2][3] = to_string(IR.size());
}


/*
	<因子> => <标识符>|<无符号整数>|(<表达式>)
*/
string Parser::Factor() {
	if (tokens[index].type == "IDENTIFIER") {
		// 识别为标识符
		if (IdentifierTable.find(tokens[index].cont) == IdentifierTable.end()) {
			throw PL0("Identifier Not Declared", tokens[index].linen);
		}
		return tokens[index].cont;
	}
	else if (tokens[index].type == "NUMBER") {
		// 识别为数字
		return tokens[index].cont;
	}
	else if (tokens[index].type == "LEFT_PAREN") {
		// 识别为表达式
		auto E = Expression();
		if (tokens[index].type != "RIGHT_PAREN") {
			throw PL0("缺少右括号", tokens[index].linen);
		}
		return E;
	}
	else {
		throw PL0("缺少因子或因子格式错误", tokens[index].linen);
	}
}

/*
	<项> => <因子>|<项><乘法运算符><因子>
	可以等价为：
	<项> => <因子> { <乘法运算符> <因子> }

	Item将指针指向Item后一项
*/
string Parser::Item() {
	auto currentItem = Factor();
	NextToken();
	while (true) {
		if (tokens[index].type == "MULTIPLY" || tokens[index].type == "DIVIDE") {
			// 存在后继因子
			// 语义处理
			auto temp = newtemp();
			auto op = tokens[index].cont;
			auto arg1 = currentItem;
			NextToken();
			auto arg2 = Factor();
			NextToken();
			emit(op, arg1, arg2, temp);
			currentItem = temp;
		}
		else
			break;
	}

	return currentItem;
}

/*
	<表达式> => [+|-]项|<表达式><加法运算符><项>
	可以等价为：
	<表达式> => [<加法运算符>] 项 { <加法运算符> 项 }

	Expression将指针指向Expression后一项
*/
string Parser::Expression() {
	bool existSingleMinus = false;
	NextToken();
	if (tokens[index].type == "PLUS" || tokens[index].type == "MINUS") {
		// 语义处理
		existSingleMinus = true;
		NextToken();
	}
	auto currentItem = Item();//Item函数此时指向了Item后一项
	while (true) {
		if (tokens[index].type == "PLUS" || tokens[index].type == "MINUS") {
			// 存在后继项
			// 语义处理
			auto temp = newtemp();
			auto op = tokens[index].cont;
			auto arg1 = currentItem;
			NextToken();
			auto arg2 = Item();
			//Item函数此时指向了Item后一项
			emit(op, arg1, arg2, temp);
			currentItem = temp;
		}
		else
			break;
	}
	return currentItem;
}

/*
	<赋值语句> => <标识符> := <表达式>
*/
void Parser::AssignmentStat() {
	// 此时token已经指向IDENTIFIER
	string sym_name = tokens[index].cont;

	NextToken();
	if (tokens[index].type != "ASSIGN") {
		throw PL0("赋值语句格式错误", tokens[index].linen);
	}
	auto E = Expression();
	// finish 以下为语义分析、中间代码生成
	if (
		IdentifierTable.find(sym_name) != IdentifierTable.end()
		)
	{
		emit(":=", E, "_", sym_name);
	}
	else
	{
		throw PL0("Identifier Not Found", tokens[index].linen);
	}
}

/*
	<复合语句> => BEGIN <语句>{;<语句>}END
*/
void Parser::CompoundStat() {
	// 此时TOKEN指向BEGIN
	tokens[index] = NextToken();
	Statement();
	while (true) {
		if (tokens[index].type == "SEMICOLON") {
			//空语句后分号
			NextToken();
			Statement();
		}
		else {
			if (tokens[index].type == "END") {
				//空语句后END（分号后END），或者嵌套复合语句会执行到这里
				if (end_flag == 0) {
					//空语句后END（分号后END）
					end_flag = 1;
					break;
				}
				else if (end_flag == 1) {
					//嵌套复合语句
					end_flag = 0;
				}
			}
			//正常语句结束
			NextToken();
			if (tokens[index].type == "SEMICOLON") {
				//正常语句后分号
				NextToken();
				Statement();
			}
			else if (tokens[index].type == "END") {
				//正常语句后END
				end_flag = 1;
				break;
			}
			else {
				throw PL0("复合语句格式错误", tokens[index].linen);
			}
		}
	}
}
/*
	<变量说明> => VAR<标识符>{,<标识符>};
*/
void Parser::VarDeclaration() {
	// 此时token已经指向VAR
	NextToken();
	if (tokens[index].type != "IDENTIFIER") {
		throw PL0("变量说明格式错误", tokens[index].linen);
	}
	string sym_name = tokens[index].cont;

	// Check for duplicate variable names
	if (isDuplicateVariable(sym_name)) {
		throw PL0("变量名重定义", tokens[index].linen);
	}

	variableNames.push_back(sym_name);

	IdentifierTable[sym_name] = "VAR";
	NextToken();

	while (true) {
		if (tokens[index].type == "COMMA") {
			NextToken();
			if (tokens[index].type != "IDENTIFIER") {
				throw PL0("变量说明格式错误", tokens[index].linen);
			}
			sym_name = tokens[index].cont;

			// Check for duplicate variable names
			if (isDuplicateVariable(sym_name)) {
				throw PL0("变量名重定义", tokens[index].linen);
			}

			variableNames.push_back(sym_name);

			IdentifierTable[sym_name] = "VAR";
			NextToken();
		}
		else {
			break;
		}
	}

	if (tokens[index].type != "SEMICOLON") {
		throw PL0("常量说明格式错误，缺少分号", tokens[index].linen);
	}
}

/*
	<常量定义> => <标识符>=<无符号整数>
*/
void Parser::ConstDefinite() {
	NextToken();
	if (tokens[index].type != "IDENTIFIER") {
		throw PL0("常量定义格式错误", tokens[index].linen);
	}
	string sym_name = tokens[index].cont;
	NextToken();
	if (tokens[index].type != "ASSIGN") {
		throw PL0("常量定义格式错误", tokens[index].linen);
	}
	NextToken();
	if (tokens[index].type != "NUMBER") {
		throw PL0("常量定义格式错误", tokens[index].linen);
	}
	string sym_num = tokens[index].cont;
	int value = stoi(sym_num);
	// finish 以下为语义分析、中间代码生成，sym_name、sym_num可用
	IdentifierTable[sym_name] = "CONST";
}

/*
	<常量说明> => CONST <常量定义>{, <常量定义>};
*/
void Parser::ConstDeclaration() {
	// 此时token已经指向CONST
	ConstDefinite();
	NextToken();
	while (true) {
		if (tokens[index].type == "COMMA") {
			ConstDefinite();
			NextToken();
		}
		else break;
	}
	if (tokens[index].type != "SEMICOLON") {
		throw PL0("常量说明格式错误，缺少分号", tokens[index].linen);
	}
	// finish 以下为语义分析、中间代码生成
}

/*
	<语句> => <赋值语句>|<条件语句>|<循环语句>|<复合语句>|<空语句>
*/
void Parser::Statement() {
	// 此时token应指向语句第一个符号
	if (tokens[index].type == "IDENTIFIER") {
		//赋值语句
		AssignmentStat();
	}
	else if (tokens[index].type == "IF") {
		//条件语句
		ConditionStat();
	}
	else if (tokens[index].type == "WHILE") {
		//循环语句
		LoopStat();
	}
	else if (tokens[index].type == "BEGIN") {
		//复合语句
		CompoundStat();
	}
	else if (tokens[index].type == "SEMICOLON" || tokens[index].type == "END") {
		//复合语句中的空语句，不作处理
	}
	else {
		throw PL0("无法识别语句种类", tokens[index].linen);
	}

}
/*
	<分程序> => [<常量说明>][<变量说明>]<语句>
	FIRST = {CONST VAR IDENTIFIER IF WHILE BEGIN END_OF_FILE}
	语句包括空语句，END_OF_FILE是空语句的FOLLOW
*/
void Parser::SubProgram() {
	NextToken();
	if (tokens[index].type == "CONST") {
		ConstDeclaration();
		NextToken();
	}
	if (tokens[index].type == "VAR") {
		VarDeclaration();
		NextToken();
	}
	if (tokens[index].type == "END_OF_FILE")
		return;//分程序中的空语句，直接返回
	Statement();

	// finish 以下为语义分析、中间代码生成

}

/*
	<程序> => <程序首部><分程序>
	FIRST = {PROGRAM}
*/
bool Parser::BeginParse() {
	try {
		ProgramHead();
		SubProgram();
		return true;
	}
	catch (PL0 err) {
		cout << err.msg() << endl;
	}
	return false;
}

/*
	<程序首部> => PROGRAM<标识符>
	FIRST = {PROGRAM}
*/
void Parser::ProgramHead() {
	//NextToken();
	//cout << tokens[index + 1].type;
	if (tokens[index].type != "PROGRAM") {
		throw PL0("程序首部缺少关键字PROGRAM", tokens[index].linen);
	}
	NextToken();
	if (tokens[index].type != "IDENTIFIER") {
		throw PL0("程序首部缺少标识符", tokens[index].linen);
	}
	// finish 以下为语义分析、中间代码生成
	string program_name = tokens[index].cont;

}

void Parser::Output(ofstream& IRout)
{

	size_t irSize = IR.size();
	for (int i = 100; i < irSize; i++)
	{
		auto [op, arg1, arg2, result] = IR[i];
		IRout << i << " " << "(" + op + "," + arg1 + "," + arg2 + "," + result + ")" << endl;
	}

}