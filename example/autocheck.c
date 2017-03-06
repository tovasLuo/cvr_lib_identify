#include <stdio.h>
#include "../THID_CVR_IDCard.h"

int main(int argc,char* argv[])
{
    int count = 1;
    //printf("The autocheck command line has %d arguments:\n", argc - 1);
    int n_ret = AutoCheckDevice((char*)argv[count]);
    if(n_ret == CVR_ERR_NONE){
        printf("%s idcard_dev connected!\n",(char*)argv[count]);
        return 1;
    }
    printf("%s idcard_dev no connected!\n",(char*)argv[count]);
    return 0;
}
