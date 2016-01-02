#include <cstdio>
#include <ctype.h>
#include <string>
#include <iostream>
#include <fstream>

#define NRSRVW		46			//保留字以及符号个数
#define TABLENMAX	307
#define	NUMLENMAX	10
#define IDLENMAX	20
#define ADDRMAX		2047
#define LVMAX		3
#define CODEASIZE	200
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
	CHARCONST = 0, CHARVAR, NUMCONST, NUMVAR, ARRAY, PROCEDURE, FUNCTION, CHARPARA, NUMPARA
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
	int addr;
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
	int offset;
	struct arrayinfo *alink;			//complement link for array
	struct pfinfo *plink;				//complement link for procedure and function
	struct item *link;
}SYMITEM, *SYMITEMLINK;


void expression();

//#include <cstdio>
//#include <ctype.h>
//#include <string>
//#include <iostream>
//#include <fstream>
//
//#define NRSRVW		46			//保留字以及符号个数
//#define TABLENMAX	307
//#define	NUMLENMAX	10
//#define IDLENMAX	20
//#define ADDRMAX		2047
//#define LVMAX		3
//#define CODEASIZE	1000
//#define COMMONSYMINDEX	13
//#define RSVWBEGININDEX	21
//#define RSVWENDINDEX	40
//
//std::string RSRVWORD[NRSRVW];
//std::string RSRVWORDT[NRSRVW];
//std::string ENUMNAME[NRSRVW];
//char SSYMT[18];
//
//enum SYMTYPE
//{
//	DQUOSYM = 0, SQUOSYM, LPAREN, RPAREN,
//	TIMES, PLUS, COMMA, MINUS,
//	PERIOD, SLASH, SEMICOLON, EQL,
//	LSQBSYM, RSQBSYM, LSS, LEQ,
//	GTR, GEQ, BECOMES, COLON,
//	NEQ, ARRAYSYM, BEGINSYM, CHARSYM,
//	CONSTSYM, DOSYM, DOWNTOSYM, ELSESYM,
//	ENDSYM, FORSYM, FUNCSYM, IFSYM,
//	INTSYM, OFSYM, PROCSYM, READSYM,
//	THENSYM, TOSYM, VARSYM, WHILESYM,
//	WRITESYM, ZIFU, ZIFUC, IDF,
//	NUMBER, NUL
//};
//enum IDTYPE
//{
//	CHARCONST = 0, CHARVAR, NUMCONST, NUMVAR, ARRAY, PROCEDURE, FUNCTION, CHARPARA, NUMPARA
//};
//
//typedef struct arrayinfo{
//	int size;		//upper bound
//	IDTYPE elementt;
//	//目前就这些吧
//}*ARRAYLINK;
//
//typedef struct pfinfo{
//	int paranum;
//	int addr;
//	IDTYPE retvaluet;
//	std::string paraname[8];
//}*FPLINK;
//
//typedef struct item{
//	std::string name;
//	IDTYPE type;
//	int level;
//	int offset;
//	struct arrayinfo *alink;			//complement link for array
//	struct pfinfo *plink;				//complement link for procedure and function
//	struct item *link;
//}SYMITEM, *SYMITEMLINK;