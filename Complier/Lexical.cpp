#pragma once
#include "Lexical.h"


//��ȡ�ļ���1���ʷ�����2�����Դ���
vector<string> readfile(int type) {
    if (type == 1 || type == 2) {
        ifstream inf;
        vector<string> v;
        string s;
        if (type == 1) {
            inf.open("D:/source/repos/complier/CYY/lexgra.txt", ios::in);//ֻ��
            string last;
            //��ȡע������Ϣ
            while (getline(inf, s)) {
                if (s[0] == '#') {
                    last = s[s.size() - 1];
                }
                if (s[0] != '#') {
                    s.insert(0, last);
                    v.push_back(s);
                }
            }
        }
        else {
            inf.open("C:/Users/43411/Desktop/Test/source.txt", ios::in);//ֻ��
            while (getline(inf, s)) {
                int po = (int)s.find_first_of('/');
                //ɾ��˫б��ע��
                if (po != -1) {
                    if (s[po + 1] == '/') {
                        s.erase(po);
                    }
                }
                v.push_back(s);
            }
        }
        if (v.empty()) {
            throw graexcep("�ļ�Ϊ�գ�");
        }
        inf.close();
        return v;
    }
    else {
        throw graexcep("�ļ���ȡ�쳣��");//�׳��ļ����쳣
    }
}

//�ַ���������ʽ
vector<prod> StrToProd(const vector<string>& prods) {
    vector<prod> pro;
    for (auto& x : prods) {
        int po1 = (int)x.find_first_of('-');
        prod p;
        p.left = x.substr(0, po1);//��
        //��->��ʼ������˫���ź������ȡ��
        for (unsigned int i = po1 + 3; i <= x.size(); i++) {
            unsigned int j = i;
            for (; j < x.size(); j++) {
                if (x[j] == '\'' || x[j] == '\"') {
                    break;
                }
            }
            p.right.push_back(x.substr(i - 1, (j - i + 1)));
            i = j;
        }
        pro.push_back(p);
    }
    return pro;
}

//����ؼ���,���
set<string> ProdToKl(const vector<prod>& v) {
    set<string> k;
    for (auto x : v) {
        k.insert(x.right[0]);
    }
    return k;
}

//����ʽ���ķ�
gra ProdToGra(const vector<prod>& p) {
    gra g;
    for (auto x : p) {
        string t = x.left;
        t.erase(0, 1);//ȥ������ʽ�󲿵�������
        x.left = t;
        g.pro.push_back(x);
        g.vn.insert(t);
        if (t[0] == '[') { g.start = t; }
        for (auto y : x.right) {
            if (y[0] == '\'') { g.vt.insert(y); }
        }
    }
    return g;
}

//�����ķ���NFA
fa GraToNFA(const gra& g) {
    fa f;
    f.word = g.vt;
    f.state = g.vn;
    f.start = g.start;
    f.state.insert("\"Z");
    f.end.insert("\"Z");
    for (auto x : g.pro) {
        string s = x.left + x.right[0];
        if (x.right.size() == 1) {
            f.fun[s].insert("\"Z");
        }
        if (x.right.size() == 2) {
            f.fun[s].insert(x.right[1]);
        }
    }
    return f;
}

//sta��w�հ���w-closure(sta)
set<string> closure(const set<string>& sta, const string& w, const map<string, set<string>>& fun) {
    set<string> cl = sta;
    vector<string> v;
    v.reserve(sta.size());
    for (const auto& x : sta) {
        v.push_back(x);
    }
    //v.size��ÿ��ѭ��ʱ�������¼��㣬��֤�²����״̬���ᱻ����
    for (int i = 0; i < v.size(); i++) {
        string s = v[i] + w;
        auto it = fun.find(s);
        if (it != fun.end()) {
            for (const auto& x : it->second) {
                v.push_back(x);
            }
        }
        cl.insert(v[i]);
    }
    return cl;
}

//sta��w��ת����move(sta,w)
set<string> move(const set<string>& sta, const string& w, const map<string, set<string>>& fun) {
    set<string> m;
    for (const auto& x : sta) {
        string s = x + w;
        auto it = fun.find(s);
        if (it != fun.end()) {
            for (const auto& y : it->second) {
                m.insert(y);
            }
        }
    }
    return m;
}

//NFA��DFA
fa NFAToDFA(const fa& n) {
    fa d;
    set<clo> c;
    clo cl;
    set<string> tem;
    tem.insert(n.start);
    cl.s = closure(tem, "'@", n.fun);//����c�г�ʼ�Ӽ�
    tem.clear();
    cl.n = "0";//K0�����Ϊ0
    c.insert(cl);
    int k = 1;//�������Ĳ�ͬ�Ӽ����
    set<clo>::iterator i;//ָ��set<clo>��ָ��i
    while (true) {
        //�ж��Ӽ��Ƿ񱻱��
        for (i = c.begin(); i != c.end(); i++) {
            if (i->sign == "n") {
                break;
            }
        }
        if (i != c.end()) {
            clo t = *i;
            t.sign = "y";
            //�޸�set��ĳһԪ�أ���ɾ���ٲ��룬Ч�ʺܵ�
            c.erase(i);
            c.insert(t);
            for (const auto& a : n.word) {
                if (a != "\'@") {
                    clo tu;
                    tu.s = closure(move(t.s, a, n.fun), "'@", n.fun);
                    if (tu.s.empty()) {
                        continue;
                    }//��ֹ������Ӽ�Ϊ��
                    tu.sign = "y";
                    auto it2 = c.find(tu);
                    if (it2 == c.end()) {
                        tu.sign = "n";
                        tu.n = to_string(k);//����Ӽ�
                        c.insert(tu);
                        k++;
                    }
                }
            }
        }
        else {
            break;
        }
    }
    d.word = n.word;
    for (auto x : c) {
        d.state.insert(x.n);
        if (x.s.find("\"Z") != x.s.end()) { d.end.insert(x.n); }
        if (x.s == cl.s) { d.start = x.n; }
        set<string> kej;
        //����ȷ����ǰ״̬�ɽ��յ��ַ�
        for (const auto& y : d.word) {
            for (const auto& z : x.s) {
                string wor = z + y;
                auto it = n.fun.find(wor);
                if (it != n.fun.end()) {
                    kej.insert(y);
                }
            }
        }
        for (const auto& y : kej) {
            if (y != "'@") {
                string s = x.n + y;
                set<string> se;
                se = closure(move(x.s, y, n.fun), "'@", n.fun);
                for (const auto& z : c) {
                    //�ҵ���һ״̬�ı������˳�ѭ��
                    if (z.s == se) {
                        d.fun[s].insert(z.n);
                        break;
                    }
                }
            }
        }
    }
    return d;
}

//����������Ҫ�޸ġ���
vector<tok> CodeToToken() {
    vector<string> prods = readfile(1);
    //�Ȼ�ȡȫ���Ĳ���ʽ���ٷֳɲ�ͬ���Ĳ���ʽ
    vector<prod> pro = StrToProd(prods);
    //����
   /* for (auto i : pro) {
        std::cout << i.left << "->";
        for (auto j : i.right) {
            cout << j;
        }
        cout << endl;
    }*/
    //�����һ�£�����ֵ���ж�
    vector<prod> keyw, ident, cons, limt, opert;
    for (auto x : pro) {
        if (x.left[0] == '1') { keyw.push_back(x); }
        if (x.left[0] == '2') { ident.push_back(x); }
        if (x.left[0] == '3') { cons.push_back(x); }
        if (x.left[0] == '4') { limt.push_back(x); }
        if (x.left[0] == '5') { opert.push_back(x); }
    }
    pro.clear();

    //�������ԣ�
    //cout << "��������" << endl;
    set<string> skeyw = ProdToKl(keyw);
    set<string> slimt = ProdToKl(limt);

    gra gIdten = ProdToGra(ident);
    gra gCons = ProdToGra(cons);
    gra gOpert = ProdToGra(opert);

    fa nfaIdten = GraToNFA(gIdten);
    fa nfaCons = GraToNFA(gCons);
    fa nfaOpert = GraToNFA(gOpert);

    fa dfaIdten = NFAToDFA(nfaIdten);
    fa dfaCons = NFAToDFA(nfaCons);
    fa dfaOpert = NFAToDFA(nfaOpert);

    vector<tok> to;//�洢token
    vector<string> codes = readfile(2);
    //��ĸ�����ֵļ��ϣ���ʶ��ʱ�ᱻ��'let��'num����
    set<string> letter;
    set<string> num;
    for (int i = 0; i < 26; ++i) {
        char ct = (char)('a' + i);
        char ctl = (char)('A' + i);
        string s(1, ct);
        string sl(1, ctl);
        letter.insert(s);
        letter.insert(sl);
    }
    for (int i = 0; i < 10; ++i) {
        char c = (char)('0' + i);
        string s(1, c);
        num.insert(s);
    }
    set<string> ope = nfaOpert.word;
    //���ϵ��£���ǰ����һ�����ַ�����
    for (int i = 1; i <= codes.size(); i++) {
        string line = codes[i - 1];//ÿһ�е�����
        int linelen = (int)line.size();
        for (int j = 1; j <= linelen; ++j) {
            string ch = line.substr(j - 1, 1);
            tok t;
            if (ch == " ") {
                continue;//�����ո�
            }
            else if (slimt.find("\'" + ch) != slimt.end()) {//������
                t.linen = i;
                t.cont = ch;
                if (OPERATORWORD.find(t.cont) != OPERATORWORD.end()) {
                    t.type = OPERATORWORD.at(t.cont);;
                }
                else
                {
                    cout << "ʶ�����" << endl;
                }
                to.push_back(t);
            }
            else if (letter.find(ch) != letter.end()) {//�����ʶ��
                set<string> end = dfaIdten.end;
                string sta = dfaIdten.start;
                string cha;
                int k;
                //��ʶ������һ���ַ�����Ҫ����ȡ�ַ�
                for (k = j; k <= linelen; k++) {
                    cha = line.substr(k - 1, 1);
                    //cout<<k<<" "<<cha<<endl;
                    string c;
                    //dfa����let��num����ָ��ĸ������
                    if (letter.find(cha) != letter.end()) {
                        c = sta + "\'let";
                    }
                    else if (num.find(cha) != num.end()) {
                        c = sta + "\'num";
                    }
                    else {
                        break; //��ʶ��ֻ����ĸ���֣����������
                    }
                    auto it = dfaIdten.fun.find(c);
                    //������һ״̬��û�����˳�ѭ��
                    if (it != dfaIdten.fun.end()) {
                        sta = *(it->second.begin());
                    }
                    else {
                        break;
                    }
                }
                //�˳�ѭ�����жϵ�ǰ�Ƿ�����̬�����Ǿ��׳��쳣
                if (end.find(sta) != end.end()) {
                    t.cont = line.substr(j - 1, k - j);
                    t.linen = i;
                    if (skeyw.find("\'" + t.cont) != skeyw.end()) {
                        t.type = t.cont;//�ؼ���
                    }
                    else {
                        t.type = "IDENTIFIER";//��ʶ��
                    }
                    to.push_back(t);
                    j = k - 1;
                }
                else {
                    string stt = "��" + to_string(i) + "�е� " + line.substr(j - 1, k - j) + "ʶ��ʧ�ܣ��ñ�ʶ���޷���ʶ��";
                    throw graexcep(stt);
                }
            }
            else if (num.find(ch) != num.end()) {//��������
                set<string> end = dfaCons.end;
                string sta = dfaCons.start;
                string cha;
                int k;
                for (k = j; k <= linelen; k++) {
                    cha = line.substr(k - 1, 1);
                    string ct = sta + "\'";
                    string c = ct + cha;
                    //������num��ָ
                    if (num.find(cha) != num.end()) {
                        c.clear();
                        c = sta + "\'num";
                    }
                    auto it = dfaCons.fun.find(c);
                    if (it != dfaCons.fun.end()) {
                        sta = *(it->second.begin());
                    }
                    else {
                        break;
                    }
                }
                if (end.find(sta) != end.end()) {
                    t.cont = line.substr(j - 1, k - j);
                    t.linen = i;
                    t.type = "NUMBER";
                    to.push_back(t);
                    j = k - 1;
                }
                else {
                    string stt = "��" + to_string(i) + "�е� " + line.substr(j - 1, k - j) + "ʶ��ʧ�ܣ��������޷���ʶ��";
                    throw graexcep(stt);
                }
            }
            else if (ope.find("\'" + ch) != ope.end()) {//���
                set<string> end = dfaOpert.end;
                string sta = dfaOpert.start;//��ʼ״̬
                string cha;
                int k;
                for (k = j; k <= linelen; k++) {
                    cha = line.substr(k - 1, 1);
                    string ct = sta + "\'";
                    string c = ct + cha;
                    auto it = dfaOpert.fun.find(c);
                    if (it != dfaOpert.fun.end()) {
                        sta = *(it->second.begin());
                    }
                    else {
                        break;
                    }
                }
                if (end.find(sta) != end.end()) {
                    t.cont = line.substr(j - 1, k - j);
                    t.linen = i;
                    if (OPERATORWORD.find(t.cont) != OPERATORWORD.end())
                    {
                        t.type = OPERATORWORD.at(t.cont);
                    }
                    else {
                        cout << "ERROR" << endl;
                    }
                    to.push_back(t);
                    j = k - 1;

                }
                else {
                    string stt = "��" + to_string(i) + "�е� " + line.substr(j - 1, k - j) + "ʶ��ʧ�ܣ��ò������޷���ʶ��";
                    throw graexcep(stt);
                }
            }
            else {//�����е�dfa������ʶ���򴴽�ʧ��
                string stt = "��" + to_string(i) + "��ʶ��ʧ�ܣ������д��ڷ����޷���ʶ��";
                throw graexcep(stt);
            }
        }
    }
    //��ӡtoken��


    ofstream out;
    out.open("C:/Users/43411/Desktop/Test/Result/token.txt", ios::out | ios::trunc);//ɾ��������token.txt
    int i = 0;
    //for ( auto& x : to) {
    //    if (x.type == 5) {
    //        x.type=4;
    //    }
    //    i++;
    //}//������ͽ����Ϊһ��
    //i = 0;
    //д��token.txt
    for (const auto& x : to) {
        if (i != (to.size() - 1)) {
            out << x.linen << "\t" << x.type << "\t" << x.cont << endl;
        }
        else {
            out << x.linen << "\t" << x.type << "\t" << x.cont;
        }
        i++;
    }
    out.close();
    return to;
}

vector<tok> test() {
    try {
        vector<tok> t = CodeToToken();
        return t;
    }
    catch (graexcep ge) {
        cout << ge.what();
    }
    catch (logic_error le) {
        cout << le.what() << " �߼�����";
    }
}
