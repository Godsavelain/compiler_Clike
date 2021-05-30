#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

//for lexical analysis
char Data_in[200];
int symbol_num=0;

int Symbol_value[100]={0};
char Symbol_id[100][10]={'\0'};

int row = 0;
struct table_item{
       char w_name[20];
       int w_kind;// 1 identifier 2 number const 3 char const
       int w_type;
       int w_val;
       int w_addr;
       struct table_item* item_next;
};
struct table_item *table_head,*table_tail;

void ShowError(){
  printf("Error at row %d\n",row );
}

void Get_Code_Context(){
    FILE *fp;
	if((fp=fopen("srcinput.txt","wb"))==NULL)
	{
		printf("\nopen file Fail,close!");
		getchar();
		exit(1);
	}
    char str[50] = "\0";
    printf("please input your codes;each line with an enter\ninput '-1' to exit\n");
    while(gets(str)){
        if(strcmp(str,"-1")==0)
            break;
        fputs(str,fp);
    }
    fclose(fp);
}

//for grammar analysis

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
    case '$': return 27;
}
}

//symbols loaded
char Datain[100];
//char Datain[100] = "ba;ba;ba;a=d;a=d;a=d;e(a>a)a=a+a+d;fa=a-a;$";
int  Value_table[100]={0};
char Name_table[100][10];
int parse_pos=0;//the position of the current symbol under parse

void Init_Load_data(){
    strcpy(Datain,Data_in);
    for(int i=0;i<100;i++){
        Value_table[i]=Symbol_value[i];
        strcpy(Name_table[i],Symbol_id[i]);
    }
}

char inter_code[100][20];//生成的三地址代码
int current_line=0;

void Add_inter_code(char *in){
    strcpy(inter_code[current_line],in);
    current_line++;
}
void Remove_inter_code(int n){
    current_line=current_line-n;
    //printf("remove %d lines\n",n);
}

int tempVariable[1000];//temp variable used to hold the
int used_temp_num=0;//the number of temp_variables

struct Sym_attr{
    int  sym_num;//symbol number
    int  sym_type;
    char name[10];
    char code[10][100];//code
    char final_code[100];
    int  codenum;//the number of lines
    int  value;
    int  value_pos;//point out the position of temp variable which hold the value of the expression
    int  is_digit;//for F
};
struct Sym_attr SymStack[100];//symbol stack
int sym_ptr = 0;

void Push_Sym_Stack(struct Sym_attr *in){
    //printf("sym_ptr before push %d\n",sym_ptr);
    //printf("push %c\n",Int2Sym(in->sym_num));
    SymStack[sym_ptr].sym_num = in->sym_num;
    SymStack[sym_ptr].sym_type = in->sym_type;
    SymStack[sym_ptr].value = in->value;
    SymStack[sym_ptr].is_digit = in->is_digit;
    for(int i=0;i<in->codenum;i++){
        strcpy(SymStack[sym_ptr].code[i],in->code[i]);
    }
    strcpy(SymStack[sym_ptr].final_code,in->final_code);
    strcpy(SymStack[sym_ptr].name,in->name);
    SymStack[sym_ptr].codenum = in->codenum;
    SymStack[sym_ptr].value_pos= in->value_pos;
    sym_ptr++;
    //printf("sym_ptr after push %d\n",sym_ptr);
}

void Pop_Sym_Stack(int num){
    //printf("sym_ptr before pop %d\n",sym_ptr);
    sym_ptr = sym_ptr-num;
    //printf("pop %d syms\n",num);
    //printf("sym_ptr before pop %d\n",sym_ptr);
}

struct Symbol{
    int sym_type;
    char name[10];
    int value;
    int has_init;//has been initialized
};

struct Symbol Symtable[100];//symbol table
int sym_number = 0;

void CreateSym(struct Symbol *in,int type,char* name,int value,int has_init){
    in->sym_type = type;
    in->value = value;
    strcpy(in->name,name);
    in->has_init = has_init;
}

int ChangeSym(char *name,int value){
    for(int i=0;i<=sym_number;i++){
       if( strcmp(name,Symtable[i].name) == 0 ){
        Symtable[i].value = value;
        Symtable[i].has_init = 1;
        return 0;
       }
    }
    return 1;

}

int AddSym(struct Symbol in){
    for(int i=0;i<=sym_number;i++){
       if( strcmp(in.name,Symtable[i].name) == 0 ){
            return 1;//error
       }
    }
    Symtable[sym_number].sym_type = in.sym_type;
    strcpy(Symtable[sym_number].name,in.name);
    Symtable[sym_number].value = in.value;
    Symtable[sym_number].has_init = in.has_init;
    sym_number++;
    return 0;
}

void Reduce_Symbol(int num){//reduce symbol with item num
    //printf("Reduce Sym %d\n",num);
    struct Symbol temp;//Symbol be initialized
    int err;
    int L1;
    struct Sym_attr D;
    D.codenum = 0;
    D.sym_num = 2;
    struct Sym_attr S;
    S.codenum = 0;
    S.sym_num = 3;
    struct Sym_attr E;
    E.codenum = 0;
    E.sym_num = 6;
    struct Sym_attr T;
    T.codenum = 0;
    T.sym_num = 8;
    struct Sym_attr F;
    F.codenum = 0;
    F.sym_num = 7;
    struct Sym_attr L;
    L.codenum = 0;
    L.sym_num = 4;
    struct Sym_attr C;
    C.codenum = 0;
    C.sym_num = 5;
    struct Sym_attr P;
    P.codenum = 0;
    P.sym_num = 1;
    char inter_code1[20];
    char inter_code2[20];
    char inter_code3[20];

    char code[100];
    char final_code[100];
    char Dcode[100];//just a temp string
    switch(num){
    case 23:
        //printf("init %s\n",SymStack[sym_ptr-2].name);
        temp.has_init = 0;
        strcpy(temp.name,SymStack[sym_ptr-2].name);
        temp.sym_type = SymStack[sym_ptr-3].sym_type;
        int err = AddSym(temp);
        if(err == 1){
            printf("error:repeated definition\n");
        }

        switch(SymStack[sym_ptr-3].sym_type){
            case 11:
                strcpy(code,"int");
                break;
            case 12:
                strcpy(code,"float");
                break;
        }

        sprintf(Dcode,"%s %s;",code,SymStack[sym_ptr-2].name);
        strcpy(D.code[0],Dcode);
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&D);
        break;
    case 2:
        //printf("init %s\n",SymStack[sym_ptr-2].name);
        temp.has_init = 0;
        strcpy(temp.name,SymStack[sym_ptr-3].name);
        temp.sym_type = SymStack[sym_ptr-4].sym_type;
        err = AddSym(temp);
        if(err == 1){
            printf("error:repeated definition\n");
        }

        switch(SymStack[sym_ptr-4].sym_type){
            case 11:
                strcpy(code,"int");
                break;
            case 12:
                strcpy(code,"float");
                break;
        }
        sprintf(Dcode,"%s %s;",code,SymStack[sym_ptr-3].name);
        strcpy(D.code[0],Dcode);
        Pop_Sym_Stack(4);
        Push_Sym_Stack(&D);
        break;

        case 1:
        Pop_Sym_Stack(2);
        P.sym_num = 1;
        Push_Sym_Stack(&P);
        break;

        case 3:
        Pop_Sym_Stack(1);
        P.sym_num = 1;
        Push_Sym_Stack(&P);
        break;
        case 4:
        L.sym_type =SymStack[sym_ptr-1].sym_num;
        L.sym_num =4;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&L);
        break;

        case 5:
        L.sym_type =SymStack[sym_ptr-1].sym_num;
        L.sym_num =4;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&L);
        break;

        case 6:
        S.sym_num = 3;
        for(int i=0;i<SymStack[sym_ptr-2].codenum;i++){
            strcpy(S.code[S.codenum],SymStack[sym_ptr-2].code[i]);
            S.codenum++;
        }
        if(SymStack[sym_ptr-2].sym_type==23){
            sprintf(Dcode,"%s=%d;",SymStack[sym_ptr-4].name,SymStack[sym_ptr-2].value);
        }
        else if(SymStack[sym_ptr-2].sym_type==9){
            sprintf(Dcode,"%s=%s;",SymStack[sym_ptr-4].name,SymStack[sym_ptr-2].name);
        }
        else{
            sprintf(Dcode,"%s=reg%d;",SymStack[sym_ptr-4].name,SymStack[sym_ptr-2].value_pos);
        }

        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;
        //printf("\n");
        //printf("= get %d codes from S\n",SymStack[sym_ptr-2].codenum);
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-2].sym_num));
        //printf("\n");
        //S.value_pos =

        Add_inter_code(Dcode);
        ChangeSym(SymStack[sym_ptr-4].name,SymStack[sym_ptr-2].value_pos);
        Pop_Sym_Stack(4);
        Push_Sym_Stack(&S);

        break;

        case 7:
        S.codenum = SymStack[sym_ptr-3].codenum;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            strcpy(S.code[i],SymStack[sym_ptr-3].code[i]);
        }
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        sprintf(Dcode,"if %s goto %d;",SymStack[sym_ptr-3].final_code,current_line+2);
        Add_inter_code(Dcode);
        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;
        S.sym_num = 3;
        sprintf(Dcode,"goto %d;",current_line+SymStack[sym_ptr-1].codenum+1);
        Add_inter_code(Dcode);
        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(S.code[S.codenum],SymStack[sym_ptr-1].code[i]);
            S.codenum++;
        }

        //S.value_pos =
        Pop_Sym_Stack(5);
        Push_Sym_Stack(&S);
        break;

        case 8:

        strcpy(S.code[0],Dcode);
        S.codenum = SymStack[sym_ptr-5].codenum;
        for(int i=0;i<SymStack[sym_ptr-5].codenum+1;i++){
            strcpy(S.code[i],SymStack[sym_ptr-5].code[i]);
        }
        //printf("\n");
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-1].sym_num));
        //printf("\n");

        Remove_inter_code(SymStack[sym_ptr-1].codenum);

        //printf("\n");
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-3].sym_num));
        //printf("\n");

        Remove_inter_code(SymStack[sym_ptr-3].codenum);

        //printf("\n");
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-5].sym_num));
        //printf("\n");

        Remove_inter_code(SymStack[sym_ptr-5].codenum);

        sprintf(Dcode,"if %s goto %d;",SymStack[sym_ptr-5].final_code,current_line+SymStack[sym_ptr-1].codenum+2);
        Add_inter_code(Dcode);

        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;
        S.sym_num = 3;
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(S.code[S.codenum],SymStack[sym_ptr-1].code[i]);
            S.codenum++;
        }
        sprintf(Dcode,"goto %d;",current_line+SymStack[sym_ptr-3].codenum+1);
        Add_inter_code(Dcode);
        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(S.code[S.codenum],SymStack[sym_ptr-3].code[i]);
            S.codenum++;
        }

        //S.value_pos =
        Pop_Sym_Stack(7);
        Push_Sym_Stack(&S);
        break;

    case 9:
        S.codenum = SymStack[sym_ptr-3].codenum;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            strcpy(S.code[i],SymStack[sym_ptr-3].code[i]);
        }
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        L1 = current_line;
        sprintf(Dcode,"if %s goto %d;",SymStack[sym_ptr-3].final_code,current_line+2);
        Add_inter_code(Dcode);
        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;
        S.sym_num = 3;
        sprintf(Dcode,"goto %d;",current_line+SymStack[sym_ptr-1].codenum+2);
        Add_inter_code(Dcode);
        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;

        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(S.code[S.codenum],SymStack[sym_ptr-1].code[i]);
            S.codenum++;
        }
        sprintf(Dcode,"goto %d;",L1);
        Add_inter_code(Dcode);
        strcpy(S.code[S.codenum],Dcode);
        S.codenum++;

        //S.value_pos =
        Pop_Sym_Stack(5);
        Push_Sym_Stack(&S);
        break;

    case 10:
        S.codenum = 0;

        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            //Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(S.code[S.codenum],SymStack[sym_ptr-3].code[i]);
            S.codenum++;
        }

        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            //Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(S.code[S.codenum],SymStack[sym_ptr-1].code[i]);
            S.codenum++;
        }
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&S);
        break;
    case 11:
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        Remove_inter_code(SymStack[sym_ptr-3].codenum);
        C.codenum=0;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(C.code[C.codenum],SymStack[sym_ptr-3].code[i]);
            C.codenum++;
        }
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(C.code[C.codenum],SymStack[sym_ptr-1].code[i]);
            C.codenum++;
        }
        if(SymStack[sym_ptr-3].sym_type == 9){
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"%s>%s",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"%s>%d",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"%s>reg%d",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        else if(SymStack[sym_ptr-3].sym_type == 23){
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"%d>%s",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"%d>%d",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"%d>reg%d",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        else{
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"reg%d>%s",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"reg%d>%d",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"reg%d>reg%d",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }

        Pop_Sym_Stack(3);
        Push_Sym_Stack(&C);
        break;
    case 12:
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        Remove_inter_code(SymStack[sym_ptr-3].codenum);
        C.codenum=0;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(C.code[C.codenum],SymStack[sym_ptr-3].code[i]);
            C.codenum++;
        }
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(C.code[C.codenum],SymStack[sym_ptr-1].code[i]);
            C.codenum++;
        }
        if(SymStack[sym_ptr-3].sym_type == 9){
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"%s<%s",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"%s<%d",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"%s<reg%d",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        else if(SymStack[sym_ptr-3].sym_type == 23){
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"%d<%s",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"%d<%d",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"%d<reg%d",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        else{
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"reg%d<%s",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"reg%d<%d",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"reg%d<reg%d",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&C);
        break;
    case 13:
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        Remove_inter_code(SymStack[sym_ptr-3].codenum);
        C.codenum=0;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(C.code[C.codenum],SymStack[sym_ptr-3].code[i]);
            C.codenum++;
        }
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(C.code[C.codenum],SymStack[sym_ptr-1].code[i]);
            C.codenum++;
        }
                if(SymStack[sym_ptr-3].sym_type == 9){
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"%s==%s",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"%s==%d",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"%s==reg%d",SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        else if(SymStack[sym_ptr-3].sym_type == 23){
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"%d==%s",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"%d==%d",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"%d==reg%d",SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        else{
                if(SymStack[sym_ptr-1].sym_type == 9){
                    sprintf(Dcode,"reg%d==%s",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].name);
                    strcpy(C.final_code,Dcode);
                }
                else if(SymStack[sym_ptr-1].sym_type == 23){
                    sprintf(Dcode,"reg%d==%d",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value);
                    strcpy(C.final_code,Dcode);
                }
                else{
                    sprintf(Dcode,"reg%d==reg%d",SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value_pos);
                    strcpy(C.final_code,Dcode);
                }
        }
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&C);
        break;
    case 14:
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        Remove_inter_code(SymStack[sym_ptr-3].codenum);
        E.codenum=0;
        //printf("\n");
        //printf("add get %d codes from S2\n",SymStack[sym_ptr-3].codenum);
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-3].sym_num));
        //printf("\n");
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(E.code[E.codenum],SymStack[sym_ptr-3].code[i]);
            E.codenum++;
        }
        //printf("\n");
        //printf("add get %d codes from S1\n",SymStack[sym_ptr-1].codenum);
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-1].sym_num));
        //printf("\n");
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(E.code[E.codenum],SymStack[sym_ptr-1].code[i]);
            E.codenum++;
        }

        if(SymStack[sym_ptr-1].sym_type == 23){
                if(SymStack[sym_ptr-3].sym_type == 23){
                    sprintf(Dcode,"reg%d=%d+%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s+%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value);
                }
                else{
                    sprintf(Dcode,"reg%d=reg%d+%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value);
                }

        }
        else if(SymStack[sym_ptr-1].sym_type == 9){
                if(SymStack[sym_ptr-3].sym_type == 23){
                    sprintf(Dcode,"reg%d=%d+%s",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].name);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s+%s",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].name);
                }
                else{
                    sprintf(Dcode,"reg%d=reg%d+%s",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].name);
                }
        }
        else{
            if(SymStack[sym_ptr-3].sym_type == 23){
                    sprintf(Dcode,"reg%d=%d+reg%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value_pos);

                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s+reg%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value_pos);
                }
                else{
                    sprintf(Dcode,"reg%d=reg%d+reg%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value_pos);
                }
        }
        E.value_pos = used_temp_num;
        used_temp_num++;

        Add_inter_code(Dcode);
        strcpy(E.code[E.codenum],Dcode);
        E.codenum++;
        E.sym_type = 6;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&E);
        break;
    case 15:
    	Remove_inter_code(SymStack[sym_ptr-1].codenum);
        Remove_inter_code(SymStack[sym_ptr-3].codenum);
        E.codenum=0;
        //printf("\n");
        //printf("sub get %d codes from S2\n",SymStack[sym_ptr-3].codenum);
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-3].sym_num));
        //printf("\n");
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(E.code[E.codenum],SymStack[sym_ptr-3].code[i]);
            E.codenum++;
        }
        //printf("\n");
        //printf("sub get %d codes from S2\n",SymStack[sym_ptr-1].codenum);
        //printf("POP SYM %c\n",Int2Sym(SymStack[sym_ptr-1].sym_num));
        //printf("\n");
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(E.code[E.codenum],SymStack[sym_ptr-1].code[i]);
            E.codenum++;
        }
        E.value_pos = used_temp_num;
        used_temp_num++;
        if(SymStack[sym_ptr-1].sym_type == 23){
                if(SymStack[sym_ptr-3].sym_type == 23){
                    sprintf(Dcode,"reg%d=%d-%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s-%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value);
                }
                else{
                    sprintf(Dcode,"reg%d=%d-%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value);
                }

        }
        else if(SymStack[sym_ptr-1].sym_type == 9){
                if(SymStack[sym_ptr-3].sym_type == 23){
                    sprintf(Dcode,"reg%d=%d-%s",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].name);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s-%s",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].name);
                }
                else{
                    sprintf(Dcode,"reg%d=reg%d-%s",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].name);
                }
        }
        else{
            if(SymStack[sym_ptr-3].sym_type == 23){
                    sprintf(Dcode,"reg%d=%d-reg%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value_pos);

                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s-reg%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value_pos);
                }
                else{
                    sprintf(Dcode,"reg%d=reg%d-reg%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value_pos);
                }
        }
        E.value_pos = used_temp_num;
        used_temp_num++;
        Add_inter_code(Dcode);
        strcpy(E.code[E.codenum],Dcode);
        E.codenum++;
        E.sym_type = 6;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&E);
        break;
    case 16:
        E.codenum=0;
        E.is_digit = SymStack[sym_ptr-1].is_digit;
        E.value = SymStack[sym_ptr-1].value;
        E.sym_type = SymStack[sym_ptr-1].sym_type;
        strcpy(E.name,SymStack[sym_ptr-1].name);
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            strcpy(E.code[E.codenum],SymStack[sym_ptr-1].code[i]);
            E.codenum++;
        }
        E.value_pos = SymStack[sym_ptr-1].value_pos;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&E);
        break;

    case 17:
        T.codenum=0;
        T.is_digit = SymStack[sym_ptr-1].is_digit;
        T.value = SymStack[sym_ptr-1].value;
        T.sym_type = SymStack[sym_ptr-1].sym_type;
        strcpy(T.name,SymStack[sym_ptr-1].name);
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            strcpy(T.code[T.codenum],SymStack[sym_ptr-1].code[i]);
            T.codenum++;
        }
        T.value_pos = SymStack[sym_ptr-1].value_pos;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&T);
        break;

    case 18:
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        Remove_inter_code(SymStack[sym_ptr-3].codenum);
        T.codenum=0;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(T.code[T.codenum],SymStack[sym_ptr-3].code[i]);
            T.codenum++;
        }
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(T.code[T.codenum],SymStack[sym_ptr-1].code[i]);
            T.codenum++;
        }
        T.value_pos = used_temp_num;
        used_temp_num++;
        if(SymStack[sym_ptr-1].sym_type == 7){
                if(SymStack[sym_ptr-3].sym_type == 7){
                    sprintf(Dcode,"reg%d=reg%d*reg%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value_pos);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s*reg%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value_pos);
                }
                else{
                    sprintf(Dcode,"reg%d=%d*reg%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value_pos);
                }

        }
        else if(SymStack[sym_ptr-1].sym_type == 9){
                if(SymStack[sym_ptr-3].sym_type == 7){
                    sprintf(Dcode,"reg%d=reg%d*%s",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].name);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s*%s",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].name);
                }
                else{
                    sprintf(Dcode,"reg%d=%d*%s",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].name);
                }
        }
        else{
            if(SymStack[sym_ptr-3].sym_type == 7){
                    sprintf(Dcode,"reg%d=reg%d*%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s*%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value);
                }
                else{
                    sprintf(Dcode,"reg%d=%d*%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value);
                }
        }
        Add_inter_code(Dcode);
        strcpy(T.code[T.codenum],Dcode);
        T.codenum++;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&T);
        break;

    case 19:
        Remove_inter_code(SymStack[sym_ptr-1].codenum);
        Remove_inter_code(SymStack[sym_ptr-3].codenum);
        T.codenum=0;
        for(int i=0;i<SymStack[sym_ptr-3].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-3].code[i]);
            strcpy(T.code[T.codenum],SymStack[sym_ptr-3].code[i]);
            T.codenum++;
        }
        for(int i=0;i<SymStack[sym_ptr-1].codenum;i++){
            Add_inter_code(SymStack[sym_ptr-1].code[i]);
            strcpy(T.code[T.codenum],SymStack[sym_ptr-1].code[i]);
            T.codenum++;
        }
        T.value_pos = used_temp_num;
        used_temp_num++;
                if(SymStack[sym_ptr-1].sym_type == 7){
                if(SymStack[sym_ptr-3].sym_type == 7){
                    sprintf(Dcode,"reg%d=reg%d+reg%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value_pos);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s+reg%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value_pos);
                }
                else{
                    sprintf(Dcode,"reg%d=%d+reg%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value_pos);
                }

        }
        else if(SymStack[sym_ptr-1].sym_type == 9){
                if(SymStack[sym_ptr-3].sym_type == 7){
                    sprintf(Dcode,"reg%d=reg%d/%s",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].name);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s/%s",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].name);
                }
                else{
                    sprintf(Dcode,"reg%d=%d/%s",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].name);
                }
        }
        else{
            if(SymStack[sym_ptr-3].sym_type == 7){
                    sprintf(Dcode,"reg%d=reg%d/%d",used_temp_num,SymStack[sym_ptr-3].value_pos,SymStack[sym_ptr-1].value);
                }
                else if(SymStack[sym_ptr-3].sym_type == 9){
                    sprintf(Dcode,"reg%d=%s/%d",used_temp_num,SymStack[sym_ptr-3].name,SymStack[sym_ptr-1].value);
                }
                else{
                    sprintf(Dcode,"reg%d=%d/%d",used_temp_num,SymStack[sym_ptr-3].value,SymStack[sym_ptr-1].value);
                }
        }
        Add_inter_code(Dcode);
        strcpy(T.code[T.codenum],Dcode);
        T.codenum++;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&T);
        break;
    case 20:
        F.codenum=0;
        F.sym_type=7;
        F.value = SymStack[sym_ptr-2].value;
        F.is_digit = SymStack[sym_ptr-2].is_digit;
        for(int i=0;i<SymStack[sym_ptr-2].codenum;i++){
            strcpy(F.code[F.codenum],SymStack[sym_ptr-2].code[i]);
            F.codenum++;
        }
        F.value_pos = SymStack[sym_ptr-2].value_pos;
        Pop_Sym_Stack(3);
        Push_Sym_Stack(&F);
        break;
    case 21:
        F.is_digit=0;
        F.sym_type=9;
        F.value = SymStack[sym_ptr-1].value;
        strcpy(F.name,SymStack[sym_ptr-1].name);
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&F);
        break;
    case 22:
        F.is_digit=1;
        F.sym_type=23;
        F.value = SymStack[sym_ptr-1].value;
        F.value = SymStack[sym_ptr-1].value;
        Pop_Sym_Stack(1);
        Push_Sym_Stack(&F);
        break;
    }
}

void Insert_Symbol_id(char *name){
    struct Sym_attr A;
    A.sym_num = 9;
    A.codenum = 0;
    strcpy(A.name,name);
    Push_Sym_Stack(&A);
}

void Insert_Symbol_digits(int digits){
    struct Sym_attr A;
    A.sym_num = 23;
    A.codenum = 0;
    A.value = digits;
    Push_Sym_Stack(&A);
}

void Insert_Symbol(int n){
    struct Sym_attr A;
    A.codenum = 0;
    A.sym_num=11;
    A.sym_type = 11;
    Push_Sym_Stack(&A);
}

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
    char str10[10] = "SS";
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

int GOTO[200][28];//GOTO表，描述状态转移
struct Action ACT[200][28];

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

void Init(){//初始化
    memset(GOTO,-1,sizeof(GOTO));
    memset(First,0,sizeof(First));
    memset(Follow,0,sizeof(Follow));

    memset(SStack.StatStack,0,sizeof(SStack.StatStack));
    SStack.topptr = 0;
    PushStack(0);

// init Action table
    for(int i=0;i<200;i++){
        for(int j=0;j<28;j++){
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
   int after_else = 0;
   Get_Code_Context();
   char sentance_input[200];
   char word_token[20];
   char next_c;
   int w_forward=0,w_next=0;
   int w_state=0;
   int x_num=0,id_x=0;


   char w_keyword[][8]={"if","else","while","do","int","float"};

   FILE *fp_soure;
   table_head =(struct table_item *)malloc(sizeof(struct table_item));
   table_tail =(struct table_item *)malloc(sizeof(struct table_item));

   table_tail=table_head=NULL;
   fp_soure=fopen("srcinput.txt", "r+");


   if (fp_soure!=NULL)
   {
      memset(sentance_input, 0x00, sizeof (char) * 200);
      printf("Token generated:\n");

      //reading all lines
      while(EOF != fscanf(fp_soure,"%[^\n]\n",sentance_input)){ //%*c
       //printf("the input is %s \n",sentance_input);
       row++;
       w_next=0;
       w_forward=0;
       w_state=0;
       x_num=0;

       while(1){
        switch (w_state){
           case 0:

                 // deleting the backspaces in front of line
                 while((next_c=sentance_input[w_next])==' ' || next_c=='\t' ){// || next_c=='\t'
                      w_next++;
                      w_forward++;
                 }
                 memset(word_token, 0x00, sizeof (char) * 20);
                 x_num=0;

                // processing the digits
                if (isdigit(next_c)){
                     while( (next_c!=' ') && (isdigit(next_c) || next_c=='.')){ //
                          w_next++;
                          next_c=sentance_input[w_next];
                     }

                     if(next_c==' '|| !(isalpha(next_c)))
                        w_next--;
                     if(isalpha(next_c))
                        {
                          ShowError();
                        }

                     x_num=0;
                     while(w_forward<=w_next){
                        word_token[x_num]=sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                     }
                     word_token[w_forward+1]='\0';
                     printf("token is [ const digits ,%s]\n",word_token);
                     //strcpy(Symbol_id[symbol_num],word_token);
                     Data_in[symbol_num] = 'd';
                     int num = atoi(word_token);
                     Symbol_value[symbol_num] = num;
                     symbol_num++;

                    w_state=0;
                    w_next= w_forward;
                    break;
                }
                // processing the id
                if (isalpha(next_c)){

                     while( (next_c!=' ') && (isalpha(next_c) || isalnum(next_c) || next_c=='_')){
                          w_next++;
                          next_c=sentance_input[w_next];

                     }

                     if(next_c==' ' || !(isalpha(next_c)))
                        w_next--;

                     x_num=0;
                     while(w_forward<=w_next){
                        word_token[x_num]=sentance_input[w_forward];
                        w_forward++;
                        x_num++;
                     }
                     word_token[w_forward+1]='\0';
                     //printf("token is %s\n",word_token);

                     for(int i=0;i<6;i++){
                        //printf("word_token:%s  w_keyword[i]:%s\n",word_token,w_keyword[i]);
                        if(strcmp(word_token,w_keyword[i])==0){
                          printf("token is [ keyword ,%s]\n",word_token);
                          id_x=1;//keyword not in table
                            switch(i){
                            case 0:
                                Data_in[symbol_num] = 'e';
                                symbol_num++;
                                break;
                            case 1:
                                Data_in[symbol_num] = 'f';
                                symbol_num++;
                                after_else=1;
                                break;
                            case 2:

                                Data_in[symbol_num] = 'g';
                                symbol_num++;
                                break;
                            case 3:
                                break;
                            case 4:

                                Data_in[symbol_num] = 'b';
                                symbol_num++;
                                break;
                            case 5:

                                Data_in[symbol_num] = 'c';
                                symbol_num++;
                                break;
                            }
                          break;
                        }
                    }
                    if (id_x==0){
                        printf("token is [ id ,%s]\n",word_token);
                        strcpy(Symbol_id[symbol_num],word_token);
                        Data_in[symbol_num] = 'a';
                        symbol_num++;
                    }

                    w_state=0;
                    id_x=0;
                    w_next= w_forward;
                    break;
                 }
                 // processing the relation operators, the calculating operators and the other operators.
                  switch (next_c){
                     case '<':w_state=1;
                              break;
                     case '=':w_state=5;
                              break;
                     case '>':w_state=6;
                              break;
                     case '+':w_state=9;
                              break;
                     case '-':w_state=10;
                              break;
                     case '*':w_state=11;
                               break;
                     case '/':w_state=12;
                              break;
                     case '(':w_state=13;
                              break;
                     case ')':w_state=14;
                              break;
                     case ';':w_state=15;
                              break;
                     case '!':w_state=16;
                              break;
                     case '\'':w_state=17;
                              break;
                    case '\0':w_state=100;
                              //sentance_input="";
                              memset(sentance_input, 0x00, sizeof (char) * 200);
                              break;
                    //default : printf("error!");break;
                    }

                    //printf("state is %d\n",w_state);
                    break;
            case 1:
                  w_next++;
                  next_c=sentance_input[w_next];
                  switch (next_c){
                    case '=': w_state=2;
                             break;
                    case '>': w_state=3;
                             break;
                    default: w_state=4;
                             Data_in[symbol_num] = '<';
                             symbol_num++;
                             break;
                  }

                  break;
            case 2:
                  x_num=0;
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward+1]='\0';
                  printf("token is [ op ,%s]\n",word_token);
                  w_state=0;
                  w_next= w_forward;
                  break;
             case 3:

                  x_num=0;
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward+1]='\0';
                  printf("token is [ op ,%s]\n",word_token);
                  w_state=0;
                  w_next= w_forward;
                  break;
             case 4:
                  w_next--;

                  x_num=0;
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward]='\0';
                  printf("token is [ op ,%s]\n",word_token);
                  w_state=0;
                  w_next= w_forward;
                  break;
             case 5:
                  x_num=0;
                  if( sentance_input[w_next+1] == '='){
                          w_next++;
                          next_c=sentance_input[w_next];
                     }
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward]='\0';
                  printf("token is [ op ,%s]\n",word_token);
                  Data_in[symbol_num] = '=';
                  symbol_num++;
                  w_state=0;
                  w_next= w_forward;
                  break;
             case 9:
             case 10:
             case 11:
             case 12:
             case 13:
             case 14:
             case 15:
             case 17:
                 switch(w_state){
                 case 9:
                    Data_in[symbol_num] = '+';
                    symbol_num++;
                    break;
                 case 10:
                    Data_in[symbol_num] = '-';
                    symbol_num++;
                    break;
                 case 11:
                    Data_in[symbol_num] = '*';
                    symbol_num++;
                    break;
                 case 12:
                    Data_in[symbol_num] = '/';
                    symbol_num++;
                    break;
                 case 13:
                    Data_in[symbol_num] = '(';
                    symbol_num++;
                    break;
                 case 14:
                    Data_in[symbol_num] = ')';
                    symbol_num++;
                    break;
                 case 15:
                     //printf("after_else:%d\n",after_else);
                    if(after_else==0){
                    Data_in[symbol_num] = ';';
                    symbol_num++;
                    }

                    break;

                 }
                  x_num=0;
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward]='\0';
                  if(!((strcmp(word_token,";")==0)&&(after_else==1)))
                  {
                      printf("token is [ op ,%s]\n",word_token);
                  }
                        if(after_else == 1){
                        after_else = 0;
                      }

                  w_state=0;
                  w_next= w_forward;
                  break;
             case 16:
                  x_num=0;
                  if( sentance_input[w_next+1] == '='){
                          w_next++;
                          next_c=sentance_input[w_next];
                     }
                  else
                    ShowError();
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward]='\0';
                  printf("token is [ op ,%s]\n",word_token);
                  w_state=0;
                  w_next= w_forward;
                  break;
             case 6:
                  w_next++;
                  next_c=sentance_input[w_next];
                  switch (next_c){
                    case '=': w_state=7;
                             break;
                    default: w_state=8;
                             Data_in[symbol_num] = '>';
                             symbol_num++;
                             break;
                  }

                  break;
             case 7:
                  x_num=0;
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward]='\0';
                  printf("token is [ op ,%s]\n",word_token);
                  w_state=0;
                  w_next= w_forward;
                  break;
             case 8:
                  w_next--;
                  x_num=0;
                  while(w_forward<=w_next){
                    word_token[x_num]=sentance_input[w_forward];
                    w_forward++;
                    x_num++;
                  }
                  word_token[w_forward]='\0';
                  printf("token is [ op ,%s]\n",word_token);
                  w_state=0;
                  w_next= w_forward;
                  break;
             case 100: break;
        }
        if (w_state==100) break;
      }

    }
    fclose(fp_soure);
   }
   else{
     printf("open file error!\n");
   }
   Data_in[symbol_num] = '$';
   symbol_num++;
   Data_in[symbol_num] = '\0';




/*
printf("\n");
printf("data in: \n");
printf("%s\n",Data_in);
printf("\n");
printf("id in: \n");
for(int i=0;i<symbol_num-1;i++){
    printf("%d:%s ",i,Symbol_id[i]);
}
printf("\n");
printf("value in: \n");
for(int i=0;i<symbol_num-1;i++){
printf("%3d",Symbol_value[i]);
}
*/
printf("lexical analysis has completed,press any key to start grammar analysis\n");
printf("grammar analysis can only use '=' '>' '<' '==' '+' '-' '*' '/' '(' ')' 'if' 'else' 'while'\n");
//printf("Data_in:%s\n",Data_in);
    //for grammar analysis
    Init();
    Init_Load_data();
    //printf("%s\n",Datain);
    GetFirst();
    GetFollow();

//初始化
    CLOSURE(&Status[0]);

    //PrintI(&Status[0]);

    int status_num_old;

    do{
    status_num_old = status_num;
    for(int sta_num = 0;sta_num <= status_num;sta_num++){//遍历所有状态
        for(int i=0;i<28;i++){//遍历每个符号
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
    for(int i=0;i<=status_num;i++){
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
                for(int j=0;j<28;j++){
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
/*

for(int i=0;i<=status_num;i++){
    printf("第%d组\n",i);
    PrintI(&Status[i]);
    printf("\n");
}

for(int i=0;i<=status_num;i++){
	for(int j=0;j<28;j++){
		printf("%2d ",ACT[i][j].reduce );
	}
	printf("\n");
}
*/
//lr(1) parse
    while(1){
        char c = Datain[parse_pos];
        int cnum = Sym2Int(c);
        int value = Value_table[parse_pos];
        char name[10];
        strcpy(name,Name_table[parse_pos]);
        if(c == '$'){
            if(GetStackTop() == 1){
                printf("\n");
                printf("grammar analysis answer:");
                printf("accept!\n");
                printf("\n");
                break;
            }
            /*
            else{
                printf("error");
                break;
            }
            */
        }

        int status = GetStackTop();
        //printf("\n");
        //printf("status:%d datain:%c goto:%d reduce:%d status:%d conf:%d ptr:%d\n",status,c,GOTO[status][cnum],ACT[status][cnum].reduce,ACT[status][cnum].status_code,Status[status].conf,SStack.topptr);
        //printf("\n");
        if(Status[status].conf == 1){
            int Itemnum = ACT[status][cnum].reduce;
            int headnum = I[Itemnum].head;
            if((Follow[headnum][cnum-9] == 1)&&(cnum != 25)){//in Follow Set  对'else'特殊规定
                    //printf("ptr before pop :%d\n",SStack.topptr);
                    PopStack(ACT[status][cnum].reducelen);
                    //printf("ptr after pop :%d\n",SStack.topptr);
                    //printf("pop %d statuses\n",ACT[status][cnum].reducelen);
                    int pre_status = GetStackTop();
                    //printf("jump to status %d with ptr = %d \n",pre_status,SStack.topptr);
                    //printf("get from reduce:%c \n",Int2Sym(I[ACT[status][cnum].reduce].head));
                    Reduce_Symbol(ACT[status][cnum].reduce);
                    //printf("go to status %d \n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    //printf("push %d status to stack\n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
        }else if(ACT[status][cnum].status_code == 0){
                printf("error");
                break;
            }else{
            int nextstatus = GOTO[status][cnum];
            PushStack(nextstatus);

            switch(cnum){
                case 9:
                    Insert_Symbol_id(name);
                    break;
                case 23:
                    Insert_Symbol_digits(value);
                    break;
                default:
                    Insert_Symbol(cnum);
            }

            //printf("push %d status to stack\n",nextstatus);
            parse_pos++;
            }
        }else{
                //printf("111status:%d cnum:%d reduce:%d \n",status,cnum,ACT[status][cnum].reduce);
                if(ACT[status][cnum].reduce != -1){
                    //printf("ptr before pop :%d\n",SStack.topptr);
                    PopStack(ACT[status][cnum].reducelen);
                    //printf("ptr after pop :%d\n",SStack.topptr);
                    //printf("pop %d statuses\n",ACT[status][cnum].reducelen);
                    int pre_status = GetStackTop();
                    //printf("jump to status %d \n",pre_status);
                    //printf("status:%d cnum:%d reduce:%d head:%d\n",status,cnum,ACT[status][cnum].reduce,I[ACT[status][cnum].reduce].head);
                    //printf("get from reduce:%c \n",Int2Sym(I[ACT[status][cnum].reduce].head));
                    Reduce_Symbol(ACT[status][cnum].reduce);
                    //printf("go to status %d \n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    PushStack(GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    //printf("push %d status to stack\n",GOTO[pre_status][I[ACT[status][cnum].reduce].head]);
                    continue;
                }
                if(ACT[status][cnum].status_code == 0){
                printf("error");
                break;
            }
            int nextstatus = GOTO[status][cnum];

            switch(cnum){
                case 9:
                    Insert_Symbol_id(name);
                    break;
                case 23:
                    Insert_Symbol_digits(value);
                    break;
                default:
                    Insert_Symbol(cnum);
            }

            PushStack(nextstatus);
            //printf("push %d status to stack\n",nextstatus);
            parse_pos++;
        }
    }

    printf("final code generated\n");
    for(int i=0;i<current_line;i++){
        printf("%2d  %s\n",i,inter_code[i]);
    }
    printf("\n");
    printf("symbol table:\n");


    for(int i=0;i<sym_number;i++){

        switch(Symtable[i].sym_type)
    {
        case 11:
        printf("symbol type:int  ");
        break;
        case 12:
        printf("symbol type:int  ");
        break;
    }

        printf("symbol name:%s\n",Symtable[i].name);

    }
    return 0;
}




