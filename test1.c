#ifndef MYBLOCKS
#define MYBLOCKS

#endif
// gcc test.c -o test -O0 -fno-stack-protector

#ifndef STDIO_H
#define STDIO_H
#endif
#include <string.h>
#include <stdint.h>
#include "11.h"

int cc=115;
int main()
{
	FILE *fp;
	char tmp[256];
	int mark=20;

        UINT8 a;
        printf("%d",aa(5));
        memset(ins_blocks,48,1024);
        
	func1();
	func2();
	
	memset(tmp,0,256);
	if(strstr(argv[2],"output")) mark=21;
	sprintf(tmp,"D:\\Mocov\\seed\\path\\%s_cov",argv[2]+mark);
	fp=fopen(tmp,"w");
	fputs(ins_blocks,fp);
	fclose(fp);
	return 0;


}


void AAA_A(int cmdlen){
    if (cmdlen==0) printf("cmdlen==0");
}
void AAA_AA(int* cmd,int cmdlen){printf("AAA_AA%d",cmd[cmdlen]);}
void func3(int* cmd)
{
	char cc;
        int a = cmd[1];
        int cmdlen = 0;       	
	switch(a)
	{
	case 0x00:
		
		AAA_AA(cmd,cmdlen);
		break;
        case 0x11:
		
		AAA_AA(cmd,cmdlen);
		break;
        default:
		
		AAA_AA(cmd,cmdlen);
		break;			
	}
	return;	
}

bool B553(UINT8* cmdd,int cmdlen){
        UINT8 ccc;
        int a = 1;
        
        switch(a)
	{
	case 0x00:
		
		AAA_A(cmdlen);
                return true;
		break;
        case 0x11:
		
		return false;
		break;
        default:
                return false;
		break;			
	}
        switch(a)
	{
	case 0x00:
		
		AAA_AA(cmdd,cmdlen);
		break;
        case 0x11:
		
		AAA_AA(cmdd,cmdlen);
		break;
        default:
		
		AAA_AA(cmdd,cmdlen);
		break;			
	}
}

void func1(){ func3();}
void func2(){ func3();}


