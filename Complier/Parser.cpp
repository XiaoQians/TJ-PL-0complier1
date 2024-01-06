
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
	<����> => <���ʽ><��ϵ�����><���ʽ>

	Condition��ָ��ָ��Condition��һ��
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
		throw PL0("�Ƿ��Ĺ�ϵ�����", tokens[index].linen);
	}
	auto op = tokens[index].cont;
	auto arg2 = Expression();
	return { op,arg1,arg2 };
}

/*
	<�������> => IF <����> THEN <���>
*/
void Parser::ConditionStat() {
	// ��ʱtoken�Ѿ�ָ��IF
	auto [op, arg1, arg2] = Condition();
	auto to_refill1 = IR.size();
	emit("j" + op, arg1, arg2, "M");
	auto to_refill2 = IR.size();
	emit("j", "_", "_", "M");
	if (tokens[index].type != "THEN") {
		throw PL0("��������ʽ����", tokens[index].linen);
	}
	NextToken();
	IR[to_refill1][3] = to_string(IR.size());
	Statement();
	IR[to_refill2][3] = to_string(IR.size());
}

/*
	<ѭ�����> => WHILE <����> DO <���>
*/
void Parser::LoopStat() {
	auto x = IR.size();
	auto [op, arg1, arg2] = Condition();
	auto to_refill1 = IR.size();
	emit("j" + op, arg1, arg2, "M");
	auto to_refill2 = IR.size();
	emit("j", "_", "_", "M");
	if (tokens[index].type != "DO") {
		throw PL0("ѭ������ʽ����", tokens[index].linen);
	}
	NextToken();
	IR[to_refill1][3] = to_string(IR.size());
	Statement();
	emit("j", "_", "_", to_string(x));
	IR[to_refill2][3] = to_string(IR.size());
}


/*
	<����> => <��ʶ��>|<�޷�������>|(<���ʽ>)
*/
string Parser::Factor() {
	if (tokens[index].type == "IDENTIFIER") {
		// ʶ��Ϊ��ʶ��
		if (IdentifierTable.find(tokens[index].cont) == IdentifierTable.end()) {
			throw PL0("Identifier Not Declared", tokens[index].linen);
		}
		return tokens[index].cont;
	}
	else if (tokens[index].type == "NUMBER") {
		// ʶ��Ϊ����
		return tokens[index].cont;
	}
	else if (tokens[index].type == "LEFT_PAREN") {
		// ʶ��Ϊ���ʽ
		auto E = Expression();
		if (tokens[index].type != "RIGHT_PAREN") {
			throw PL0("ȱ��������", tokens[index].linen);
		}
		return E;
	}
	else {
		throw PL0("ȱ�����ӻ����Ӹ�ʽ����", tokens[index].linen);
	}
}

/*
	<��> => <����>|<��><�˷������><����>
	���Եȼ�Ϊ��
	<��> => <����> { <�˷������> <����> }

	Item��ָ��ָ��Item��һ��
*/
string Parser::Item() {
	auto currentItem = Factor();
	NextToken();
	while (true) {
		if (tokens[index].type == "MULTIPLY" || tokens[index].type == "DIVIDE") {
			// ���ں������
			// ���崦��
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
	<���ʽ> => [+|-]��|<���ʽ><�ӷ������><��>
	���Եȼ�Ϊ��
	<���ʽ> => [<�ӷ������>] �� { <�ӷ������> �� }

	Expression��ָ��ָ��Expression��һ��
*/
string Parser::Expression() {
	bool existSingleMinus = false;
	NextToken();
	if (tokens[index].type == "PLUS" || tokens[index].type == "MINUS") {
		// ���崦��
		existSingleMinus = true;
		NextToken();
	}
	auto currentItem = Item();//Item������ʱָ����Item��һ��
	while (true) {
		if (tokens[index].type == "PLUS" || tokens[index].type == "MINUS") {
			// ���ں����
			// ���崦��
			auto temp = newtemp();
			auto op = tokens[index].cont;
			auto arg1 = currentItem;
			NextToken();
			auto arg2 = Item();
			//Item������ʱָ����Item��һ��
			emit(op, arg1, arg2, temp);
			currentItem = temp;
		}
		else
			break;
	}
	return currentItem;
}

/*
	<��ֵ���> => <��ʶ��> := <���ʽ>
*/
void Parser::AssignmentStat() {
	// ��ʱtoken�Ѿ�ָ��IDENTIFIER
	string sym_name = tokens[index].cont;

	NextToken();
	if (tokens[index].type != "ASSIGN") {
		throw PL0("��ֵ����ʽ����", tokens[index].linen);
	}
	auto E = Expression();
	// finish ����Ϊ����������м��������
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
	<�������> => BEGIN <���>{;<���>}END
*/
void Parser::CompoundStat() {
	// ��ʱTOKENָ��BEGIN
	tokens[index] = NextToken();
	Statement();
	while (true) {
		if (tokens[index].type == "SEMICOLON") {
			//������ֺ�
			NextToken();
			Statement();
		}
		else {
			if (tokens[index].type == "END") {
				//������END���ֺź�END��������Ƕ�׸�������ִ�е�����
				if (end_flag == 0) {
					//������END���ֺź�END��
					end_flag = 1;
					break;
				}
				else if (end_flag == 1) {
					//Ƕ�׸������
					end_flag = 0;
				}
			}
			//����������
			NextToken();
			if (tokens[index].type == "SEMICOLON") {
				//��������ֺ�
				NextToken();
				Statement();
			}
			else if (tokens[index].type == "END") {
				//��������END
				end_flag = 1;
				break;
			}
			else {
				throw PL0("��������ʽ����", tokens[index].linen);
			}
		}
	}
}
/*
	<����˵��> => VAR<��ʶ��>{,<��ʶ��>};
*/
void Parser::VarDeclaration() {
	// ��ʱtoken�Ѿ�ָ��VAR
	NextToken();
	if (tokens[index].type != "IDENTIFIER") {
		throw PL0("����˵����ʽ����", tokens[index].linen);
	}
	string sym_name = tokens[index].cont;

	// Check for duplicate variable names
	if (isDuplicateVariable(sym_name)) {
		throw PL0("�������ض���", tokens[index].linen);
	}

	variableNames.push_back(sym_name);

	IdentifierTable[sym_name] = "VAR";
	NextToken();

	while (true) {
		if (tokens[index].type == "COMMA") {
			NextToken();
			if (tokens[index].type != "IDENTIFIER") {
				throw PL0("����˵����ʽ����", tokens[index].linen);
			}
			sym_name = tokens[index].cont;

			// Check for duplicate variable names
			if (isDuplicateVariable(sym_name)) {
				throw PL0("�������ض���", tokens[index].linen);
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
		throw PL0("����˵����ʽ����ȱ�ٷֺ�", tokens[index].linen);
	}
}

/*
	<��������> => <��ʶ��>=<�޷�������>
*/
void Parser::ConstDefinite() {
	NextToken();
	if (tokens[index].type != "IDENTIFIER") {
		throw PL0("���������ʽ����", tokens[index].linen);
	}
	string sym_name = tokens[index].cont;
	NextToken();
	if (tokens[index].type != "ASSIGN") {
		throw PL0("���������ʽ����", tokens[index].linen);
	}
	NextToken();
	if (tokens[index].type != "NUMBER") {
		throw PL0("���������ʽ����", tokens[index].linen);
	}
	string sym_num = tokens[index].cont;
	int value = stoi(sym_num);
	// finish ����Ϊ����������м�������ɣ�sym_name��sym_num����
	IdentifierTable[sym_name] = "CONST";
}

/*
	<����˵��> => CONST <��������>{, <��������>};
*/
void Parser::ConstDeclaration() {
	// ��ʱtoken�Ѿ�ָ��CONST
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
		throw PL0("����˵����ʽ����ȱ�ٷֺ�", tokens[index].linen);
	}
	// finish ����Ϊ����������м��������
}

/*
	<���> => <��ֵ���>|<�������>|<ѭ�����>|<�������>|<�����>
*/
void Parser::Statement() {
	// ��ʱtokenӦָ������һ������
	if (tokens[index].type == "IDENTIFIER") {
		//��ֵ���
		AssignmentStat();
	}
	else if (tokens[index].type == "IF") {
		//�������
		ConditionStat();
	}
	else if (tokens[index].type == "WHILE") {
		//ѭ�����
		LoopStat();
	}
	else if (tokens[index].type == "BEGIN") {
		//�������
		CompoundStat();
	}
	else if (tokens[index].type == "SEMICOLON" || tokens[index].type == "END") {
		//��������еĿ���䣬��������
	}
	else {
		throw PL0("�޷�ʶ���������", tokens[index].linen);
	}

}
/*
	<�ֳ���> => [<����˵��>][<����˵��>]<���>
	FIRST = {CONST VAR IDENTIFIER IF WHILE BEGIN END_OF_FILE}
	����������䣬END_OF_FILE�ǿ�����FOLLOW
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
		return;//�ֳ����еĿ���䣬ֱ�ӷ���
	Statement();

	// finish ����Ϊ����������м��������

}

/*
	<����> => <�����ײ�><�ֳ���>
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
	<�����ײ�> => PROGRAM<��ʶ��>
	FIRST = {PROGRAM}
*/
void Parser::ProgramHead() {
	//NextToken();
	//cout << tokens[index + 1].type;
	if (tokens[index].type != "PROGRAM") {
		throw PL0("�����ײ�ȱ�ٹؼ���PROGRAM", tokens[index].linen);
	}
	NextToken();
	if (tokens[index].type != "IDENTIFIER") {
		throw PL0("�����ײ�ȱ�ٱ�ʶ��", tokens[index].linen);
	}
	// finish ����Ϊ����������м��������
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