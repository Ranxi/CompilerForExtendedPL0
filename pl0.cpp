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
varsym, procsym, readsym, writesym);            {*����������*}
alfa = packed array[1..al] of char;
objecttyp = (constant, variable, prosedure);      {*��ʶ������*}
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
code: array[0..cxmax] of instruction;        {*ָ�*}
word: array[1..norw] of alfa;
wsym: array[1..norw] of symbol;
ssym: array[char] of symbol;
mnemonic: array[fct] of
	packed array[1..5] of char;
		  declbegsys, statbegsys, facbegsys : symset; {*������ʼ���š���俪ʼ���š����ӿ�ʼ����*}
	  table: array[0..txmax] of{ *���ű�* }
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
						  writeln('****', ' ':cc - 1, '^', n : 2);       {*���������Ϣ�Լ�����λ��*}
				  err: = err + 1                                 {*������ + 1 * }
					  end; { error }

					  procedure getsym;  { P384 }
					  var i, j, k : integer;
					  procedure getch; { P384 }
					  begin
					  if cc = ll{ get character to end of line }then{ *��һ���Ѿ����꣬����һ��* }
					  begin{ read next line }
					  if eof(fin)
						  then begin
						  writeln('program incomplete');  {*����δ��ȫ����*}
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
				  ch: = line[cc]       {*����һ�е��ַ����з�����һ���ַ�*}
					  end; { getch }
					  begin{ procedure getsym;  P384 }
					  while ch = ' ' do
					  getch;     {*�˹��հ׷�*}
					  if ch in['a'..'z']    {*�������ͣ����ch��Сд��ĸ��˵�������Ƕ�����һ����ʶ�����߱����� *}
					  then begin{ identifier of reserved word }
				  k: = 0;
					  repeat
					  if k < al{ *��ʶ�����Ȳ�Ӧ����10* }
					  then begin
					  k : = k + 1;
					  a[k] : = ch
						  end;
					  getch
						  until not(ch in['a'..'z', '0'..'9']);    {*ֱ�Ӱ�������Ŵ����꣬һ�������ֻ��߱�ʶ��Ӧ��������ĸ����������ɵĴ� *}
					  if k >= kk{ kk: last identifier length }
					  then kk : = k
					  else repeat
					  a[kk] : = ' ';
				  kk: = kk - 1
					  until kk = k;         {*��֤kkһ���Ǵ�a�ĳ��� *}
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
					  else sym : = ident{ *���ֲ���ȷ�������a�Ƿ���һ�������֣���������a���Ǳ����֣������a����һ����ʶ�� * }
					  end{ *ʣ�µ������Ȼ���ڱ����ַ�Χ * }
					  else if ch in['0'..'9'] then begin{ number }   {*�������ַ���һ�����֣�˵����������������һ��������*}
				  k: = 0;
				  num: = 0;
				  sym: = number;
					  repeat
					  num : = 10 * num + (ord(ch) - ord('0'));
				  k: = k + 1;
					  getch
						  until not(ch in['0'..'9']);               {*������������������ַ���ת����һ�����γ���*}
					  if k > nmax
						  then error(30)                              { *��������λ�������ȶ�Ҫ�󣬱���* }
					  end
					  else if ch = ':' then begin
					  getch;
					  if ch = '='                              {*: = ��Pascal��ֵ����*}
					  then begin
					  sym : = becomes;
					  getch
						  end
					  else sym : = nul{ *������Ǹ�ֵ���ţ�����һ��û���ҵ��ı�����* }
					  end
					  else if ch = '<'
					  then begin
					  getch;
					  if ch = '='                          {*С�ڵ��ڷ���*}
					  then begin
					  sym : = leq;
					  getch
						  end
					  else if ch = '>'                     {*�����ڷ���*}
					  then begin
					  sym : = neq;
					  getch
						  end
					  else sym : = lss{ *С�ڷ���* }
					  end
					  else if ch = '>'
					  then begin
					  getch;
					  if ch = '='                     {*���ڵ��ڷ���*}
					  then begin
					  sym : = geq;
					  getch
						  end
					  else sym : = gtr{ *���ڷ���* }
					  end
					  else begin
				  sym : = ssym[ch];                {*ֱ���ұ�����*}
					  getch
						  end
						  end; { getsym }

					  procedure gen(x: fct; y, z : integer); { P385 }   {*����һ��ָ��*}
					  begin
					  if cx > cxmax
						  then begin
						  writeln('program too long');     {*���̫����*}
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
					 if not(sym in s1)                      { *�����ǰtoken����s1������* }
					 then begin
						 error(n);                          {*����*}
				 s1: = s1 + s2;
					 while not(sym in s1) do           { *����ָ�* }
					 getsym
						 end
						 end; { test }

					 procedure block(lev, tx : integer; fsys: symset); { P386 }
					 var  dx : integer;  { data allocation index }
				 tx0: integer;  { initial table index }
				 cx0: integer;  { initial code index }

					 procedure enter(k : objecttyp); { P386 }
					 begin{ enter object into table }
				 tx: = tx + 1;                            {*���ű��С����һλ*}
					 with table[tx] do
					 begin
					 name : = id;                         {*��¼����*}
				 kind: = k;                          {*����*}
					 case k of
					 constant : begin
						 if num > amax{ *��������������Ԥ��Ĵ�С��������0����* }
						 then begin
							 error(30);
					 num: = 0
						 end;
					  val: = num{ *��ֵ* }
						  end;
					  variable: begin
					  level : = lev;       {*��¼��κš���ַ�ռ䲢�����µĿռ�*}
					  adr: = dx;
					  dx: = dx + 1
						  end;
					  prosedure: level : = lev;         {*��¼��κ�*}
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
					  position: = i{ *���������ű�ı�ʶ����ʼ���ҵ�ǰ��ʶ��������Ҳ����򷵻�0* }
						  end;  { position }

						  procedure constdeclaration; { P386 }
						  begin
						  if sym = ident{ *��һ�����ű�Ȼ�Ǳ�ʶ��* }
						  then begin
							  getsym;           {*��һ�¸�token*}
						  if sym in[eql, becomes]    {*�ȺŻ�ֵ��*}
						  then begin
						  if sym = becomes
							  then error(1);      {*����������Ӧ���ǵȺ�*}
						  getsym;             {*����ֵ��Ҳ���յȺŴ�������һ��token*}
						  if sym = number{ *������* }
						  then begin
							  enter(constant);       {*��¼�����ű�*}
						  getsym{ *����һ��token* }
						  end
						  else error(2)                 { *����������治��������������* }
						  end
						  else error(3)                        { *��ʶ�����治�Ǹ�ֵ�Ż��ߵȺ�* }
						  end
						  else error(4)                               { *���Ǳ�ʶ��* }
						  end; { constdeclaration }

						  procedure vardeclaration; { P387 }
						  begin
						  if sym = ident{ *�����������̿�ʼ�����ĵ�һ�����ű�ȻӦΪ��ʶ��* }
						  then begin
							  enter(variable);  {*��¼�����ű�*}
						  getsym{ *����һ��token* }
						  end
						  else error(4)
						  end; { vardeclaration }

						  procedure listcode;  { P387 }
						  var i : integer;
						  begin

						  for i : = cx0 to cx - 1 do    { *��ǰ�ֳ���Ĵ����* }
						  with code[i] do
						  writeln(i : 4, mnemonic[f] : 7, l : 3, a : 5)    { *�������* }
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
							  test(facbegsys, fsys, 24);   {*��鵱ǰtoken�Ƿ������ӿ�ʼ���ż����У�������ڣ���fsys������ָ����߲�����*}
						  while sym in facbegsys do      { *ѭ����������* }
						  begin
						  if sym = ident{ *�����˱�ʶ��* }
						  then begin
						  i : = position(id);  {*����ʶ���ڷ��ű��е�λ��*}
						  if i = 0{ *û�ҵ�* }
						  then error(11)
						  else
						  with table[i] do
						  case kind of
						  constant : gen(lit, 0, val);       {*�������㣬����һ����������ջ*}
						  variable: gen(lod, lev - level, adr);  {*�������㣬���ڴ��м���ֵ��ջ*}
						  prosedure: error(21)                { *���̲��ܲ�������* }
							  end;
							  getsym{ *����һ��token* }
							  end
						  else if sym = number{ *������������* }
							  then begin
							  if num > amax{ *��������������������ֵ��������0����* }
							  then begin
								  error(30);
						  num: = 0
							  end;
							   gen(lit, 0, num);                    {*������������ջ*}
							   getsym{ *����һ��token* }
							   end
							  else if sym = lparen{ *������������* }
							   then begin
								   getsym;                       {*��token�������Ǳ��ʽ*}
							   expression([rparen] + fsys);    {*�������ʽ������)��Ϊ����ָ�����*}
							   if sym = rparen{ *���������ţ�˵����ǰ���ӽ���* }
							   then getsym{ *����һ��token* }
							   else error(22)                { *���Ų�ƥ��* }
							   end;
							   test(fsys, [lparen], 23)                         { *��鵱ǰ״̬��������Ϸ�����������Ϊ����ָ����߲�����* }
							   end
								   end; { factor }
							   begin{ procedure term(fsys : symset);   P388
								   var mulop : symbol; }
							   factor(fsys + [times, slash]);            {*�����һ�����ӣ����� + �� - ��Ϊ����ָ�����*}
							   while sym in[times, slash] do           { *ѭ���������ӣ�������*�� / ����* }
							   begin
							   mulop : = sym;                       {*��¼�˳������*}
							   getsym;
							   factor(fsys + [times, slash]);       {*����һ��token����������*}
							   if mulop = times{ *����ǳ˷�* }
							   then gen(opr, 0, 4)                 { *�˷�����* }
							   else gen(opr, 0, 5)                  { *��������* }
							   end
								   end; { term }
							   begin{ procedure expression(fsys: symset);  P388
								   var addop : symbol; }
							   if sym in[plus, minus]        {*������*}
							   then begin
							   addop : = sym;           {*��¼������*}
							   getsym;
							   term(fsys + [plus, minus]);   {*����һ��token������ǰ����� + �� - ��Ϊ����ָ�����*}
							   if addop = minus{ *����Ǹ���* }
							   then gen(opr, 0, 1)           { *ȡ��* }
							   end
							   else term(fsys + [plus, minus]);     {*�����ǰû�������ţ�ֱ�Ӱ������Ŵ�����*}
							   while sym in[plus, minus] do       { *ѭ������������� + �� - ����* }
							   begin
							   addop : = sym;                  {*��¼�����*}
							   getsym;
							   term(fsys + [plus, minus]);     {*����һ��token������ǰ����� + �� - ��Ϊ����ָ�����*}
							   if addop = plus{ *����� + ��* }
							   then gen(opr, 0, 2)             { *�ӷ�����* }
							   else gen(opr, 0, 3)             { *��������* }
							   end
								   end; { expression }

							   procedure condition(fsys : symset); { P388 }
							   var relop : symbol;
							   begin
							   if sym = oddsym{ *odd�����* }
							   then begin
								   getsym;     {*��ȡ��һ��token*}
							   expression(fsys);       {*��odd�ı��ʽ���д���ͼ���*}
							   gen(opr, 0, 6)            { *����6��ָ���ż�ж�* }
							   end
							   else begin
							   expression([eql, neq, lss, gtr, leq, geq] + fsys); {*�����󲿱��ʽ*}
							   if not(sym in[eql, neq, lss, leq, gtr, geq])    { *�󲿱��ʽ���治���߼������������* }
							   then error(20)
							   else begin
						   relop : = sym;                         {*��¼�߼������*}
							   getsym;
							   expression(fsys);                     {*�����Ҳ����ʽ*}
							   case relop of{ *������������������ɶ�Ӧ��ָ��* }
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
							   i : = position(id);     {*�ڷ��ű��в��Ҹñ�ʶ������λ��*}
							   if i = 0{ *û�ҵ���������* }
							   then error(11)
							   else if table[i].kind <> variable{ *��ʶ�����Ǳ�����* }
							   then begin{ giving value to non - variation }
							   error(12);
						   i: = 0                   {*��ͬû�ҵ�������*}
							   end;
							   getsym;    {*����һ��token�������Ǹ�ֵ��*}
							   if sym = becomes{ *��ֵ��* }
							   then getsym{ *����һ��token�������Ǳ��ʽ* }
							   else error(13);             {*����*}
							   expression(fsys);           {*������ʽ*}
							   if i <> 0                   {*���û����*}
							   then
								   with table[i] do
							   gen(sto, lev - level, adr)   { *����STOָ������ʽ��ֵд��ָ����ַ* }
							   end
							   else if sym = callsym{ *call���* }
							   then begin
								   getsym;                      {*��ȡ��һ��token��������һ����ʶ������������򱨴�*}
							   if sym <> ident
								   then error(14)
							   else begin
						   i : = position(id);    {*���������ʶ���ڷ��ű��λ��*}
							   if i = 0{ *û�ҵ�������* }
							   then error(11)
							   else
							   with table[i] do
							   if kind = prosedure{ *����ñ�ʶ��ȷʵ��һ������* }
							   then gen(cal, lev - level, adr)       { *���ɹ��̵������* }
							   else error(15);
							   getsym{ *��ȡ��һ��token* }
							   end
								   end
							   else if sym = ifsym{ *if���* }
							   then begin
								   getsym;                 {*��ȡ��һ��token��������һ���߼����ʽ*}
							   condition([thensym, dosym] + fsys);       {*�����߼����ʽ������then��do��Ϊ����ָ�����*}
							   if sym = thensym{ *�߼����ʽ��Ӧ����һ��then* }
							   then getsym{ *��ȡ��һ��token��������һ��statement* }
							   else error(16);
						   cx1: = cx;
							   gen(jpc, 0, 0);                          {*����һ��������ת��䣬��ʱ��������תλ��*}
							   statement(fsys);                       {*����if��������*}
							   code[cx1].a : = cx{ *����jpc����תλ��* }
							   end
							   else if sym = beginsym{ *begin��䣬��Ӧ����һ���������* }
							   then begin
								   getsym;            {*��ȡ��һ��token�����������*}
							   statement([semicolon, endsym] + fsys);           {*������䣬����; ��end��Ϊ����ָ�����*}
							   while sym in([semicolon] + statbegsys) do      { *ѭ��������䣬token������; ��������俪ʼ����* }
							   begin
							   if sym = semicolon{ *�Ƿֺţ�˵�������Ľ�β����ô����һ��token* }
							   then getsym
							   else error(10);
							   statement([semicolon, endsym] + fsys)        { *������䣬����; ��end��Ϊ����ָ�����* }
							   end;
							   if sym = endsym{ *begin�������Ľ���Ӧ����end* }
							   then getsym{ *��ȡ��һ��token* }
							   else error(17)
								   end
							   else if sym = whilesym{ *whileѭ�����* }
							   then begin
							   cx1 : = cx;                   {*��¼ѭ����ʼ�Ĵ����λ��*}
							   getsym;                      {*��ȡ��һ��token��������һ���߼����ʽ*}
							   condition([dosym] + fsys);     {*�����߼����ʽ������do��Ϊ����ָ�����*}
						   cx2: = cx;                   {*��¼ѭ�������*}
							   gen(jpc, 0, 0);                {*����������ת��䣬��ʱ��������ת��ַ*}
							   if sym = dosym{ *�߼����ʽ֮��Ӧ����һ��do* }
							   then getsym{ *��ȡ��һ��token* }
							   else error(18);
							   statement(fsys);             {*����ѭ�������*}
							   gen(jmp, 0, cx1);              {*ѭ�������������������ת��䣬��ת��ѭ����ʼλ��*}
							   code[cx2].a : = cx{ *ѭ��������λ�ø���jpc������ת��ַ* }
							   end
							   else if sym = readsym{ *read���* }
							   then begin
								   getsym;            {*��ȡ��һ��token, ������������*}
							   if sym = lparen
								   then
								   repeat
								   getsym;        {*��ȡ��һ��token�������Ǳ�ʶ��*}
							   if sym = ident
								   then begin
							   i : = position(id);                       {*���ұ�ʶ���ڷ��ű��е�λ��*}
							   if i = 0{ *û�ҵ�������* }
							   then error(11)
							   else if table[i].kind <> variable{ *����һ����ʶ��������* }
							   then begin
								   error(12);
						   i: = 0                       {*�����־*}
							   end
							   else with table[i] do               { *����һ��redָ���ȡ������ֵ* }
							   gen(red, lev - level, adr)
								   end
							   else error(4);
							   getsym;                                         {*����һ��token*}
							   until sym <> comma{ *��������Ҫ���ı���* }
							   else error(40);                                     {*û�ҵ������ţ�����*}
							   if sym <> rparen{ *����ʱ���������������ţ����򱨴�* }
							   then error(22);
							   getsym{ *����һ��token* }
							   end
							   else if sym = writesym{ *write���* }
							   then begin
								   getsym;                                        {*��ȡ��һ��token, ������������*}
							   if sym = lparen
								   then begin
								   repeat
								   getsym;                               {*��ȡ��һ��token*}
							   expression([rparen, comma] + fsys);      {*������ʽ�����ڳ���ָ��ļ����ټ��������źͶ���*}
							   gen(wrt, 0, 0);                         {*����������*}
							   until sym <> comma;                     {*�������������������*}
							   if sym <> rparen{ *����ʱ���������������ţ����򱨴�* }
							   then error(22);
							   getsym{ *��ȡ��һ��token* }
							   end
							   else error(40)
							   end;
							   test(fsys, [], 19)                    { *��鵱ǰ״̬�Ƿ�Ϸ�* }
							   end; { statement }

							   begin{ procedure block(lev, tx : integer; fsys: symset);    P390
								   var  dx : integer;  /* data allocation index */
						   tx0: integer;  /*initial table index */
						   cx0: integer;  /* initial code index */ }

						   dx: = 3;        {*����3����Ԫ�������ڴ�ž�̬������̬���ͷ��ص�ַ*}
						   tx0: = tx;
							   table[tx].adr : = cx;         {*��¼��ǰ���ű�λ��*}
							   gen(jmp, 0, 0); { jump from declaration part to statement part }
							   {*��ʱ��֪���������ںδ���ʼ������Ϊ0*}
							   if lev > levmax
								   then error(32);     {*�����趨�ĳ�������*}

							   repeat
							   if sym = constsym then{ *[<����˵������>] * }
							   begin
								   getsym;                      {*��ȡtoken*}
							   repeat
								   constdeclaration;          {*�������token�ĳ�������*}
							   while sym = comma do       { *��һ��������������ĳ���������ѭ������* }
							   begin
								   getsym;
							   constdeclaration
								   end;
							   if sym = semicolon{ *��һ�䳣���������ֽ���* }
							   then getsym
							   else error(5)              { *һ�䳣������Ӧ��; ���������򱨴�* }
							   until sym <> ident
								   end;

							   if sym = varsym then{ *[<����˵������>] * }
							   begin
								   getsym;                      {*��ȡtoken*}
							   repeat
								   vardeclaration;            {*�������token�ı�������*}
							   while sym = comma do       { *��һ��������������ı���������ѭ������* }
							   begin
								   getsym;
							   vardeclaration
								   end;
							   if sym = semicolon{ *��һ������������ֽ���* }
							   then getsym
							   else error(5)              { *һ�����������Ӧ��; ���������򱨴�* }
							   until sym <> ident;          {*�����Ǳ�ʶ����˵�������������ֽ���*}
							   end;

							   while sym = procsym do               { *[<����˵������>] * } {*�����ײ�*}
							   begin
								   getsym;
							   if sym = ident{ *<�����ײ�> :: = procedure<��ʶ��>; *}
							   then begin
								   enter(prosedure);         {*��¼������̵����ֱ���*}
							   getsym
								   end
							   else error(4);
							   if sym = semicolon{ *��������Ӧ��; ��β* }
							   then getsym
							   else error(5);
							   block(lev + 1, tx, [semicolon] + fsys); {*<����˵������> :: = <�����ײ�><�ֳ���>{; <����˵������>}; *} {*�ݹ鴦����һ��εķֳ���*}
							   if sym = semicolon
								   then begin
								   getsym;
							   test(statbegsys + [ident, procsym], fsys, 6)    { *��鵱ǰ״̬�Ƿ�Ϸ��������Ϸ�����fsys�ָ��﷨����������* }
							   end
							   else error(5)
							   end;
							   test(statbegsys + [ident], declbegsys, 7)                { *��鵱ǰ״̬�Ƿ�Ϸ��������Ϸ�����������ʼ���Żָ��﷨����������* }
							   until not(sym in declbegsys);
							   {*�������ֽ�����������䲿�֣����ֳ�����ʼ��ַ*}
							   code[table[tx0].adr].a : = cx;  { back enter statement code's start adr. }  {*JMPָ����ת��ַ��ֵ*}
								   with table[tx0] do
							   begin
							   adr : = cx; { code's start address }
									 end;
						   cx0: = cx;     {*��¼��ǰ�������λ��*}
							   gen(int, 0, dx); { topstack point to operation area }
							   statement([semicolon, endsym] + fsys);    {*������������*}
							   gen(opr, 0, 0); { return }
							   test(fsys, [], 8);  {*��fsys��鵱ǰ״̬�Ƿ�Ϸ�*}
							   listcode;            {*�г�����PCODE����*}
							   end{ block };

							   procedure interpret;  { P391 }    {*����ִ��*}
							   const stacksize = 500;          {*Ԥ���ջ��С500*}
							   var p, b, t: integer; { program - , base - , topstack - register }
						   i: instruction; { instruction register }
						   s: array[1..stacksize] of integer; { data store }
							   function base(l : integer) : integer;
							   var b1 : integer;
							   begin{ find base l levels down }
						   b1: = b;
							   while l > 0 do             { *����εĲ�ֵ���һ���ַ��һ��һ�������ֱ����ֵΪ0* }
							   begin
							   b1 : = s[b1];
						   l: = l - 1
							   end;
						  base: = b1{ *���ز��ҵ��Ļ���ַ* }
							  end; { base }
							  begin{ P392 }
							  writeln('START PL/0');
						  t: = 0;     (*ջ���Ĵ�����0������ַ��1��PC��0*)
						  b : = 1;
						  p: = 0;
							  s[1] : = 0;
							  s[2] : = 0;
							  s[3] : = 0;    {*�������SL��DL��RA��Ϊ0*}
							  repeat
							  i : = code[p];                          {*��ȡһ��ָ��*}
						  p: = p + 1;                              {*׼����һ��ָ��*}
							  with i do
							  case f of{ *ָ������* }
						  lit: begin{ *����������* }
						  t: = t + 1;
							  s[t]: = a;                  {*��������ջ*}
							  end;
						  opr: case a of{ operator }       {*����������*}
							  0: begin{ return }       {*���ز���*}
						  t: = b - 1;            {*�ͷ��ӹ���ռ�õ�ջ�ڴ�*}
						  p: = s[t + 3];         {*��ȡ���ص�ַ*}
						  b: = s[t + 2];         {*��ȡ����ǰ�Ļ���ַ*}
							  end;
							  1 : s[t] : = -s[t];         {*ȡ��*}
							  2 : begin{ *�ӷ�* }
						  t: = t - 1;
							  s[t] : = s[t] + s[t + 1]
								  end;
							  3 : begin{ *����* }
						  t: = t - 1;
							  s[t] : = s[t] - s[t + 1]
								  end;
							  4 : begin{ *�˷�* }
						  t: = t - 1;
							  s[t] : = s[t] * s[t + 1]
								  end;
							  5 : begin{ *����* }
						  t: = t - 1;
							  s[t] : = s[t]div s[t + 1]
								  end;
							  {*ord(true) = 1, ord(false) = 0 * }
							  6 : s[t] : = ord(odd(s[t]));  {*and 1 * }
							  8 : begin{ *�ж����* }
						  t: = t - 1;
							  s[t] : = ord(s[t] = s[t + 1])
								  end;
							  9 : begin{ *�жϲ����* }
						  t: = t - 1;
							  s[t] : = ord(s[t]<>s[t + 1])
								  end;
							  10: begin{ *�ж�С��* }
						  t: = t - 1;
							  s[t] : = ord(s[t]< s[t + 1])
								  end;
							  11: begin{ *�жϴ��ڵ���* }
						  t: = t - 1;
							  s[t] : = ord(s[t] >= s[t + 1])
								  end;
							  12: begin{ *�жϴ���* }
						  t: = t - 1;
							  s[t] : = ord(s[t] > s[t + 1])
								  end;
							  13: begin{ *�ж�С�ڵ���* }
						  t: = t - 1;
							  s[t] : = ord(s[t] <= s[t + 1])
								  end;
							  end;
						  lod: begin{ *��ָ���ڴ��ж�ȡ����* }
						  t: = t + 1;
							  s[t] : = s[base(l) + a]         {*���뵱ǰ���Ϊl��Ļ���ַ��ӵ�ǰ���ݵĵ�ַƫ�ƶ�ȡ����*}
							  end;
						  sto: begin{ *������д���ڴ�* }
							  s[base(l) + a] : = s[t];  { writeln(s[t]); }     {*���뵱ǰ���Ϊl��Ļ���ַ��ӵ�ǰ���ݵĵ�ַƫ�ƴ洢����*}
						  t: = t - 1
							  end;
						 cal: begin{ generate new block mark }{ *����ָ��* }
							 s[t + 1] : = base(l);                            {*�洢��̬��SL*}
							 s[t + 2] : = b;                                  {*�洢��ǰ��������ַDL*}
							 s[t + 3] : = p;                                  {*�洢���ص�ַRA*}
						 b: = t + 1;                                     {*����ǰ��������ַָ��SL���ڵ�λ��*}
						 p: = a;                                       {*��PC��Ϊ��ת��ַ��*}
							 end;
							 int : t : = t + a;                                       {*ջ������a���ռ䣬�¿���a���ռ�*}
						 jmp: p : = a;                                         {*��������ת��ֱ�ӽ�PC��Ϊ��ת��ַ*}
						 jpc: begin{ *������ת����鵱ǰջ����ֵ�Ƿ�Ϊ1* }
							 if s[t] = 0
								 then p : = a;                                  {*���ջ��Ϊ0����ת����ջ*}
						 t: = t - 1;
							 end;
						 red: begin{ *�ӱ�׼�����ȡ���ݣ����ڶ�Ӧ���ڴ��ַ��* }
							 writeln('??:');
							 readln(s[base(l) + a]);
							 end;
						 wrt: begin{ *��ջ��Ԫ�ص�ֵд���׼���* }
							 writeln(s[t]);
						 t: = t + 1
							 end
							 end{ with, case }
							until p = 0;                                                {*�������RA��0��˵��ִ�й����д������򷵻�*}
							writeln('END PL/0');
							end; { interpret }

							begin{ main }
							writeln('please input source program file name : ');
							readln(sfile);
							assign(fin, sfile);
							reset(fin);
							for ch : = 'A' to ';' do
							ssym[ch] : = nul;
							{*�����ָ�ֵ*}
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
							{*ָ������*}
							mnemonic[lit] : = 'LIT  '; mnemonic[opr] : = 'OPR  ';
							mnemonic[lod] : = 'LOD  '; mnemonic[sto] : = 'STO  ';
							mnemonic[cal] : = 'CAL  '; mnemonic[int] : = 'INT  ';
							mnemonic[jmp] : = 'JMP  '; mnemonic[jpc] : = 'JPC  ';
							mnemonic[red] : = 'RED  '; mnemonic[wrt] : = 'WRT  ';
							{*������ʼ�������ʼ��������ʼ*}
						declbegsys: = [constsym, varsym, procsym];
						statbegsys: = [beginsym, callsym, ifsym, whilesym];
						facbegsys: = [ident, number, lparen];
						{*��ʼ��*}
						err: = 0;
						cc: = 0;
						cx: = 0;
						ll: = 0;
						ch: = ' ';
						kk: = al;
							getsym;                                        {*��ȡһ��token*}
							block(0, 0, [period] + declbegsys + statbegsys);   {*<����>: = <�ֳ���>.*}
							if sym <> period then error(9);                {*����Ӧ��.��Ϊ��β*}
							if err = 0
								then interpret{ *���û�д��󣬽���ִ�п�ʼ* }
							else write('ERRORS IN PL/0 PROGRAM');
							writeln;
							close(fin)
								end.
