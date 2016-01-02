#include "datastructure.h"
using namespace std;
extern ofstream  outimcode;

extern string id;
extern int lnum, cc, err;


extern SYMTYPE sym;						// the type of the last token

void reportError(int errorcode){

	string errorinfo;
	//abc = 1;
	switch (errorcode) {
	case 0:
		errorinfo = "程序末尾没有\".\"";break;
	case 1:
		errorinfo = "\"" + id + "\"" + "is defined duplicately!"; break;
	case 2:
		errorinfo = "不可向常量或过程、函数赋值"; break;
	case 3:
		errorinfo = "常量定义中等式右边出现 [+|-]时应该接数字"; break;
	case 4:
		errorinfo = "常量定义只能是无符号整数或者字符"; break;
	case 5:
		errorinfo = "常量定义中应该使用\"=\""; break;
	case 6:
		errorinfo = "常量或变量定义中const、var或者\",\"后面应该是标识符"; break;
	case 7:
		errorinfo = "数组元素类型不是基本类型"; break;
	case 8:
		errorinfo = "数组声明过程中缺少of"; break;
	case 9:
		errorinfo = "数组定义或使用缺少\"[\"或\"]\""; break;
	case 10:
		errorinfo = "数组声明过程中定义大小必须用无符号整数"; break;
	case 11:
		errorinfo = "\"" + id + "\"" + "is not defined!"; break;
	case 12:
		errorinfo = "变量类型未指定"; break;
	case 13:
		errorinfo = "变量声明没有以冒号结尾"; break;
	case 14:
		errorinfo = "变量声明过程中指定变量类型时缺少\":\""; break;
	case 15:
		errorinfo = "参数类型只能是integer或者char"; break;
	case 16:
		errorinfo = "参数声明中没有指定参数名字"; break;
	case 17:
		errorinfo = "参数个数超过上限"; break;
	case 18:
		errorinfo = "函数定义中返回值类型不是基本类型"; break;
	case 19:
		errorinfo = "没有定义函数的返回值类型"; break;
	case 20:
		errorinfo = "函数或过程声明出现错误"; break;
	case 21:
		errorinfo = "此处缺少\")\""; break;
	case 22:
		errorinfo = "过程调用语句不能用作表达式中的因子"; break;
	case 23:
		errorinfo = "赋值语句缺少\":=\""; break;
	case 24:
		errorinfo = "无法识别的关系运算符"; break;
	case 25:
		errorinfo = "过程调用缺少参数"; break;
	case 26:
		errorinfo = "不可向常量或过程、函数以及数组整体赋值"; break;
	case 27:
		errorinfo = "if语句缺少then"; break;
	case 28:
		errorinfo = "语句之间缺少\";\""; break;
	case 29:
		errorinfo = "此处应为分号或end"; break;
	case 30:
		errorinfo = "do while语句缺少while"; break;
	case 31:
		errorinfo = "for 后面应该有循环变量"; break;
	case 32:
		errorinfo = "循环变量类型不正确"; break;
	case 33:
		errorinfo = "for 循环语句中缺少 to 或者downto"; break;
	case 34:
		errorinfo = "for 循环语句中缺少 do"; break;
	case 35:
		errorinfo = "表达式不能以此符号开始"; break;
	case 36:
		errorinfo = "因子后不能接此符号"; break;
	case 37:
		errorinfo = "过程说明后的符号不正确"; break;
	case 38:
		errorinfo = "此处应为语句"; break;
	case 39:
		errorinfo = "语句后的符号不正确"; break;
	case 40:
		errorinfo = "嵌套层次太深，本宝宝受不了啦"; break;
	case 41:
		errorinfo = "参数个数或类型错误"; break;
	case 42:
		errorinfo = "赋值语句等号两边类型不匹配"; break;
	case 43:
		errorinfo = "数组下标类型只能是整型"; break;
	case 44:
		errorinfo = "条件表达式中比较运算符两边的类型不正确"; break;
	case 45:
		errorinfo = "过程或函数没有参数时无需加()"; break;
	case 46:
		errorinfo = "ELSE前的符号不合法";break;
	case 47:
		errorinfo = "此立即数太大！"; break;
	case 48:
		errorinfo = "WHILE前的符号不合法";break;
	default:
		errorinfo = "Other Error !";
	}
	cout << "Error " << errorcode << " : " << errorinfo << endl;
	cout << "\t" << "line " << lnum << ", column " << cc << " !" << endl;
	err++;
}

void test(set<SYMTYPE> &s1, set<SYMTYPE> &s2, int errorcode){
	if (s1.count(sym) < 0){
		reportError(errorcode);
		s1.insert(s2.begin(), s2.end());
		while (s1.count(sym) < 1){
			getsym();
		}
	}
}

void recover(set<SYMTYPE> &s){
	if (s.count(sym) < 1){
		while (s.count(sym) < 1){
			getsym();
		}
	}
}