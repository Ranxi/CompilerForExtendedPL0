#include "datastructure.h"
using namespace std;

extern ifstream  in;
extern ofstream  outimcode;

extern SYMTYPE sym;						// the type of the last token
extern string id;						// the last identifer read by far
extern int digits;						// the last number read by far
extern string line;						// the line buffer in getch() function
extern string tokenbuf;					// the token buffer in getsym() function
extern int lnum, cc, ll, err;			// go to PLO.cpp for details
extern char ch;							// a char in the getch() function

extern std::string RSRVWORD[NRSRVW];
extern char SSYMT[18];


void getsymt(char ss){				//此处使用二分查找获取单个字符的类型
	int i, j, k;
	i = 0;
	j = COMMONSYMINDEX;
	while (i <= j){
		k = (i + j) / 2;
		if (ss < SSYMT[k])
			j = k - 1;
		else if (ss == SSYMT[k])
			break;
		else
			i = k + 1;
	}
	if (i <= j)
		sym = (SYMTYPE)k;
	else
		sym = NUL;
}

void getch(){
	while (cc == ll){		//read the next line
		if (!getline(in, line) && sym != PERIOD){
			printf("Program incomplete!");
			closefiles();
			exit(0);
		}
		else if (sym == PERIOD){
			return;
		}
		line += '\n';
		lnum++;
		ll = line.length();
		cc = 0;
	}
	ch = line.at(cc);
	cc++;
}

void getsym(){
	int i, j, k;
	while (ch == ' ' || ch == '\t' || ch == '\n')
		getch();							//跳过空白字符
	if (islower(ch) || isupper(ch)){		//可能是一个标识符或者保留字
		k = 0;
		tokenbuf = "";
		while (islower(ch) || isupper(ch) || isdigit(ch)){
			tokenbuf += ch;
			getch();
		}
		k = tokenbuf.length();
		if (k > IDLENMAX){		//标识符过长
			printf("The identifier is too long!");
			closefiles();
			exit(0);
		}
		id = tokenbuf;
		i = RSVWBEGININDEX;
		j = RSVWENDINDEX;
		while (i <= j){		//二分搜索
			k = (i + j) / 2;
			if (id < RSRVWORD[k])
				j = k - 1;
			else if (id == RSRVWORD[k])
				break;
			else
				i = k + 1;
		}
		if (i <= j)
			sym = (SYMTYPE)k;
		else
			sym = IDF;
	}
	else if (isdigit(ch)){
		tokenbuf = "";
		k = 0;
		digits = 0;
		sym = NUMBER;
		while (isdigit(ch) && k <= NUMLENMAX){
			tokenbuf += ch;
			digits = digits * 10 + (ch - '0');
			k++;
			getch();
		}
		if (isdigit(ch) && k > NUMLENMAX){
			reportError(47);			//NUMBER 太大
			while (isdigit(ch)){
				getch();
			}
			digits = 0;
			//这里是直接把后面的数字舍弃，然后置num为0
		}
	}
	else if (ch == ':'){
		tokenbuf = ch;
		getch();
		if (ch == '='){
			tokenbuf += ch;
			sym = BECOMES;
			getch();
		}
		else sym = COLON;
	}
	else if (ch == '<'){
		tokenbuf = ch;
		getch();
		if (ch == '='){
			tokenbuf += ch;
			sym = LEQ;
			getch();
		}
		else if (ch == '>'){
			tokenbuf += ch;
			sym = NEQ;
			getch();
		}
		else
			sym = LSS;
	}
	else if (ch == '>'){
		tokenbuf = ch;
		getch();
		if (ch == '='){
			tokenbuf += ch;
			sym = GEQ;
			getch();
		}
		else
			sym = GTR;
	}
	else if (ch == '\''){
		tokenbuf = "";
		getch();
		if (islower(ch) || isupper(ch) || isdigit(ch)){
			tokenbuf += ch;
			getch();
			if (ch == '\''){
				sym = ZIFU;
				getch();
			}
			else{
				sym = NUL;
				reportError(49);
			}
		}
		else{
			sym = NUL;
			reportError(49);
		}
	}
	else if (ch == '\"'){
		tokenbuf = "";
		getch();
		while (ch == ' ' || ch == '!' || ('$' <= ch && ch <= '~')){
			tokenbuf += ch;
			getch();
		}
		if (ch == '\"'){
			sym = ZIFUC;
			getch();
		}
		else{
			sym = NUL;
			reportError(49);
		}
	}
	//else if (ch == '\/'){
	//	getch();
	//	if (ch == '\/'){	//滤去注释
	//		while (ch != '\n')
	//			getch();
	//		getch();
	//	}
	//	else
	//		getsymt('\/');
	//}
	else{
		tokenbuf = ch;
		getsymt(ch);
		getch();
	}
}

