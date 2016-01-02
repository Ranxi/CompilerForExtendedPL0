program pl0(input, output, fin);  { version 1.0 oct.1989 }
{ PL / 0 compiler with code generation }
const norw = 13;          { no.of reserved words }
txmax = 100;        { length of identifier table }
nmax = 14;          { max.no.of digits in numbers }
al = 10;            { length of identifiers }
amax = 2047;        { maximum address }
levmax = 3;         { maximum depth of block nesting }
cxmax = 200;        { size of code array }

type symbol =
(nul, ident, number, plus, minus, times, slash, oddsym, eql, neq, lss,
leq, gtr, geq, lparen, rparen, comma, semicolon, period, becomes,
beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym,
varsym, procsym, readsym, writesym);            {*保留字类型*}
alfa = packed array[1..al] of char;
objecttyp = (constant, variable, prosedure);      {*标识符类型*}
symset = set of symbol;
fct = (lit, opr, lod, sto, cal, int, jmp, jpc, red, wrt); { functions }
instruction = packed record
f : fct;            { function code }
l: 0..levmax;      { level }
a: 0..amax;        { displacement address }
end;
{   lit 0, a : load constant a
opr 0, a : execute operation a
lod l, a : load variable l, a
sto l, a : store variable l, a
cal l, a : call procedure a at level l
int 0, a : increment t - register by a
jmp 0, a : jump to a
jpc 0, a : jump conditional to a
red l, a : read variable l, a
wrt 0, 0 : write stack - top
}

var   ch : char;      { last character read }
sym: symbol;    { last symbol read }
id: alfa;      { last identifier read }
num: integer;   { last number read }
cc: integer;   { character count }
ll: integer;   { line length }
kk, err: integer;
cx: integer;   { code allocation index }
line: array[1..81] of char;
a: alfa;
code: array[0..cxmax] of instruction;        {*指令集*}
word: array[1..norw] of alfa;
wsym: array[1..norw] of symbol;
ssym: array[char] of symbol;
mnemonic: array[fct] of
	packed array[1..5] of char;
		  declbegsys, statbegsys, facbegsys : symset; {*声明开始符号、语句开始符号、因子开始符号*}
	  table: array[0..txmax] of{ *符号表* }
		  record
		  name : alfa;
                  case kind: objecttyp of
				  constant : (val : integer);
					  variable, prosedure: (level, adr: integer)
						  end;
				  fin: text;     { source program file }
				  sfile: string;  { source program file name }

					  procedure error(n : integer);  { P384 }
					  begin
						  writeln('****', ' ':cc - 1, '^', n : 2);       {*输出错误信息以及错误位置*}
				  err: = err + 1                                 {*错误数 + 1 * }
					  end; { error }

					  procedure getsym;  { P384 }
					  var i, j, k : integer;
					  procedure getch; { P384 }
					  begin
					  if cc = ll{ get character to end of line }then{ *这一行已经读完，读下一行* }
					  begin{ read next line }
					  if eof(fin)
						  then begin
						  writeln('program incomplete');  {*程序未完全结束*}
					  close(fin);
					  exit;
					  end;
				  ll: = 0;
				  cc: = 0;
					  write(cx:4, ' ');  { print code address }
					  while not eoln(fin) do
					  begin
					  ll : = ll + 1;
					  read(fin, ch);
					  write(ch);
					  line[ll] : = ch
						  end;
					  writeln;
					  readln(fin);
				  ll: = ll + 1;
					  line[ll] : = ' ' { process end - line }
					  end;
				  cc: = cc + 1;
				  ch: = line[cc]       {*从这一行的字符串中返回下一个字符*}
					  end; { getch }
					  begin{ procedure getsym;  P384 }
					  while ch = ' ' do
					  getch;     {*滤过空白符*}
					  if ch in['a'..'z']    {*集合类型，如果ch是小写字母，说明可能是读到了一个标识符或者保留字 *}
					  then begin{ identifier of reserved word }
				  k: = 0;
					  repeat
					  if k < al{ *标识符长度不应超过10* }
					  then begin
					  k : = k + 1;
					  a[k] : = ch
						  end;
					  getch
						  until not(ch in['a'..'z', '0'..'9']);    {*直接把这个符号串读完，一个保留字或者标识符应该是由字母或者数字组成的串 *}
					  if k >= kk{ kk: last identifier length }
					  then kk : = k
					  else repeat
					  a[kk] : = ' ';
				  kk: = kk - 1
					  until kk = k;         {*保证kk一定是串a的长度 *}
				  id: = a;
				  i: = 1;
				  j: = norw;   { binary search reserved word table }
					  repeat
					  k : = (i + j) div 2;
					  if id <= word[k]
						  then j : = k - 1;
					  if id >= word[k]
						  then i : = k + 1
						  until i > j;
					  if i - 1 > j
						  then sym : = wsym[k]
					  else sym : = ident{ *二分查找确定这个串a是否是一个保留字，如果这个串a不是保留字，这个串a就是一个标识符 * }
					  end{ *剩下的情况必然属于保留字范围 * }
					  else if ch in['0'..'9'] then begin{ number }   {*如果这个字符是一个数字，说明接下来的输入是一个立即数*}
				  k: = 0;
				  num: = 0;
				  sym: = number;
					  repeat
					  num : = 10 * num + (ord(ch) - ord('0'));
				  k: = k + 1;
					  getch
						  until not(ch in['0'..'9']);               {*读立即数，并将这个字符串转化成一个整形常量*}
					  if k > nmax
						  then error(30)                              { *立即数的位数超过既定要求，报错* }
					  end
					  else if ch = ':' then begin
					  getch;
					  if ch = '='                              {*: = ，Pascal赋值符号*}
					  then begin
					  sym : = becomes;
					  getch
						  end
					  else sym : = nul{ *如果不是赋值符号，返回一个没有找到的保留字* }
					  end
					  else if ch = '<'
					  then begin
					  getch;
					  if ch = '='                          {*小于等于符号*}
					  then begin
					  sym : = leq;
					  getch
						  end
					  else if ch = '>'                     {*不等于符号*}
					  then begin
					  sym : = neq;
					  getch
						  end
					  else sym : = lss{ *小于符号* }
					  end
					  else if ch = '>'
					  then begin
					  getch;
					  if ch = '='                     {*大于等于符号*}
					  then begin
					  sym : = geq;
					  getch
						  end
					  else sym : = gtr{ *大于符号* }
					  end
					  else begin
				  sym : = ssym[ch];                {*直接找保留字*}
					  getch
						  end
						  end; { getsym }

					  procedure gen(x: fct; y, z : integer); { P385 }   {*生成一条指令*}
					  begin
					  if cx > cxmax
						  then begin
						  writeln('program too long');     {*语句太多了*}
					  close(fin);
					  exit
						  end;
					  with code[cx] do
					  begin
					  f : = x;
				  l: = y;
				  a: = z
					  end;
				 cx: = cx + 1
					 end; { gen }

					 procedure test(s1, s2 :symset; n: integer);  { P386 }
					 begin
					 if not(sym in s1)                      { *如果当前token不在s1集合中* }
					 then begin
						 error(n);                          {*报错*}
				 s1: = s1 + s2;
					 while not(sym in s1) do           { *错误恢复* }
					 getsym
						 end
						 end; { test }

					 procedure block(lev, tx : integer; fsys: symset); { P386 }
					 var  dx : integer;  { data allocation index }
				 tx0: integer;  { initial table index }
				 cx0: integer;  { initial code index }

					 procedure enter(k : objecttyp); { P386 }
					 begin{ enter object into table }
				 tx: = tx + 1;                            {*符号表大小增大一位*}
					 with table[tx] do
					 begin
					 name : = id;                         {*记录名字*}
				 kind: = k;                          {*类型*}
					 case k of
					 constant : begin
						 if num > amax{ *常量立即数超过预设的大小，报错并按0处理* }
						 then begin
							 error(30);
					 num: = 0
						 end;
					  val: = num{ *赋值* }
						  end;
					  variable: begin
					  level : = lev;       {*记录层次号、地址空间并开辟新的空间*}
					  adr: = dx;
					  dx: = dx + 1
						  end;
					  prosedure: level : = lev;         {*记录层次号*}
						  end
							  end
							  end; { enter }

						  function position(id : alfa) : integer; { P386 }
						  var i : integer;
						  begin
							  table[0].name : = id;
					  i: = tx;
						  while table[i].name <> id do
					  i : = i - 1;
					  position: = i{ *从最近入符号表的标识符开始查找当前标识符，如果找不到则返回0* }
						  end;  { position }

						  procedure constdeclaration; { P386 }
						  begin
						  if sym = ident{ *下一个符号必然是标识符* }
						  then begin
							  getsym;           {*读一下个token*}
						  if sym in[eql, becomes]    {*等号或赋值号*}
						  then begin
						  if sym = becomes
							  then error(1);      {*常量声明里应该是等号*}
						  getsym;             {*将赋值号也按照等号处理，读下一个token*}
						  if sym = number{ *立即数* }
						  then begin
							  enter(constant);       {*登录到符号表*}
						  getsym{ *度下一个token* }
						  end
						  else error(2)                 { *常量定义后面不是立即数，报错* }
						  end
						  else error(3)                        { *标识符后面不是赋值号或者等号* }
						  end
						  else error(4)                               { *不是标识符* }
						  end; { constdeclaration }

						  procedure vardeclaration; { P387 }
						  begin
						  if sym = ident{ *变量声明过程开始遇到的第一个符号必然应为标识符* }
						  then begin
							  enter(variable);  {*登录到符号表*}
						  getsym{ *读下一个token* }
						  end
						  else error(4)
						  end; { vardeclaration }

						  procedure listcode;  { P387 }
						  var i : integer;
						  begin

						  for i : = cx0 to cx - 1 do    { *当前分程序的代码段* }
						  with code[i] do
						  writeln(i : 4, mnemonic[f] : 7, l : 3, a : 5)    { *输出代码* }
						  end; { listcode }

						  procedure statement(fsys : symset); { P387 }
						  var i, cx1, cx2: integer;
						  procedure expression(fsys: symset); {P387 }
						  var addop : symbol;
						  procedure term(fsys : symset);  { P387 }
						  var mulop : symbol;
						  procedure factor(fsys : symset); { P387 }
						  var i : integer;
						  begin
							  test(facbegsys, fsys, 24);   {*检查当前token是否在因子开始符号集合中，如果不在，用fsys做错误恢复工具并报错*}
						  while sym in facbegsys do      { *循环处理因子* }
						  begin
						  if sym = ident{ *遇到了标识符* }
						  then begin
						  i : = position(id);  {*检查标识符在符号表中的位置*}
						  if i = 0{ *没找到* }
						  then error(11)
						  else
						  with table[i] do
						  case kind of
						  constant : gen(lit, 0, val);       {*常量运算，加载一个立即数入栈*}
						  variable: gen(lod, lev - level, adr);  {*变量运算，从内存中加载值入栈*}
						  prosedure: error(21)                { *过程不能参与运算* }
							  end;
							  getsym{ *读下一个token* }
							  end
						  else if sym = number{ *遇到了立即数* }
							  then begin
							  if num > amax{ *超过了立即数允许的最大值，报错，按0处理* }
							  then begin
								  error(30);
						  num: = 0
							  end;
							   gen(lit, 0, num);                    {*加载立即数入栈*}
							   getsym{ *读下一个token* }
							   end
							  else if sym = lparen{ *遇到了左括号* }
							   then begin
								   getsym;                       {*读token，期望是表达式*}
							   expression([rparen] + fsys);    {*解析表达式，加入)作为错误恢复工具*}
							   if sym = rparen{ *遇到右括号，说明当前因子结束* }
							   then getsym{ *读下一个token* }
							   else error(22)                { *括号不匹配* }
							   end;
							   test(fsys, [lparen], 23)                         { *检查当前状态，如果不合法用左括号作为错误恢复工具并报错* }
							   end
								   end; { factor }
							   begin{ procedure term(fsys : symset);   P388
								   var mulop : symbol; }
							   factor(fsys + [times, slash]);            {*处理第一个因子，加入 + 和 - 作为错误恢复工具*}
							   while sym in[times, slash] do           { *循环处理因子，因子由*或 / 连接* }
							   begin
							   mulop : = sym;                       {*记录乘除运算符*}
							   getsym;
							   factor(fsys + [times, slash]);       {*读下一个token，解析因子*}
							   if mulop = times{ *如果是乘法* }
							   then gen(opr, 0, 4)                 { *乘法运算* }
							   else gen(opr, 0, 5)                  { *除法运算* }
							   end
								   end; { term }
							   begin{ procedure expression(fsys: symset);  P388
								   var addop : symbol; }
							   if sym in[plus, minus]        {*正负号*}
							   then begin
							   addop : = sym;           {*记录正负号*}
							   getsym;
							   term(fsys + [plus, minus]);   {*读下一个token，处理当前项，加入 + 和 - 作为错误恢复工具*}
							   if addop = minus{ *如果是负号* }
							   then gen(opr, 0, 1)           { *取反* }
							   end
							   else term(fsys + [plus, minus]);     {*如果项前没有正负号，直接按照正号处理项*}
							   while sym in[plus, minus] do       { *循环处理多个项，项由 + 或 - 连接* }
							   begin
							   addop : = sym;                  {*记录运算符*}
							   getsym;
							   term(fsys + [plus, minus]);     {*读下一个token，处理当前项，加入 + 和 - 作为错误恢复工具*}
							   if addop = plus{ *如果是 + 号* }
							   then gen(opr, 0, 2)             { *加法运算* }
							   else gen(opr, 0, 3)             { *减法运算* }
							   end
								   end; { expression }

							   procedure condition(fsys : symset); { P388 }
							   var relop : symbol;
							   begin
							   if sym = oddsym{ *odd运算符* }
							   then begin
								   getsym;     {*读取下一个token*}
							   expression(fsys);       {*对odd的表达式进行处理和计算*}
							   gen(opr, 0, 6)            { *生成6号指令，奇偶判断* }
							   end
							   else begin
							   expression([eql, neq, lss, gtr, leq, geq] + fsys); {*计算左部表达式*}
							   if not(sym in[eql, neq, lss, leq, gtr, geq])    { *左部表达式后面不是逻辑运算符，报错* }
							   then error(20)
							   else begin
						   relop : = sym;                         {*记录逻辑运算符*}
							   getsym;
							   expression(fsys);                     {*计算右部表达式*}
							   case relop of{ *根据运算符的类型生成对应的指令* }
						   eql: gen(opr, 0, 8);
						   neq: gen(opr, 0, 9);
						   lss: gen(opr, 0, 10);
						   geq: gen(opr, 0, 11);
						   gtr: gen(opr, 0, 12);
						   leq: gen(opr, 0, 13);
							   end
								   end
								   end
								   end; { condition }
							   begin{ procedure statement(fsys : symset);  P389
								   var i, cx1, cx2: integer; }
							   if sym = ident
								   then begin
							   i : = position(id);     {*在符号表中查找该标识符所在位置*}
							   if i = 0{ *没找到……报错* }
							   then error(11)
							   else if table[i].kind <> variable{ *标识符不是变量名* }
							   then begin{ giving value to non - variation }
							   error(12);
						   i: = 0                   {*形同没找到，错误*}
							   end;
							   getsym;    {*读下一个token，期望是赋值号*}
							   if sym = becomes{ *赋值号* }
							   then getsym{ *读下一个token，期望是表达式* }
							   else error(13);             {*报错*}
							   expression(fsys);           {*处理表达式*}
							   if i <> 0                   {*如果没出错*}
							   then
								   with table[i] do
							   gen(sto, lev - level, adr)   { *生成STO指令，将表达式的值写入指定地址* }
							   end
							   else if sym = callsym{ *call语句* }
							   then begin
								   getsym;                      {*读取下一个token，期望是一个标识符，如果不是则报错*}
							   if sym <> ident
								   then error(14)
							   else begin
						   i : = position(id);    {*查找这个标识符在符号表的位置*}
							   if i = 0{ *没找到，报错* }
							   then error(11)
							   else
							   with table[i] do
							   if kind = prosedure{ *如果该标识符确实是一个过程* }
							   then gen(cal, lev - level, adr)       { *生成过程调用语句* }
							   else error(15);
							   getsym{ *读取下一个token* }
							   end
								   end
							   else if sym = ifsym{ *if语句* }
							   then begin
								   getsym;                 {*读取下一个token，期望是一个逻辑表达式*}
							   condition([thensym, dosym] + fsys);       {*解析逻辑表达式，加入then和do作为错误恢复工具*}
							   if sym = thensym{ *逻辑表达式后应该是一个then* }
							   then getsym{ *读取下一个token，期望是一个statement* }
							   else error(16);
						   cx1: = cx;
							   gen(jpc, 0, 0);                          {*生成一个条件跳转语句，暂时不设置跳转位置*}
							   statement(fsys);                       {*处理if里面的语句*}
							   code[cx1].a : = cx{ *加入jpc的跳转位置* }
							   end
							   else if sym = beginsym{ *begin语句，这应该是一个复合语句* }
							   then begin
								   getsym;            {*读取下一个token，期望是语句*}
							   statement([semicolon, endsym] + fsys);           {*解析语句，加入; 和end作为错误恢复工具*}
							   while sym in([semicolon] + statbegsys) do      { *循环分析语句，token可以是; 或者是语句开始符号* }
							   begin
							   if sym = semicolon{ *是分号，说明是语句的结尾，那么读下一个token* }
							   then getsym
							   else error(10);
							   statement([semicolon, endsym] + fsys)        { *解析语句，加入; 和end作为错误恢复工具* }
							   end;
							   if sym = endsym{ *begin复合语句的结束应该是end* }
							   then getsym{ *读取下一个token* }
							   else error(17)
								   end
							   else if sym = whilesym{ *while循环语句* }
							   then begin
							   cx1 : = cx;                   {*记录循环开始的代码段位置*}
							   getsym;                      {*读取下一个token，期望是一个逻辑表达式*}
							   condition([dosym] + fsys);     {*解析逻辑表达式，加入do作为错误恢复工具*}
						   cx2: = cx;                   {*记录循环体语句*}
							   gen(jpc, 0, 0);                {*设立条件跳转语句，暂时不设立跳转地址*}
							   if sym = dosym{ *逻辑表达式之后应该是一个do* }
							   then getsym{ *读取下一个token* }
							   else error(18);
							   statement(fsys);             {*解析循环体语句*}
							   gen(jmp, 0, cx1);              {*循环体后面设立无条件跳转语句，跳转到循环开始位置*}
							   code[cx2].a : = cx{ *循环体跳出位置赋给jpc语句的跳转地址* }
							   end
							   else if sym = readsym{ *read语句* }
							   then begin
								   getsym;            {*读取下一个token, 期望是左括号*}
							   if sym = lparen
								   then
								   repeat
								   getsym;        {*读取下一个token，期望是标识符*}
							   if sym = ident
								   then begin
							   i : = position(id);                       {*查找标识符在符号表中的位置*}
							   if i = 0{ *没找到，报错* }
							   then error(11)
							   else if table[i].kind <> variable{ *不是一个标识符，报错* }
							   then begin
								   error(12);
						   i: = 0                       {*错误标志*}
							   end
							   else with table[i] do               { *生成一条red指令，读取变量的值* }
							   gen(red, lev - level, adr)
								   end
							   else error(4);
							   getsym;                                         {*读下一个token*}
							   until sym <> comma{ *还有其他要读的变量* }
							   else error(40);                                     {*没找到左括号，报错*}
							   if sym <> rparen{ *结束时，期望遇到右括号，否则报错* }
							   then error(22);
							   getsym{ *读下一个token* }
							   end
							   else if sym = writesym{ *write语句* }
							   then begin
								   getsym;                                        {*读取下一个token, 期望是左括号*}
							   if sym = lparen
								   then begin
								   repeat
								   getsym;                               {*读取下一个token*}
							   expression([rparen, comma] + fsys);      {*处理表达式，用于出错恢复的集合再加上右括号和逗号*}
							   gen(wrt, 0, 0);                         {*生成输出语句*}
							   until sym <> comma;                     {*还有其他待输出的内容*}
							   if sym <> rparen{ *结束时，期望遇到右括号，否则报错* }
							   then error(22);
							   getsym{ *读取下一个token* }
							   end
							   else error(40)
							   end;
							   test(fsys, [], 19)                    { *检查当前状态是否合法* }
							   end; { statement }

							   begin{ procedure block(lev, tx : integer; fsys: symset);    P390
								   var  dx : integer;  /* data allocation index */
						   tx0: integer;  /*initial table index */
						   cx0: integer;  /* initial code index */ }

						   dx: = 3;        {*分配3个单元供运行期存放静态链、动态链和返回地址*}
						   tx0: = tx;
							   table[tx].adr : = cx;         {*记录当前符号表位置*}
							   gen(jmp, 0, 0); { jump from declaration part to statement part }
							   {*暂时不知道主程序在何处开始，先置为0*}
							   if lev > levmax
								   then error(32);     {*超出设定的程序层次数*}

							   repeat
							   if sym = constsym then{ *[<常量说明部分>] * }
							   begin
								   getsym;                      {*获取token*}
							   repeat
								   constdeclaration;          {*处理这个token的常量声明*}
							   while sym = comma do       { *此一句如果还有其他的常量声明，循环处理* }
							   begin
								   getsym;
							   constdeclaration
								   end;
							   if sym = semicolon{ *此一句常量声明部分结束* }
							   then getsym
							   else error(5)              { *一句常量声明应以; 结束，否则报错* }
							   until sym <> ident
								   end;

							   if sym = varsym then{ *[<变量说明部分>] * }
							   begin
								   getsym;                      {*获取token*}
							   repeat
								   vardeclaration;            {*处理这个token的变量声明*}
							   while sym = comma do       { *此一句如果还有其他的变量声明，循环处理* }
							   begin
								   getsym;
							   vardeclaration
								   end;
							   if sym = semicolon{ *此一句变量声明部分结束* }
							   then getsym
							   else error(5)              { *一句变量量声明应以; 结束，否则报错* }
							   until sym <> ident;          {*遇到非标识符，说明常量声明部分结束*}
							   end;

							   while sym = procsym do               { *[<过程说明部分>] * } {*过程首部*}
							   begin
								   getsym;
							   if sym = ident{ *<过程首部> :: = procedure<标识符>; *}
							   then begin
								   enter(prosedure);         {*登录这个过程到名字表中*}
							   getsym
								   end
							   else error(4);
							   if sym = semicolon{ *过程声明应以; 结尾* }
							   then getsym
							   else error(5);
							   block(lev + 1, tx, [semicolon] + fsys); {*<过程说明部分> :: = <过程首部><分程序>{; <过程说明部分>}; *} {*递归处理下一层次的分程序*}
							   if sym = semicolon
								   then begin
								   getsym;
							   test(statbegsys + [ident, procsym], fsys, 6)    { *检查当前状态是否合法，若不合法，以fsys恢复语法分析并报错* }
							   end
							   else error(5)
							   end;
							   test(statbegsys + [ident], declbegsys, 7)                { *检查当前状态是否合法，若不合法则用声明开始符号恢复语法分析并报错* }
							   until not(sym in declbegsys);
							   {*声明部分结束，进入语句部分，即分程序起始地址*}
							   code[table[tx0].adr].a : = cx;  { back enter statement code's start adr. }  {*JMP指令跳转地址赋值*}
								   with table[tx0] do
							   begin
							   adr : = cx; { code's start address }
									 end;
						   cx0: = cx;     {*记录当前代码分配位置*}
							   gen(int, 0, dx); { topstack point to operation area }
							   statement([semicolon, endsym] + fsys);    {*处理语句和语句块*}
							   gen(opr, 0, 0); { return }
							   test(fsys, [], 8);  {*用fsys检查当前状态是否合法*}
							   listcode;            {*列出本层PCODE代码*}
							   end{ block };

							   procedure interpret;  { P391 }    {*解释执行*}
							   const stacksize = 500;          {*预设堆栈大小500*}
							   var p, b, t: integer; { program - , base - , topstack - register }
						   i: instruction; { instruction register }
						   s: array[1..stacksize] of integer; { data store }
							   function base(l : integer) : integer;
							   var b1 : integer;
							   begin{ find base l levels down }
						   b1: = b;
							   while l > 0 do             { *按层次的差值查找基地址，一层一层迭代，直到差值为0* }
							   begin
							   b1 : = s[b1];
						   l: = l - 1
							   end;
						  base: = b1{ *返回查找到的基地址* }
							  end; { base }
							  begin{ P392 }
							  writeln('START PL/0');
						  t: = 0;     (*栈顶寄存器设0，基地址设1，PC设0*)
						  b : = 1;
						  p: = 0;
							  s[1] : = 0;
							  s[2] : = 0;
							  s[3] : = 0;    {*主程序的SL、DL、RA均为0*}
							  repeat
							  i : = code[p];                          {*读取一条指令*}
						  p: = p + 1;                              {*准备下一条指令*}
							  with i do
							  case f of{ *指令类型* }
						  lit: begin{ *加载立即数* }
						  t: = t + 1;
							  s[t]: = a;                  {*立即数入栈*}
							  end;
						  opr: case a of{ operator }       {*操作符类型*}
							  0: begin{ return }       {*返回操作*}
						  t: = b - 1;            {*释放子过程占用的栈内存*}
						  p: = s[t + 3];         {*读取返回地址*}
						  b: = s[t + 2];         {*读取调用前的基地址*}
							  end;
							  1 : s[t] : = -s[t];         {*取反*}
							  2 : begin{ *加法* }
						  t: = t - 1;
							  s[t] : = s[t] + s[t + 1]
								  end;
							  3 : begin{ *减法* }
						  t: = t - 1;
							  s[t] : = s[t] - s[t + 1]
								  end;
							  4 : begin{ *乘法* }
						  t: = t - 1;
							  s[t] : = s[t] * s[t + 1]
								  end;
							  5 : begin{ *整除* }
						  t: = t - 1;
							  s[t] : = s[t]div s[t + 1]
								  end;
							  {*ord(true) = 1, ord(false) = 0 * }
							  6 : s[t] : = ord(odd(s[t]));  {*and 1 * }
							  8 : begin{ *判断相等* }
						  t: = t - 1;
							  s[t] : = ord(s[t] = s[t + 1])
								  end;
							  9 : begin{ *判断不相等* }
						  t: = t - 1;
							  s[t] : = ord(s[t]<>s[t + 1])
								  end;
							  10: begin{ *判断小于* }
						  t: = t - 1;
							  s[t] : = ord(s[t]< s[t + 1])
								  end;
							  11: begin{ *判断大于等于* }
						  t: = t - 1;
							  s[t] : = ord(s[t] >= s[t + 1])
								  end;
							  12: begin{ *判断大于* }
						  t: = t - 1;
							  s[t] : = ord(s[t] > s[t + 1])
								  end;
							  13: begin{ *判断小于等于* }
						  t: = t - 1;
							  s[t] : = ord(s[t] <= s[t + 1])
								  end;
							  end;
						  lod: begin{ *从指定内存中读取数据* }
						  t: = t + 1;
							  s[t] : = s[base(l) + a]         {*从与当前层差为l层的基地址外加当前数据的地址偏移读取数据*}
							  end;
						  sto: begin{ *将数据写回内存* }
							  s[base(l) + a] : = s[t];  { writeln(s[t]); }     {*在与当前层差为l层的基地址外加当前数据的地址偏移存储数据*}
						  t: = t - 1
							  end;
						 cal: begin{ generate new block mark }{ *调用指令* }
							 s[t + 1] : = base(l);                            {*存储静态链SL*}
							 s[t + 2] : = b;                                  {*存储当前数据区基址DL*}
							 s[t + 3] : = p;                                  {*存储返回地址RA*}
						 b: = t + 1;                                     {*将当前数据区基址指向SL所在的位置*}
						 p: = a;                                       {*将PC置为跳转地址处*}
							 end;
							 int : t : = t + a;                                       {*栈顶上移a个空间，新开辟a个空间*}
						 jmp: p : = a;                                         {*无条件跳转，直接将PC置为跳转地址*}
						 jpc: begin{ *条件跳转，检查当前栈顶的值是否为1* }
							 if s[t] = 0
								 then p : = a;                                  {*如果栈顶为0，跳转并弹栈*}
						 t: = t - 1;
							 end;
						 red: begin{ *从标准输入读取数据，存在对应的内存地址中* }
							 writeln('??:');
							 readln(s[base(l) + a]);
							 end;
						 wrt: begin{ *将栈顶元素的值写入标准输出* }
							 writeln(s[t]);
						 t: = t + 1
							 end
							 end{ with, case }
							until p = 0;                                                {*主程序的RA是0，说明执行过程中从主程序返回*}
							writeln('END PL/0');
							end; { interpret }

							begin{ main }
							writeln('please input source program file name : ');
							readln(sfile);
							assign(fin, sfile);
							reset(fin);
							for ch : = 'A' to ';' do
							ssym[ch] : = nul;
							{*保留字赋值*}
							word[1] : = 'begin        '; word[2] : = 'call         ';
							word[3] : = 'const        '; word[4] : = 'do           ';
							word[5] : = 'end          '; word[6] : = 'if           ';
							word[7] : = 'odd          '; word[8] : = 'procedure    ';
							word[9] : = 'read         '; word[10]: = 'then         ';
							word[11]: = 'var          '; word[12]: = 'while        ';
							word[13]: = 'write        ';

							wsym[1] : = beginsym;      wsym[2] : = callsym;
							wsym[3] : = constsym;      wsym[4] : = dosym;
							wsym[5] : = endsym;        wsym[6] : = ifsym;
							wsym[7] : = oddsym;        wsym[8] : = procsym;
							wsym[9] : = readsym;       wsym[10]: = thensym;
							wsym[11]: = varsym;        wsym[12]: = whilesym;
							wsym[13]: = writesym;

							ssym['+'] : = plus;        ssym['-'] : = minus;
							ssym['*'] : = times;       ssym['/'] : = slash;
							ssym['('] : = lparen;      ssym[')'] : = rparen;
							ssym['='] : = eql;         ssym[','] : = comma;
							ssym['.'] : = period;
							ssym['<'] : = lss;         ssym['>'] : = gtr;
							ssym[';'] : = semicolon;
							{*指令类型*}
							mnemonic[lit] : = 'LIT  '; mnemonic[opr] : = 'OPR  ';
							mnemonic[lod] : = 'LOD  '; mnemonic[sto] : = 'STO  ';
							mnemonic[cal] : = 'CAL  '; mnemonic[int] : = 'INT  ';
							mnemonic[jmp] : = 'JMP  '; mnemonic[jpc] : = 'JPC  ';
							mnemonic[red] : = 'RED  '; mnemonic[wrt] : = 'WRT  ';
							{*声明起始、语句起始、因子起始*}
						declbegsys: = [constsym, varsym, procsym];
						statbegsys: = [beginsym, callsym, ifsym, whilesym];
						facbegsys: = [ident, number, lparen];
						{*初始化*}
						err: = 0;
						cc: = 0;
						cx: = 0;
						ll: = 0;
						ch: = ' ';
						kk: = al;
							getsym;                                        {*读取一个token*}
							block(0, 0, [period] + declbegsys + statbegsys);   {*<程序>: = <分程序>.*}
							if sym <> period then error(9);                {*程序应以.作为结尾*}
							if err = 0
								then interpret{ *如果没有错误，解释执行开始* }
							else write('ERRORS IN PL/0 PROGRAM');
							writeln;
							close(fin)
								end.
