#include <cstdio>
#include <ctype.h>
#include <string>
#include <iostream>
#include <fstream>

#define NRSRVW		44			//保留字以及符号个数
#define TABLENMAX	100
#define	NUMLENMAX	14
#define IDLENMAX	20
#define ADDRMAX		2047
#define LVMAX		3
#define CODEASIZE	200
#define COMMONSYMINDEX	13
#define RSVWBEGININDEX	21
#define RSVWENDINDEX	40

std::string RSRVWORD[NRSRVW];
std::string RSRVWORDT[NRSRVW];
std::string ENUMNAME[NRSRVW];
char SSYMT[18];

enum SYMTYPE
{
	DQUOSYM = 0, SQUOSYM, LPAREN, RPAREN,
	TIMES, PLUS, COMMA, MINUS,
	PERIOD, SLASH, SEMICOLON, EQL,
	LSQBSYM, RSQBSYM, LSS, LEQ,
	GTR, GEQ, BECOMES, COLON,
	NEQ, ARRAYSYM, BEGINSYM, CHARSYM,
	CONSTSYM, DOSYM, DOWNTOSYM, ELSESYM,
	ENDSYM, FORSYM, FUNCSYM, IFSYM,
	INTSYM, OFSYM, PROCSYM, READSYM,
	THENSYM, TOSYM, VARSYM, WHILESYM,
	WRITESYM, IDF, NUMBER, NUL
};
enum IDTYPE
{
	CONSTANT = 0,VARIABLE,PROCEDURE,FUNCTION
};


std::ifstream  in;
char ch;
SYMTYPE sym;
std::string id;
int num;
int cc;
int lnum;
int ll;
int kk, err;
std::string line;
std::string a;


void error(int errorcode){
	printf("Error %d: line %d, column %d !\n",errorcode,lnum,cc);
	err++;
}

void getsymt(char ss){
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
	if (cc == ll){		//read the next line
		if (!getline(in,line)){
			printf("Program incomplete!");
			in.close();
			exit(0);
		}
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
		getch();						//跳过空白字符
	if (islower(ch)||isupper(ch)){		//可能是一个标识符或者保留字
		k = 0;
		a = "";
		while (islower(ch) || isupper(ch) || isdigit(ch)){
			a += ch;
			getch();
		}
		k = a.length();
		if (k > IDLENMAX){		//标识符过长
			printf("The identifier is too long!");
			in.close();
			exit(0);
		}
		id = a;
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
		a = "";
		k = 0;
		num = 0;
		sym = NUMBER;
		while (isdigit(ch) && k <= NUMLENMAX){
			a = ch;
			num = num * 10 + (ch - '0');
			k++;
			getch();
		}
		if (k > NUMLENMAX)
			error(30);
	}
	else if (ch == ':'){
		a = ch;
		getch();
		if (ch == '='){
			a += ch;
			sym = BECOMES;
			getch();
		}
		else sym = COLON;
	}
	else if (ch == '<'){
		a = ch;
		getch();
		if (ch == '='){
			a += ch;
			sym = LEQ;
			getch();
		}
		else if (ch == '>'){
			a += ch;
			sym = NEQ;
			getch();
		}
		else
			sym = LSS;
	}
	else if (ch == '>'){
		a = ch;
		getch();
		if (ch == '='){
			a += ch;
			sym = GEQ;
			getch();
		}
		else
			sym = GTR;
	}
	else{
		a = ch;
		getsymt(ch);
		getch();
	}
}

int main(){
	/*
	enum SYMTYPE
	{
		DQUOSYM = 0,SQUOSYM, LPAREN, RPAREN,
		TIMES,PLUS ,COMMA,  MINUS,
		PERIOD,SLASH,SEMICOLON,EQL,
		LSQBSYM,RSQBSYM, LSS, LEQ,
		GTR, GEQ, BECOMES,COLON,
		NEQ, BEGINSYM,ENDSYM, IFSYM,
		THENSYM, ELSESYM,INTSYM, CHARSYM,
		ARRAYSYM, OFSYM, WHILESYM,DOSYM,
		FORSYM, DOWNTOSYM, TOSYM, CONSTSYM,
		VARSYM, PROCSYM, FUNCSYM,READSYM,
		WRITESYM, IDF, NUMBER, NUL
	};
	enum IDTYPE
	{
		CONSTANT = 0, VARIABLE, PROCEDURE, FUNCTION
	};*/
	int i = 1;

	//initialize
	RSRVWORD[0] = '\"';			RSRVWORD[1] = '\'';			RSRVWORD[2] = '(';			RSRVWORD[3] = ')';
	RSRVWORD[4] = '*';			RSRVWORD[5] = '+';			RSRVWORD[6] = ',';			RSRVWORD[7] = '-';
	RSRVWORD[8] = '.';			RSRVWORD[9] = '/';			RSRVWORD[10] = ';';			RSRVWORD[11] = '=';
	RSRVWORD[12] = '[';			RSRVWORD[13] = ']';			RSRVWORD[14] = "<";			RSRVWORD[15] = "<=";
	RSRVWORD[16] = ">";			RSRVWORD[17] = ">=";		RSRVWORD[18] = ":=";		RSRVWORD[19] = ":";
	RSRVWORD[20] = "<>";		RSRVWORD[21] = "array";		RSRVWORD[22] = "begin";		RSRVWORD[23] = "char";
	RSRVWORD[24] = "const";		RSRVWORD[25] = "do";		RSRVWORD[26] = "downto";	RSRVWORD[27] = "else";
	RSRVWORD[28] = "end";		RSRVWORD[29] = "for";		RSRVWORD[30] = "function";	RSRVWORD[31] = "if";
	RSRVWORD[32] = "integer";	RSRVWORD[33] = "of";		RSRVWORD[34] = "procedure";	RSRVWORD[35] = "read";
	RSRVWORD[36] = "then";		RSRVWORD[37] = "to";		RSRVWORD[38] = "var";		RSRVWORD[39] = "while";
	RSRVWORD[40] = "write";		RSRVWORD[41] = "IDF";		RSRVWORD[42] = "NUMBER";	RSRVWORD[43] = "NUL";

	RSRVWORDT[0] = "DQUOSYM";	RSRVWORDT[1] = "SQUOSYM";	RSRVWORDT[2] = "LPAREN";	RSRVWORDT[3] = "RPAREN";
	RSRVWORDT[4] = "TIMES";		RSRVWORDT[5] = "PLUS";		RSRVWORDT[6] = "COMMA";		RSRVWORDT[7] = "MINUS";
	RSRVWORDT[8] = "PERIOD";	RSRVWORDT[9] = "SLASH";		RSRVWORDT[10] = "SEMICOLON"; RSRVWORDT[11] = "EQL";
	RSRVWORDT[12] = "LSQBSYM";	RSRVWORDT[13] = "RSQBSYM";	RSRVWORDT[14] = "LSS";		RSRVWORDT[15] = "LEQ";
	RSRVWORDT[16] = "GTR";		RSRVWORDT[17] = "GEQ";		RSRVWORDT[18] = "BECOMES";	RSRVWORDT[19] = "COLON";
	RSRVWORDT[20] = "NEQ";		RSRVWORDT[21] = "ARRAYSYM";	RSRVWORDT[22] = "BEGINSYM";	RSRVWORDT[23] = "CHARSYM";
	RSRVWORDT[24] = "CONSTSYM";	RSRVWORDT[25] = "DOSYM";	RSRVWORDT[26] = "DOWNTOSYM"; RSRVWORDT[27] = "ELSESYM";
	RSRVWORDT[28] = "ENDSYM";	RSRVWORDT[29] = "FORSYM";	RSRVWORDT[30] = "FUNCSYM";	RSRVWORDT[31] = "IFSYM";
	RSRVWORDT[32] = "INTSYM";	RSRVWORDT[33] = "OFSYM";	RSRVWORDT[34] = "PROCSYM";	RSRVWORDT[35] = "READSYM";
	RSRVWORDT[36] = "THENSYM";	RSRVWORDT[37] = "TOSYM";	RSRVWORDT[38] = "VARSYM";	RSRVWORDT[39] = "WHILESYM";
	RSRVWORDT[40] = "WRITESYM";	RSRVWORDT[41] = "IDF";		RSRVWORDT[42] = "NUMBER";	RSRVWORDT[43] = "NUL";

	SSYMT[0] = '\"';		SSYMT[1] = '\'';	SSYMT[2] = '(';		SSYMT[3] = ')';
	SSYMT[4] = '*';			SSYMT[5] = '+';		SSYMT[6] = ',';		SSYMT[7] = '-';
	SSYMT[8] = '.';			SSYMT[9] = '/';		SSYMT[10] = ';';	SSYMT[11] = '=';
	SSYMT[12] = '[';		SSYMT[13] = ']';
	
	/*for (i = 1; i <= NRSRVW; i++){
		ENUMNAME[i] = (SYMTYPE)i;
	}*/

	char filename[20];
	printf("Please input the source program file name : ");
	scanf("%s", filename);
	in.open(filename,std::ios::in);
	i = 1;
	ch = ' ';
	getsym();
	printf("%d ", i);
	std::cout <<  RSRVWORDT[(int)sym] + ' ' + a << std::endl;
	while (sym != PERIOD){
		i++;
		getsym();
		printf("%d ", i);
		std::cout << RSRVWORDT[(int)sym] + ' '+ a << std::endl;
	}
	in.close();
	return 0;
}
