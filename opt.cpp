#include "datastructure.h"
using namespace std;

extern SYMITEM STABLE[TABLENMAX];			// the symbol table
extern int BlockIndex[LVMAX];				// �ֳ���������
extern int BItop;							// �ֳ������������ջ��ָ��
extern IMC CODE[CODEASIZE];					// �м��������
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

// 1.����Ԫ�أ�
void optLocalExpr(int begin,int terminus){
	int k = begin;
	int sheet = blockDivide(begin, terminus);
	while (sheet >= 0){
		//�����ֲ������ӱ��ʽ
		k = sheet + 1;
		sheet = blockDivide(k, terminus);
	}
}