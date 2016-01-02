#include "datastructure.h"
using namespace std;

extern SYMITEM STABLE[TABLENMAX];			// the symbol table
extern int BlockIndex[LVMAX];				// 分程序索引表
extern int BItop;							// 分程序索引表里的栈顶指针
extern IMC CODE[CODEASIZE];					// 中间代码数组
extern int lvl;								// current level



int blockDivide(int begin, int terminus){
	int i = begin;
	if (begin > terminus)
		return -1;
	while (i <= terminus){
		if ((BEQ <= CODE[i].instrT && CODE[i].instrT <= ELB) || CODE[i].instrT == CALL)
			return i;
		i++;
	}
	return terminus;
}

// 1.数组元素：
void optLocalExpr(int begin,int terminus){
	int k = begin;
	int sheet = blockDivide(begin, terminus);
	while (sheet >= 0){
		//消除局部公共子表达式
		k = sheet + 1;
		sheet = blockDivide(k, terminus);
	}
}