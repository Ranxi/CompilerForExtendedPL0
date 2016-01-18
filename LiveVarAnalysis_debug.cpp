#include "datastructure.h"
using namespace std;
#define BLOCKNUMUPPER		256

extern SYMITEM STABLE[TABLENMAX];			// the symbol table
extern IMC CODE[CODEASIZE];					// �м��������
int blockNo;								// �������Ŵ� 1 ��ʼ
int blocktail[BLOCKNUMUPPER];				// ������������¼������ĩβ��תָ���λ��
int Successor[BLOCKNUMUPPER][2];			// �洢������ĺ�̻�����ţ�������������̻�����
bool evolved;
map<string, int> LabelTable;				// ��ǩ�б�

set<string> useB[BLOCKNUMUPPER];
set<string> defB[BLOCKNUMUPPER];
set<string> inB[BLOCKNUMUPPER];
set<string> outB[BLOCKNUMUPPER];


// �ж��Ƿ���Լ���� i ��������� use[B]
void judgeUse(int i, string &name){
	if (name.substr(0, 2) == "t_" || name.substr(0, 2) == "C_")			// ������Ϊ��ʱ��������ȫ�ּĴ���
		return;		//����������
	if (defB[blockNo].count(name) <= 0)
		useB[blockNo].insert(name);
}
// �ж��Ƿ���Լ���� i ��������� def[B]
void judgeDef(int i, string &name){
	if (name.substr(0, 2) == "t_" || name.substr(0, 2)=="C_")			// ������Ϊ��ʱ��������ȫ�ּĴ���
		return;		//����������
	if (useB[blockNo].count(name) <= 0)
		defB[blockNo].insert(name);
}

//	�����������ã�
//	1.���ֻ�����, CALL ָ�����Ϊ�����������־
//	2.׼���ø�������� use[B] �� def[B]
//	3.��¼�¸�����ǩ���ڵĻ�������
int blockDivide2(int begin, int terminus){
	int i = begin;
	if (begin > terminus)
		return -1;
	if (blockNo >= BLOCKNUMUPPER){
		printf("Sorry, Too many basic blocks, you have to adjust the upper limit!");
		exit(0);
	}
	while (CODE[i].instrT == ELB || CODE[i].instrT == NOP){			//�˴�û�м� i <= terminus ����������Ϊ��������һ��һ����RET
		if (CODE[i].instrT == ELB)
			LabelTable[CODE[i].dest] = blockNo;
		i++;
	}
	while (i <= terminus){
		switch (CODE[i].instrT)
		{
		case NOP:
			break;
		case ADD:
		case SUB:
		case MUL:
		case DIV:
			//���۵�һ��������
			judgeUse(i, CODE[i].op1);
			//���۵ڶ���������
			judgeUse(i, CODE[i].op2);
			//���۵�����������
			judgeDef(i, CODE[i].dest);
			break;
		case INC:
		case DEC:
			//���۵�һ��������
			judgeUse(i, CODE[i].op1);
			// a = a + 1 ���־��ӿ϶�����ʹ���ٶ���
			break;
		case MNS:
		case MOV:
			//���۵�һ��������
			judgeUse(i, CODE[i].op1);
			//���۵�����������
			judgeDef(i, CODE[i].dest);
			break;
		case MOVA:
			//���۵�һ��������
			judgeUse(i, CODE[i].op1);
			//���۵ڶ���������
			judgeUse(i, CODE[i].op2);
			break;
		case LA:
			//��һ��������Ϊ����
			//���۵ڶ���������
			judgeUse(i, CODE[i].op2);
			//���۵�����������
			judgeDef(i, CODE[i].dest);
			break;
		case STEAX:		//����������Ŀ�������ֻ��������ʱ����
			break;
		
		/************************�������β�ж�**************************/
		case BNE:
		case BEQ:
		case BGE:
		case BGT:
		case BLE:
		case BLT:
			//���۵�һ��������
			judgeUse(i, CODE[i].op1);
			//���۵ڶ���������
			judgeUse(i, CODE[i].op2);
			blocktail[blockNo++] = i;
			break;
		case JMP:
			blocktail[blockNo++] = i;
			break;
		case ELB:
			if (i > begin &&((BEQ <= CODE[i - 1].instrT && CODE[i-1].instrT <= JMP) || CODE[i-1].instrT == CALL)){
			}
			else{
				blocktail[blockNo] = i - 1;		// ELB ǰ�������������������������
				blockNo++;
			}
			while (CODE[i].instrT == ELB || CODE[i].instrT == NOP){			//�˴�û�м� i <= terminus ����������Ϊ��������һ��һ����RET
				if (CODE[i].instrT == ELB)
					LabelTable[CODE[i].dest] = blockNo;
				i++;
			}
			i--;
			LabelTable[CODE[i].dest] = blockNo;
			break;

		/************************�������β�ж�**************************/

		case PARA:
		case PARAQ:
			//���۵�һ��������
			judgeUse(i, CODE[i].op1);
			break;
		case CALL:
			//..........
			blocktail[blockNo++] = i;
			break;
		case RET:
			blocktail[blockNo++] = i;
			return i;
		case WRT:
			if (CODE[i].op1 == "")			//���۵ڶ���������
				judgeUse(i, CODE[i].op2);
			break;
		case RED:
			//���۵�����������
			judgeDef(i, CODE[i].dest);
			break;
		default:		//INI, ��������������⣬���ﲻ�����INI
			break;
		}
		i++;
	}
	blocktail[blockNo] = i;
	return terminus;			//����������һ�����϶���RET
}

void setSuccessor(){
	int i ;
	int ptr;
	for (i = 1; i <= blockNo; i++){
		ptr = blocktail[i];
		if (BEQ <= CODE[ptr].instrT && CODE[ptr].instrT <= BLT){
			Successor[i][0] = i + 1;
			if (LabelTable.find(CODE[ptr].dest) != LabelTable.end())
				Successor[i][1] = LabelTable.find(CODE[ptr].dest)->second;
			else
				printf("Jump destination is not founded!");
		}
		else if (CODE[ptr].instrT == JMP){
			if (LabelTable.find(CODE[ptr].dest) != LabelTable.end())
				Successor[i][0] = LabelTable.find(CODE[ptr].dest)->second; 
			else
				printf("Jump destination is not founded!");
		}
		else if (CODE[ptr].instrT == RET){
			Successor[i][0] = 0;
			//�ֳ��������һ�������飬���ΪBexit
		}
		else
			Successor[i][0] = i + 1;
	}
}

void computeOutIn(){
	evolved = false;
	int i;
	set<string> tmpIn ;
	set<string>::iterator defit;
	for (i = blockNo ; i > 0; i--){		// �������������һ������������һ�����һ����RET������޺��
		outB[i].insert(inB[Successor[i][0]].begin(), inB[Successor[i][0]].end());
		if (Successor[i][0] > 0)
			outB[i].insert(inB[Successor[i][1]].begin(), inB[Successor[i][1]].end());
		tmpIn.clear();
		tmpIn.insert(useB[i].begin(), useB[i].end());
		if (defB[i].size() > 0){
			defit = defB[i].begin();
			while (defit != defB[i].end()){
				if (outB[i].count(*defit) > 0)
					outB[i].erase(*defit);		// out[B] - def[B]
				defit++;
			}
		}
			
		tmpIn.insert(outB[i].begin(), outB[i].end());
		if (tmpIn != inB[i]){
			evolved = true;
			inB[i].insert(tmpIn.begin(),tmpIn.end());
		}
	}
}

void globalRegAlloc(int begin, int terminus){
	//���ֻ����飬���� use[B] �� def[B]
	blockNo = 1;
	blockDivide2(begin, terminus);
	blockNo--;
	//������л�����ĺ�̻�������
	setSuccessor();
	//���� out[B] �� in[B]
	while (true){
		computeOutIn();
		if (evolved == false)
			break;
	}
	int i;
	set<string>::iterator init;
	for (i = 1; i <= blockNo; i++){
		init = outB[i].begin();
		std::cout << "\tBlock " << i << " out : ";
		while (init != outB[i].end()){
			std::cout << *init << " ; ";
			init++;
		}
		defB[i].clear();
		outB[i].clear();
		std::cout << endl;
	}
	for (i = 1; i <= blockNo; i++){
		init = inB[i].begin();
		std::cout << "\tBlock " << i << " in : ";
		while (init != inB[i].end()){
			std::cout << *init << " ; ";
			init++;
		}
		useB[i].clear();
		inB[i].clear();
		std::cout << endl;
	}
	LabelTable.clear();
	//ͳ����Ҫ����ȫ�ּĴ����ı���

	/*map<string ,int > acrossBlkVar;
	set<string>::iterator it;
	for(i = 1; i <= blockNo; i++){
		it = inB[i].begin();
		while (it != inB[i].end()){
			if (acrossBlkVar.find(*it) == acrossBlkVar.end())
				acrossBlkVar[*it] = 1;
			else
				acrossBlkVar[*it]++;
		}
	}
	vector<string> canditates;
	map<string, int>::iterator abvIt = acrossBlkVar.begin();
	while (abvIt != acrossBlkVar.end()){
		if (abvIt->second > 1){
			canditates.push_back(abvIt->first);
		}
		abvIt++;
	}
	int m = canditates.size();*/
	// ����ͻͼ
	// ͼ��ɫ�㷨����Ĵ���
}