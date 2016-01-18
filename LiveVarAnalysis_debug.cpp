#include "datastructure.h"
using namespace std;
#define BLOCKNUMUPPER		256

extern SYMITEM STABLE[TABLENMAX];			// the symbol table
extern IMC CODE[CODEASIZE];					// 中间代码数组
int blockNo;								// 基本块标号从 1 开始
int blocktail[BLOCKNUMUPPER];				// 此数组用来记录基本块末尾跳转指令的位置
int Successor[BLOCKNUMUPPER][2];			// 存储基本块的后继基本块号，至多有两个后继基本块
bool evolved;
map<string, int> LabelTable;				// 标签列表

set<string> useB[BLOCKNUMUPPER];
set<string> defB[BLOCKNUMUPPER];
set<string> inB[BLOCKNUMUPPER];
set<string> outB[BLOCKNUMUPPER];


// 判断是否可以加入第 i 个基本块的 use[B]
void judgeUse(int i, string &name){
	if (name.substr(0, 2) == "t_" || name.substr(0, 2) == "C_")			// 不考虑为临时变量分配全局寄存器
		return;		//满满的歧视
	if (defB[blockNo].count(name) <= 0)
		useB[blockNo].insert(name);
}
// 判断是否可以加入第 i 个基本块的 def[B]
void judgeDef(int i, string &name){
	if (name.substr(0, 2) == "t_" || name.substr(0, 2)=="C_")			// 不考虑为临时变量分配全局寄存器
		return;		//满满的歧视
	if (useB[blockNo].count(name) <= 0)
		defB[blockNo].insert(name);
}

//	本函数的作用：
//	1.划分基本块, CALL 指令不考虑为基本块结束标志
//	2.准备好各基本块的 use[B] 与 def[B]
//	3.记录下各个标签所在的基本块编号
int blockDivide2(int begin, int terminus){
	int i = begin;
	if (begin > terminus)
		return -1;
	if (blockNo >= BLOCKNUMUPPER){
		printf("Sorry, Too many basic blocks, you have to adjust the upper limit!");
		exit(0);
	}
	while (CODE[i].instrT == ELB || CODE[i].instrT == NOP){			//此处没有加 i <= terminus 的条件是因为程序块最后一条一定是RET
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
			//讨论第一个操作数
			judgeUse(i, CODE[i].op1);
			//讨论第二个操作数
			judgeUse(i, CODE[i].op2);
			//讨论第三个操作数
			judgeDef(i, CODE[i].dest);
			break;
		case INC:
		case DEC:
			//讨论第一个操作数
			judgeUse(i, CODE[i].op1);
			// a = a + 1 这种句子肯定是先使用再定义
			break;
		case MNS:
		case MOV:
			//讨论第一个操作数
			judgeUse(i, CODE[i].op1);
			//讨论第三个操作数
			judgeDef(i, CODE[i].dest);
			break;
		case MOVA:
			//讨论第一个操作数
			judgeUse(i, CODE[i].op1);
			//讨论第二个操作数
			judgeUse(i, CODE[i].op2);
			break;
		case LA:
			//第一个操作数为数组
			//讨论第二个操作数
			judgeUse(i, CODE[i].op2);
			//讨论第三个操作数
			judgeDef(i, CODE[i].dest);
			break;
		case STEAX:		//依赖条件：目标操作数只可能是临时变量
			break;
		
		/************************基本块结尾判断**************************/
		case BNE:
		case BEQ:
		case BGE:
		case BGT:
		case BLE:
		case BLT:
			//讨论第一个操作数
			judgeUse(i, CODE[i].op1);
			//讨论第二个操作数
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
				blocktail[blockNo] = i - 1;		// ELB 前面的语句可能是任意计算类型语句
				blockNo++;
			}
			while (CODE[i].instrT == ELB || CODE[i].instrT == NOP){			//此处没有加 i <= terminus 的条件是因为程序块最后一条一定是RET
				if (CODE[i].instrT == ELB)
					LabelTable[CODE[i].dest] = blockNo;
				i++;
			}
			i--;
			LabelTable[CODE[i].dest] = blockNo;
			break;

		/************************基本块结尾判断**************************/

		case PARA:
		case PARAQ:
			//讨论第一个操作数
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
			if (CODE[i].op1 == "")			//讨论第二个操作数
				judgeUse(i, CODE[i].op2);
			break;
		case RED:
			//讨论第三个操作数
			judgeDef(i, CODE[i].dest);
			break;
		default:		//INI, 但是如果不出意外，这里不会出现INI
			break;
		}
		i++;
	}
	blocktail[blockNo] = i;
	return terminus;			//基本块的最后一条语句肯定是RET
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
			//分程序内最后一个基本块，后继为Bexit
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
	for (i = blockNo ; i > 0; i--){		// 依赖条件：最后一个基本块的最后一条语句一定是RET，因此无后继
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
	//划分基本块，计算 use[B] 与 def[B]
	blockNo = 1;
	blockDivide2(begin, terminus);
	blockNo--;
	//填好所有基本块的后继基本块编号
	setSuccessor();
	//计算 out[B] 与 in[B]
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
	//统计需要分配全局寄存器的变量

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
	// 画冲突图
	// 图着色算法分配寄存器
}