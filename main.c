#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//symbols loaded
char Datain[100] = "ba;ba;ba;a=d;a=d;e(a>a)a=a+a;fa=a-a;$";
int parse_pos=0;//the position of the current symbol under parse

//项的右部
    char str[10] = "P";
    char str1[10] = "DS";
    char str2[10] = "La;D";
    char str3[10] = "S";
    char str4[10] = "b";
    char str5[10] = "c";
    char str6[10] = "a=E;";
    char str7[10] = "e(C)S";
    char str8[10] = "e(C)SfS";
    char str9[10] = "g(C)S";
    char str10[10] = "S;S";
    char str11[10] = "E>E";
    char str12[10] = "E<E";
    char str13[10] = "EhE";
    char str14[10] = "E+T";
    char str15[10] = "E-T";
    char str16[10] = "T";
    char str17[10] = "F";
    char str18[10] = "T*F";
    char str19[10] = "T/F";
    char str20[10] = "(E)";
    char str21[10] = "a";
    char str22[10] = "d";
    char str23[10] = "La;";

int First[9][20];// First Set 18 is '$' 19 is 'null'
int Follow[9][20];//18 is '$'

struct Item{
    int head;//头编号
    char body[10];//体
    int len;//长度
};

struct Item  I[24];//23个生成式项

struct ISet{//一个状态的项集
    int SetItem[24][10];
    int validItem[24];
    int conf;//存在冲突
    int reducehead;//if exist item which can be reduced,record its head num
};

void InitISet(struct ISet *in){
    in->conf = 0;
    for(int i=0;i<24;i++){
            for(int j=0;j<10;j++){
                in->validItem[i] = 0;
                in->SetItem[i][j] = 0;
            }
    }
}

struct ISet Status[200];//所有状态
int status_num = 0;//状态数目

struct Action{
    int status_code;//0 for error 1 for normal 2 for acc
    int movein;//为1表示移入
    int movenum;//表示移入的状态
    int reduce;//为-1表示不规约
    int reducelen;//回退长度
};

struct StatusStack{
    int StatStack[200];
    int topptr;
};

struct StatusStack SStack;
int GetStackTop(){
    return SStack.StatStack[SStack.topptr-1];
}

int PopStack(int n){
    int a = SStack.topptr;
    SStack.topptr = SStack.topptr-n;
    //printf("ptr sub from %d to %d\n",a,SStack.topptr);
}

void PushStack(int n){
    int a = SStack.topptr;
    SStack.StatStack[SStack.topptr] = n;
    SStack.topptr = SStack.topptr+1;
    //printf("ptr add from %d to %d\n",a,SStack.topptr);
}

char GetNextSym(){//get next symbol to be parsed
    char c = Datain[parse_pos];
    parse_pos++;
    return c;
}

int GOTO[200][27];//GOTO表，描述状态转移
struct Action ACT[200][27];

char Int2Sym(int a){
switch (a){
    case 0: return 'p'; //p = P'
    case 1: return 'P';
    case 2: return 'D';
    case 3: return 'S';
    case 4: return 'L';
    case 5: return 'C';
    case 6: return 'E';
    case 7: return 'F';
    case 8: return 'T';
    case 9: return 'a';//a = id
    case 10: return ';';
    case 11: return 'b';//b = int
    case 12: return 'c';//c = float
    case 13: return '=';
    case 14: return '(';
    case 15: return ')';
    case 16: return '>';
    case 17: return '<';
    case 18: return 'h';//h = '=='
    case 19: return '+';
    case 20: return '-';
    case 21: return '*';
    case 22: return '/';
    case 23: return 'd';//d = digits
    case 24: return 'e';//e = if
    case 25: return 'f';//f = else
    case 26: return 'g';//g = while
    case 27: return '$';
    case 28: return 'n';//n = null
}
return ' ';
}

int Sym2Int(char a){
switch (a){
    case 'p': return 0; //p = P'
    case 'P': return 1;
    case 'D': return 2;
    case 'S': return 3;
    case 'L': return 4;
    case 'C': return 5;
    case 'E': return 6;
    case 'F': return 7;
    case 'T': return 8;
    case 'a': return 9;//a = id
    case ';': return 10;
    case 'b': return 11;//b = int
    case 'c': return 12;//c = float
    case '=': return 13;
    case '(': return 14;
    case ')': return 15;
    case '>': return 16;
    case '<': return 17;
    case 'h': return 18;//h = '=='
    case '+': return 19;
    case '-': return 20;
    case '*': return 21;
    case '/': return 22;
    case 'd': return 23;//d = digits
    case 'e': return 24;//e = if
    case 'f': return 25;//f = else
    case 'g': return 26;//g = while
}
}
void Convert(char a){
switch (a){
    case 'p': printf("P");break; //p = P'
    case 'P': printf("P\'");break;
    case 'D': printf("D");break;
    case 'S': printf("S");break;
    case 'L': printf("L");break;
    case 'C': printf("C");break;
    case 'E': printf("E");break;
    case 'F': printf("F");break;
    case 'T': printf("T");break;
    case 'a': printf("id");break; //a = id
    case ';': printf(";");break;
    case 'b': printf("int");break; //b = int
    case 'c': printf("float");break; //c = float
    case '=': printf("=");break;
    case '(': printf("(");break;
    case ')': printf(")");break;
    case '>': printf(">");break;
    case '<': printf("<");break;
    case 'h': printf("==");break; //h = '=='
    case '+': printf("+");break;
    case '-': printf("-");break;
    case '*': printf("*");break;
    case '/': printf("/");break;
    case 'd': printf("digits");break; //d = digits
    case 'e': printf("if");break; //e = if
    case 'f': printf("else");break; //f = else
    case 'g': printf("while");break; //g = while
}
}

int SetEqual(struct ISet* in1,struct ISet* in2){
    for(int i=0;i<24;i++){
            for(int j=0;j<10;j++){
        if(((in1->SetItem[i][j] != in2->SetItem[i][j])&&(in1->validItem[i] == 1))||(in1->validItem[i]!=in2->validItem[i])){
            return 0;//0表示不相等
        }
        }
    }
    return 1;//1表示相等
}

void SetCopy(struct ISet* in1,struct ISet* in2){
    for(int i=0;i<24;i++){
            for(int j=0;j<10;j++){
                in1->SetItem[i][j] = in2->SetItem[i][j];
                in1->validItem[i] = in2->validItem[i];
            }
    }
    in1->conf = in2->conf;
}

void CLOSURE(struct ISet* in){
    int num=0;
    for(int i=0;i<24;i++){
            if(in->validItem[i] == 1)
                num++;
    }
    int newnum;
    do{
        newnum = num;
        for(int i=0;i<24;i++){
            if(in->validItem[i] == 1){//存在该项
                for(int k=0;k<10;k++){
                if((k<I[i].len)&&(in->SetItem[i][k]==1)){
                    char c1 = I[i].body[k];
                    for(int j=0;j<24;j++){//存在产生式头部为非终极符号
                        int headnum = I[j].head;
                        char c2 = Int2Sym(headnum);
                        if(c2 == c1){
                            if((in->validItem[j] == 0) || (in->SetItem[j][0] == 0)){
                                newnum++;
                                in->validItem[j] = 1;
                                in->SetItem[j][0] = 1;
                            }
                        }
                    }
                }
                }
            }
        }
    }while(num != newnum);

    /*
    for(int i=0;i<23;i++){
        printf("%d",in->SetItem[i]);
    }*/
}

int SetValid(struct ISet* in){
    int a=0;
    for(int i=0;i<24;i++){
        for(int j=0;j<10;j++)
        if((in->validItem[i] == 1)&&(in->SetItem[i][j] == 1)){
            a=1;
        }
    }
    return a;
}

int ItemValid(struct ISet* in,int j){//若in在j项有效，返回1
    int a=0;
    for(int i=0;i<=I[j].len;i++){
        a = a+in->SetItem[j][i];
    }
    if(a == 0){
        return 0;
    }
    return 1;
}

void PrintI(struct ISet* in){
    for(int k=0;k<24;k++){//打印第k个表达式
        if(in->validItem[k] == 1){
            for(int i=0;i<10;i++){
            if(in->SetItem[k][i] == 1){
            //printf("%d:",k);
            //printf("i: %d:",i);
            printf("%c->",Int2Sym(I[k].head));
            //printf("S:%s\n",I[k].body);
            for(int m=0;m<I[k].len;m++){
                if(m==i){
                    printf("%c",'.');
                    //printf("%c",I[k].body[0]);
                    Convert(I[k].body[m]);
                }
                else{
                    Convert(I[k].body[m]);
                    //printf("%c",I[k].body[0]);
                }
            }
            printf("\n");
        }
        }
        }
    }
}

void Init(){//初始化
    memset(GOTO,-1,sizeof(GOTO));
    memset(First,0,sizeof(First));
    memset(Follow,0,sizeof(Follow));

    memset(SStack.StatStack,0,sizeof(SStack.StatStack));
    SStack.topptr = 0;
    PushStack(0);

// init Action table
    for(int i=0;i<200;i++){
        for(int j=0;j<27;j++){
            ACT[i][j].status_code = 0;
            ACT[i][j].movein = 0;
            ACT[i][j].movenum = 0;
            ACT[i][j].reduce = -1;
            ACT[i][j].reducelen = 0;
        }
    }

    First[2][19] = 1;
    Follow[0][18] = 1;

    I[0].head = 0;
    strcpy(I[0].body , str);
    I[0].len = strlen(str);
    I[1].head = 1;
    strcpy(I[1].body , str1);
    I[1].len = strlen(str1);
    I[2].head = 2;
    strcpy(I[2].body , str2);
    I[2].len = strlen(str2);
    I[3].head = 1;
    strcpy(I[3].body , str3);
    I[3].len = strlen(str3);
    I[4].head = 4;
    strcpy(I[4].body , str4);
    I[4].len = strlen(str4);
    I[5].head = 4;
    strcpy(I[5].body , str5);
    I[5].len = strlen(str5);
    I[6].head = 3;
    strcpy(I[6].body , str6);
    I[6].len = strlen(str6);
    I[7].head = 3;
    strcpy(I[7].body , str7);
    I[7].len = strlen(str7);
    I[8].head = 3;
    strcpy(I[8].body , str8);
    I[8].len = strlen(str8);
    I[9].head = 3;
    strcpy(I[9].body , str9);
    I[9].len = strlen(str9);
    I[10].head = 3;
    strcpy(I[10].body , str10);
    I[10].len = strlen(str10);
    I[11].head = 5;
    strcpy(I[11].body , str11);
    I[11].len = strlen(str11);
    I[12].head = 5;
    strcpy(I[12].body , str12);
    I[12].len = strlen(str12);
    I[13].head = 5;
    strcpy(I[13].body , str13);
    I[13].len = strlen(str13);
    I[14].head = 6;
    strcpy(I[14].body , str14);
    I[14].len = strlen(str14);
    I[15].head = 6;
    strcpy(I[15].body , str15);
    I[15].len = strlen(str15);
    I[16].head = 6;
    strcpy(I[16].body , str16);
    I[16].len = strlen(str16);
    I[17].head = 8;
    strcpy(I[17].body , str17);
    I[17].len = strlen(str17);
    I[18].head = 8;
    strcpy(I[18].body , str18);
    I[18].len = strlen(str18);
    I[19].head = 8;
    strcpy(I[19].body , str19);
    I[19].len = strlen(str19);
    I[20].head = 7;
    strcpy(I[20].body , str20);
    I[20].len = strlen(str20);
    I[21].head = 7;
    strcpy(I[21].body , str21);
    I[21].len = strlen(str21);
    I[22].head = 7;
    strcpy(I[22].body , str22);
    I[22].len = strlen(str22);
    I[23].head = 2;
    strcpy(I[23].body , str23);
    I[23].len = strlen(str23);
    for(int i=0;i<200;i++){
            for(int j=0;j<24;j++){
                Status[i].validItem[j] = 0;
                for(int k=0;k<10;k++){
                Status[i].SetItem[j][k] = 0;
                }
            }
    }

    //初始化ACTION表

   for(int i=0;i<200;i++){
    InitISet(&Status[i]);
   }

   Status[0].validItem[0] = 1;
   Status[0].SetItem[0][0] = 1;
}

void MergeFirst(int i,int j){//merge j to i
    for(int k=0;k<19;k++){
        if(First[j][k] == 1){
            First[i][k] = 1;
        }
    }
}

void MergeFollow(int i,int j){//merge j to i
    //printf("mergeFollow:%c %c\n",Int2Sym(i),Int2Sym(j));
    for(int k=0;k<19;k++){
        if(Follow[j][k] == 1){
            Follow[i][k] = 1;
        }
    }
}

void MergeFollowFirst(int i,int j){//merge j to i
    //printf("mergeFollowFirst:%c %c\n",Int2Sym(i),Int2Sym(j));
    for(int k=0;k<19;k++){
        if(First[j][k] == 1){
            Follow[i][k] = 1;
        }
    }
}

int CalFirst(){
    int num=0;
    for(int i=0;i<9;i++){
        for(int j=0;j<20;j++){
            if(First[i][j] == 1){
                num++;
            }
        }
    }
    //printf("%d\n",num);
    return num;
}

int CalFollow(){
    int num=0;
    for(int i=0;i<9;i++){
        for(int j=0;j<20;j++){
            if(Follow[i][j] == 1){
                num++;
            }
        }
    }
    //printf("%d\n",num);
    return num;
}


void GetFirst(){
    for(int i=0;i<24;i++){
            char c = I[i].body[0];
            int number = Sym2Int(c);
            if(number>8){
                First[I[i].head][number-9] = 1;
            }
    }
    int oldnum = CalFirst();
    int truenum = oldnum;

    do{
        oldnum = truenum;
        for(int i=0;i<24;i++){
            for(int j=0;j<I[i].len;j++){
                char c= I[i].body[j];
                int number = Sym2Int(c);
                if(number > 8){
                    First[I[i].head][number-9] = 1;
                    break;
                }
                else{
                    MergeFirst(I[i].head,number);
                    if(First[number][19] != 1){
                        break;
                    }
                }
                    if(j == I[i].len - 1){
                            if(First[j][19] == 1){
                                First[I[i].head][19] = 1;
                            }

                    }
            }

        }
    truenum = CalFirst();
    //printf("truenum:%d  oldnum:%d \n",truenum,oldnum);
    }while(truenum != oldnum);
}

void GetFollow(){
    for(int i=0;i<24;i++){
            for(int j=0;j<I[i].len-1;j++){
                char c1 = I[i].body[j];
                char c2 = I[i].body[j+1];
                int number1 = Sym2Int(c1);
                int number2 = Sym2Int(c2);
                if((number1 < 9) && (number2 > 8)){
                    Follow[number1][number2-9] = 1;
                    //printf("add %c to %c\n",Int2Sym(number2),Int2Sym(number1));
            }

            }
    }
    int oldnum = CalFollow();
    int truenum = oldnum;

    do{
        oldnum = truenum;
        for(int i=0;i<24;i++){
            for(int j=0;j<I[i].len-1;j++){
                char c1 = I[i].body[j];
                char c2 = I[i].body[j+1];
                int number1 = Sym2Int(c1);
                int number2 = Sym2Int(c2);
                if((number1 < 9)&&(number2 < 9)){
                    MergeFollowFirst(number1,number2);
                    for(int k=number2;k<I[i].len-1;k++){
                        if(First[k][19] == 1){
                            MergeFollowFirst(number1,k+1);
                            if(k == I[i].len-2){
                                if(First[k+1][19] == 1){
                                    MergeFollow(number1,I[i].head);
                                }
                            }
                        }
                        else{
                            break;
                        }

                    }
                }

            }
            char c3 = I[i].body[I[i].len-1];
            int num = Sym2Int(c3);
            if(num<9){
                MergeFollow(num,I[i].head);
            }

        }
    truenum = CalFollow();
    //printf("truenum:%d  oldnum:%d \n",truenum,oldnum);
    }while(truenum != oldnum);
}


int main()
{
    Init();
    GetFirst();
    GetFollow();

//初始化
    CLOSURE(&Status[0]);

    //PrintI(&Status[0]);

    int status_num_old;

    do{
    status_num_old = status_num;
    for(int sta_num = 0;sta_num <= status_num;sta_num++){//遍历所有状态
        for(int i=0;i<27;i++){//遍历每个符号
            struct ISet TempSet;
            SetCopy(&TempSet,&Status[sta_num]);
            //去除移动到末尾的式子并标记规约
            for(int j=0;j<24;j++){
                if(TempSet.SetItem[j][I[j].len] == 1){
                    TempSet.SetItem[j][I[j].len]=0;
                    if(ItemValid(&TempSet,j)==0){
                       TempSet.validItem[j]=0;
                    }
                }
            }
            char c = Int2Sym(i);
            for(int j=0;j<24;j++){//该符号读入后的情况
                for(int ind=0;ind<10;ind++)
                {
                if(Status[sta_num].SetItem[j][ind] == 1){
                int pos = ind;
                if((Status[sta_num].validItem[j])&&(pos < I[j].len)&&(I[j].body[pos] == c)){
                    TempSet.SetItem[j][pos+1] =1;
                    TempSet.SetItem[j][pos]   =0;
                }
                else{
                    //TempSet.validItem[j] =0;
                    TempSet.SetItem[j][pos]   =0;
                }
            }
            if(ItemValid(&TempSet,j)==0){
                TempSet.validItem[j] =0;
            }
            }
            }
            CLOSURE(&TempSet);
            int exist = 0;//该状态不存在
            int matchSet = 0;//Set turned to
            for(int sta_num2 = 0;sta_num2 <= status_num;sta_num2++){
                if(SetEqual(&Status[sta_num2],&TempSet)){
                    exist = 1;
                    //printf("存在%d相同",sta_num2);
                    matchSet = sta_num2;
                    break;
                }
            }
            if(exist == 0){
                    if(SetValid(&TempSet) == 1){
                    status_num++;
                    GOTO[sta_num][i] = status_num;
                    ACT[sta_num][i].status_code = 1;
                    SetCopy(&Status[status_num],&TempSet);
                    //printf("from status %d\n",sta_num);
                    //printf("%d\n",status_num);
                    //PrintI(&Status[status_num]);
                    //printf("\n");
                    }

            }
            else{
            	GOTO[sta_num][i] = matchSet;
            	ACT[sta_num][i].status_code = 1;
            }
        }
    }
    }while(status_num != status_num_old);


    //get the info of ISETS
    for(int i=0;i<status_num;i++){
        int has_reduce = 0;
        int reduce_num;
        int has_move = 0;//exist Item that has not moved to its' end
        for(int j=0;j<24;j++){

            int length = I[j].len;
            //printf("| i:%d j:%d |",i,j);
            //printf("%d ",length);
                    //printf("%d:%d ",j,length);
            if(Status[i].SetItem[j][length]==1){
                has_reduce = 1;
                reduce_num = j;
                break;
            }
        //printf("/ i:%d j:%d /",i,j);
        }
        if(has_reduce == 1){
                for(int j=0;j<27;j++){
                ACT[i][j].reduce = reduce_num;
                ACT[i][j].reducelen = I[reduce_num].len;
                }

        }

        //printf("i:%d ",i);
        //printf("\n");
        //printf("aaaaa ");
        for(int j=0;j<24;j++){
            for(int k=0;k<I[j].len;k++){
                if(Status[i].SetItem[j][k] == 1){
                    has_move = 1;
                    break;
                    //printf("i:%d j:%d k:%d ",i,j,k);
                }
            }
        }

        if((has_move == 1)&&(has_reduce == 1)){
            Status[i].conf = 1;

        }

        //printf("i:%d  ",i);
    }

    /*
for(int i=0;i<23;i++){
    printf("validItem:%d\n",Status[0].validItem[i]);s
    printf("SetItem:%d\n",Status[0].SetItem[i]);
}
*/
    //PrintI(&Status[0]);

for(int i=0;i<=status_num;i++){
    printf("第%d组\n",i);
    PrintI(&Status[i]);
    printf("\n");
}
/*
for(int i=0;i<=status_num;i++){
	for(int j=0;j<27;j++){
		printf("%2d ",GOTO[i][j] );
	}
	printf("\n");
}
*/
//lr(1) parse
    while(1){
        char c = Datain[parse_pos];
        int cnum = Sym2Int(c);
        if(c == '$'){
            if(GetStackTop() == 1){
                printf("accept");
                break;
            }
            else{
                printf("error");
                break;
            }
        }else{
        int status = GetStackTop();
        printf("\n");
        printf("status:%d datain:%c goto:%d reduce:%d status:%d conf:%d ptr:%d\n",status,c,GOTO[status][cnum],ACT[status][cnum].reduce,ACT[status][cnum].status_code,Status[status].conf,SStack.topptr);
        printf("\n");
        if(Status[status].conf == 1){
            int Itemnum = ACT[status][cnum].reduce;
            int headnum = I[Itemnum].head;
            if(Follow[headnum][cnum-9] == 1){//in Follow Set
                    printf("ptr before pop :%d\n",SStack.topptr);
                    PopStack(ACT[status][cnum].reducelen);
                    printf("ptr after pop :%d\n",SStack.topptr);
                    printf("pop %d statuses\n",ACT[status][cnum].reducelen);
                    int pre_status = GetStackTop();
                    printf("jump to status %d with ptr = %d \n",pre_status,SStack.topptr);
                    printf("get from reduce:%c \n",Int2Sym(I[ACT[status][cnum].reduce].head));
                    printf("go to status %d \n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    printf("push %d status to stack\n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
        }else if(ACT[status][cnum].status_code == 0){
                printf("error");
                break;
            }else{
            int nextstatus = GOTO[status][cnum];
            PushStack(nextstatus);
            printf("push %d status to stack\n",nextstatus);
            parse_pos++;
            }
        }else{
                if(ACT[status][cnum].reduce != -1){
                    printf("ptr before pop :%d\n",SStack.topptr);
                    PopStack(ACT[status][cnum].reducelen);
                    printf("ptr after pop :%d\n",SStack.topptr);
                    printf("pop %d statuses\n",ACT[status][cnum].reducelen);
                    int pre_status = GetStackTop();
                    printf("jump to status %d \n",pre_status);
                    printf("get from reduce:%c \n",Int2Sym(I[ACT[status][cnum].reduce].head));
                    printf("go to status %d \n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    printf("push %d status to stack\n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    continue;
                }
                if(ACT[status][cnum].status_code == 0){
                printf("error");
                break;
            }
            int nextstatus = GOTO[status][cnum];
            PushStack(nextstatus);
            printf("push %d status to stack\n",nextstatus);
            parse_pos++;
        }
    }
    }
    return 0;
}




