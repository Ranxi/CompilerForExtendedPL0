#include <cstdio>
#include <ctype.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <map>

#define DEBUG
#define DAGOPT

#define NRSRVW		46			//保留字以及符号个数
#define TABLENMAX	1009		//符号表大小上限
#define	NUMLENMAX	10 			//数字长度上限
#define IDLENMAX	20			//标识符长度限制
#define LVMAX		6			//嵌套层次的最大限度
#define CODEASIZE	2000		//四元式数量上限
#define COMMONSYMINDEX	13
#define RSVWBEGININDEX	21
#define RSVWENDINDEX	40
#define PARANUMMAX		10		//参数个数上限
#define STRCONSTMAX		50		//字符串常量数量上限


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
	CHARCONST = 0,	CHARVAR,	CHAREXP,	CHARREF,
	NUMCONST,	NUMVAR,		NUMEXP,		NUMREF,
	ARRAY, PROCEDURE, FUNCTION
};

enum INSTRTYPE
{
	NOP = 0, ADD , SUB, MUL, DIV, INC, DEC, MNS, MOV, MOVA, LA, STEAX,
	BEQ, BNE, BGE, BGT, BLE, BLT, JMP, ELB,
	PARA, PARAQ, CALL, INI, RET, WRT, RED
};

enum GLOBALREG
{
	EBX = 0, EDI, ESI
};

typedef struct arrayinfo{
	int size;		//upper bound
	IDTYPE elementt;
}*ARRAYLINK;

struct parainfo{
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

typedef struct dnode{
	IMC Op;
	bool isleaf;
	std::string firsttmp;
	std::vector<int> fathers;
	std::vector<std::string> eqlVars;
}DagNode;


void closefiles();

void reportError(int errorcode);

void test(std::set<SYMTYPE> &s1, std::set<SYMTYPE> &s2, int errorcode);

void recover(std::set<SYMTYPE> &s);

void getsym();

void registe(SYMITEM &item);

int locate(std::string &name);

void genImc(INSTRTYPE iT, std::string op1, std::string op2, std::string dest);

void listImc();

void genAsm(int cxbg, int cxend);

bool exprTypecheck(IDTYPE dest, IDTYPE src);

void optLocalExpr(int begin, int terminus);

IDTYPE expression(int &tmpindex, std::string &opname, std::set<SYMTYPE> &fsys);