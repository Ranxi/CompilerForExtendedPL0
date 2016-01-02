#include <cstdio>
#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#define DEBUG

#define NRSRVW		46			//保留字以及符号个数
#define TABLENMAX	1009
#define	NUMLENMAX	10
#define IDLENMAX	20
#define ADDRMAX		2047
#define LVMAX		3			//嵌套层次的最大限度
#define CODEASIZE	2000
#define COMMONSYMINDEX	13
#define RSVWBEGININDEX	21
#define RSVWENDINDEX	40
#define PARANUMMAX		10		//参数个数上限

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
	WRITESYM, ZIFU, ZIFUC, IDF,
	NUMBER, NUL
};
enum IDTYPE
{
	CHARCONST,	CHARVAR,	CHAREXP,	CHARREF,
	NUMCONST,	NUMVAR,		NUMEXP,		NUMREF,
	ARRAY, PROCEDURE, FUNCTION
};

enum INSTRTYPE
{
	ADD, SUB, MUL, DIV, INC, DEC, MNS, MOV, MOVA, LA, STEAX,
	BEQ, BNE, BGE, BGT, BLE, BLT, JMP, ELB,
	PARA, PARAQ, CALL, INI, RET, WRT, RED
};

typedef struct arrayinfo{
	int size;		//upper bound
	IDTYPE elementt;
	//目前就这些吧
}*ARRAYLINK;

typedef struct parainfo{
	std::string name;
	bool		isVar;
	IDTYPE		type;
};

typedef struct pfinfo{
	int paranum;
	int varsize;
	std::string addr;
	IDTYPE retvaluet;
	parainfo paras[PARANUMMAX];
	/*std::string paraname[PARANUMMAX];
	IDTYPE		paratype[PARANUMMAX];
	bool		paraisVar[PARANUMMAX];*/
}*FPLINK;

typedef struct item{
	std::string name;
	IDTYPE type;
	int level;
	union{
		int offset;
		int constv;
	};
	struct arrayinfo *alink;			//complement link for array
	struct pfinfo *plink;				//complement link for procedure and function
	int link;
}SYMITEM, *SYMITEMLINK;

typedef struct instr{
	INSTRTYPE instrT;
	std::string op1;
	std::string op2;
	std::string dest;
}IMC;



IDTYPE expression(int &tmpindex, std::string &opname, std::set<SYMTYPE> fsys);