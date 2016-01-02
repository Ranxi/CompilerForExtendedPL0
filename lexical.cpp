#include "datastructure.h"

using namespace std;

ifstream  in;
ofstream  out;
ofstream  outasm;
char ch;							// a char in the getch() function
SYMTYPE sym;						// the type of the last token
string id;							// the last identifer read by far
int digits;							// the last number read by far
int cc;								// pointer in line
int lnum;							// No. of line
int ll;								// length of a line
int kk, err;
int tp;							// pointer in symbol table
int lvl;							// current level
int curOffset;						// 本过程当前的变量偏移
int cx;								// point out where the next instruction will be stored
string line;						// the line buffer in getch() function
string tokenbuf;					// the token buffer in getsym() function
SYMITEM STABLE[TABLENMAX];			// the symbol table
int hashtable[TABLENMAX];			// the hash table for symbol table
int BlockIndex[LVMAX];				// 分程序索引表
int BItop;							// 分程序索引表里的栈顶指针
IMC CODE[CODEASIZE];				// 中间代码数组
int labelNo;						// LABEL 编号

set<SYMTYPE>  DECLARAFIRST;
set<SYMTYPE>  STMTFIRST;
set<SYMTYPE>  FACTORFIRST;
set<SYMTYPE>  RELATIONALOP;


void reportError(int errorcode){
	
	string errorinfo;
	//abc = 1;
	switch (errorcode) {
		case 11:
				errorinfo = "\"" + id + "\"" +"is not defined!"; break;
		case 1:
				errorinfo = "\"" + id + "\"" + "is defined duplicately!"; break;
		case 2:
				errorinfo = "不可向常量或过程、函数赋值";break;
		case 3:
				errorinfo = "常量定义中等式右边出现 [+|-]时应该接数字";break;
		case 4:
				errorinfo = "常量定义只能是无符号整数或者字符";break;
		case 5:
				errorinfo = "常量定义中应该使用\"=\"";break;
		case 6:
				errorinfo = "常量或变量定义中const、var或者\",\"后面应该是标识符";break;
		case 7:
				errorinfo = "数组元素类型不是基本类型";break;
		case 8:
				errorinfo = "数组声明过程中缺少of";break;
		case 9:
				errorinfo = "数组定义或使用缺少\"[\"或\"]\"";break;
		case 10:
				errorinfo = "数组声明过程中定义大小必须用无符号整数";break;
		case 12:
				errorinfo = "变量类型未指定";break;
		case 13:
				errorinfo = "变量声明没有以冒号结尾";break;
		case 14:
				errorinfo = "变量声明过程中指定变量类型时缺少\":\"";break;
		case 15:
				errorinfo = "参数类型只能是integer或者char";break;
		case 16:
				errorinfo = "参数声明中没有指定参数名字";break;
		case 17:
				errorinfo = "参数个数超过上限";break;
		case 18:
				errorinfo = "函数定义中返回值类型不是基本类型";break;
		case 19:
				errorinfo = "没有定义函数的返回值类型";break;
		case 20:
				errorinfo = "函数或过程声明出现错误";break;
		case 21:
				errorinfo = "此处缺少\")\"";break;
		case 22:
				errorinfo = "过程调用语句不能用作表达式中的因子";break;
		case 23:
				errorinfo = "赋值语句缺少\":=\"";break;
		case 24:
				errorinfo = "无法识别的关系运算符";break;
		case 25:
				errorinfo = "过程调用缺少参数";break;
		case 26:
				errorinfo = "不可向常量或过程、函数赋值";break;
		case 27:
				errorinfo = "if语句缺少then";break;
		case 28:
				errorinfo = "语句之间缺少\";\"";break;
		case 29:
				errorinfo = "此处应为分号或end";break;
		case 30:
				errorinfo = "此立即数太大";break;
		case 31:
				errorinfo = "for 后面应该有循环变量";break;
		case 32:
				errorinfo = "循环变量类型不正确";break;
		case 33:
				errorinfo = "for 循环语句中缺少 to 或者downto";break;
		case 34:
				errorinfo = "for 循环语句中缺少 do";break;
		case 35:
				errorinfo = "表达式不能以此符号开始"; break;
		case 36:
				errorinfo = "因子后不能接此符号"; break;
		case 37:
				errorinfo = "过程说明后的符号不正确";break;
		case 38:
				errorinfo = "此处应为语句";break;
		case 39:
				errorinfo = "语句后的符号不正确";break;
		case 40:
				errorinfo = "嵌套层次太深，本宝宝受不了啦";break;
		case 41:
				errorinfo = "参数个数或类型错误";break;
		case 42:
				errorinfo = "赋值语句等号两边类型不匹配";break;
		case 43:
				errorinfo = "数组下标类型只能是整型";break;
		case 44:
				errorinfo = "条件表达式中比较运算符两边的类型不正确";break;
		default:
			errorinfo = "Other Error !";
	}
	cout << "Error "<< errorcode << " : " << errorinfo << endl;
	cout << "\t" <<  "line " << lnum << ", column " << cc << " !" << endl;
	err++;
	out.close();
}

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
			fprintf(stdout, "Program incomplete!");
			in.close();
			exit(0);
		}
		else if (sym == PERIOD){
			return;
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
			in.close();
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
		if ( isdigit(ch) && k > NUMLENMAX){
			reportError(30);			//NUMBER 太大
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
			else
				sym = NUL;
		}
		else
			sym = NUL;
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
		else
			sym = NUL;
	}
	else{
		tokenbuf = ch;
		getsymt(ch);
		getch();
	}
}




//hash表，便于查询符号表
int Hash(string name){
	int unsigned t;
	int len = name.length();
	int j = 0;
	t = 0;
	for (j = 0; j < len; j++){
		t = (t << 5) | (t >> 27);
		t += (int)name.at(j);
	}
	t %= TABLENMAX;
	return t;
}

//登记标识符进符号表
void registe(SYMITEM &item){
	tp++;
	if (tp >= TABLENMAX){
		printf("符号表已满，被迫终止编译程序");
		in.close();
		exit(0);
	}
	STABLE[tp] = item;					//bit copy
	int index = Hash(item.name);
	if (hashtable[index] != 0){
		STABLE[tp].link = hashtable[index];
		hashtable[index] = tp;
	}
	else
		hashtable[index] = tp;
}

//查询符号表
int locate(string name){
	int index = Hash(name);
	int q = hashtable[index];
	if (hashtable[index] == 0)
		return -1;
	else{
		SYMITEMLINK anitem = &STABLE[hashtable[index]];
		while(anitem->name != name && anitem->link >= 0){
		// 这里还是有问题，如果只是名字相同，但实际上不是同一个东西。因此不支持根据类型重载变量名
			q = anitem->link;
			anitem = &STABLE[q];
		}
		if (anitem->name == name)
			return q;
		else
			return -1;
		//return hashtable[index];
	}
}
//此函数用于在编译完一个分程序后，删除这个子程序在符号表上的相关记录
void popstable(){
	int lasttop = BlockIndex[BItop] - 1;
	while (tp > lasttop){
		if (STABLE[tp].link != NULL){
			hashtable[Hash(STABLE[tp].name)] = STABLE[tp].link;
		}
		tp--;
	}
	BItop--;
	lvl--;
}

//typedef struct instr{
//	INSTRTYPE instrT;
//	std::string op1;
//	std::string op2;
//	std::string dest;
//}IMC;

void genImc(INSTRTYPE iT, string op1, string op2, string dest){
	CODE[cx].instrT = iT;
	CODE[cx].op1 = op1;
	CODE[cx].op2 = op2;
	CODE[cx].dest = dest;
	cx++;
}

void listImc(){
	int p = 0;
	INSTRTYPE iT;
	while (p < cx){
		iT = CODE[p].instrT;
		switch (iT){
			case ADD:{
				out << "\t\tADD\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case SUB:{
				out << "\t\tSUB\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case MUL:{
				out << "\t\tMUL\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case DIV:{
				out << "\t\tDIV\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case INC:{
				out << "\t\tINC\t" << CODE[p].op1 << endl;
				break;
			}
			case DEC:{
				out << "\t\tDEC\t" << CODE[p].op1 << endl;
				break;
			}
			case MNS:{
				out << "\t\tMNS\t" << CODE[p].op1 << ", " << CODE[p].dest << endl;
				break;
			}
			case MOV:{
				out << "\t\tMOV\t" << CODE[p].op1 << ", " << CODE[p].dest << endl;
				break;
			}
			case MOVA:{
				out << "\t\tMOVA\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case LA:{
				out << "\t\tLA\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case STEAX:{
				out << "\t\tSTEAX\t" << "EAX, " << CODE[p].dest << endl;
				break;
			}
			case BEQ:{
				out << "\t\tBEQ\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BNE:{
				out << "\t\tBNE\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BGE:{
				out << "\t\tBGE\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BGT:{
				out << "\t\tBGT\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BLE:{
				out << "\t\tBLE\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BLT:{
				out << "\t\tBLT\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case JMP:{
				out << "\t\tJMP\t" << CODE[p].dest << endl;
				break;
			}
			case ELB:{
				out << CODE[p].dest << ":" << endl;
				break;
			}
			case PARA:{
				out << "\t\tPARA\t" << CODE[p].op1 << endl;
				break;
			}
			case PARAQ:{
				out << "\t\tPARAQ\t" << CODE[p].op1 << endl;
				break;
			}
			case CALL:{
				out << "\t\tCALL\t" << CODE[p].dest << endl;
				break;
			}
			case INI : {
				out << "\t\tINI\t" << CODE[p].dest << endl;
				break;
			}
			case RET:{
				out << "\t\tRET\n" << endl;
				break;
			}
			case WRT:{
				if(CODE[p].op1 == "")
					out << "\t\tWRT\t" << CODE[p].op2 << endl;
				else
					out << "\t\tWRT\t" << CODE[p].op1 << endl;
				break;
			}
			case RED:{
				out << "\t\tRED\t" << CODE[p].dest << endl;
				break;
			}
			default:
				break;
		}
		p++;
	}
}

void loadexternVar(int i){
	//将外层变量的所在层地址加载到eax
	int lvldist = lvl - STABLE[i].level;		// 引用了外层变量
	outasm << "\tmov\teax,[ebp+8]" << endl;		// SL位于EBP下方第二个单元
	while (lvldist > 0){
		outasm << "\tmov\teax,[eax]" << endl;
		lvldist--;
	}
}

void cmpIncondition(int i1,int i2){
	//先取得操作数1的数值, 放入edx
	if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
		outasm << "\tmov\tedx," << STABLE[i1].constv << endl;
	}
	else{
		if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
			if (STABLE[i1].level == lvl){
				outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
			}
			else{
				loadexternVar(i1);//将外层变量的所在层地址加载到eax
				outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
			}
		}
		else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
			if (STABLE[i1].level == lvl){
				outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
			}
			else{
				loadexternVar(i1);
				outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
			}
			outasm << "\tmov\tedx,[eax]" << endl;
		}
	}
	//与操作数2进行比较
	if (STABLE[i2].type == CHARCONST || STABLE[i2].type == NUMCONST){
		outasm << "\tcmp\tedx," << STABLE[i2].constv << endl;
	}
	else{
		if (STABLE[i2].type == CHARVAR || STABLE[i2].type == NUMVAR){
			if (STABLE[i2].level == lvl){
				outasm << "\tcmp\tedx,[ebp-" << STABLE[i2].offset << "]" << endl;
			}
			else{
				loadexternVar(i2);//将外层变量的所在层地址加载到eax
				outasm << "\tcmp\tedx,[eax-8-" << STABLE[i2].offset << "]" << endl;
			}
		}
		else{//(STABLE[i2].type == CHARREF || STABLE[i2].type == NUMREF)
			if (STABLE[i2].level == lvl){
				outasm << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
			}
			else{
				loadexternVar(i2);
				outasm << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
			}
			outasm << "\tcmp\tedx,[eax]" << endl;
		}
	}
}

void genAsm(int cxbg,int cxend){
	int p = cxbg;
	int i1, i2, idest;
	INSTRTYPE iT;
	while (p < cxend){
		iT = CODE[p].instrT;
		switch (iT){
			case MOV: {		//genImc(MOV,opname,"",STABLE[i].name);
				//
				i1 = locate(CODE[p].op1);
				idest = locate(CODE[p].dest);
				if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
					if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){		//目标为引用变量
						//hold,此处还需考虑全局变量（也就是lvl0的变量）
						//这里将引用变量所指的地址加载到eax
						if (STABLE[idest].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
						}
						else{
							loadexternVar(idest);
							outasm << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
						}
						//对目标变量进行赋值
						outasm << "\tmov\tdword ptr [eax]," << STABLE[i1].constv << endl;
					}
					else{	//目标为直接变量, 下面直接对目标变量进行赋值
						if (STABLE[idest].level == lvl){
							outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << "," << STABLE[i1].constv << endl;
						}
						else{
							loadexternVar(idest);
							outasm << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],"<< STABLE[i1].constv << endl;
						}
					}
				}
				else{
					//先取出源数值, 放入edx
					if(STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\tedx,[eax]" << endl;
					}

					if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
						//hold,此处还应考虑全局变量（也就是lvl0的变量）
						if (STABLE[idest].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
						}
						else{
							loadexternVar(idest);
							outasm << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
						}
						outasm << "\tmov\tdword ptr [eax],edx" << endl;
					}
					else{
						if (STABLE[idest].level == lvl){
							outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",edx" << endl;
						}
						else{
							loadexternVar(idest);
							outasm << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
						}
					}
				}
				break;
			}
			case ADD: {		//genImc(ADD,op1,opname,atemp.name);
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				idest = locate(CODE[p].dest);
				//先取得操作数1的数值, 放入edx
				if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
					outasm << "\tmov\tedx," << STABLE[i1].constv << endl;
				}
				else{
					if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\tedx,[eax]" << endl;
					}
				}
				//将edx加上操作数2
				if (STABLE[i2].type == CHARCONST || STABLE[i2].type == NUMCONST){
					outasm << "\tadd\tedx," << STABLE[i2].constv << endl;
				}
				else{
					if (STABLE[i2].type == CHARVAR || STABLE[i2].type == NUMVAR){
						if (STABLE[i2].level == lvl){
							outasm << "\tadd\tedx,[ebp-" << STABLE[i2].offset << "]" << endl;
						}
						else{
							loadexternVar(i2);//将外层变量的所在层地址加载到eax
							outasm << "\tadd\tedx,[eax-8-" << STABLE[i2].offset << "]" << endl;
						}
					}
					else{//(STABLE[i2].type == CHARREF || STABLE[i2].type == NUMREF)
						if (STABLE[i2].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
						}
						else{
							loadexternVar(i2);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
						}
						outasm << "\tadd\tedx,[eax]" << endl;
					}
				}

				//把edx的值赋给目标变量
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//hold,此处还应考虑全局变量（也就是lvl0的变量）
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
					}
					outasm << "\tmov\tdword ptr [eax],edx" << endl;
				}
				else{
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",edx" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
					}
				}
				break;
			}
			case SUB: {		//genImc(SUB, op1, opname, atemp.name);
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				idest = locate(CODE[p].dest);
				//先取得操作数1的数值, 放入edx
				if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
					outasm << "\tmov\tedx," << STABLE[i1].constv << endl;
				}
				else{
					if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\tedx,[eax]" << endl;
					}
				}
				//将edx减去操作数2
				if (STABLE[i2].type == CHARCONST || STABLE[i2].type == NUMCONST){
					outasm << "\tsub\tedx," << STABLE[i2].constv << endl;
				}
				else{
					if (STABLE[i2].type == CHARVAR || STABLE[i2].type == NUMVAR){
						if (STABLE[i2].level == lvl){
							outasm << "\tsub\tedx,[ebp-" << STABLE[i2].offset << "]" << endl;
						}
						else{
							loadexternVar(i2);//将外层变量的所在层地址加载到eax
							outasm << "\tsub\tedx,[eax-8-" << STABLE[i2].offset << "]" << endl;
						}
					}
					else{//(STABLE[i2].type == CHARREF || STABLE[i2].type == NUMREF)
						if (STABLE[i2].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
						}
						else{
							loadexternVar(i2);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
						}
						outasm << "\tsub\tedx,[eax]" << endl;
					}
				}

				//把edx的值赋给目标变量
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//hold,此处还应考虑全局变量（也就是lvl0的变量）
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
					}
					outasm << "\tmov\tdword ptr [eax],edx" << endl;
				}
				else{
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",edx" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
					}
				}
				break;
			}
			case MUL: {		//genImc(MUL, op1, opname, atemp.name);
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				idest = locate(CODE[p].dest);
				bool hasConst = false;
				if(STABLE[i1].type==CHARCONST||STABLE[i1].type==NUMCONST){
					int tmp = i2;
					i2 = i1;
					i1 = tmp;
					hasConst = true;
				}
				//把乘法结果放到edx
				if(STABLE[i1].type==CHARCONST||STABLE[i1].type==NUMCONST){		//说明两个操作数都是常量 
					outasm << "\tmov\tedx," << STABLE[i1].constv << endl;
					outasm << "\timul\tedx,edx," << STABLE[i2].constv << endl;	//edx中放着乘法结果
				}
				else{//先把操作数1加载到edx
					if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\tedx,[eax]" << endl;
					}
					//现在edx里面放着操作数1
					if(hasConst)		//操作数2是常数
						outasm << "\timul\tedx,edx," << STABLE[i2].constv << endl;
					else{				//操作数2是变量或引用
						if (STABLE[i2].type == CHARVAR || STABLE[i2].type == NUMVAR){
							if (STABLE[i2].level == lvl){
								outasm << "\timul\tedx,[ebp-" << STABLE[i2].offset << "]" << endl;
							}
							else{
								loadexternVar(i2);//将外层变量的所在层地址加载到eax
								outasm << "\timul\tedx,[eax-8-" << STABLE[i2].offset << "]" << endl;
							}
						}
						else{//(STABLE[i2].type == CHARREF || STABLE[i2].type == NUMREF)
							if (STABLE[i2].level == lvl){
								outasm << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
							}
							else{
								loadexternVar(i2);
								outasm << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
							}
							outasm << "\timul\tedx,[eax]" << endl;
						}
					}
					//现在edx里放着乘法结果
				}
				//把edx的值赋给目标变量
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//hold,此处还应考虑全局变量（也就是lvl0的变量）
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
					}
					outasm << "\tmov\tdword ptr [eax],edx" << endl;
				}
				else{
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",edx" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
					}
				}
				break;
			}
			case DIV: 
			{
						  //genImc(DIV, op1, opname, atemp.name);
			//idiv指令完成整数除法操作，idiv只有一个操作数，此操作数为除数，而被除数则为EDX:EAX中的内容（一个64位的整数）,
			//操作的结果有两部分：商和余数，其中商放在eax寄存器中，而余数则放在edx寄存器中。
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				idest = locate(CODE[p].dest);
				//先把操作数1放入eax,因为被除数显然是32位的，所以只需使用eax即可
				if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
					outasm << "\tmov\teax," << STABLE[i1].constv << endl;
				}
				else{
					if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\teax,[eax]" << endl;
					}
				}
				outasm << "\txor\tedx,edx" << endl;		//此处将edx置0
				//接下来是除法操作
				if (STABLE[i2].type == CHARCONST || STABLE[i2].type == NUMCONST){
					outasm << "\tmov\tecx," << STABLE[i2].constv << endl;
					outasm << "\tidiv\tecx"<< endl;
				}
				else{
					if (STABLE[i2].type == CHARVAR || STABLE[i2].type == NUMVAR){
						if (STABLE[i2].level == lvl){
							outasm << "\tidiv\t[ebp-" << STABLE[i2].offset << "]" << endl;
						}
						else{
							int lvldist = lvl - STABLE[i2].level;		// 引用了外层变量
							outasm << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP上方一个单位
							while (lvldist > 0){
								outasm << "\tmov\tecx,[ecx]" << endl;
								lvldist--;
							}
							outasm << "\tidiv\t[ecx-8-" << STABLE[i2].offset << "]" << endl;
						}
					}
					else{//(STABLE[i2].type == CHARREF || STABLE[i2].type == NUMREF)
						if (STABLE[i2].level == lvl){
							outasm << "\tmov\tecx,[ebp-" << STABLE[i2].offset << "]" << endl;
						}
						else{
							int lvldist = lvl - STABLE[i2].level;		// 引用了外层变量
							outasm << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP上方一个单位
							while (lvldist > 0){
								outasm << "\tmov\tecx,[ecx]" << endl;
								lvldist--;
							}
							outasm << "\tmov\tecx,[ecx-8-" << STABLE[i2].offset << "]" << endl;
						}
						outasm << "\tidiv\t[ecx]" << endl;
					}
				}

				//把eax的值(商)赋给目标变量
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//hold,此处还应考虑全局变量（也就是lvl0的变量）
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\tecx,[ebp-" << STABLE[idest].offset << "]" << endl;
					}
					else{
						int lvldist = lvl - STABLE[idest].level;		// 引用了外层变量
						outasm << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP上方一个单位
						while (lvldist > 0){
							outasm << "\tmov\tecx,[ecx]" << endl;
							lvldist--;
						}
						outasm << "\tmov\tecx,[ecx-8-" << STABLE[idest].offset << "]" << endl;
					}
					outasm << "\tmov\tdword ptr [ecx],eax" << endl;
				}
				else{
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",eax" << endl;
					}
					else{
						int lvldist = lvl - STABLE[idest].level;		// 引用了外层变量
						outasm << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP上方一个单位
						while (lvldist > 0){
							outasm << "\tmov\tecx,[ecx]" << endl;
							lvldist--;
						}
						outasm << "\tmov\tdword ptr [ecx-8-" << STABLE[idest].offset << "],eax" << endl;
					}
				}
				break;
			}
			case MNS: {		//genImc(MNS,op1,"",opname);
				i1 = locate(CODE[p].op1);
				idest = locate(CODE[p].dest);
				//将操作数1加载到edx
				if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
					outasm << "\tmov\tedx," << STABLE[i1].constv << endl;
				}
				else{
					if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\tedx,[eax]" << endl;
					}
				}
				outasm << "\tneg\tedx" << endl;
				//把edx的值赋给目标变量
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//hold,此处还应考虑全局变量（也就是lvl0的变量）
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
					}
					outasm << "\tmov\tdword ptr [eax],edx" << endl;
				}
				else{
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",edx" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
					}
				}
				break;
			}
			case MOVA: {		//genImc(MOVA, opname, indexResult, STABLE[i].name);
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				idest = locate(CODE[p].dest);
				//将操作数1加载到edx
				if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
					outasm << "\tmov\tedx," << STABLE[i1].constv << endl;
				}
				else{
					if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\tedx,[eax]" << endl;
					}
				}
				//将数组下标加载到ecx
				if(STABLE[i2].type == NUMCONST)
					outasm << "\tmov\tecx," << STABLE[i2].constv << endl;
				else if(STABLE[i2].type == NUMREF){
					if (STABLE[i2].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
					}
					else{
						loadexternVar(i2);
						outasm << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
					}
					outasm << "\tmov\tecx,[eax]" << endl;
				}
				else{
					if (STABLE[i2].level == lvl){
						outasm << "\tmov\tecx,[ebp-" << STABLE[i2].offset << "]" << endl;
					}
					else{
						loadexternVar(i2);//将外层变量的所在层地址加载到eax
						outasm << "\tmov\tecx,[eax-8-" << STABLE[i2].offset << "]" << endl;
					}
				}
				//将edx存储到数组里
				if (STABLE[idest].level == lvl){
					outasm << "\tlea\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
				}
				else{
					loadexternVar(idest);//将外层变量的所在层地址加载到eax
					outasm << "\tlea\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
				}
				outasm << "\tlea\teax,[eax-4*ecx]" << endl;		//计算地址
				outasm << "\tmov\tdword ptr [eax],edx" << endl;
				break;
			}
			case LA: {		//genImc(LA,STABLE[i].name,indexResult,opname);
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				idest = locate(CODE[p].dest);
				//将数组开始地址放在edx
				if (STABLE[i1].level == lvl){
					outasm << "\tlea\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
				}
				else{
					loadexternVar(i1);//将外层变量的所在层地址加载到eax
					outasm << "\tlea\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
				}
				//将数组下标加载到ecx
				if(STABLE[i2].type == NUMCONST)
					outasm << "\tmov\tecx," << STABLE[i2].constv << endl;
				else if(STABLE[i2].type == NUMREF){
					if (STABLE[i2].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
					}
					else{
						loadexternVar(i2);
						outasm << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
					}
					outasm << "\tmov\tecx,[eax]" << endl;
				}
				else{
					if (STABLE[i2].level == lvl){
						outasm << "\tmov\tecx,[ebp-" << STABLE[i2].offset << "]" << endl;
					}
					else{
						loadexternVar(i2);//将外层变量的所在层地址加载到eax
						outasm << "\tmov\tecx,[eax-8-" << STABLE[i2].offset << "]" << endl;
					}
				}
				//取出数组对应位置的数值
				outasm << "\tlea\tedx,[edx-4*ecx]" << endl;		//计算地址
				outasm << "\tmov\tedx,[edx]" << endl;
				//把edx的值赋给目标变量
				if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
					//hold,此处还应考虑全局变量（也就是lvl0的变量）
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
					}
					outasm << "\tmov\tdword ptr [eax],edx" << endl;
				}
				else{
					if (STABLE[idest].level == lvl){
						outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",edx" << endl;
					}
					else{
						loadexternVar(idest);
						outasm << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
					}
				}
				break;
			}
			case STEAX:{		//genImc(STEAX,"","",opname);
				idest = locate(CODE[p].dest);		//只会是同一层的临时变量，charvar 或者 numvar
				outasm << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << ",eax" << endl;
			}
			case PARA: {		//genImc(PARA,opname,"","");
				i1 = locate(CODE[p].op1);
				//将操作数1加载到edx
				if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
					outasm << "\tmov\tedx," << STABLE[i1].constv << endl;
				}
				else{
					if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);//将外层变量的所在层地址加载到eax
							outasm << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
					}
					else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
						if (STABLE[i1].level == lvl){
							outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
						}
						else{
							loadexternVar(i1);
							outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
						}
						outasm << "\tmov\tedx,[eax]" << endl;
					}
				}
				outasm << "\tpush\tedx" << endl;
				break;
			}
			case PARAQ: {		//genImc(PARAQ,opname,"","");
				i1 = locate(CODE[p].op1);
				//将操作数1加载到edx
				if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
					if (STABLE[i1].level == lvl){
						outasm << "\tlea\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
					}
					else{
						loadexternVar(i1);//将外层变量的所在层地址加载到eax
						outasm << "\tlea\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
					}
				}
				else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
					if (STABLE[i1].level == lvl){
						outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
					}
					else{
						loadexternVar(i1);
						outasm << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
					}
					outasm << "\tmov\tedx,eax" << endl;
				}
				outasm << "\tpush\tedx" << endl;
				break;
			}
			case CALL: {	//genImc(CALL,"","",STABLE[fptr].name);
				idest = locate(CODE[p].dest);
				int paraoffset;
				int lvldist = lvl - STABLE[idest].level;
				if(lvl==0 && lvldist==0){
					outasm << "\tmov\teax,ebp" << endl;
					outasm << "\tpush\teax" << endl;			//SL
				}
				else if(lvldist == 0){		//调用嵌套的子程序
					outasm << "\tlea\teax,[ebp+8]" << endl;
					outasm << "\tpush\teax" << endl;			//SL
				}
				//else if(lvldist == 1){	//调用与当前模块同层的程序
				//	outasm << "\tmov\teax,[ebp+8]" << endl;
				//	outasm << "\tpush\teax" << endl;			//SL
				//}
				else{
					outasm << "\tmov\teax,[ebp+8]" << endl;
					lvldist--;
					while(lvldist > 0){
						outasm << "\tmov\teax,[eax]" << endl;
						lvldist--;
					}
					outasm << "\tpush\teax" << endl;			//SL
				}
				outasm << "\tcall\t" << STABLE[idest].plink->addr <<endl;
				paraoffset = STABLE[idest].plink->paranum + 1;	//SL
				paraoffset <<= 2;
				outasm << "\tadd\tesp," << paraoffset<< endl;
				break;
			}
			case INI : {	//genImc(INI,"","",STABLE[btp].name);
				idest = locate(CODE[p].dest);
				outasm << "\tpush\tebp" << endl;
				outasm << "\tmov\tebp,esp" << endl;
				outasm << "\tsub\tesp," << STABLE[idest].plink->varsize << endl;
				outasm << "\tpush\tebx" << endl;
				outasm << "\tpush\tesi" << endl;
				outasm << "\tpush\tedi" << endl;
			}
			case RET : {		//genImc(RET, "", "", "");
				if(BItop > 0){
					i1 = locate(STABLE[BlockIndex[BItop - 1]].name + "_retv");
					if(i1 > 0){
						outasm << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
					}
				}
				outasm << "\tpop\tedi" << endl;
				outasm << "\tpop\tesi" << endl;
				outasm << "\tpop\tebx" << endl;
				outasm << "\tmov\tesp,ebp" << endl;
				outasm << "\tpop\tebp" << endl;
				outasm << "\tret" << endl;
				break;
			}
			case BEQ: {		//genImc(BEQ, op1, opname, LABEL);
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				cmpIncondition(i1,i2);
				outasm << "\tje\t" << CODE[p].dest << endl;
				break;
			}
			case BNE: {		//genImc(BNE, op1, opname, "");
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				cmpIncondition(i1,i2);
				outasm << "\tjne\t" << CODE[p].dest << endl;
				break;
			}
			case BGE: {		//genImc(BGE, op1, opname, "");
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				cmpIncondition(i1,i2);
				outasm << "\tjge\t" << CODE[p].dest << endl;
				break;
			}
			case BGT: {		//genImc(BGT, op1, opname, "");
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				cmpIncondition(i1,i2);
				outasm << "\tjg\t" << CODE[p].dest << endl;
				break;
			}
			case BLE: {		//genImc(BLE, op1, opname, "");
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				cmpIncondition(i1,i2);
				outasm << "\tjle\t" << CODE[p].dest << endl;
				break;
		
			}
			case BLT: {		//genImc(BLT, op1, opname, "");
				i1 = locate(CODE[p].op1);
				i2 = locate(CODE[p].op2);
				cmpIncondition(i1,i2);
				outasm << "\tjl\t" << CODE[p].dest << endl;
				break;
		
			}
			case JMP: {		//genImc(JMP, "", "", LABEL);
				outasm << "\tjmp\t" << CODE[p].dest << endl;
				break;
			}
			case ELB: {		//genImc(ELB,"","","LABEL"+to_string(labelNo));
				outasm << CODE[p].dest << ":" << endl;
			}
			case WRT: {		//genImc(WRT,tokenbuf,"","");		//genImc(WRT,"",opname,"");

				break;
			}
			case RED: {		//genImc(RED,"","",id);

				break;
			}
		}
		p++;
	}
}

void test(set<SYMTYPE> s1, set<SYMTYPE> s2, int errorcode){
	if (s1.count(sym) < 0){
		reportError(errorcode);
		s1.insert(s2.begin(), s2.end());
		while (s1.count(sym) < 1){
			getsym();
		}
	}
}

void recover(set<SYMTYPE> s){
	if (s.count(sym) < 1){
		while (s.count(sym) < 1){
			getsym();
		}
	}
}

void constdeclaration(set<SYMTYPE> fsys){
	int flag = 1;
	int i;
	fsys.insert(COMMA);
	fsys.insert(SEMICOLON);
	if (sym == IDF){
		getsym();
		if (sym == EQL || sym == BECOMES){
			if (sym == BECOMES)
				reportError(2);		//you can't assign to a const			//无法向常量赋值，此处应该为"="
			//ignore the mindless mistake, continue our analysis work.
			getsym();
			i = locate(id);
			//此处检查是否重复定义
			if(i >= BlockIndex[BItop]){
				reportError(1);			// duplicated define
				recover(fsys);			// 直接忽略这个标识符
				return;
			}
			if (sym == NUMBER || sym == PLUS || sym == MINUS){		//num
				if (sym == MINUS){
					//做好标记，需要将后面的常数取相反数
					//hold, 此处的处理方法还有斟酌
					flag = -1;
				}
				if(sym != NUMBER)
					getsym();
				if (sym == NUMBER){
					digits *= flag;
					SYMITEM anumconst;
					anumconst.name = id;
					anumconst.type = NUMCONST;
					anumconst.level = lvl;
					anumconst.constv = digits;
					anumconst.alink = NULL;
					anumconst.plink = NULL;
					anumconst.link = -1;
					registe(anumconst);
					getsym();
				}
				else{
					reportError(3);			//常量定义中等式右边出现 [+|-]时应该接数字
					recover(fsys);
					return;
				}
			}
			else if (sym == ZIFU){
				SYMITEM azifuconst;
				azifuconst.name = id;
				azifuconst.type = CHARCONST;
				azifuconst.level = lvl;
				azifuconst.constv = (int)tokenbuf.at(0);
				azifuconst.alink = NULL;
				azifuconst.plink = NULL;
				azifuconst.link = -1;
				registe(azifuconst);
				getsym();
			}
			else{
				reportError(4);		//常量定义中"="后面应该是数字或者字符
				recover(fsys);
				return;
			}
		}
		else{
			reportError(5);			//常量定义中标识符后面应该是 "="
			recover(fsys);
			return;
		}
	}
	else{
		reportError(6);					//常量定义中const或者","后面应该是标识符
		recover(fsys);
		return;
	}
	out << "This is a const declaration!" << endl;
}

void vardeclaration(int &offset, set<SYMTYPE> fsys){
	int nvar = 0;
	int i ;
	fsys.insert(SEMICOLON);
	if (sym == IDF){
		while(sym == IDF){
			i = locate(id);
			if(i >= BlockIndex[BItop]){
				reportError(1);				//重复定义
				set<SYMTYPE> fsys2(fsys);
				fsys2.insert(COMMA);
				recover(fsys2);
				if (sym == COMMA)
					continue;
				else
					return;
			}
			SYMITEM avar;
			avar.name = id;
			//avar.type 待定
			avar.level = lvl;
			//avar.offset = offset;
			avar.alink = NULL;
			avar.plink = NULL;
			avar.link = -1;
			registe(avar);
			nvar ++;
			getsym();
			if(sym == COMMA)
				getsym();
			else
				break;
		}
		//回填变量类型
		if(sym == COLON){
			getsym();
			IDTYPE vart  = (IDTYPE)0;
			if (sym == INTSYM)
				vart = NUMVAR;
			else if (sym == CHARSYM)
				vart = CHARVAR;
			else if (sym == ARRAYSYM){
				getsym();
				if (sym == LSQBSYM){
					getsym();
					if (sym == NUMBER){
						int upperbd = digits;
						getsym();
						if (sym == RSQBSYM){
							getsym();
							if(sym == OFSYM){
								getsym();
								if (sym == INTSYM || sym == CHARSYM){
									if(sym==INTSYM)
										vart = NUMVAR;			//var此处用作数组元素的类型
									else
										vart = CHARVAR;
									for(nvar--;nvar >= 0;nvar--){	//回填符号表
										int x = tp-nvar;
										STABLE[x].type = ARRAY;
										STABLE[x].offset = offset;
										offset += upperbd << 2;
										STABLE[x].alink = new arrayinfo;
										STABLE[x].alink->size = upperbd;
										STABLE[x].alink->elementt = vart;
									}
									//分号将会在调用本函数的地方进行处理
								}
								else{
									reportError(7);			//数组元素类型不是基本类型
									recover(fsys);
									return;
								}
							}
							else{
								reportError(8);			//数组声明过程中缺少of
								recover(fsys);
								return;
							}
						}
						else{
							reportError(9);			//数组声明过程中定义大小时缺少"]"		数组定义或使用缺少"["或"]"
							recover(fsys);
							return;
						}
					}
					else{
						reportError(10);		//数组声明过程中定义大小请用无符号整数
						recover(fsys);
						return;
					}
				}
				else{
					reportError(9);		//数组定义或使用缺少"["或"]"
					recover(fsys);
					return;
				}
			}
			else{
				reportError(12);		//未定义变量类型
				recover(fsys);
				return;					// 
			}
			if(vart > 0){
				//此处忽略了数组声明的情况，是因为如果上面已经对数组声明填好了符号表，那么nvar < 0,如果出错，那么vart将依然为0
				for(nvar--;nvar >= 0;nvar--){
					STABLE[tp - nvar].type = vart;
					STABLE[tp - nvar].offset = offset;
					offset += 4;
				}
			}
			getsym();
			//if (sym != SEMICOLON)
			//	reportError(13);		//变量声明没有以冒号结尾，处理时，报告完错误后直接跳过
			//getsym();					//读取下一个token为之后的处理做准备
		}
		else{
			reportError(14);			//变量声明过程中指定变量类型时缺少":"
			recover(fsys);
			return;
		}
	}
	else{
		reportError(6);					//变量定义中var或者","后面应该接标识符
		recover(fsys);
		return;
	}
	out << "This is a variable declaration!" << endl;
}

void parameterdec(bool isprocedure, set<SYMTYPE> fsys){
	int paran = 0;
	int offset = -8;					//EBP 与 第一个参数之间隔了返回地址和SL
	if (sym == LPAREN){
		getsym();
		while (sym == VARSYM || sym == IDF){
			if(sym==VARSYM){			//参数类型为引用类型（传地址）
				getsym();
				if (sym == IDF){
					while (sym == IDF){
						STABLE[tp].plink->paras[paran].name = id;
						STABLE[tp].plink->paras[paran].isVar = true;
						paran++;
						if(paran >= PARANUMMAX){
							getsym();
							break;
						}
						getsym();
						if (sym == COMMA)
							getsym();
						else
							break;
					}
					if (sym == COLON){
						getsym();
						IDTYPE vart = (IDTYPE)0;
						if (sym == INTSYM)
							vart = NUMREF;
						else if (sym == CHARSYM)
							vart = CHARREF;
						else{
							reportError(15);				//参数类型只能是integer或者char
							set<SYMTYPE> fsys2(fsys);
							fsys2.insert(RPAREN);
							recover(fsys2);
							fsys2.erase(RPAREN);
							fsys2.insert(SEMICOLON);
							recover(fsys2);
							return ;
						}
						for (int j = 0; j < paran; j++)
							STABLE[tp].plink->paras[j].type = vart;
					}
					else if(paran >= PARANUMMAX){
						reportError(17);					//参数个数超过上限
						set<SYMTYPE> fsys2(fsys);
						fsys2.insert(RPAREN);
						recover(fsys2);
						fsys2.erase(RPAREN);
						fsys2.insert(SEMICOLON);
						recover(fsys2);
						return ;
					}
					else{
						reportError(14);					//缺少对参数类型的定义，缺少":"
						set<SYMTYPE> fsys2(fsys);
						fsys2.insert(RPAREN);
						recover(fsys2);
						fsys2.erase(RPAREN);
						fsys2.insert(SEMICOLON);
						recover(fsys2);
						return ;
					}
				}
				else{
					reportError(16);						//参数声明中没有指定参数名字
					set<SYMTYPE> fsys2(fsys);
					fsys2.insert(RPAREN);
					recover(fsys2);
					fsys2.erase(RPAREN);
					fsys2.insert(SEMICOLON);
					recover(fsys2);
					return ;
				}
			}
			else{					//参数类型为变量类型（传值）
				while (sym == IDF){
					STABLE[tp].plink->paras[paran].name = id;
					STABLE[tp].plink->paras[paran].isVar = false;
					paran++;
					getsym();
					if (sym == COMMA)
						getsym();
					else
						break;
				}
				if (sym == COLON){
					getsym();
					IDTYPE vart = (IDTYPE)0;
					if (sym == INTSYM)
						vart = NUMVAR;
					else if (sym == CHARSYM)
						vart = CHARVAR;
					else{
						reportError(15);					//参数类型只能是integer或者char
						set<SYMTYPE> fsys2(fsys);
						fsys2.insert(RPAREN);
						recover(fsys2);
						fsys2.erase(RPAREN);
						fsys2.insert(SEMICOLON);
						recover(fsys2);
						return ;
					}
					for (int j = 0; j < paran; j++)
						STABLE[tp].plink->paras[j].type = vart;
				}
				else{
					reportError(14);						//缺少对参数类型的定义，缺少":"
					set<SYMTYPE> fsys2(fsys);
					fsys2.insert(RPAREN);
					recover(fsys2);
					fsys2.erase(RPAREN);
					fsys2.insert(SEMICOLON);
					recover(fsys2);
					return ;
				}
			}
			getsym();
			if (sym == SEMICOLON)
				getsym();
			else if (sym == RPAREN)
				break;
			else{
				reportError(21);							//参数结尾没有")"		此处缺少")"
				break;
			}
		}
		//把参数填入符号表
		int pftp = tp;			//记录下正在处理的过程或函数在符号表中的位置
		STABLE[tp].plink->paranum = paran;
		for (int j = 0; j < paran; j++){
			SYMITEM apara;
			apara.name = STABLE[pftp].plink->paras[j].name;
			apara.type = STABLE[pftp].plink->paras[j].type;
			apara.level = lvl;
			apara.offset = offset;
			offset -= 4;
			apara.plink = NULL;
			apara.alink = NULL;
			apara.link = -1;
			registe(apara);
			//tp++;
		}
		if (sym != RPAREN){
			//
			//reportError(21);								//参数结尾没有")"
		}
		else{
			if (!isprocedure){
				getsym();
				if (sym == COLON){
					getsym();
					if (sym == CHARSYM || sym == INTSYM){
						STABLE[pftp].plink->retvaluet = (sym == CHARSYM) ? CHARVAR : NUMVAR;
						SYMITEM atemp;
						atemp.name = STABLE[pftp].name + "_retv";
						atemp.type = STABLE[pftp].plink->retvaluet;
						atemp.level = lvl;
						atemp.offset = 4;			//将函数的返回值放到EBP上面一个单元
						registe(atemp);
					}
					else{
						reportError(18);					//函数定义中返回值类型不是基本类型
						fsys.insert(SEMICOLON);
						recover(fsys);
						return ;
					}
				}
				else{
					reportError(19);						//没有定义函数的返回值类型
					fsys.insert(SEMICOLON);
					recover(fsys);
					return ;
				}
			}
		}
		getsym();
	}
	else if(sym==COLON && !isprocedure){
		//本函数没有参数
		STABLE[tp].plink->paranum = 0;
		getsym();
		if (sym == CHARSYM || sym == INTSYM){
			STABLE[tp].plink->retvaluet = (sym == CHARSYM) ? CHARVAR : NUMVAR;
			SYMITEM atemp;
			atemp.name = STABLE[tp].name + "_retv";
			atemp.type = STABLE[tp].plink->retvaluet;
			atemp.level = lvl;
			atemp.offset = 4;			//将函数的返回值放到EBP上面一个单元
			registe(atemp);
			getsym();
		}
		else{
			reportError(18);					//函数定义中返回值类型不是基本类型
			fsys.insert(SEMICOLON);
			recover(fsys);
			return ;
		}
	}
	else if (sym == SEMICOLON && isprocedure){
		//本过程没有参数
		STABLE[tp].plink->paranum = 0;
		//printf("本过程没有参数!\n");
	}
	else{
		reportError(20);						//函数或过程声明出现错误
		fsys.insert(SEMICOLON);
		recover(fsys);
		return ;
	}
	out << "This is a "<< ((isprocedure)?"procedure ":"function ") <<"declaration!" << endl;
}



void callp(int tmpindex,int pptr, set<SYMTYPE> fsys){
	//hold
	int pNo = 0;	//参数序号
	IMC TMPPARA[PARANUMMAX];
	string opname;
	if (sym == LPAREN){					//过程名称后面有参数列表
		set<SYMTYPE> fsys2(fsys);
		fsys2.insert(RPAREN);		fsys2.insert(COMMA);
		FPLINK pinfo = STABLE[pptr].plink;
		IDTYPE fpt;						//形参类型
		IDTYPE apt;						//实参类型
		do{
			getsym();
			fpt = pinfo->paras[pNo].type;
			apt = expression(tmpindex, opname, fsys2);
			if (pNo >= pinfo->paranum){
				pNo++;					//放大错误
				break;
			}
			else if (fpt == NUMVAR && (NUMVAR <= apt && apt <= NUMREF)){
				if(apt == NUMREF){		//something different
					//genImc(PARAQ,opname,"","");
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			} 
			else if (fpt == NUMREF && (apt==NUMVAR||apt==NUMREF)){
				if(apt == NUMREF){		//something different
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else if (fpt == CHARVAR && (CHARVAR <= apt && apt <= CHARREF)){
				if(apt == NUMREF){		//something different
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else if (fpt == CHARREF && (apt==CHARVAR||apt==CHARREF)){
				if(apt == NUMREF){		//something different
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else 
				break;
			pNo++;
			// 参数个数、类型检查
			//genImc();					//生成传递参数的指令
		} while (sym == COMMA);
		if(pNo!=pinfo->paranum){
			reportError(41);			//参数个数或类型错误
			recover(fsys);				//直接把这条调用语句忽略
			return;
		}
		else{			//将参数反序压入栈中
			for(pNo--;pNo >= 0;pNo--){	//genImc(PARAQ,opname,"","");
				genImc(TMPPARA[pNo].instrT,TMPPARA[pNo].op1,"","");
			}
		}
		if (sym == RPAREN)
			getsym();
		else{
			reportError(21);			//过程参数缺少")"				此处缺少")"
			recover(fsys);
		}
	}
	else if (sym == SEMICOLON){			//过程名称后面参数列表为空
		if (STABLE[pptr].plink->paranum != 0){
			reportError(17);			//函数或过程调用缺少参数
			recover(fsys);
			return;
		}
	}
	else{
		reportError(25);				//过程调用缺少参数
		recover(fsys);
		return;
	}
	genImc(CALL,"","",STABLE[pptr].name);
	out << "This is a procedure call!" << endl;
}

void callf(int tmpindex, string &opname,int fptr, set<SYMTYPE> fsys){
	//hold
	int pNo = 0;	//参数序号
	IMC TMPPARA[PARANUMMAX];
	if (sym == LPAREN){					//函数名称后面有参数列表
		set<SYMTYPE> fsys2(fsys);
		fsys2.insert(RPAREN);		fsys2.insert(COMMA);
		FPLINK finfo = STABLE[fptr].plink;
		IDTYPE fpt;						//形参类型
		IDTYPE apt;						//实参类型
		do{
			getsym();
			fpt = finfo->paras[pNo].type;
			apt = expression(tmpindex,opname,fsys2);
			if (pNo >= finfo->paranum){
				pNo++;					//放大错误
				break;
			}
			else if (fpt == NUMVAR && (NUMVAR <= apt && apt <= NUMREF)){
				if(apt == NUMREF){		//something different
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			} 
			else if (fpt == NUMREF && (apt==NUMVAR||apt==NUMREF)){
				if(apt==NUMREF){		//something different
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else if (fpt == CHARVAR && (CHARVAR <= apt && apt <= CHARREF)){
				if(apt==CHARREF){		//something different
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else if (fpt == CHARREF && (apt==CHARVAR||apt==CHARREF)){
				if(apt==CHARREF){		//something different
					TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else 
				break;
			pNo++;
			// 参数个数、类型检查
		} while (sym == COMMA);
		if(pNo!=finfo->paranum){
			reportError(41);			//参数个数或类型错误
			recover(fsys);				//直接把这条调用语句忽略
			return;
		}
		else{			//将参数反序压入栈中
			for(pNo--;pNo >= 0;pNo--){	//genImc(PARAQ,opname,"","");
				genImc(TMPPARA[pNo].instrT,TMPPARA[pNo].op1,"","");
			}
		}
		if (sym == RPAREN)
			getsym();
		else{
			reportError(21);			//函数调用时传递参数缺少")" 		此处缺少")"
			recover(fsys);
		}
	}
	//else if (sym == SEMICOLON){		//函数名称后面参数列表为空
		//参数是否为空检查
		//getsym();
	//}
	else{
		//参数是否为空检查，如在此种情况下 P1(F1); F1后面是可以没有分号的
		//reportError(117);				//函数调用语句缺少";"
		//printf("P2(F1)");
		if (STABLE[fptr].plink->paranum != 0){
			reportError(17);			//函数或过程调用缺少参数
			return;
		}
	}
	genImc(CALL,"","",STABLE[fptr].name);
	//在符号表中注册函数的返回值
	tmpindex++;
	SYMITEM atemp;
	atemp.name = "t_" + to_string(tmpindex);
	opname = atemp.name;
	atemp.type = STABLE[fptr].plink->retvaluet;
	atemp.level = lvl;
	atemp.offset = curOffset;
	registe(atemp);
	curOffset += 4;
	genImc(STEAX,"","",opname);
	out << "This is a function call!" << endl;
}

IDTYPE factor(int &tmpindex,string &opname,set<SYMTYPE> fsys){
	IDTYPE factorType;
	int i;		//标识符在符号表中的位置
	test(FACTORFIRST,fsys,35);			// 表达式不能以此符号开头
	while (sym == IDF || sym == NUMBER||sym == LPAREN){				//
		if (sym == IDF){				// @<标识符> | @<标识符>'['<表达式>']' | @<标识符>[<实在参数表>]
			i = locate(id);
			if (i < 0){
				reportError(11);		// 未定义的标识符
				recover(fsys);
				return (IDTYPE)0;
			}
			else{
				IDTYPE idft = STABLE[i].type;
				if (idft == ARRAY){		// 数组
					string indexResult;
					IDTYPE indexType;
					factorType = STABLE[i].alink->elementt;
					getsym();
					if (sym == LSQBSYM){
						getsym();
						set<SYMTYPE> fsys2(fsys);
						fsys2.insert(RSQBSYM);
						indexType = expression(tmpindex,opname,fsys2);
						//下标类型检查
						indexResult = opname;

						//在符号表中注册临时变量
						tmpindex++;
						SYMITEM atemp;
						atemp.name = "t_" + to_string(tmpindex);
						opname = atemp.name;
						atemp.type = factorType;
						atemp.level = lvl;
						atemp.offset = curOffset;
						registe(atemp);
						curOffset += 4;
						genImc(LA,STABLE[i].name,indexResult,opname);//生成LOAD指令，从数组中取出元素到临时变量
						if (sym == RSQBSYM)
							getsym();
						else{
							reportError(9);			//使用数组元素时缺少"]"		数组定义或使用缺少"["或"]"
							recover(fsys);
							return (IDTYPE)0;
						}
					}
					else{
						reportError(9);			//数组定义或使用缺少"["或"]"
						recover(fsys);
						return (IDTYPE)0;
					}
				}
				else if (idft == FUNCTION){		//函数调用语句
					if (STABLE[i].plink->retvaluet == CHARVAR)
						factorType = CHAREXP;
					else
						factorType = NUMEXP;
					getsym();
					callf(tmpindex,opname,i,fsys);
				}
				else{
					if (idft != PROCEDURE){
						//CHARCONST,CHARVAR, @CHARREF,    NUMCONST,NUMVAR, @NUMREF,
						if(idft == CHARREF||idft==NUMREF){		//引用
							factorType = idft;
							opname = STABLE[i].name;
						}
						else{
							switch(idft){
								case CHARCONST	:	factorType = CHAREXP;	break;
								case NUMCONST	:	factorType = NUMEXP;	break;
								default			:	factorType = idft;
							}
							opname = STABLE[i].name;
						}
						getsym();
					}
					else{
						reportError(22);			//过程调用语句不能用作表达式中的因子
						recover(fsys);
						return (IDTYPE)0;
					}
				}
			}
		}
		else if (sym == NUMBER){	//<无符号整数>.0
			factorType = NUMEXP;
			tmpindex++;
			SYMITEM atemp;
			atemp.name = "t_" + to_string(tmpindex);
			opname = atemp.name;
			atemp.type = NUMCONST;
			atemp.level = lvl;
			atemp.constv = digits;
			registe(atemp);
			//在符号表中注册这个常量
			getsym();
		}
		else if (sym == LPAREN){	//'('<表达式>')'
			getsym();
			set<SYMTYPE> fsys2(fsys);
			fsys2.insert(RPAREN);
			factorType = expression(tmpindex,opname,fsys2);		//expression([rparen]+fsys);
			if (sym == RPAREN)
				getsym();
			else{
				reportError(21);					//此表达式缺少")"			此处缺少")"
				recover(fsys);
				return (IDTYPE)0;
			}
		}
		set<SYMTYPE> fsys2;
		fsys2.insert(LPAREN);
		test(fsys, fsys2,36);
	}
	return factorType;
}

IDTYPE term(int &tmpindex,string &opname,set<SYMTYPE> fsys){
	IDTYPE termType;
	SYMTYPE muldivop;
	string op1;
	set<SYMTYPE> fsys2(fsys);
	fsys2.insert(TIMES);	fsys2.insert(SLASH);
	termType = factor(tmpindex,opname,fsys2);
	while (sym == TIMES || sym == SLASH){
		string op1 = opname;
		termType = NUMEXP;
		muldivop = sym;
		getsym();
		factor(tmpindex,opname,fsys2);
		//在符号表里注册"t_(tmpindex+1)"
		tmpindex++;
		SYMITEM atemp;
		atemp.name = "t_" + to_string(tmpindex);
		atemp.type = NUMVAR;
		atemp.level = lvl;
		atemp.offset = curOffset;
		registe(atemp);
		curOffset += 4;
		//注册完毕
		if (muldivop == TIMES){
			genImc(MUL, op1, opname, atemp.name);					//生成乘法指令
		}
		else{
			genImc(DIV, op1, opname, atemp.name);					//生成除法指令
		}
		opname = atemp.name;		//更新当前操作数名称
	}
	return termType;
}

IDTYPE expression(int &tmpindex,string &opname,set<SYMTYPE> fsys){
	IDTYPE exprType;
	SYMTYPE addsubop;
	string op1;
	set<SYMTYPE> fsys2(fsys);
	fsys2.insert(PLUS);		fsys2.insert(MINUS);
	if (sym == PLUS || sym == MINUS){
		exprType = NUMEXP;
		addsubop = sym;
		getsym();
		term(tmpindex,opname,fsys2);
		op1 = opname;
		if (addsubop == MINUS){
			//在符号表里注册"t_(tmpindex+1)"
			tmpindex++;
			SYMITEM atemp;
			atemp.name = "t_" + to_string(tmpindex);
			opname = atemp.name;
			atemp.type = NUMVAR;
			atemp.level = lvl;
			atemp.offset = curOffset;
			registe(atemp);
			curOffset += 4;
			//注册完毕
			genImc(MNS,op1,"",opname);		//生成操作数栈栈顶取相反数指令
		}
	}
	else
		exprType = term(tmpindex,opname,fsys2);
	while (sym == PLUS || sym == MINUS){
		op1 = opname;
		exprType = NUMEXP;
		addsubop = sym;
		getsym();
		term(tmpindex,opname,fsys2);
		//在符号表里注册"t_(tmpindex+1)"
		tmpindex++;
		SYMITEM atemp;
		atemp.name = "t_" + to_string(tmpindex);
		atemp.type = NUMVAR;
		atemp.level = lvl;
		atemp.offset = curOffset;
		registe(atemp);
		curOffset += 4;
		//注册完毕
		if (addsubop == PLUS)
			genImc(ADD,op1,opname,atemp.name);		//生成操作数栈栈顶取与次栈顶相加指令
		else
			genImc(SUB,op1,opname,atemp.name);		//生成操作数栈栈顶取与次栈顶相减指令
		opname = atemp.name;
	}
	return exprType;
}

bool exprTypecheck(IDTYPE dest, IDTYPE src){
	if ((dest == NUMVAR || dest == NUMREF) && (CHARVAR <= src && src <= NUMREF)){
		return true;
	}
	else if ((dest == CHARVAR || dest == CHARREF) && (CHARVAR <= src && src <= CHARREF)){
		return true;
	}
	else
		return false;
}

void assignment(int &tmpindex,int i, set<SYMTYPE> fsys){
	IDTYPE idft;
	IDTYPE exprType;
	string opname;
	idft = STABLE[i].type;
	if (idft == FUNCTION){
		getsym();
		if (sym == BECOMES){
			getsym();
			exprType = expression(tmpindex,opname,fsys);
			if (!exprTypecheck(STABLE[i].plink->retvaluet, exprType))
				reportError(42);			//赋值语句等号两边类型不匹配
			else{
				genImc(MOV,opname,"",STABLE[i].name+"_retv");
			}
		}
		else{
			reportError(23);				// 函数赋值语句后面缺少赋值符号":="		赋值语句缺少":="
			recover(fsys);
			return;
		}
	}
	else if (idft == ARRAY){
		string indexResult;
		getsym();
		if (sym == LSQBSYM){
			IDTYPE indexType;
			getsym();
			indexType = expression(tmpindex,opname,fsys);
			if(NUMVAR <= indexType && indexType <= NUMREF){
				//
			}
			else {
				reportError(43);			//数组下标类型只能是整型
				recover(fsys);
				return;
			}
			indexResult = opname;
			if (sym != RSQBSYM){
				reportError(9);				// 数组下标缺少"]"		数组定义或使用缺少"["或"]"
				recover(fsys);
				return;
			}
			getsym();		//hold,这里的expression顺序问题
			if (sym == BECOMES){
				getsym();
				exprType = expression(tmpindex,opname,fsys);
				if (!exprTypecheck(STABLE[i].alink->elementt, exprType)){
					reportError(42);
				}
				else{
					//根据前面记录下的indexResult(也就是数组下标)生成STORE指令
					genImc(MOVA, opname, indexResult, STABLE[i].name);
					//			 等式右边,数组下标    ,数组名字
				}
			}
			else{
				reportError(23);			// 数组元素赋值操作缺少赋值符号":=" 		赋值语句缺少":="
				recover(fsys);
				return;
			}
		}
		else{
			reportError(9);				// 数组整体不能参与赋值操作	数组定义或使用缺少"["或"]"
			recover(fsys);
			return;
		}
	}
	//else if(idft == CHARREF|| idft == NUMREF){
	//	getsym();
	//	if (sym == BECOMES){
	//		getsym();
	//		expression(fsys);
	//		genImc();						//需加载地址
	//	}
	//	else{
	//		reportError(23);				//赋值操作缺少赋值符号":=" 				赋值语句缺少":="
	//		recover(fsys);
	//		return;
	//	}
	//}
	else{		//	var
		getsym();
		if (sym == BECOMES){
			getsym();
			exprType = expression(tmpindex, opname, fsys);
			if(!exprTypecheck(idft,exprType)){
				reportError(42);
			}
			else{
				genImc(MOV,opname,"",STABLE[i].name);
			}
		}
		else{
			reportError(23);				//赋值操作缺少赋值符号":=" 				赋值语句缺少":="
			recover(fsys);
			return;
		}
	}
	out << "This is a assignment!" << endl;
}

void condition(int &tmpindex,bool isIF,set<SYMTYPE> fsys){
	SYMTYPE relop;		//关系表达式
	IDTYPE exprType1,exprType2;
	string opname;
	string op1;
	set<SYMTYPE> fsys2(fsys);
	fsys2.insert(RELATIONALOP.begin(), RELATIONALOP.end());
	exprType1 = expression(tmpindex,opname,fsys2);
	op1 = opname;
	if (RELATIONALOP.count(sym) > 0 ){	//  EQL,NEQ,LSS,LEQ,GTR,GEQ
		relop = sym;
		getsym();
		exprType2 = expression(tmpindex, opname, fsys2);
		if(!((CHARCONST <= exprType1 && exprType1 <= NUMREF) && (CHARCONST <= exprType2 && exprType2 <= NUMREF))){
			reportError(44);			//条件表达式中比较运算符两边的类型不正确
			return;
		}
		if (isIF){
			switch (relop){
				case EQL:	genImc(BNE, op1, opname, ""); break;		//dest 未定，将会回填
				case NEQ:	genImc(BEQ, op1, opname, ""); break;
				case LSS:	genImc(BGE, op1, opname, ""); break;
				case LEQ:	genImc(BGT, op1, opname, ""); break;
				case GTR:	genImc(BLE, op1, opname, ""); break;
				case GEQ:	genImc(BGT, op1, opname, ""); break;
			}
		}
		else{
			switch (relop){
				case EQL:	genImc(BEQ, op1, opname, ""); break;		//dest 未定，将会回填
				case NEQ:	genImc(BNE, op1, opname, ""); break;
				case LSS:	genImc(BLT, op1, opname, ""); break;
				case LEQ:	genImc(BLE, op1, opname, ""); break;
				case GTR:	genImc(BGT, op1, opname, ""); break;
				case GEQ:	genImc(BGE, op1, opname, ""); break;
			}
		}
	}
	else{
		reportError(24);					//无法识别的关系运算符
		recover(fsys);
		return;
	}
	out << "This is a condition statement!" << endl;
}


void read(set<SYMTYPE> fsys){
	int i;
	if (sym == LPAREN){
		do{
			getsym();
			if(sym!=IDF)
				break;
			i = locate(id);
			if (i < 0 ){
				reportError(11);		//未定义的标识符
				recover(fsys);
				return;
			}
			else{
				IDTYPE idft = STABLE[i].type;
				if (idft == CHARCONST || idft == NUMCONST || idft == PROCEDURE || idft == FUNCTION)
					reportError(26);	//不可向常量或过程、函数赋值
				else{
					genImc(RED,"","",id);
				}
			}
			getsym();
		} while (sym == COMMA);

		if (sym != RPAREN){
			reportError(21);				//read语句缺少")"		此处缺少")"
			recover(fsys);
			return;
		}
	}
	else{
		reportError(21);				//read语句缺少"("		此处缺少")"或"("
	}
	getsym();
	out << "This is a read statement!" << endl;
}

void write(int &tmpindex,set<SYMTYPE> fsys){
	//int tmpindex = 0;
	string opname;
	if (sym == LPAREN){
		getsym();
		set<SYMTYPE> fsys2(fsys);
		fsys2.insert(RPAREN);
		if (sym == ZIFUC){
			genImc(WRT,tokenbuf,"","");
			getsym();
			if (sym == COMMA){
				getsym();
				expression(tmpindex,opname,fsys2);
				genImc(WRT,"",opname,"");
			}
		}
		else{
			expression(tmpindex,opname,fsys2);
			genImc(WRT,"",opname,"");
		}
		if (sym != RPAREN){
			reportError(21);			//write语句缺少")"
			recover(fsys);
			return;
		}
	}
	else{
		reportError(21);				//write语句缺少"("
		recover(fsys);
		return;
	}
	getsym();
	out << "This is a write statement!" << endl;
}

void statement(int &tmpindex,set<SYMTYPE> fsys){
	int i,cx1,cx2;
	if (sym == IDF){		//有可能是赋值语句，或者过程调用语句
		i = locate(id);
		if (i < 0){
			reportError(11);			//未定义的标识符
			recover(fsys);
			return;
		}
		else{
			//string opname;
			IDTYPE idft = STABLE[i].type;
			if (idft == PROCEDURE){
				getsym();
				callp(tmpindex,i,fsys);
				//genImc();			//生成跳转指令
			}
			else if (idft == CHARCONST || idft == NUMCONST){
				reportError(26);		//不可向常量赋值
				recover(fsys);
				return;
			}
			else{		//赋值
				assignment(tmpindex,i,fsys);
			}
		}
	}
	else if (sym == IFSYM){
		getsym();
		set<SYMTYPE> fsys2(fsys);
		fsys2.insert(THENSYM);
		condition(tmpindex,true,fsys2);
		cx1 = cx-1;
		if (sym == THENSYM){
			getsym();
			//genImc();						//条件跳转到then后面的语句因此需要回填跳转地址,  gen(jpc,0,0);
			fsys2.clear();
			fsys2.insert(fsys.begin(), fsys.end());
			fsys2.insert(ELSESYM);
			statement(tmpindex,fsys2);
		if (sym == ELSESYM){
				cx2 = cx;
				genImc(JMP, "", "", "");						//无条件跳转到ELSE后面的语句，gen(jmp,0,0)
				CODE[cx1].dest = "LABEL"+to_string(labelNo);	//(* write back the target address of the jump code *)
				genImc(ELB,"","","LABEL"+to_string(labelNo));	//生成LABEL
				labelNo++;
				getsym();
				statement(tmpindex,fsys);
				CODE[cx2].dest = "LABEL"+to_string(labelNo);	//(* write back the target address of the jump code *)
				genImc(ELB,"","","LABEL"+to_string(labelNo));			//生成LABEL
				labelNo++;
			}
			else{
				recover(fsys2);
				//if (sym == ELSESYM)
					//goto  ELSE;
				//else
				//	reportError(41);
				genImc(ELB,"","","LABEL"+to_string(labelNo));
				CODE[cx1].dest = "LABEL"+to_string(labelNo);//(* write back the target address of the jump code *)
				labelNo++;
			}

		}
		else{
			reportError(27);				//if语句缺少then
			recover(fsys2);
			return;
		}
	}
	else if (sym == BEGINSYM){
		getsym();
		set<SYMTYPE> fsys2(fsys);
		fsys2.insert(SEMICOLON);	fsys2.insert(ENDSYM);
		statement(tmpindex,fsys2);						//statement([semicolon,endsym]+fsys);
		while (sym == SEMICOLON || STMTFIRST.count(sym) > 0){// 这里还需要考虑其他情况
			if (sym != SEMICOLON){
				reportError(28);				//语句之间缺少";"
			}
			else
				getsym();
			statement(tmpindex,fsys2);					//statement([semicolon,endsym]+fsys)
		}
		if (sym == ENDSYM)
			getsym();
		else{
			reportError(29);				//此处应为分号或end
			recover(fsys2);
			return;
		}
		out << "This is a compound statement!" << endl;
	}
	else if (sym == DOSYM){
		string lbname = "LABEL" + to_string(labelNo);
		genImc(ELB,"","",lbname);			//生成LABEL
		labelNo++;
		getsym();
		set<SYMTYPE> fsys2(fsys);
		fsys2.insert(WHILESYM);
		statement(tmpindex,fsys2);
		if (sym == WHILESYM){
			getsym();
			condition(tmpindex,false,fsys);	//condition([dosym]+fsys);
			CODE[cx-1].dest = lbname;		//填跳转地址，跳转回 do 后面的语句
		}
		else{
			reportError(30);				//do while语句缺少while
			recover(fsys2);
			return;
		}
		out << "This is a do while cycle!" << endl;
	}
	else if (sym == FORSYM){
		getsym();
		if (sym = IDF){
			i = locate(id);
			if (i < 0){
				reportError(11);			//未识别的标识符
				recover(fsys);
				return;
			}
			else{
				string loopvar = id;
				string opname;
				IDTYPE idft = STABLE[i].type;
				if (idft == NUMVAR || idft == NUMREF){
					assignment(tmpindex,i,fsys);
					if (sym == DOWNTOSYM || sym == TOSYM){
						SYMTYPE DWNRORTO = sym;
						string lbname = "LABEL" + to_string(labelNo);
						genImc(ELB, "", "", lbname);			//生成LABEL
						labelNo++;
						getsym();
						set<SYMTYPE> fsys2(fsys);
						fsys2.insert(DOSYM);
						expression(tmpindex, opname, fsys2);
						cx1 = cx ;
						if (DWNRORTO==DOWNTOSYM)
							genImc(BLT, loopvar, opname, "");			//生成比较指令，然后再生成条件跳转指令
						else
							genImc(BGT, loopvar, opname, "");
						if (sym == DOSYM){
							getsym();
							statement(tmpindex,fsys);
							if (DWNRORTO == DOWNTOSYM)
								genImc(DEC, loopvar, "", "");
							else
								genImc(INC, loopvar, "", "");
							genImc(JMP,"","",lbname);		//生成无条件跳转指令，跳回for循环比较条件
							lbname = "LABEL" + to_string(labelNo);
							genImc(ELB, "", "", lbname);	//生成LABEL
							labelNo++;
							CODE[cx1].dest = lbname;		//(* write back the target address of the jump code *)
						}
						else{
							reportError(34);			//for 循环语句缺少 do
							recover(fsys2);
							return;
						}
					}
					else{
						reportError(33);			//for 循环语句缺少 to 或者downto
						recover(fsys);
						return;
					}
				}
				else{
					reportError(32);			//循环变量类型不正确
					recover(fsys);
					return;
				}
			}
		}
		else{
			reportError(31);				//for 后面应该有循环变量
			recover(fsys);
			return;
		}
		out << "This is a for cycle!" << endl;
	}
	else if (sym == READSYM){
		getsym();
		read(fsys);
	}
	else if (sym == WRITESYM){
		getsym();
		write(tmpindex,fsys);
	}
	else{
		//
	}
	set<SYMTYPE> fsys2;
	test(fsys, fsys2, 39);							//test(fsys,[],19)		语句后的符号不正确
}

void Block(int btp,int offset, set<SYMTYPE> fsys){
	int dx = 3, cxbg,cxstbg,cxsted, cx1;
	int tmpindex = 0;
	cxbg = cx;
	//int offset = 0;						//hold,Offset不一定是从0开始，因为还有实参
	string lbname = "LABEL" + to_string(labelNo);
	genImc(ELB, "", "", lbname);			//生成LABEL
	labelNo++;
	STABLE[btp].plink->addr = lbname;		//此处先给过程设一个入口地址，为了方便子程序调用本程序
	genImc(INI,"","",STABLE[btp].name);
	cx1 = cx;
	genImc(JMP,"","","");					//无条件跳转 { jump from declaration part to statement part }
	if (lvl > LVMAX){
		reportError(40);				//嵌套层次太深，本宝宝受不了啦
		exit(0);
	}
	do{
		if (sym == CONSTSYM){
			getsym();
			set<SYMTYPE> fsys2(fsys);
			fsys2.insert(VARSYM);
			do{
				constdeclaration(fsys2);
				while (sym == COMMA){
					getsym();
					constdeclaration(fsys);
				}
				if (sym == SEMICOLON)
					getsym();
				else{
					reportError(28);	//常量声明结束缺少";"
					recover(fsys);
					break;
				}
			} while (sym == IDF);
		}
		if (sym == VARSYM){
			getsym();
			do{
				vardeclaration(offset,STMTFIRST);
				if(sym==SEMICOLON)
					getsym();
				else{
					reportError(13);
				}
				if (sym == PROCSYM || sym == FUNCSYM || sym == BEGINSYM)
					break;
			} while (sym == IDF);
		}
		while (sym == PROCSYM||sym==FUNCSYM){
			bool isprocedure;
			if (sym == PROCSYM)
				isprocedure = true;
			else
				isprocedure = false;
			getsym();
			int abtp;
			if (sym == IDF){
				SYMITEM aporf;
				aporf.name = id;
				aporf.type = (isprocedure) ? PROCEDURE : FUNCTION;
				aporf.level = lvl;
				aporf.plink = new pfinfo;
				aporf.alink = NULL;
				aporf.link = -1;
				registe(aporf);
				abtp = tp;
				getsym();
				BlockIndex[++BItop] = tp + 1;					//分程序索引表
				lvl++;
				parameterdec(isprocedure,fsys);		//顺带把参数填进符号表
			}
			if (sym == SEMICOLON)
				getsym();
			else{
				reportError(28);			//过程或函数调用语句结尾缺少";"
				recover(fsys);
			}
			set<SYMTYPE> fsys2(fsys);
			fsys2.insert(SEMICOLON);
			if(isprocedure)
				Block(abtp,4,fsys2);				//block(lev+1,tx,[semicolon]+fsys);
			else
				Block(abtp,8,fsys2);				//block(lev+1,tx,[semicolon]+fsys);
			if (sym == SEMICOLON){
				getsym();
				fsys2.clear();
				fsys2.insert(STMTFIRST.begin(), STMTFIRST.end());
				fsys2.insert(IDF);		fsys2.insert(PROCSYM);		fsys2.insert(FUNCSYM);
				test(fsys2,fsys,37);				//test( statbegsys+[ident,procsym],fsys,6)		过程说明后的符号不正确
			}
			else{
				reportError(28);			//子程序没有以";"结尾
			}
		}
		set<SYMTYPE> fsys2(STMTFIRST);
		fsys2.insert(IDF);
		test(fsys2,DECLARAFIRST,38);				//test( statbegsys+[ident],declbegsys,7)		此处应为语句
		//为了忽略错误，继续编译，所以加了这么一个do while
	}	while (sym == CONSTSYM || sym == VARSYM || sym == PROCEDURE || sym == FUNCSYM);
	cxstbg = cx;
	lbname = "LABEL" + to_string(labelNo);
	genImc(ELB, "", "", lbname);			//生成LABEL
	labelNo++;
	CODE[cx1].dest = lbname;
	STABLE[btp].plink->addr = lbname;		//重新设定本程序的入口地址
	//genImc();		申请活动记录, 分配空间, 这里的空间大小需要回填
	set<SYMTYPE> fsys2(fsys);
	fsys2.insert(SEMICOLON);	fsys2.insert(ENDSYM);
	curOffset = offset;
	statement(tmpindex,fsys2);		//statement( [semicolon,endsym]+fsys);
	STABLE[btp].plink->varsize = curOffset;
	//回填申请空间, 计算方法应为 offset + tmpindex*4 + display区等等，实际上就等于curOffset + display + ...
	genImc(RET, "", "", "");
	cxsted = cx;
	fsys2.clear();
	test(fsys,fsys2,39);
	genAsm(cxbg,cxbg+3);
	genAsm(cxstbg,cxsted);
	//hold,删除当前子活动记录，并删除符号表中的本活动记录的内容
	popstable();
}


void initial(){
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
	RSRVWORD[40] = "write";		RSRVWORD[41] = "ZIFU";		RSRVWORD[42] = "ZIFUC";		RSRVWORD[43] = "IDF";
	RSRVWORD[44] = "NUMBER";	RSRVWORD[45] = "NUL";

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
	RSRVWORDT[40] = "WRITESYM";	RSRVWORDT[41] = "ZIFU";		RSRVWORDT[42] = "ZIFUC";	RSRVWORDT[43] = "IDF";
	RSRVWORDT[44] = "NUMBER";	RSRVWORDT[45] = "NUL";

	SSYMT[0] = '\"';		SSYMT[1] = '\'';	SSYMT[2] = '(';		SSYMT[3] = ')';
	SSYMT[4] = '*';			SSYMT[5] = '+';		SSYMT[6] = ',';		SSYMT[7] = '-';
	SSYMT[8] = '.';			SSYMT[9] = '/';		SSYMT[10] = ';';	SSYMT[11] = '=';
	SSYMT[12] = '[';		SSYMT[13] = ']';

	DECLARAFIRST	= { CONSTSYM, VARSYM, PROCSYM, FUNCSYM };
	STMTFIRST		= { BEGINSYM, IFSYM, DOSYM, FORSYM, READSYM, WRITESYM };
	FACTORFIRST		= { IDF, NUMBER, LPAREN };
	RELATIONALOP	= { EQL, NEQ, GTR, GEQ, LSS, LEQ };
}



int main(){
	
	int i = 1;

	initial();

#ifndef DEBUG
	char filename[20];
	printf("Please input the source program file name : ");
	scanf("%s", filename);
	in.open(filename);
#endif // !DEBUG
	
#ifdef DEBUG
	in.open("a.txt");
#endif // DEBUG

	out.open("result.txt");
	outasm.open("asm.txt");
	i = 1;
	ch = ' ';
	tp = -1;
	getsym();
	SYMITEM amain;
	amain.name = "main";
	amain.type = PROCEDURE;
	amain.level = 0;
	amain.plink = new pfinfo;
	amain.alink = NULL;
	amain.link = -1;
	registe(amain);
	BlockIndex[BItop] = 0;			//初始化分程序索引表
	set<SYMTYPE> fsys(STMTFIRST);						//生成参数
	fsys.insert(DECLARAFIRST.begin(),DECLARAFIRST.end());			//block( 0,0,[period]+declbegsys+statbegsys );
	genImc(JMP,"","","LABEL0");
	outasm << "\tjmp\tLABEL0" << endl;
	Block(0, 4, fsys);
	if (sym != PERIOD)
		reportError(142);
	if (err == 0){
		printf("The compiler work is done!\n");
		out << endl;
		out << "The Immediate Code:" << endl;
		listImc();
	}
	else
		printf("ERRORS IN YOUR PROGRAM\n");
	char asdc = getchar();
	asdc = getchar();
	asdc = getchar();
	asdc = getchar();
	in.close();
	out.close();
	

	return 0;
}