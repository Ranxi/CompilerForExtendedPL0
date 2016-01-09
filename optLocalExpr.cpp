#include "datastructure.h"
using namespace std;
#define DAGSIZE 300

extern SYMITEM STABLE[TABLENMAX];			// the symbol table
extern IMC CODE[CODEASIZE];					// 中间代码数组

DagNode Dag[DAGSIZE];						// DAG结构
map<std::string, int> NodeTable;			// 结点表
int ntabtop;

// 划分基本块，此处考虑将 CALL 指令作为基本块的结束标志
int blockDivide(int begin, int terminus){
	int i = begin;
	if (begin > terminus)
		return -1;
	while (i <= terminus){
		if ((BEQ <= CODE[i].instrT && CODE[i].instrT <= ELB) || CODE[i].instrT == CALL || CODE[i].instrT == INI )
			return i;
		i++;
	}
	return terminus;			//基本块的最后一条语句肯定是RET
}

// 搜索结点的第一个临时变量，为替换相同的临时变量做准备
void searchFirstTmp(int index){
	//如果结点本身就是临时变量，那么在新建结点的时候就已经初始化了firsttmp
	if (Dag[index].firsttmp != "")
		return;					//  ""还没有搜索过
	else {
		Dag[index].firsttmp = "_";			//代表已经搜索过
		vector<string>::iterator it = Dag[index].eqlVars.begin();
		while (it != Dag[index].eqlVars.end()){
			if ((*it).substr(0, 2) == "t_"){
				Dag[index].firsttmp = (*it);
				break;
			}
			it++;
		}
	}
}


// 此处依赖条件：临时变量没有跨基本块使用
// 但是在for循环中由于终止表达式有可能存储在临时变量中，因此这一情形破坏了上述条件
// 采取的解决办法是终止表达式的结果如果需要存储在临时变量中
// 那么该临时变量将以"T_"开头而不是"t_"开头，因此处理时并不会把这个变量当做临时变量
void replaceSrc(bool isNo1){
	int tmpNo;
	string *operand;
	if (isNo1)
		operand = &Dag[ntabtop].Op.op1;
	else
		operand = &Dag[ntabtop].Op.op2;
	if (operand->substr(0, 2) == "t_"){
		tmpNo = NodeTable.find(*operand)->second;		//此处依赖条件：临时变量没有跨基本块使用
		if (!Dag[tmpNo].isleaf){
			searchFirstTmp(tmpNo);
			if (Dag[tmpNo].firsttmp != "" && Dag[tmpNo].firsttmp != "_"){
				NodeTable.erase(*operand);				//此处依赖条件：临时变量只会使用一次，使用完即扔
				*operand = Dag[tmpNo].firsttmp;			//替换源操作数（用一个临时变量换另外一个临时变量）
			}
		}
	}
}

// 判断临时变量是否已经被用过了，如果还没有被用过说明该临时变量可能会跨基本块使用（也有可能已经被用过但是被等价的临时变量替换了）
// 所以目前的算法还不完善，但能保证是安全的。
// 当然还有其他办法，如登记每个临时变量，在建DAG的过程中对被使用的临时变量作标记
bool isused(string &name){
	vector<int>::iterator fathers = Dag[ntabtop].fathers.begin();
	while (fathers != Dag[ntabtop].fathers.end()){
		if (Dag[*fathers].Op.op1 == name || Dag[*fathers].Op.op2 == name)
			return true;
		fathers++;
	}
	return false;
}

// 处理基本块出口处可能活跃的临时变量（也就是还没有用过的临时变量）
void handleActiveTmp(int &ptr, vector<string>::iterator &itEql){
	itEql = Dag[ntabtop].eqlVars.begin();
	while (itEql != Dag[ntabtop].eqlVars.end()){
		if ((*itEql).substr(0, 2) == "t_"){
			if (NodeTable.find(*itEql) != NodeTable.end() &&!isused(*itEql)){
				// 如果 NodeTable中找不到该临时变量了，说明已经被用过了
				//isused(*itEql);
				CODE[ptr].instrT = MOV;
				CODE[ptr].op1 = Dag[ntabtop].Op.dest;
				CODE[ptr].op2 = "";
				CODE[ptr].dest = (*itEql);
				string tmp = (*itEql);
				itEql = Dag[ntabtop].eqlVars.erase(itEql);
				ptr--;
			}
			else
				itEql++;
		}
		else
			itEql++;
	}
}

// 对于等价的临时变量，用节点中的第一个临时变量（firsttmp）代替他们
void genEqlImc(int &ptr, vector<string>::iterator &itEql){
	bool hasprinttmp = false;
	if (Dag[ntabtop].Op.dest.substr(0, 2) == "t_")
		hasprinttmp = true;
	itEql = Dag[ntabtop].eqlVars.begin();
	while (itEql != Dag[ntabtop].eqlVars.end()){
		if ((*itEql).substr(0, 2) == "t_"){
			if (!hasprinttmp){
				CODE[ptr].instrT = MOV;
				CODE[ptr].op1 = Dag[ntabtop].Op.dest;
				CODE[ptr].op2 = "";
				CODE[ptr].dest = (*itEql);
				ptr--;
				hasprinttmp = true;
			}
			//除了第一个临时变量外其他的临时变量都舍弃
		}
		else{
			CODE[ptr].instrT = MOV;
			CODE[ptr].op1 = Dag[ntabtop].Op.dest;
			CODE[ptr].op2 = "";
			CODE[ptr].dest = (*itEql);
			ptr--;
		}
		itEql++;
	}
}

// 从DAG图中导出代码
void exportImc(int b, int t){
	int ptr = t;
	vector<string>::iterator itEql;
	//导出代码（顺序是从后往前导出）
	ntabtop--;				//ntabtop一直都是先+后使用
	int topbackup = ntabtop;
	while (ntabtop >= 0){
		//这里的依赖条件是当前DAG中结点编号最大的一定是没有父节点且应该此时导出的代码
		if (Dag[ntabtop].isleaf){
			ntabtop--;
			continue;
		}
		switch (Dag[ntabtop].Op.instrT)
		{
		case NOP:
			handleActiveTmp(ptr, itEql);
			genEqlImc(ptr, itEql);		//此处考虑 MOV 5, var 的情况
			ptr++;			//由于这里无需打印本结点的指令，所以ptr无需减一
			break;
		case ADD:
		case SUB:
		case MUL:
		case DIV:
			handleActiveTmp(ptr, itEql);
			//讨论第一个操作数
			replaceSrc(true);
			//讨论第二个操作数
			replaceSrc(false);
			//输出目标操作数为等价结点的MOV指令，这里如果临时变量多于一个，则全部都使用找到的第一个临时变量
			genEqlImc(ptr, itEql);
			//再输出本结点的代码
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case INC:
		case DEC:
		case MNS:
			handleActiveTmp(ptr, itEql);
			genEqlImc(ptr, itEql);
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case MOV:
			handleActiveTmp(ptr, itEql);
			replaceSrc(true);
			genEqlImc(ptr, itEql);
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case MOVA:
			handleActiveTmp(ptr, itEql);
			replaceSrc(true);
			replaceSrc(false);
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case LA:
			//handleActiveTmp(ptr, itEql);	//注释原因见楼下
			replaceSrc(false);			//此处只需考虑第二个操作数，即数组下标是否是临时变量，是否需要替换即可
			//genEqlImc(ptr, itEql);	//此处注释原因：目标操作数必定是临时变量，因此如果有等价结点，完全由本结点代替即可。
			//且由于这里的等价结点都是REF型变量，MOV指令会将值存到由临时变量指向的地址中而不是赋给该临时变量
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case STEAX:
			handleActiveTmp(ptr, itEql);
			genEqlImc(ptr, itEql);
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case BNE:
		case BEQ:
		case BGE:
		case BGT:
		case BLE:
		case BLT:
			handleActiveTmp(ptr, itEql);
			replaceSrc(true);
			replaceSrc(false);
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case PARA:
		case PARAQ:
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case WRT:
			replaceSrc(false);
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		case RED:
			handleActiveTmp(ptr, itEql);
			genEqlImc(ptr, itEql);
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		default:
			CODE[ptr] = Dag[ntabtop].Op;
			break;
		}
		if (ptr < b){
			printf("BASIC BLOCK is spilled!\n");
			return;
		}
		ptr--;
		ntabtop--;
	}
	ntabtop = 0;
	if (ptr >= b){
		while (ptr >= b){
			CODE[ptr].instrT = NOP;
			CODE[ptr].dest = "";
			CODE[ptr].op1 = "";
			CODE[ptr].op2 = "";
			ptr--;
		}
	}
	//清空DAG图
	while (topbackup >= 0){
		Dag[topbackup].Op.instrT = NOP;
		Dag[topbackup].eqlVars.clear();
		Dag[topbackup].fathers.clear();
		Dag[topbackup].isleaf = true;
		Dag[topbackup].firsttmp = "";
		topbackup--;
	}
	NodeTable.clear();
}

void createLeaf(string &name){
	Dag[ntabtop].Op.instrT = NOP;
	Dag[ntabtop].Op.dest = name;
	Dag[ntabtop].isleaf = true;
	NodeTable[name] = ntabtop ;
}

// 1.数组元素：MOVA, 将数组结点更新; LA 可以继续加进图中
// 2.写REF类变量：导出，重建
// 3.为了降低导出、重建的频率，将INC,DEC,WRT,RED,PARA,PARAQ指令当做叶结点加入图中
// 4.当发现公共表达式，如果修改后的结点编号比当前变量的结点编号小，那么就新建一个MOV结点，并修改结点表，使得导出顺序能遵循原有四元式顺序
//   也就是使得同一变量的编号只能递增，不可递减，防止变量的值出现错误
void optLocalExpr(int begin,int terminus){
	ntabtop = 0;
	bool bothexist;			//当前的所有操作数是否已经存在DAG中
	bool findeql;			//标记是否找到等价的结点
	int k = begin;
	int i = 0, p = k;
	int idest;
	int nodeNo1, nodeNo2, nodeNo3;	//操作数的结点编号
	int sheet = blockDivide(begin, terminus);
	while (sheet >= 0){
		//消除局部公共子表达式
		p = k;
		i = k;
		while (i <= sheet){
			IMC *curinstr = &CODE[i];
			findeql = false;
			bothexist = true;
			switch (curinstr->instrT)
			{
			case ADD:
			case SUB:
			case MUL:
			case DIV:
				idest = locate(curinstr->dest);
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//导出DAG图，并从头构建
					exportImc(p,i-1);
					p = i;
					//break;		导出后可重新从该四元式开始构建DAG
				}
				//讨论第一个源操作数
				if (NodeTable.find(curinstr->op1) != NodeTable.end()){
					bothexist &= true;
					nodeNo1 = NodeTable.find(curinstr->op1)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op1);
					nodeNo1 = ntabtop;
					ntabtop++;
				}
				//讨论第二个源操作数
				if (NodeTable.find(curinstr->op2) != NodeTable.end()){
					bothexist &= true;
					nodeNo2 = NodeTable.find(curinstr->op2)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op2);
					nodeNo2 = ntabtop;
					ntabtop++;
				}
				//搜索是否有相同的结点
				if (bothexist){
					vector<int>::iterator it1 = Dag[nodeNo1].fathers.begin();
					vector<int>::iterator it2 = Dag[nodeNo2].fathers.begin();
					while (it1 != Dag[nodeNo1].fathers.end()){
						it2 = Dag[nodeNo2].fathers.begin();
						while (it2 != Dag[nodeNo2].fathers.end()){
							if (*it1 == *it2 && Dag[*it2].Op.instrT == curinstr->instrT){
								if (NodeTable.find(Dag[*it2].Op.op1)->second != nodeNo1 || NodeTable.find(Dag[*it2].Op.op2)->second != nodeNo2)
									break;
								if (curinstr->dest.substr(0, 2) != "t_"){
									//1.保持结点编号递增	2.防止提前将变量赋新值，因为原先的结点可能有表达式关联
									Dag[ntabtop].Op.instrT = MOV;
									Dag[ntabtop].Op.op1 = Dag[*it2].Op.dest;		//此处还可优化
									Dag[ntabtop].Op.dest = curinstr->dest;
									Dag[ntabtop].isleaf = false;
									Dag[ntabtop].firsttmp = "";
									NodeTable[curinstr->dest] = ntabtop;
									Dag[nodeNo1].fathers.push_back(ntabtop);
									ntabtop++;
								}
								else{
									//依赖条件：临时变量只会赋值一次，即其结点编号固定
									Dag[*it2].eqlVars.push_back(curinstr->dest);
									NodeTable[curinstr->dest] = *it2;
								}
								findeql = true;
								break;
							}
							it2++;
						}
						if (findeql)
							break;
						it1++;
					}
				}
				if (!findeql){			//建新的中间结点
					Dag[ntabtop].Op = *curinstr;
					Dag[ntabtop].isleaf = false;
					if (curinstr->dest.substr(0, 2) == "t_")
						Dag[ntabtop].firsttmp = curinstr->dest;
					else
						Dag[ntabtop].firsttmp = "";
					NodeTable[curinstr->dest] = ntabtop;
					Dag[nodeNo1].fathers.push_back(ntabtop);
					Dag[nodeNo2].fathers.push_back(ntabtop);
					ntabtop++;
				}
				break;
			case INC:
			case DEC:
				idest = locate(curinstr->dest);
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//导出DAG图，并从头构建
					exportImc(p, i - 1);
					p = i;
				}
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				NodeTable[curinstr->dest] = ntabtop;
				ntabtop++;
				break;
			case MNS:
				idest = locate(curinstr->dest);
				//i1 = locate(curinstr->op1);
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//导出DAG图，并从头构建
					exportImc(p, i - 1);
					p = i;
				}
				if (NodeTable.find(curinstr->op1) != NodeTable.end()){
					bothexist &= true;
					nodeNo1 = NodeTable.find(curinstr->op1)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op1);
					nodeNo1 = ntabtop;
					ntabtop++;
				}
				if (bothexist){
					vector<int>::iterator it1 = Dag[nodeNo1].fathers.begin();
					while (it1 != Dag[nodeNo1].fathers.end()){
						if (Dag[*it1].Op.instrT == MNS){
							if (curinstr->dest.substr(0, 2) != "t_"){
								//1.保持结点编号递增	2.防止提前将变量赋新值，因为原先的结点可能有表达式关联
								Dag[ntabtop].Op.instrT = MOV;
								Dag[ntabtop].Op.op1 = Dag[*it1].Op.dest;		//此处还可优化
								Dag[ntabtop].Op.dest = curinstr->dest;
								Dag[ntabtop].isleaf = false;
								Dag[ntabtop].firsttmp = "";
								NodeTable[curinstr->dest] = ntabtop;
								Dag[nodeNo1].fathers.push_back(ntabtop);
								ntabtop++;
							}
							else{
								//依赖条件：临时变量只会赋值一次，即其结点编号固定
								Dag[*it1].eqlVars.push_back(curinstr->dest);
								NodeTable[curinstr->dest] = *it1;
							}
							findeql = true;
							break;
						}
						it1++;
					}
				}
				if(!findeql){
					Dag[ntabtop].Op = *curinstr;
					Dag[ntabtop].isleaf = false;
					if (curinstr->dest.substr(0, 2) == "t_")
						Dag[ntabtop].firsttmp = curinstr->dest;
					else
						Dag[ntabtop].firsttmp = "";
					NodeTable[curinstr->dest] = ntabtop;
					Dag[nodeNo1].fathers.push_back(ntabtop);
					ntabtop++;
				}
				break;
			case MOV:			//此处有依赖条件：
				idest = locate(curinstr->dest);
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//导出DAG图，并从头构建
					exportImc(p, i - 1);
					p = i;
				}
				//讨论第一个源操作数
				if (NodeTable.find(curinstr->op1) != NodeTable.end()){
					bothexist &= true;
					nodeNo1 = NodeTable.find(curinstr->op1)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op1);
					nodeNo1 = ntabtop;
					ntabtop++;
				}
				if (curinstr->dest.substr(0, 2) != "t_"){
					//1.保持结点编号递增	2.防止提前将变量赋新值，因为原先的结点可能有表达式关联
					Dag[ntabtop].Op = *curinstr;
					Dag[ntabtop].isleaf = false;
					Dag[ntabtop].firsttmp = "";
					NodeTable[curinstr->dest] = ntabtop;
					Dag[nodeNo1].fathers.push_back(ntabtop);
					ntabtop++;
				}
				else{
					//依赖条件：临时变量只会赋值一次，即其结点编号固定
					if (Dag[nodeNo1].isleaf==true){
						Dag[nodeNo1].Op.instrT = NOP;
						Dag[nodeNo1].isleaf = false;
					}
					NodeTable[curinstr->dest] = nodeNo1;
					Dag[nodeNo1].eqlVars.push_back(curinstr->dest);
				}
				break;
			case MOVA:
				//讨论第一个源操作数
				if (NodeTable.find(curinstr->op1) != NodeTable.end()){
					bothexist &= true;
					nodeNo1 = NodeTable.find(curinstr->op1)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op1);
					nodeNo1 = ntabtop;
					ntabtop++;
				}
				//讨论第二个源操作数
				if (NodeTable.find(curinstr->op2) != NodeTable.end()){
					bothexist &= true;
					nodeNo2 = NodeTable.find(curinstr->op2)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op2);
					nodeNo2 = ntabtop;
					ntabtop++;
				}
				//此操作有三个源操作数，第三个操作即目标数组
				if (NodeTable.find(curinstr->dest) != NodeTable.end()){
					bothexist &= true;
					nodeNo3 = NodeTable.find(curinstr->dest)->second;
				}
				else{//新建一个叶节点————数组
					bothexist = false;
					createLeaf(curinstr->dest);
					nodeNo3 = ntabtop;
					ntabtop++;
				}
				//因为此操作必然会改变数组，所以无需寻找是否有此类型的公共表达式，直接建新结点
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				NodeTable[curinstr->dest] = ntabtop;
				Dag[nodeNo1].fathers.push_back(ntabtop);
				Dag[nodeNo2].fathers.push_back(ntabtop);
				Dag[nodeNo3].fathers.push_back(ntabtop);
				ntabtop++;
				break;
			case LA:		//此处的依赖条件是目标操作数肯定是临时变量，且是REF型临时变量
				//讨论第一个源操作数
				if (NodeTable.find(curinstr->op1) != NodeTable.end()){
					bothexist &= true;
					nodeNo1 = NodeTable.find(curinstr->op1)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op1);
					nodeNo1 = ntabtop;
					ntabtop++;
				}
				//讨论第二个源操作数
				if (NodeTable.find(curinstr->op2) != NodeTable.end()){
					bothexist &= true;
					nodeNo2 = NodeTable.find(curinstr->op2)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op2);
					nodeNo2 = ntabtop;
					ntabtop++;
				}
				//搜索是否有相同的结点
				if (bothexist){
					vector<int>::iterator it1 = Dag[nodeNo1].fathers.begin();
					vector<int>::iterator it2 = Dag[nodeNo2].fathers.begin();
					while (it1 != Dag[nodeNo1].fathers.end()){
						it2 = Dag[nodeNo2].fathers.begin();
						while (it2 != Dag[nodeNo2].fathers.end()){
							if (*it1 == *it2 && Dag[*it2].Op.instrT == curinstr->instrT){
								//因为此处有依赖条件，目标操作数必定是新的临时变量，所以可以不考虑结点编号变小的问题
								Dag[*it2].eqlVars.push_back(curinstr->dest);
								NodeTable[curinstr->dest] = *it2;
								findeql = true;
								break;
							}
							it2++;
						}
						if (findeql)
							break;
						it1++;
					}
				}
				if (!findeql){			//建新的中间结点
					Dag[ntabtop].Op = *curinstr;
					Dag[ntabtop].isleaf = false;
					Dag[ntabtop].firsttmp = curinstr->dest;		//依赖条件：目标操作数必定是新的临时变量
					NodeTable[curinstr->dest] = ntabtop;
					Dag[nodeNo1].fathers.push_back(ntabtop);
					Dag[nodeNo2].fathers.push_back(ntabtop);
					ntabtop++;
				}
				break;
			case STEAX:
				idest = locate(curinstr->dest);
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//导出DAG图，并从头构建
					exportImc(p, i - 1);
					p = i;
				}
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				NodeTable[curinstr->dest] = ntabtop;
				ntabtop++;
				break;
			case BNE:
			case BEQ:
			case BGE:
			case BGT:
			case BLE:
			case BLT:
				if (NodeTable.find(curinstr->op1) == NodeTable.end()){
					createLeaf(curinstr->op1);
					ntabtop++;
				}
				if (NodeTable.find(curinstr->op2) == NodeTable.end()){
					createLeaf(curinstr->op2);
					ntabtop++;
				}
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				ntabtop++;
				break;
			case PARA:
			case PARAQ:
				if (NodeTable.find(curinstr->op1) != NodeTable.end()){
					bothexist &= true;
					nodeNo1 = NodeTable.find(curinstr->op1)->second;
					Dag[nodeNo1].fathers.push_back(ntabtop);
				}
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				//NodeTable[curinstr->dest] = ntabtop;
				ntabtop++;
				break;
			case WRT:
				if (curinstr->op2 == ""){
					Dag[ntabtop].Op = *curinstr;
					Dag[ntabtop].isleaf = false;
					ntabtop++;
					break;
				}
				if (NodeTable.find(curinstr->op2) != NodeTable.end()){
					bothexist &= true;
					nodeNo2 = NodeTable.find(curinstr->op2)->second;
				}
				else{//新建一个叶节点
					bothexist = false;
					createLeaf(curinstr->op2);
					nodeNo2 = ntabtop;
					ntabtop++;
				}
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				Dag[nodeNo2].fathers.push_back(ntabtop);
				ntabtop++;
				break;
			case RED:
				idest = locate(curinstr->dest);
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//导出DAG图，并从头构建
					exportImc(p, i - 1);
					p = i;
				}
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				NodeTable[curinstr->dest] = ntabtop;
				ntabtop++;
				break;
			default:		//JMP, ELB, CALL, INI, RET, 
				Dag[ntabtop].Op = *curinstr;
				Dag[ntabtop].isleaf = false;
				ntabtop++;
				break;
			}
			i++;
		}
		exportImc(p, sheet);		//基本块的最后一条语句不可覆盖
		//更新基本块起始位置
		k = sheet + 1;
		//更新基本块结束位置
		sheet = blockDivide(k, terminus);
	}
}