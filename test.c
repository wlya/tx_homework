#include <stdio.h>
#include "string.h"

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
    return 0;
}
