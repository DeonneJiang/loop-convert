#include <stdio.h>


int function1(){ int a = 1; return a;}

int function2(){ int a = 2; return a;}


int main(){
    int a[100] = {0};
    function1();
    function2();
    return 0;
    
}