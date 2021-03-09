#include <stdio.h>
#include "string.h"


typedef struct test
{
    int aa;
    char bb[12];
    int cc;
}test;

#define sp(fff, ...)  printf( "%d:%s:" fff ,__LINE__,__FUNCTION__,__VA_ARGS__)
int main(int argc, char const *argv[])
{
    int ret = 234;
    ret = 234;
    char s1[] = "1234567";
    char s2[] = "123";
    char s3[] = "456";
    ret = strstr(s1, s2);
    printf("ret= %d, %d\n", ret, s1);
    ret = 234;
    ret = strstr(s1, s3);
    printf("ret= %d, %d\n", ret, s1);
    ret = 234;
    ret = strstr("1234 56787", "4444");
    printf("ret= %d\n", ret);
    char b0[] = "1.2.44.56 fasdfnasdfasdf";
    char* b1 = strchr(b0, ' ');
    char b2[16];
    memset(b2, 0 , 16);
    strncpy(b2, b0, b1-b0);
    // b2[b1-b0]='\0';
    printf("%s\n", b2);

    test aa = {998877};
    printf("%d\n", aa.aa);

    sp("[%s]", "helloworld");
    return 0;
}
