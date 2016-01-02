#include "datastructure.h"

std::ifstream  in;
std::ofstream  out;
char ch;							// a char in the getch() function
SYMTYPE sym;						// the type of the last token
std::string id;						// the last identifer read by far
int num;							// the last number read by far
int cc;								// pointer in line
int lnum;							// No. of line
int ll;								// length of a line
int kk, err;
int tp = 0;								// pointer in symbol table
int cx;								// point out where the next instruction will be stored
std::string line;					// the line buffer in getch() function
std::string a;						// the token buffer in getsym() function
SYMITEM STABLE[TABLENMAX];			// the symbol table
int hashtable[TABLENMAX];	// the hash table for symbol table


void reportError(int errorcode){
	out << "Error " << errorcode << ": line" << lnum << ", column " << cc << " !" << std::endl;
	std::string errorinfo;
	//abc = 1;
	switch (errorcode) {
	case 0:
		errorinfo = "\"" + id + "\"" +"is not defined!"; break;
		case 1:
			errorinfo = "\"" + id + "\"" + "is defined duplicately!"; break;
		/*case 2:
		case 3:
		case 4:
		case 5:*/
		default:
			errorinfo = "Other Error !";
	}
	std::cout << errorinfo <<" The errorcode is :"<< errorcode << std::endl;
	err++;
	out.close();
}

void getsymt(char ss){
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
		if (!getline(in, line) && sym != PERIOD)
		{
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
		a = "";
		while (islower(ch) || isupper(ch) || isdigit(ch)){
			a += ch;
			getch();
		}
		k = a.length();
		if (k > IDLENMAX){		//标识符过长
			printf("The identifier is too long!");
			in.close();
			exit(0);
		}
		id = a;
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
		a = "";
		k = 0;
		num = 0;
		sym = NUMBER;
		while (isdigit(ch) && k <= NUMLENMAX){
			a += ch;
			num = num * 10 + (ch - '0');
			k++;
			getch();
		}
		if (k > NUMLENMAX){
			reportError(30);
			while (isdigit(ch)){
				getch();
			}
			num = 0;
			//这里是直接把后面的数字舍弃，然后置num为0
		}
	}
	else if (ch == ':'){
		a = ch;
		getch();
		if (ch == '='){
			a += ch;
			sym = BECOMES;
			getch();
		}
		else sym = COLON;
	}
	else if (ch == '<'){
		a = ch;
		getch();
		if (ch == '='){
			a += ch;
			sym = LEQ;
			getch();
		}
		else if (ch == '>'){
			a += ch;
			sym = NEQ;
			getch();
		}
		else
			sym = LSS;
	}
	else if (ch == '>'){
		a = ch;
		getch();
		if (ch == '='){
			a += ch;
			sym = GEQ;
			getch();
		}
		else
			sym = GTR;
	}
	else if (ch == '\''){
		a = "";
		getch();
		if (islower(ch) || isupper(ch) || isdigit(ch)){
			a += ch;
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
		a = "";
		getch();
		while (ch == ' ' || ch == '!' || ('$' <= ch && ch <= '~')){
			a += ch;
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
		a = ch;
		getsymt(ch);
		getch();
	}
}

void genImc(){
	//
}

//hash表，便于查询符号表
int hash(std::string name){
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
	int index = hash(item.name);
	if (hashtable[index] != 0){
		STABLE[tp].link = &STABLE[hashtable[index]];
		hashtable[index] = tp;
	}
	else
		hashtable[index] = tp;
}

//查询符号表
int locate(std::string name){
	int index = hash(name);
	if (hashtable[index] == 0)
		return -1;
	else
		return hashtable[index];
}

void test(SYMTYPE s1[],SYMITEM s2[], int errorcode ){
	if (sym != s1[1]){
		reportError(201);
		//hold
		//s1 = s1 + s2;
		//跳过错误字符
	}
	else
		return;
}

void constdeclaration(int lvl,int offset){
	int flag = 1;
	if (sym == IDF){
		getsym();
		if (sym == EQL || sym == BECOMES){
			if (sym == BECOMES)
				reportError(2);		//you can't assign to a const
			//ignore the mindless mistake, continue our analysis work.
			getsym();
			if (sym == NUMBER || sym == PLUS || sym == MINUS){		//num
				if (sym == MINUS){
					//做好标记，需要将后面的常数取相反数
					flag = -1;
				}
				if(sym != NUMBER)
					getsym();
				if (sym == NUMBER){
					num *= flag;
					SYMITEM anumconst;
					anumconst.name = id;
					anumconst.type = NUMCONST;
					anumconst.level = lvl;
					anumconst.offset = offset;
					anumconst.alink = NULL;
					anumconst.plink = NULL;
					anumconst.link = NULL;
					//hold,此处还需检查是否重复定义
					registe(anumconst);
					getsym();
				}
				else
					reportError(100);			//常量定义中等式右边出现 [+|-]时应该接数字
			}
			else if (sym == ZIFU){
				SYMITEM azifuconst;
				azifuconst.name = id;
				azifuconst.type = CHARCONST;
				azifuconst.level = lvl;
				azifuconst.offset = offset;
				azifuconst.alink = NULL;
				azifuconst.plink = NULL;
				azifuconst.link = NULL;
				//hold,此处还需检查是否重复定义
				registe(azifuconst);
				getsym();
			}
			else
				reportError(101);		//常量定义中"="后面应该是数字或者字符
		}
		else
			reportError(102);			//常量定义中标识符后面应该是 "="
	}
	else
		reportError(4);					//常量定义中const或者","后面应该是标识符
	out << "This is a const declaration!" << std::endl;
}

void vardeclaration(int lvl, int offset){
	int nvar = 0;
	if (sym == IDF){
		while(sym == IDF){
			SYMITEM avar;
			avar.name = id;
			//avar.type 待定
			avar.level = lvl;
			avar.offset = offset;
			avar.alink = NULL;
			avar.plink = NULL;
			avar.link = NULL;
			//hold ,此处还需检查是否有重复定义
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
						int upperbd = num;
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
										STABLE[x].alink = new arrayinfo;
										STABLE[x].alink->size = upperbd;
										STABLE[x].alink->elementt = vart;
									}
									//分号将会在调用本函数的地方进行处理
								}
								else
									reportError(103);
							}
							else
								reportError(104);
						}
						else
							reportError(105);
					}
					else
						reportError(106);
				}
				else
					reportError(107);	//数组声明缺少"["
			}
			else{
				reportError(108);		//此处应出现integer或char或array
				return;					// hold
			}
			if(vart !=0){
				//此处忽略了数组声明的情况，是因为如果上面已经对数组声明填好了符号表，那么nvar < 0,如果出错，那么vart将依然为0
				for(nvar--;nvar >= 0;nvar--){
					STABLE[tp - nvar].type = vart;
				}
			}
			getsym();
			if (sym != SEMICOLON)
				reportError(109);		//变量声明没有以冒号结尾
			else
				getsym();				//读取下一个token为之后的处理做准备
		}
		else
			reportError(110);			//变量声明过程中指定变量类型时缺少":"
	}
	else
		reportError(4);					//变量定义中var或者","后面应该接标识符
	out << "This is a variable declaration!" << std::endl;
}

void parameterdec(int lvl,bool isprocedure){
	int paran = 0;
	int offset = 0;
	if (sym == LPAREN){
		getsym();
		while (sym == VARSYM){
			getsym();
			if (sym == IDF){
				while (sym == IDF){
					STABLE[tp].plink->paras[paran].name = id;
					STABLE[tp].plink->paras[paran].isVar = true;
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
						reportError(99);
						return;
					}
					for (int j = 0; j < paran; j++)
						STABLE[tp].plink->paras[j].type = vart;
					getsym();
				}
				else
					reportError(98);
			}
			else
				reportError(97);
		}
		while (sym == IDF){
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
					reportError(96);
					return;
				}
				for (int j = 0; j < paran; j++)
					STABLE[tp].plink->paras[j].type = vart;
			}
			else
				reportError(95);
		}
		//把参数填入符号表
		int pftp = tp;			//记录下正在处理的过程或函数在符号表中的位置
		for (int j = 0; j < paran; j++){
			SYMITEM apara;
			apara.name = STABLE[pftp].plink->paras[j].name;
			apara.type = STABLE[pftp].plink->paras[j].type;
			apara.level = lvl;
			apara.offset = offset++;
			apara.link = NULL;
			apara.alink = NULL;
			apara.plink = NULL;
			registe(apara);
			//tp++;
		}
		if (sym != RPAREN)
			reportError(94);
		else{
			if (!isprocedure){
				getsym();
				if (sym == COLON){
					getsym();
					if (sym == CHARSYM || sym == INTSYM){
						STABLE[pftp].plink->retvaluet = (sym == CHARSYM) ? CHARVAR : NUMVAR;
					}
					else
						reportError(95);
				}
				else
					reportError(94);
			}
		}
		getsym();
	}
	else
		reportError(91);
}

void callf(){
	//hold
	if (sym == LPAREN){
		getsym();
		do{
			expression();
			//参数个数、类型检查
			genImc();					//生成传递参数的指令
		} while (sym == COMMA);
	}
	else
		reportError(117);
	out << "This is a function call!" << std::endl;
}

void factor(){
	int i;		//标识符在符号表中的位置
	//hold, test(24);
	while (sym == IDF){				//hold
		if (sym == IDF){			//<标识符>|<标识符>'['<表达式>']'|<标识符>[<实在参数表>]
			i = locate(id);
			if (i < 0)
				reportError(111);
			else{
				if (STABLE[i].type == ARRAY){		//数组
					getsym();
					if (sym == LSQBSYM){
						getsym();
						if (sym == NUMBER){
							//RSRVWORDT[100] = NUL;
							genImc();
							getsym();
							if (sym == RSQBSYM)
								getsym();
							else
								reportError(112);
						}
						else
							reportError(113);
					}
					else
						reportError(114);
				}
				else if (STABLE[i].type == FUNCTION){
					callf();
					genImc();
				}
				else{
					if (STABLE[i].type != PROCEDURE){
						//常量或者变量
						genImc();
					}
					else
						reportError(115);
				}
			}
		}
		else if (sym == NUMBER){	//<无符号整数>
			genImc();
		}
		else if (sym == LPAREN){	//'('<表达式>')'
			getsym();
			expression();		//hold
			if (sym == RPAREN)
				getsym();
			else
				reportError(116);
		}
		//test(NULL, LPAREN,) hold
		getsym();
	}
}

void term(){
	SYMTYPE muldivop;
	factor();
	while (sym == TIMES || sym == SLASH){
		muldivop = sym;
		getsym();
		factor();
		if (muldivop == TIMES)
			genImc();		//生成乘法指令
		else
			genImc();		//生成出发指令
	}
}

void expression(){
	SYMTYPE addsubop;
	if (sym == PLUS || sym == MINUS){
		addsubop = sym;
		getsym();
		term();
		if (addsubop == MINUS)
			genImc();		//生成操作数栈栈顶取相反数指令
	}
	else
		term();
	while (sym == PLUS || sym == MINUS){
		addsubop = sym;
		getsym();
		term();
		if (addsubop == PLUS)
			genImc();		//生成操作数栈栈顶取与次栈顶相加指令
		else
			genImc();		//生成操作数栈栈顶取与次栈顶相减指令
	}
}

void assignment(int i){
	IDTYPE idft;
	idft = STABLE[i].type;
	if (idft == FUNCTION){
		getsym();
		if (sym == BECOMES){
			getsym();
			expression();
			genImc();
		}
		else
			reportError(87);
	}
	else if (idft == ARRAY){
		int index;
		getsym();
		if (sym == LSQBSYM){
			getsym();
			if (sym == NUMBER){
				index = num;
				getsym();
				if (sym != RSQBSYM)
					reportError(90);
				getsym();		//hold,这里的expression顺序问题
				if (sym == BECOMES){
					getsym();
					expression();
					genImc();
				}
				else
					reportError(87);
			}
			else
				reportError(89);
		}
		else
			reportError(88);
	}
	else{		//	var
		getsym();
		if (sym == BECOMES){
			getsym();
			expression();
			genImc();
		}
		else
			reportError(87);
	}
	out << "This is a assignment!" << std::endl;
}

void condition(){
	SYMTYPE relop;		//关系表达式
	expression();
	if (sym == EQL){// hold, NEQ,LSS,LEQ,GTR,GEQ
		relop = sym;
		getsym();
		expression();
		switch (relop){
			case EQL:	genImc(); break;
			case NEQ:	genImc(); break;
			case LSS:	genImc(); break;
			case LEQ:	genImc(); break;
			case GTR:	genImc(); break;
			case GEQ:	genImc(); break;
		}
	}
	else
		reportError(118);
	out << "This is a condition statement!" << std::endl;
}

void callp(int i){
	//hold
	if (sym == LPAREN){
		getsym();
		do{
			expression();
			//参数个数、类型检查
			genImc();					//生成传递参数的指令
		} while (sym == COMMA);
	}
	else
		reportError(117);
	out << "This is a procedure call!" << std::endl;
}

void read(){
	int i;
	if (sym == LPAREN){
		getsym();
		if (sym == IDF){
			do{
				i = locate(id);
				if (i < 0 )
					reportError(119);
				else{
					IDTYPE idft = STABLE[i].type;
					if (idft == CHARCONST || idft == NUMCONST || idft == PROCEDURE || idft == FUNCTION)
						reportError(120);
					else{
						genImc();
					}
				}
				getsym();
			} while (sym != COMMA);
		}
		else
			reportError(121);
	}
	else
		reportError(122);
	if (sym != RPAREN)
		reportError(123);
	getsym();
	out << "This is a read statement!" << std::endl;
}

void write(){
	if (sym == LPAREN){
		getsym();
		if (sym == ZIFUC){
			genImc();
			getsym();
			if (sym == RPAREN)
				return;
			else if (sym == COMMA){
				getsym();
				expression();
				genImc();
			}
		}
		else{
			expression();
			genImc();
		}
		if (sym != RPAREN)
			reportError(124);
		getsym();
	}
	else
		reportError(125);
	getsym();
	out << "This is a write statement!" << std::endl;
}

void statement(){
	int i,cx1,cx2;
	if (sym == IDF){		//有可能是赋值语句，或者过程调用语句
		i = locate(id);
		if (i < 0)
			reportError(126);
		else{
			IDTYPE idft = STABLE[i].type;
			if (idft == PROCEDURE){
				getsym();
				callp(i);
			}
			else if (idft == CHARCONST || idft == NUMCONST || idft == PROCEDURE)
				reportError(127);
			else{		//赋值
				assignment(i);
			}
		}
	}
	else if (sym == IFSYM){
		getsym();
		condition();						//condition([thensym,dosym]+fsys);
		if (sym == THENSYM){
			getsym();
			cx1 = cx;
			genImc();						//条件跳转到then后面的语句因此需要回填跳转地址,  gen(jpc,0,0);
			statement();					
			getsym();
			if (sym == ELSESYM){
				//code[cx1].a := cx               (* write back the target address of the jump code *)
				cx2 = cx;
				genImc();					//无条件跳转到ELSE后面的语句，gen(jmp,0,0)
				statement();
				//code[cx2].a := cx               (* write back the target address of the jump code *)
			}
			else{
				//code[cx1].a := cx               (* write back the target address of the jump code *)
			}

		}
		else
			reportError(129);
	}
	else if (sym == BEGINSYM){
		getsym();
		statement();						//statement([semicolon,endsym]+fsys);
		while (sym == SEMICOLON){// hold,这里还需要考虑其他情况
			if (sym != SEMICOLON)
				reportError(130);				//语句之前缺少分号
			else
				getsym();
			statement();					//statement([semicolon,endsym]+fsys)
		}
		if (sym == ENDSYM)
			getsym();
		else
			reportError(131);				//此处应为分号或end
		out << "This is a compound statement!" << std::endl;
	}
	else if (sym == DOSYM){
		cx1 = cx;
		getsym();
		statement();
		getsym();
		if (sym == WHILESYM){
			getsym();
			condition();					//condition([SEMICOLON]+fsys);
			genImc();						//gen(jpc,0,0);条件跳转,目标为cx1
		}
		else
			reportError(132);
		out << "This is a do while cycle!" << std::endl;
	}
	else if (sym == FORSYM){
		getsym();
		if (sym = IDF){
			i = locate(id);
			if (i < 0)
				reportError(133);
			else{
				IDTYPE idft = STABLE[i].type;
				if (idft == CHARVAR || idft == NUMVAR || idft == CHARPARA || idft == NUMPARA){
					assignment(i);
					getsym();
					if (sym == DOWNTOSYM || sym == TOSYM){
						cx1 = cx;
						getsym();
						expression();		//dosym;
						cx2 = cx;
						genImc();			//生成比较指令，然后再生成条件跳转指令
						if (sym == DOSYM){
							getsym();
							statement();	//sys;
							genImc();		//生成无条件跳转指令，跳回for循环比较条件
							////code[cx].a := cx1
							//code[cx2].a := cx               (* write back the target address of the jump code *)
						}
						else
							reportError(134);
					}
					else
						reportError(135);
				}
				else
					reportError(136);
				getsym();
			}
		}
		else
			reportError(137);
		out << "This is a for cycle!" << std::endl;
	}
	else if (sym == READSYM){
		read();
	}
	else if (sym == WRITESYM){
		write();
	}
	//test()  hold
}

void subprogram(int lvl){
	int dx = 3, tx0;
	int offset = 0;						//hold,Offset不一定是从0开始，因为还有实参
	tx0 = tp;
	//STABLE[tp].plink->addr = cx;		//just a temporary value
	genImc();							//无条件跳转 { jump from declaration part to statement part }
	if (lvl > LVMAX)
		reportError(138);
	do{
		if (sym == CONSTSYM){
			getsym();
			do{
				constdeclaration(lvl,offset++);
				while (sym == COMMA){
					getsym();
					constdeclaration(lvl, offset++);
				}
				if (sym == SEMICOLON)
					getsym();
				else
					reportError(139);
			} while (sym == IDF);
		}
		if (sym == VARSYM){
			getsym();
			do{
				vardeclaration(lvl,offset++);
				if (sym == PROCSYM || sym == FUNCSYM || sym == BEGINSYM)
					break;
				/*while (sym == SEMICOLON){
					getsym();
					if (sym == PROCSYM || sym == FUNCSYM || sym == BEGINSYM)
						break;
					else
						vardeclaration(lvl, offset++);
				}
				if (sym == PROCSYM || sym == FUNCSYM || sym == BEGINSYM){
					break;
				}
				else
					reportError(140);*/
			} while (sym == IDF);
		}
		while (sym == PROCSYM||sym==FUNCSYM){
			bool isprocedure;
			if (sym == PROCSYM)
				isprocedure = true;
			else
				isprocedure = false;
			getsym();
			if (sym == IDF){
				SYMITEM aporf;
				aporf.name = id;
				aporf.type = (isprocedure) ? PROCEDURE : FUNCTION;
				aporf.level = lvl;
				aporf.plink = new pfinfo;
				aporf.link = NULL;
				aporf.alink = NULL;
				registe(aporf);
				getsym();
				parameterdec(lvl+1,isprocedure);		//顺带把参数填进符号表
			}
			if (sym == SEMICOLON)
				getsym();
			else
				reportError(140);
			subprogram(lvl + 1);			//block(lev+1,tx,[semicolon]+fsys);
			if (sym == SEMICOLON){
				getsym();
				//test();			test( statbegsys+[ident,procsym],fsys,6)
			}
			else
				reportError(141);
		}
		//test();		test( statbegsys+[ident],declbegsys,7)
	} while (sym == CONSTSYM || sym == VARSYM || sym == PROCEDURE || sym == FUNCSYM);
	//code[table[tx0].adr].a : = cx;
	//STABLE[tp].plink->addr = cx;
	genImc();
	statement();		//statement( [semicolon,endsym]+fsys);
	//test();
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
}

int main(){
	
	int i = 1;

	initial();

	char filename[20];
	//printf("Please input the source program file name : ");
	//scanf("%s", filename);
	in.open("a.txt");
	out.open("result.txt");
	i = 1;
	ch = ' ';
	getsym();
	SYMITEM amain;
	amain.name = "main";
	amain.type = PROCEDURE;
	amain.level = 0;
	amain.plink = new pfinfo;
	amain.link = NULL;
	amain.alink = NULL;
	registe(amain);
	subprogram(0);
	if (sym != PERIOD)
		reportError(142);
	if (err == 0){
		printf("The compiler work is done!");
	}
	else
		printf("ERRORS IN YOUR PROGRAM");
	/*out << i;
	out << ' ' << RSRVWORDT[(int)sym] << ' ' << a << std::endl;
	while (sym != PERIOD){
		i++;
		getsym();
		out << i;
		out << ' ' << RSRVWORDT[(int)sym] << ' ' << a << std::endl;
	}*/
	char asdc = getchar();
	asdc = getchar();
	asdc = getchar();
	asdc = getchar();
	in.close();
	out.close();

	return 0;
}
