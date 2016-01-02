#include "datastructure.h"
using namespace std;

ifstream  in;
ifstream  inasmtmp;					// 从中间代码文件中读取指令
ofstream  outimcode;				// 中间代码文件
ofstream  outasmtmp;				// 目标代码临时文件
ofstream  outfinalcode;				// 目标代码最终文件

char ch;							// a char in the getch() function
SYMTYPE sym;						// the type of the last token
string id;							// the last identifer read by far
int digits;							// the last number read by far
int cc;								// char pointer in line
int lnum;							// No. of line
int ll;								// length of a line
int err;							// error number in total.
string line;						// the line buffer in getch() function
string tokenbuf;					// the token buffer in getsym() function

int tp;								// pointer in symbol table
int lvl;							// current level
int curOffset;						// 本过程当前的变量偏移
int cx;								// point out where the next instruction will be stored
SYMITEM STABLE[TABLENMAX];			// the symbol table
int hashtable[TABLENMAX];			// the hash table for symbol table
int BlockIndex[LVMAX];				// 分程序索引表
int BItop;							// 分程序索引表里的栈顶指针
int globalvartop;					// 所有全局变量在符号表中最大的下标

IMC CODE[CODEASIZE];				// 中间代码数组
int labelNo;						// LABEL No.
string strConst[STRCONSTMAX];		// 字符串常量表
int strCstNo;						// 字符串常量表下标

set<SYMTYPE>  DECLARAFIRST;			// 声明部分FIRST集
set<SYMTYPE>  STMTFIRST;			// 语句部分FIRST集
set<SYMTYPE>  FACTORFIRST;			// 因子FIRST集
set<SYMTYPE>  RELATIONALOP;			// 比较运算符号集

string RSRVWORD[NRSRVW];			// 保留字表
string RSRVWORDT[NRSRVW];			// 保留字表相对应的保留字类型
char SSYMT[18];


void closefiles(){
	in.close();
	inasmtmp.close();
	outimcode.close();
	outasmtmp.close();
	outfinalcode.close();
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
		closefiles();
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
	if (hashtable[index] < 0)
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


void constdeclaration(set<SYMTYPE> &fsys){
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
}

void vardeclaration(int &offset, set<SYMTYPE> &fsys){
	int nvar = 0;
	int k = 0;
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
									for(k = nvar-1;k >= 0;k--){	//回填符号表
										int x = tp-k;
										STABLE[x].type = ARRAY;
										STABLE[x].alink = new arrayinfo;
										STABLE[x].alink->size = upperbd;
										STABLE[x].alink->elementt = vart;
									}
									if(lvl==0){
										for(k = nvar-1; k>=0;k--)
											outasmtmp << "\t" << STABLE[tp - k].name << "\t\tdd\t\t" << upperbd 
											<< "\t\tdup(?)" << endl;
									}
									else{
										for(k = nvar-1;k>=0;k--){
											STABLE[tp-k].offset = offset;
											offset += upperbd << 2;
										}
									}
									nvar = -1;
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
			if(nvar > 0){			//回填符号表
				//此处忽略了数组声明的情况，是因为如果上面已经对数组声明填好了符号表，那么nvar < 0,如果出错，那么nvar将依然为0
				for(k=nvar-1;k >= 0;k--)
					STABLE[tp - k].type = vart;
				if (lvl == 0){
					for (k = nvar - 1; k >= 0; k--)
						outasmtmp << "\t" << STABLE[tp - k].name << "\t\tdd\t\t?" << endl;
				}
				else{
					for(k = nvar-1;k >= 0;k--){
						STABLE[tp - k].offset = offset;
						offset += 4;
					}
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
}

void parameterdec(bool isprocedure, set<SYMTYPE> &fsys){
	int paran = 0;
	int offset = -12;					//EBP 与 第一个参数之间隔了返回地址和SL
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
			if (paran == 0)
				reportError(45);							//过程或函数没有参数时无需加()
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
						atemp.alink = NULL;
						atemp.plink = NULL;
						atemp.link = -1;
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
			atemp.alink = NULL;
			atemp.plink = NULL;
			atemp.link = -1;
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
}



void callp(int &tmpindex,int pptr, set<SYMTYPE> &fsys){
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
					if (opname.substr(0, 2) == "t_")		//数组元素
						TMPPARA[pNo].instrT = PARA;
					else
						TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			} 
			else if (fpt == NUMREF && (apt==NUMVAR||apt==NUMREF)){
				TMPPARA[pNo].instrT = PARAQ;
				TMPPARA[pNo].op1 = opname;
			}
			else if (fpt == CHARVAR && (CHARVAR <= apt && apt <= CHARREF)){
				if(apt==CHARREF){		//something different
					if (opname.substr(0, 2) == "t_")		//数组元素
						TMPPARA[pNo].instrT = PARA;
					else
						TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else if (fpt == CHARREF && (apt==CHARVAR||apt==CHARREF)){
				TMPPARA[pNo].instrT = PARAQ;
				TMPPARA[pNo].op1 = opname;
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
	else if (sym == SEMICOLON||sym == ENDSYM){			//过程名称后面参数列表为空
		if (STABLE[pptr].plink->paranum != 0){
			reportError(25);			//函数或过程调用缺少参数
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
}

void callf(int &tmpindex, string &opname,int fptr, set<SYMTYPE> &fsys){
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
					//genImc(PARAQ,opname,"","");
					if (opname.substr(0, 2) == "t_")		//数组元素
						TMPPARA[pNo].instrT = PARA;
					else
						TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			} 
			else if (fpt == NUMREF && (apt==NUMVAR||apt==NUMREF)){
				TMPPARA[pNo].instrT = PARAQ;
				TMPPARA[pNo].op1 = opname;
			}
			else if (fpt == CHARVAR && (CHARVAR <= apt && apt <= CHARREF)){
				if(apt==CHARREF){		//something different
					if (opname.substr(0, 2) == "t_")		//数组元素
						TMPPARA[pNo].instrT = PARA;
					else
						TMPPARA[pNo].instrT = PARAQ;
					TMPPARA[pNo].op1 = opname;
				}
				else{
					TMPPARA[pNo].instrT = PARA;
					TMPPARA[pNo].op1 = opname;
				}
			}
			else if (fpt == CHARREF && (apt==CHARVAR||apt==CHARREF)){
				TMPPARA[pNo].instrT = PARAQ;
				TMPPARA[pNo].op1 = opname;
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
			reportError(25);			//函数或过程调用缺少参数
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
	atemp.alink = NULL;
	atemp.plink = NULL;
	atemp.link = -1;
	registe(atemp);
	curOffset += 4;
	genImc(STEAX,"","",opname);
}

IDTYPE factor(int &tmpindex,string &opname,set<SYMTYPE> &fsys){
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
						if( indexType < NUMCONST && NUMREF < indexType ){
							reportError(43);			//数组下标类型只能是整型
							recover(fsys);
							return (IDTYPE)0;
						}
						indexResult = opname;

						//在符号表中注册临时变量
						tmpindex++;
						SYMITEM atemp;
						atemp.name = "t_" + to_string(tmpindex);
						opname = atemp.name;
						if(factorType == NUMVAR)
							atemp.type = NUMREF;
						else
							atemp.type = CHARREF;
						factorType = atemp.type;
						atemp.level = lvl;
						atemp.offset = curOffset;
						atemp.alink = NULL;
						atemp.plink = NULL;
						atemp.link = -1;
						registe(atemp);
						curOffset += 4;
						genImc(LA,STABLE[i].name,indexResult,opname);//生成LOAD指令，将数组元素所在地址存到临时变量（引用）
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
			atemp.alink = NULL;
			atemp.plink = NULL;
			atemp.link = -1;
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

IDTYPE term(int &tmpindex,string &opname,set<SYMTYPE> &fsys){
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
		atemp.alink = NULL;
		atemp.plink = NULL;
		atemp.link = -1;
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

IDTYPE expression(int &tmpindex,string &opname,set<SYMTYPE> &fsys){
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
			atemp.alink = NULL;
			atemp.plink = NULL;
			atemp.link = -1;
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
		atemp.alink = NULL;
		atemp.plink = NULL;
		atemp.link = -1;
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
	if ((dest == NUMVAR || dest == NUMREF) && (CHARVAR <= src && src <= NUMREF))
		return true;
	else if ((dest == CHARVAR || dest == CHARREF) && (CHARVAR <= src && src <= CHARREF))
		return true;
	else
		return false;
}

void assignment(int &tmpindex,int i, set<SYMTYPE> &fsys){
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
			else
				genImc(MOV,opname,"",STABLE[i].name+"_retv");
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
			set<SYMTYPE> fsys2(fsys);
			fsys2.insert(RSQBSYM);
			indexType = expression(tmpindex,opname,fsys2);
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
				if (!exprTypecheck(STABLE[i].alink->elementt, exprType))
					reportError(42);
				else{
					//根据前面记录下的indexResult(也就是数组下标)生成STORE指令
					genImc(MOVA, opname, indexResult, STABLE[i].name);
					//			 等式右边,数组下标    ,数组名字
				}
			}
			else{
				reportError(23);
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
}

void condition(int &tmpindex,bool isIF,set<SYMTYPE> &fsys){
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
				case GEQ:	genImc(BLT, op1, opname, ""); break;
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
}


void read(set<SYMTYPE> &fsys){
	int i;
	if (sym == LPAREN){
		do{
			getsym();
			set<SYMTYPE> fsys2(fsys);
			fsys2.insert(RPAREN);
			if(sym!=IDF)
				break;
			i = locate(id);
			if (i < 0 ){
				reportError(11);		//未定义的标识符
				recover(fsys2);
				return;
			}
			else{
				IDTYPE idft = STABLE[i].type;
				if (idft == CHARCONST || idft == NUMCONST || (ARRAY <= idft && idft <= FUNCTION)){
					reportError(26);	//不可向常量或过程、函数以及数组整体赋值
					recover(fsys2);
					break;
				}
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
}

void write(int &tmpindex,set<SYMTYPE> &fsys){
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
}

void statement(int &tmpindex,set<SYMTYPE> &fsys){
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
ELSE1:		if (sym == ELSESYM){
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
				//fsys2.erase(SEMICOLON);
				//recover(fsys2);
				//fsys2.insert(SEMICOLON);
				//if (sym == ELSESYM){
				//	reportError(46);								//ELSE前的符号不合法
				//	goto ELSE1;
				//}
				//else
				recover(fsys2);
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
	}
	else if (sym == DOSYM){
		string lbname = "LABEL" + to_string(labelNo);
		genImc(ELB,"","",lbname);			//生成LABEL
		labelNo++;
		getsym();
		set<SYMTYPE> fsys2(fsys);
		fsys2.insert(WHILESYM);
		statement(tmpindex,fsys2);
WHILE2:	if (sym == WHILESYM){
			getsym();
			condition(tmpindex,false,fsys);	//condition([dosym]+fsys);
			CODE[cx-1].dest = lbname;		//填跳转地址，跳转回 do 后面的语句
		}
		else{
			fsys2.erase(SEMICOLON);
			recover(fsys2);
			fsys2.insert(SEMICOLON);
			if(sym==WHILESYM){
				reportError(48);			//WHILE前的符号不合法
				goto WHILE2;
			}
			else
				reportError(30);			//do while语句缺少while
			return;
		}
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
				if (idft == NUMVAR || idft == NUMREF){		//循环变量只能是整型变量或整型引用
					assignment(tmpindex,i,fsys);
					if (sym == DOWNTOSYM || sym == TOSYM){
						SYMTYPE DWNRORTO = sym;
						getsym();
						set<SYMTYPE> fsys2(fsys);
						fsys2.insert(DOSYM);
						expression(tmpindex, opname, fsys2);
						if(opname.substr(0,2) != "t_"){		//如果表达式结果变量不是临时变量，将用临时变量存储
							tmpindex ++;
							SYMITEM atemp;
							atemp.name = "t_" + to_string(tmpindex);
							atemp.type = NUMVAR;
							atemp.level = lvl;
							atemp.offset = curOffset;
							atemp.alink = NULL;
							atemp.plink = NULL;
							atemp.link = -1;
							registe(atemp);
							curOffset += 4;
							genImc(MOV,opname,"",atemp.name);
							opname = atemp.name;
						}
						string lbname = "LABEL" + to_string(labelNo);
						genImc(ELB, "", "", lbname);			//生成LABEL
						labelNo++;
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

void Block(int btp,int offset, set<SYMTYPE> &fsys){
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
					constdeclaration(fsys2);
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
			if(lvl==0)
				outasmtmp << ".data?\n" << endl;
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
		if (lvl == 0){
			globalvartop = tp;
			outasmtmp << "\n.code\n\nSTART:\n" << endl;
			outasmtmp << "\tjmp\tLABEL0" << endl;
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
				reportError(28);			//过程或函数声明语句结尾缺少";"
				recover(fsys);
			}
			set<SYMTYPE> fsys2(fsys);
			fsys2.insert(SEMICOLON);
			if(isprocedure)
				Block(abtp,4,fsys2);				//block(lev+1,tx,[semicolon]+fsys);
			else
				Block(abtp,8,fsys2);				//函数的返回值放在第一个位置
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
	set<SYMTYPE> fsys2(fsys);
	fsys2.insert(SEMICOLON);	fsys2.insert(ENDSYM);
	curOffset = offset;
	statement(tmpindex,fsys2);		//statement( [semicolon,endsym]+fsys);
	STABLE[btp].plink->varsize = curOffset - 4;		//这时的curOffset比实际需要的多4
	//回填申请空间, 计算方法应为 offset + tmpindex*4 + display区等等，实际上就等于
	genImc(RET, "", "", "");
	cxsted = cx;
	fsys2.clear();
	test(fsys,fsys2,39);
	genAsm(cxbg,cxbg+2);
	genAsm(cxstbg+1,cxsted);
	//hold,删除当前子活动记录，并删除符号表中的本活动记录的内容
	popstable();
}


void initial(){
	/*
	enum SYMTYPE
	{
	DQUOSYM = 0,	SQUOSYM,	LPAREN,		RPAREN,
	TIMES,			PLUS ,		COMMA,		MINUS,
	PERIOD,			SLASH,		SEMICOLON,	EQL,
	LSQBSYM,		RSQBSYM,	LSS,		LEQ,
	GTR,			GEQ,		BECOMES,	COLON,
	NEQ,			BEGINSYM,	ENDSYM,		IFSYM,
	THENSYM,		ELSESYM,	INTSYM,		CHARSYM,
	ARRAYSYM,		OFSYM,		WHILESYM,	DOSYM,
	FORSYM,			DOWNTOSYM,	TOSYM,		CONSTSYM,
	VARSYM,			PROCSYM,	FUNCSYM,	READSYM,
	WRITESYM,		IDF,		NUMBER,		NUL
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

//下面是对目标代码文件的初始化
outfinalcode << ".386\n\
.model flat, stdcall\n\
option casemap : none\n\n\
include \\masm32\\include\\windows.inc\n\
include \\masm32\\include\\user32.inc\n\
include \\masm32\\include\\kernel32.inc\n\
include \\masm32\\include\\masm32.inc\n\
include \\masm32\\include\\msvcrt.inc\n\n\
includelib \\masm32\\lib\\user32.lib\n\
includelib \\masm32\\lib\\kernel32.lib\n\
includelib \\masm32\\lib\\masm32.lib\n\
includelib \\masm32\\lib\\msvcrt.lib\n\
include \\masm32\\macros\\macros.asm\n\n" << endl;

//打印读取指令必要的变量
outfinalcode << ".data\n\
\tprint_char\t\tdb\t\"%c\",0\n\
\tprint_int\t\tdb\t\"%d\",0\n\
\tprint_str\t\tdb\t\"%s\",0\n\
\tread_char\t\tdb\t\"%c\",0\n\
\tread_int\t\tdb\t\"%d\",0" << endl;		//接下来是常量字符串
}



int main(){
	

#ifndef DEBUG
	char filename[20];
	printf("Please input the source program file name : ");
	scanf("%s", filename);
	in.open(filename);
#endif // !DEBUG
	
#ifdef DEBUG
	in.open("a.txt");
#endif // DEBUG

	outimcode.open("imcode.txt");
	outfinalcode.open("D:\\masm32\\Hello\\PL0code.asm");
	outasmtmp.open("temp.txt");

	initial();
	ch = ' ';
	tp = -1;
	getsym();
	SYMITEM amain;
	amain.name = "_main";
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
	Block(0, 4, fsys);
	locate("abcdefsgef");
	if (sym != PERIOD)
		reportError(0);
	if (err == 0){
		printf("The compiler work is done!\n");
		outimcode << endl;
		outimcode << "The Immediate Code:" << endl;
		listImc();
	}
	else
		printf("ERRORS IN YOUR PROGRAM\n");
	outasmtmp << "end START" << endl;
	in.close();
	outimcode.close();
	outasmtmp.close();
	//再把outasmtmp中的内容复制到outfinalcode中
	inasmtmp.open("temp.txt");
	while(getline(inasmtmp,line)){
		outfinalcode << line << endl;
	}
	outfinalcode.close();
	//复制完毕
	char asdc = getchar();
	asdc = getchar();
	return 0;
}