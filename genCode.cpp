#include "datastructure.h"
using namespace std;

extern ifstream  in;
extern ofstream  outimcode;					// 中间代码文件
extern ofstream  outasmtmp;					// 目标代码临时文件
extern ofstream  outfinalcode;				// 目标代码最终文件
extern SYMITEM STABLE[TABLENMAX];			// the symbol table
extern int BlockIndex[LVMAX];				// 分程序索引表
extern int BItop;							// 分程序索引表里的栈顶指针
extern IMC CODE[CODEASIZE];					// 中间代码数组
extern int cx;								// point out where the next instruction will be stored
extern int lvl;								// current level
extern string strConst[STRCONSTMAX];		// 字符串常量表
extern int strCstNo;						// 字符串常量表下标
extern int globalvartop;					// 所有全局变量在符号表中最大的下标

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
	if (cx >= CODEASIZE){
		printf("Sorry, the program is too long!");
		closefiles();
		exit(0);
	}
}

void listImc(){
	int p = 0;
	INSTRTYPE iT;
	while (p < cx){
		iT = CODE[p].instrT;
		switch (iT){
			case NOP:
				break;
			case ADD:{
				outimcode << "\t\tADD\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case SUB:{
				outimcode << "\t\tSUB\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case MUL:{
				outimcode << "\t\tMUL\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case DIV:{
				outimcode << "\t\tDIV\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case INC:{
				outimcode << "\t\tINC\t" << CODE[p].op1 << endl;
				break;
			}
			case DEC:{
				outimcode << "\t\tDEC\t" << CODE[p].op1 << endl;
				break;
			}
			case MNS:{
				outimcode << "\t\tMNS\t" << CODE[p].op1 << ", " << CODE[p].dest << endl;
				break;
			}
			case MOV:{
				outimcode << "\t\tMOV\t" << CODE[p].op1 << ", " << CODE[p].dest << endl;
				break;
			}
			case MOVA:{
				outimcode << "\t\tMOVA\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case LA:{
				outimcode << "\t\tLA\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case STEAX:{
				outimcode << "\t\tSTEAX\t" << "EAX, " << CODE[p].dest << endl;
				break;
			}
			case BEQ:{
				outimcode << "\t\tBEQ\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BNE:{
				outimcode << "\t\tBNE\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BGE:{
				outimcode << "\t\tBGE\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BGT:{
				outimcode << "\t\tBGT\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BLE:{
				outimcode << "\t\tBLE\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case BLT:{
				outimcode << "\t\tBLT\t" << CODE[p].op1 << ", " << CODE[p].op2 << ", " << CODE[p].dest << endl;
				break;
			}
			case JMP:{
				outimcode << "\t\tJMP\t" << CODE[p].dest << endl;
				break;
			}
			case ELB:{
				outimcode << CODE[p].dest << ":" << endl;
				break;
			}
			case PARA:{
				outimcode << "\t\tPARA\t" << CODE[p].op1 << endl;
				break;
			}
			case PARAQ:{
				outimcode << "\t\tPARAQ\t" << CODE[p].op1 << endl;
				break;
			}
			case CALL:{
				outimcode << "\t\tCALL\t" << CODE[p].dest << endl;
				break;
			}
			case INI : {
				outimcode << "\t\tINI\t" << CODE[p].dest << endl;
				break;
			}
			case RET:{
				outimcode << "\t\tRET\n" << endl;
				break;
			}
			case WRT:{
				if(CODE[p].op1 == "")
					outimcode << "\t\tWRT\t" << CODE[p].op2 << endl;
				else
					outimcode << "\t\tWRT\t" << CODE[p].op1 << endl;
				break;
			}
			case RED:{
				outimcode << "\t\tRED\t" << CODE[p].dest << endl;
				break;
			}
			default:
				outimcode << "Something Wrong!" << endl;
				break;
		}
		p++;
	}
}

void loadexternVar(int i){
	//将外层变量的所在层地址加载到eax
	int lvldist = lvl - STABLE[i].level;		// 引用了外层变量
	outasmtmp << "\tmov\teax,[ebp+8]" << endl;		// SL位于EBP下方第二个单元
	lvldist--;
	while (lvldist > 0){
		outasmtmp << "\tmov\teax,[eax]" << endl;
		lvldist--;
	}
}

void loadOp1ToEDX(int i1){
	//先取得操作数1的数值, 放入edx
	if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST)
		outasmtmp << "\tmov\tedx," << STABLE[i1].constv << endl;
	else{
		if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
			if(STABLE[i1].level == 0 && i1 <= globalvartop)
				outasmtmp << "\tmov\tedx,dword ptr[" << STABLE[i1].name << "]" << endl;
			else if (STABLE[i1].level == lvl)
				outasmtmp << "\tmov\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
			else{
				loadexternVar(i1);//将外层变量的所在层地址加载到eax
				outasmtmp << "\tmov\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
			}
		}
		else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
			if (STABLE[i1].level == lvl)
				outasmtmp << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
			else{
				loadexternVar(i1);
				outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
			}
			outasmtmp << "\tmov\tedx,[eax]" << endl;
		}
	}
}


void manipEDXwithOp2(int i2,string manip){
	if (STABLE[i2].type == CHARCONST || STABLE[i2].type == NUMCONST)
		outasmtmp << "\t" << manip <<"\tedx," << STABLE[i2].constv << endl;
	else{
		if (STABLE[i2].type == CHARVAR || STABLE[i2].type == NUMVAR){
			if(STABLE[i2].level == 0 && i2 <= globalvartop)
				outasmtmp << "\t"<<manip << "\tedx,dword ptr[" << STABLE[i2].name << "]" << endl;
			else if (STABLE[i2].level == lvl)
				outasmtmp << "\t" << manip << "\tedx,[ebp-" << STABLE[i2].offset << "]" << endl;
			else{
				loadexternVar(i2);//将外层变量的所在层地址加载到eax
				outasmtmp << "\t" << manip << "\tedx,[eax-8-" << STABLE[i2].offset << "]" << endl;
			}
		}
		else{//(STABLE[i2].type == CHARREF || STABLE[i2].type == NUMREF)
			if (STABLE[i2].level == lvl)
				outasmtmp << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
			else{
				loadexternVar(i2);
				outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
			}
			outasmtmp << "\t" << manip << "\tedx,[eax]" << endl;
		}
	}
}

void movEDXtoTarget(int idest){
	//把edx的值赋给目标变量
	if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
		//全局变量中不可能有引用变量
		if (STABLE[idest].level == lvl)
			outasmtmp << "\tmov\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
		else{
			loadexternVar(idest);
			outasmtmp << "\tmov\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
		}
		outasmtmp << "\tmov\tdword ptr [eax],edx" << endl;
	}
	else{
		if(STABLE[idest].level == 0 && idest <= globalvartop)
			outasmtmp << "\tmov\tdword ptr[" << STABLE[idest].name << "],edx" << endl;
		else if (STABLE[idest].level == lvl)
			outasmtmp << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << "],edx" << endl;
		else{
			loadexternVar(idest);
			outasmtmp << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
		}
	}
}

void cmpIncondition(int i1,int i2){
	//先取得操作数1的数值, 放入edx
	loadOp1ToEDX(i1);
	//与操作数2进行比较
	manipEDXwithOp2(i2,"cmp");
}

void genAsm(int cxbg,int cxend){
	int p = cxbg;
	int i1, i2, idest;
	INSTRTYPE iT;
	int lvldist;
	int paraoffset;
	bool hasConst = false;
	int tmp;
	while (p < cxend){
		iT = CODE[p].instrT;
		switch (iT)
		{
		case NOP:
			break;
		case ADD:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			idest = locate(CODE[p].dest);
			//先取得操作数1的数值, 放入edx
			loadOp1ToEDX(i1);
			//将edx加上操作数2
			manipEDXwithOp2(i2,"add");
			//把edx的值赋给目标变量
			movEDXtoTarget(idest);
			break;
		case SUB:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			idest = locate(CODE[p].dest);
			//先取得操作数1的数值, 放入edx
			loadOp1ToEDX(i1);
			//将edx减去操作数2
			manipEDXwithOp2(i2,"sub");
			//把edx的值赋给目标变量
			movEDXtoTarget(idest);
			break;
		case MUL:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			idest = locate(CODE[p].dest);
			hasConst = false;
			if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
				tmp = i2;
				i2 = i1;
				i1 = tmp;
				hasConst = true;
			}
			//把乘法结果放到edx
			if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){		//说明两个操作数都是常量 
				outasmtmp << "\tmov\tedx," << STABLE[i1].constv << endl;
				outasmtmp << "\timul\tedx,edx," << STABLE[i2].constv << endl;	//edx中放着乘法结果
			}
			else{//先把操作数1加载到edx
				loadOp1ToEDX(i1);
				//现在edx里面放着操作数1
				if (hasConst)		//操作数2是常数
					outasmtmp << "\timul\tedx,edx," << STABLE[i2].constv << endl;
				else{				//操作数2是变量或引用
					manipEDXwithOp2(i2,"imul");
				}
				//现在edx里放着乘法结果
			}
			//把edx的值赋给目标变量
			movEDXtoTarget(idest);
			break;
		case DIV:
			//genImc(DIV, op1, opname, atemp.name);
			//idiv指令完成整数除法操作，idiv只有一个操作数，此操作数为除数，而被除数则为EDX:EAX中的内容（一个64位的整数）,
			//操作的结果有两部分：商和余数，其中商放在eax寄存器中，而余数则放在edx寄存器中。
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			idest = locate(CODE[p].dest);
			//hold,这里使用了ecx, 会对优化有一定影响
			//先把操作数1放入eax,因为被除数显然是32位的，所以只需使用eax即可
			if (STABLE[i1].type == CHARCONST || STABLE[i1].type == NUMCONST){
				outasmtmp << "\tmov\teax," << STABLE[i1].constv << endl;
			}
			else{
				if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
					if(STABLE[i1].level == 0 && i1 <= globalvartop)
						outasmtmp << "\tmov\teax,dword ptr[" << STABLE[i1].name << "]" << endl;
					else if (STABLE[i1].level == lvl)
						outasmtmp << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
					else{
						loadexternVar(i1);//将外层变量的所在层地址加载到eax
						outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
					}
				}
				else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
					if (STABLE[i1].level == lvl){
						outasmtmp << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
					}
					else{
						loadexternVar(i1);
						outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
					}
					outasmtmp << "\tmov\teax,[eax]" << endl;
				}
			}
			outasmtmp << "\tcdq" << endl;		//此处将eax的符号位扩展到edx
			//接下来是除法操作
			if (STABLE[i2].type == CHARCONST || STABLE[i2].type == NUMCONST){
				outasmtmp << "\tmov\tecx," << STABLE[i2].constv << endl;
				outasmtmp << "\tidiv\tecx" << endl;
			}
			else{
				if (STABLE[i2].type == CHARVAR || STABLE[i2].type == NUMVAR){
					if(STABLE[i2].level == 0 && i2 <= globalvartop)
						outasmtmp << "\tidiv\tdword ptr[" << STABLE[i2].name << "]" << endl;
					else if (STABLE[i2].level == lvl){
						outasmtmp << "\tidiv\tdword ptr[ebp-" << STABLE[i2].offset << "]" << endl;
					}
					else{
						lvldist = lvl - STABLE[i2].level;		// 引用了外层变量
						outasmtmp << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP下方第二个单元
						while (lvldist > 0){
							outasmtmp << "\tmov\tecx,[ecx]" << endl;
							lvldist--;
						}
						outasmtmp << "\tidiv\tdword ptr[ecx-8-" << STABLE[i2].offset << "]" << endl;
					}
				}
				else{//(STABLE[i2].type == CHARREF || STABLE[i2].type == NUMREF)
					if (STABLE[i2].level == lvl){
						outasmtmp << "\tmov\tecx,[ebp-" << STABLE[i2].offset << "]" << endl;
					}
					else{
						lvldist = lvl - STABLE[i2].level;		// 引用了外层变量
						outasmtmp << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP下方第二个单元
						while (lvldist > 0){
							outasmtmp << "\tmov\tecx,[ecx]" << endl;
							lvldist--;
						}
						outasmtmp << "\tmov\tecx,[ecx-8-" << STABLE[i2].offset << "]" << endl;
					}
					outasmtmp << "\tidiv\tdword ptr[ecx]" << endl;
				}
			}

			//把eax的值(商)赋给目标变量
			if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
				
				if (STABLE[idest].level == lvl)
					outasmtmp << "\tmov\tecx,[ebp-" << STABLE[idest].offset << "]" << endl;
				else{
					lvldist = lvl - STABLE[idest].level;		// 引用了外层变量
					outasmtmp << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP下方第二个单元
					while (lvldist > 0){
						outasmtmp << "\tmov\tecx,[ecx]" << endl;
						lvldist--;
					}
					outasmtmp << "\tmov\tecx,[ecx-8-" << STABLE[idest].offset << "]" << endl;
				}
				outasmtmp << "\tmov\tdword ptr [ecx],eax" << endl;
			}
			else{
				if(STABLE[idest].level == 0 && idest <= globalvartop)//此处考虑全局变量（也就是lvl0的变量）
					outasmtmp << "\tmov\tdword ptr [" << STABLE[idest].name << "],eax" << endl;
				else if (STABLE[idest].level == lvl)
					outasmtmp << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << "],eax" << endl;
				else{
					lvldist = lvl - STABLE[idest].level;		// 引用了外层变量
					outasmtmp << "\tmov\tecx,[ebp+8]" << endl;		// SL位于EBP下方第二个单元
					while (lvldist > 0){
						outasmtmp << "\tmov\tecx,[ecx]" << endl;
						lvldist--;
					}
					outasmtmp << "\tmov\tdword ptr [ecx-8-" << STABLE[idest].offset << "],eax" << endl;
				}
			}
			break;
		case INC:
			i1 = locate(CODE[p].op1);
			if (STABLE[i1].type == NUMVAR){
				if(STABLE[i1].level == 0 && i1 <= globalvartop)
					outasmtmp << "\tinc\tdword ptr[" << STABLE[i1].name << "]" << endl;
				else if (STABLE[i1].level == lvl)
					outasmtmp << "\tinc\tdword ptr[ebp-" << STABLE[i1].offset << "]" << endl;
				else{
					loadexternVar(i1);//将外层变量的所在层地址加载到eax
					outasmtmp << "\tinc\tdword ptr[eax-8-" << STABLE[i1].offset << "]" << endl;
				}
			}
			else{//(STABLE[i2].type == NUMREF)
				if (STABLE[i1].level == lvl)
					outasmtmp << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
				else{
					loadexternVar(i1);
					outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
				}
				outasmtmp << "\tinc\tdword ptr[eax]" << endl;
			}
			break;
		case DEC:
			i1 = locate(CODE[p].op1);
			if (STABLE[i1].type == NUMVAR){
				if(STABLE[i1].level == 0 && i1 <= globalvartop)
					outasmtmp << "\tdec\tdword ptr[" << STABLE[i1].name << "]" << endl;
				else if (STABLE[i1].level == lvl)
					outasmtmp << "\tdec\tdword ptr[ebp-" << STABLE[i1].offset << "]" << endl;
				else{
					loadexternVar(i1);//将外层变量的所在层地址加载到eax
					outasmtmp << "\tdec\tdword ptr[eax-8-" << STABLE[i1].offset << "]" << endl;
				}
			}
			else{//(STABLE[i2].type == NUMREF)
				if (STABLE[i1].level == lvl)
					outasmtmp << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
				else{
					loadexternVar(i1);
					outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
				}
				outasmtmp << "\tdec\tdword ptr[eax]" << endl;
			}
			break;
		case MNS:
			i1 = locate(CODE[p].op1);
			idest = locate(CODE[p].dest);
			//将操作数1加载到edx
			loadOp1ToEDX(i1);
			outasmtmp << "\tneg\tedx" << endl;
			//把edx的值赋给目标变量
			movEDXtoTarget(idest);
			break;
		case MOV:
			i1 = locate(CODE[p].op1);
			idest = locate(CODE[p].dest);
			loadOp1ToEDX(i1);
			movEDXtoTarget(idest);
			break;
		case MOVA:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			idest = locate(CODE[p].dest);
			//将操作数1加载到edx
			loadOp1ToEDX(i1);
			//将数组下标加载到ecx
			if (STABLE[i2].type == NUMCONST)
				outasmtmp << "\tmov\tecx," << STABLE[i2].constv << endl;
			else if (STABLE[i2].type == NUMREF){
				if (STABLE[i2].level == lvl){
					outasmtmp << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
				}
				else{
					loadexternVar(i2);
					outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
				}
				outasmtmp << "\tmov\tecx,[eax]" << endl;
			}
			else{
				if(STABLE[i2].level == 0 && i2 <= globalvartop)
					outasmtmp << "\tmov\tecx,dword ptr[" << STABLE[i2].name << "]" << endl;
				else if (STABLE[i2].level == lvl){
					outasmtmp << "\tmov\tecx,[ebp-" << STABLE[i2].offset << "]" << endl;
				}
				else{
					loadexternVar(i2);//将外层变量的所在层地址加载到eax
					outasmtmp << "\tmov\tecx,[eax-8-" << STABLE[i2].offset << "]" << endl;
				}
			}
			//将edx存储到数组里
			if(STABLE[idest].level == 0 && idest <= globalvartop)		//全局变量
				outasmtmp << "\tlea\teax,[" << STABLE[idest].name << "]" << endl;
			else if (STABLE[idest].level == lvl)
				outasmtmp << "\tlea\teax,[ebp-" << STABLE[idest].offset << "]" << endl;
			else{
				loadexternVar(idest);//将外层变量的所在层地址加载到eax
				outasmtmp << "\tlea\teax,[eax-8-" << STABLE[idest].offset << "]" << endl;
			}
			outasmtmp << "\tshl\tecx,2" << endl;		//计算地址
			if(STABLE[idest].level == 0 && idest <= globalvartop)
				outasmtmp << "\tadd\teax,ecx" << endl;		//计算地址
			else
				outasmtmp << "\tsub\teax,ecx" << endl;		//计算地址
			//outasmtmp << "\tlea\teax,[eax-4*ecx]" << endl;		//计算地址
			outasmtmp << "\tmov\tdword ptr [eax],edx" << endl;
			break;
		case LA:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			idest = locate(CODE[p].dest);
			//将数组开始地址放在edx
			if(STABLE[i1].level == 0 && i1 <= globalvartop)
				outasmtmp << "\tlea\tedx,[" << STABLE[i1].name << "]" << endl;
			else if (STABLE[i1].level == lvl)
				outasmtmp << "\tlea\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
			else{
				loadexternVar(i1);//将外层变量的所在层地址加载到eax
				outasmtmp << "\tlea\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
			}
			//将数组下标加载到ecx
			if (STABLE[i2].type == NUMCONST)
				outasmtmp << "\tmov\tecx," << STABLE[i2].constv << endl;
			else if (STABLE[i2].type == NUMREF){
				if (STABLE[i2].level == lvl){
					outasmtmp << "\tmov\teax,[ebp-" << STABLE[i2].offset << "]" << endl;
				}
				else{
					loadexternVar(i2);
					outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i2].offset << "]" << endl;
				}
				outasmtmp << "\tmov\tecx,[eax]" << endl;
			}
			else{
				if(STABLE[i2].level == 0 && i2 <= globalvartop)
					outasmtmp << "\tmov\tecx,dword ptr[" << STABLE[i2].name << "]" << endl;
				else if (STABLE[i2].level == lvl)
					outasmtmp << "\tmov\tecx,[ebp-" << STABLE[i2].offset << "]" << endl;
				else{
					loadexternVar(i2);//将外层变量的所在层地址加载到eax
					outasmtmp << "\tmov\tecx,[eax-8-" << STABLE[i2].offset << "]" << endl;
				}
			}
			//取出数组对应位置的数值
			outasmtmp << "\tshl\tecx,2" << endl;		//计算地址
			if(STABLE[i1].level == 0 && i1 <= globalvartop)
				outasmtmp << "\tadd\tedx,ecx" << endl;		//计算地址
			else
				outasmtmp << "\tsub\tedx,ecx" << endl;		//计算地址
			//outasmtmp << "\tlea\tedx,[edx-4*ecx]" << endl;		//计算地址
			//outasmtmp << "\tmov\tedx,edx" << endl;
			//把edx的值赋给目标变量
			if (STABLE[idest].level == lvl)
				outasmtmp << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << "],edx" << endl;
			else{
				loadexternVar(idest);
				outasmtmp << "\tmov\tdword ptr [eax-8-" << STABLE[idest].offset << "],edx" << endl;
			}
			//outasmtmp << "\tmov\tdword ptr [eax],edx" << endl;
			//movEDXtoTarget(idest);
			break;
		case STEAX:
			idest = locate(CODE[p].dest);	//	//优化前：只会是同一层的临时变量，charvar 或者 numvar
			outasmtmp << "\tmov\tedx,eax" << endl;
			movEDXtoTarget(idest);
			//outasmtmp << "\tmov\tdword ptr [ebp-" << STABLE[idest].offset << "],eax" << endl;
			break;
		case BEQ:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			cmpIncondition(i1, i2);
			outasmtmp << "\tje\t" << CODE[p].dest << endl;
			break;
		case BNE:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			cmpIncondition(i1, i2);
			outasmtmp << "\tjne\t" << CODE[p].dest << endl;
			break;
		case BGE:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			cmpIncondition(i1, i2);
			outasmtmp << "\tjge\t" << CODE[p].dest << endl;
			break;
		case BGT:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			cmpIncondition(i1, i2);
			outasmtmp << "\tjg\t" << CODE[p].dest << endl;
			break;
		case BLE:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			cmpIncondition(i1, i2);
			outasmtmp << "\tjle\t" << CODE[p].dest << endl;
			break;
		case BLT:
			i1 = locate(CODE[p].op1);
			i2 = locate(CODE[p].op2);
			cmpIncondition(i1, i2);
			outasmtmp << "\tjl\t" << CODE[p].dest << endl;
			break;
		case JMP:
			outasmtmp << "\tjmp\t" << CODE[p].dest << endl;
			break;
		case ELB:
			outasmtmp << CODE[p].dest << ":" << endl;
			break;
		case PARA:
			i1 = locate(CODE[p].op1);
			//将操作数1加载到edx
			loadOp1ToEDX(i1);
			outasmtmp << "\tpush\tedx" << endl;
			break;
		case PARAQ:
			i1 = locate(CODE[p].op1);
			//将操作数1加载到edx
			if (STABLE[i1].type == CHARVAR || STABLE[i1].type == NUMVAR){
				if(STABLE[i1].level == 0 && i1 <= globalvartop)
					outasmtmp << "\tlea\tedx,[" << STABLE[i1].name << "]" << endl;
				else if (STABLE[i1].level == lvl)
					outasmtmp << "\tlea\tedx,[ebp-" << STABLE[i1].offset << "]" << endl;
				else{
					loadexternVar(i1);//将外层变量的所在层地址加载到eax
					outasmtmp << "\tlea\tedx,[eax-8-" << STABLE[i1].offset << "]" << endl;
				}
			}
			else{//(STABLE[i1].type == CHARREF || STABLE[i1].type == NUMREF)
				if (STABLE[i1].level == lvl)
					outasmtmp << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
				else{
					loadexternVar(i1);
					outasmtmp << "\tmov\teax,[eax-8-" << STABLE[i1].offset << "]" << endl;
				}
				outasmtmp << "\tmov\tedx,eax" << endl;
			}
			outasmtmp << "\tpush\tedx" << endl;
			break;
		case CALL:
			idest = locate(CODE[p].dest);
			lvldist = lvl - STABLE[idest].level;
			if (lvl == 0 && lvldist == 0){
				outasmtmp << "\tmov\teax,ebp" << endl;
				outasmtmp << "\tpush\teax" << endl;			//SL
			}
			else if (lvldist == 0){		//调用嵌套的子程序
				outasmtmp << "\tlea\teax,[ebp+8]" << endl;
				outasmtmp << "\tpush\teax" << endl;			//SL
			}
			else{
				outasmtmp << "\tmov\teax,[ebp+8]" << endl;
				lvldist--;
				while (lvldist > 0){
					outasmtmp << "\tmov\teax,[eax]" << endl;
					lvldist--;
				}
				outasmtmp << "\tpush\teax" << endl;			//SL
			}
			outasmtmp << "\tcall\t" << STABLE[idest].plink->addr << endl;
			paraoffset = STABLE[idest].plink->paranum + 1;	//SL
			paraoffset <<= 2;
			outasmtmp << "\tadd\tesp," << paraoffset << endl;
			break;
		case INI:
			idest = locate(CODE[p].dest);
			outasmtmp << "\tpush\tebp" << endl;
			outasmtmp << "\tmov\tebp,esp" << endl;
			outasmtmp << "\tsub\tesp," << (STABLE[idest].plink->varsize) << endl;
			outasmtmp << "\tpush\tebx" << endl;
			outasmtmp << "\tpush\tedi" << endl;
			outasmtmp << "\tpush\tesi" << endl;
			break;
		case RET:
			if (BItop > 0){
				i1 = locate(STABLE[BlockIndex[BItop]-1].name + "_retv");
				if (i1 > 0){
					outasmtmp << "\tmov\teax,[ebp-" << STABLE[i1].offset << "]" << endl;
				}
			}
			outasmtmp << "\tpop\tesi" << endl;
			outasmtmp << "\tpop\tedi" << endl;
			outasmtmp << "\tpop\tebx" << endl;
			outasmtmp << "\tmov\tesp,ebp" << endl;
			outasmtmp << "\tpop\tebp" << endl;
			outasmtmp << "\tret" << endl;
			break;
		case WRT:
			//outimcode << "\t\tWRT\t" << CODE[p].op2 << endl;
			if (CODE[p].op1 == ""){		//表达式
				i2 = locate(CODE[p].op2);
				loadOp1ToEDX(i2);
				if (STABLE[i2].type <= CHARREF){		//字符
					outasmtmp << "\tpush\tedx" << endl;
					outasmtmp << "\tpush\toffset print_char" << endl;
					outasmtmp << "\tcall\tcrt_printf" << endl;
					outasmtmp << "\tadd\tesp,8" << endl;
					//outasmtmp << "\tinvoke\tcrt_printf, addr print_char, edx" << endl;
				}
				else{									//整型
					outasmtmp << "\tpush\tedx" << endl;
					outasmtmp << "\tpush\toffset print_int" << endl;
					outasmtmp << "\tcall\tcrt_printf" << endl;
					outasmtmp << "\tadd\tesp,8" << endl;
					//outasmtmp << "\tinvoke\tcrt_printf, addr print_int, edx" << endl;
				}
			}
			else{			//字符串
				int suffix = strCstNo;
				string astr = CODE[p].op1;
				for(int k=0; k<strCstNo;k++){			//这里查找是否有相同的字符串常量
					if(astr == strConst[k]){
						suffix = k;
						break;
					}
				}
				if(suffix >= strCstNo){
					suffix = strCstNo;
					if(strCstNo >= STRCONSTMAX){
						printf("字符串常量太多，被迫终止编译程序");
						closefiles();
						exit(0);
					}
					strConst[suffix] = CODE[p].op1;
					strCstNo++;
					outfinalcode << "\tZIFUC_"<< suffix << "\t\tdb\t\t\"" << CODE[p].op1 << "\",0" << endl;
				}
				outasmtmp << "\tpush\toffset ZIFUC_" << suffix << endl;
				outasmtmp << "\tpush\toffset print_str" << endl;
				outasmtmp << "\tcall\tcrt_printf" << endl;
				outasmtmp << "\tadd\tesp,8" << endl;
				//outasmtmp << "\tinvoke\tcrt_printf, addr print_str,offset ZIFUC_" << suffix << endl;
			}
			break;
		case RED:
			idest = locate(CODE[p].dest);
			if (STABLE[idest].type == CHARREF || STABLE[idest].type == NUMREF){
				//hold,此处还应考虑全局变量（也就是lvl0的变量）全局变量最后一个参数格式为：offset + 名字
				if (STABLE[idest].level == lvl)
					outasmtmp << "\tmov\tedx,[ebp-" << STABLE[idest].offset << "]" << endl;
				else{
					loadexternVar(idest);
					outasmtmp << "\tmov\tedx,[eax-8-" << STABLE[idest].offset << "]" << endl;
				}
				outasmtmp << "\tpush\tedx" << endl;
				if (STABLE[idest].type == CHARREF)
					outasmtmp << "\tpush\toffset read_char" << endl;
					//outasmtmp << "\tinvoke\tcrt_scanf, addr read_char,edx" << endl;
				else
					outasmtmp << "\tpush\toffset read_int" << endl;
					//outasmtmp << "\tinvoke\tcrt_scanf, addr read_int,edx" << endl;
				outasmtmp << "\tcall\tcrt_scanf" << endl;
				outasmtmp << "\tadd\tesp,8" << endl;
			}
			else{//把变量地址加载到edx
				if(STABLE[idest].level == 0 && idest <= globalvartop)		//全局变量
					outasmtmp << "\tlea\tedx,[" << STABLE[idest].name << "]" << endl;
				else if (STABLE[idest].level == lvl)
					outasmtmp << "\tlea\tedx,[ebp-" << STABLE[idest].offset << "]" << endl;
				else{
					loadexternVar(idest);
					outasmtmp << "\tlea\tedx,[eax-8-" << STABLE[idest].offset << "]" << endl;
				}
				outasmtmp << "\tpush\tedx" << endl;
				if (STABLE[idest].type == CHARVAR)
					outasmtmp << "\tpush\toffset read_char" << endl;
					//outasmtmp << "\tinvoke\tcrt_scanf, addr read_char,edx" << endl;
				else
					outasmtmp << "\tpush\toffset read_int" << endl;
					//outasmtmp << "\tinvoke\tcrt_scanf, addr read_int,edx" << endl;
				outasmtmp << "\tcall\tcrt_scanf" << endl;
				outasmtmp << "\tadd\tesp,8" << endl;
			}
			break;
		default:
			outasmtmp << "Something Wrong!" << endl;
			break;
		}
		p++;
	}
}
