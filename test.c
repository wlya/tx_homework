#include <stdio.h>
#include "string.h"
#include "stdint.h"
#include "base.h"
#include "time.h"
typedef struct test
{
    int aa;
    char bb[12];
    int cc;
}test;

#define sp(fff, ...)  printf( "%d:%s:" fff ,__LINE__,__FUNCTION__,__VA_ARGS__)


int is_blocked(uint64_t src, uint32_t dst_fd){
    uint64_t tmp = 0x1 << dst_fd;
    if(src & tmp){
        printf("%d is blocked %08x\n", dst_fd, src);
        return 1;
    }
    printf("%d not is blocked %08x\n", dst_fd, src);
    return 0;
}

typedef struct VClientList
{
    VClient* client;
    struct VClientList* next;
}VClientList;


void dump_list(VClientList* head){
    printf("------------\n");
    VClientList* cur_ptr = head;
    while (cur_ptr && cur_ptr->client)
    {
        printf("ptr->port %d\n", cur_ptr->client->port);
        cur_ptr = cur_ptr->next;
    }
    printf("------------\n");

}

VClientList* array_to_sorted_list(VClient* vclients, size_t size){
    VClientList* listHead = (VClientList*) malloc(sizeof(VClientList));
    listHead->next = NULL;
    int i = 0;
    printf("000\n");
    // for(i = 0;i < size; i++){
    //     if(vclients[i].status != UN_SEE){
    //         listHead->client = &vclients[i];
    //         break;
    //     }
    // }
    printf("1111\n");
    // listHead->client = 0;
    for(i = 0; i < size; i++){
        if(vclients[i].status == UN_SEE) continue;
        printf("1111\n");

        VClientList* repeat_ptr = listHead->next;
        VClientList* tptr = (VClientList*)malloc(sizeof(VClientList));
        tptr->client = &vclients[i];
        printf("%016x, %p\n", repeat_ptr->next, tptr);

        while (repeat_ptr != NULL && tptr->client->port > repeat_ptr->client->port){
            printf("%016x, %p\n", repeat_ptr, tptr, tptr);
            repeat_ptr = repeat_ptr->next;
        }
        tptr->next = repeat_ptr->next;
        repeat_ptr->next = tptr;
        dump_list(listHead->next);

    }
    return listHead;
    
}
int cmp_vclient(VClient* t1, VClient* t2){
    return t1->port > t2->port?1:-1;
}
int main(int argc, char const *argv[])
{
    srand((unsigned)time(NULL)); 

    VClient tmpClients[MAX_CLIENTS_LIMIT];
    for(int i = 0; i < 20; i++){
        tmpClients[i].status = LOGGED_IN;
        tmpClients[i].fd = 22;
        tmpClients[i].port = rand()%100+1;
    }
    for (size_t i = 0; i < 20; i++){
        printf("unsorted: %d, %d\n", i, tmpClients[i].port);
    }
    printf("=====================\n");
    qsort(tmpClients, 20, sizeof(VClient), cmp_vclient);
    for (size_t i = 0; i < 20; i++){
        printf("sorted: %d, %d\n", i, tmpClients[i].port);
    }
    exit(0);

    // array_to_sorted_list(tmpClients, 20);
    // memcpy(tmpClients, tmpClients)
    // VClientList* listHead = (VClientList*) malloc(sizeof(VClientList));
    // // listHead->client = 0;
    // VClientList* cur_ptr = listHead;
    // for(int i = 0; i < 10; i++){
    //     cur_ptr->client = malloc(sizeof(VClient));
    //     memset(cur_ptr->client, 0 , sizeof(VClient));
    //     cur_ptr->client->fd = rand()%100;
    //     if(i == 9) break;
    //     cur_ptr->next = malloc(sizeof(VClient));
    //     cur_ptr = cur_ptr->next;
    // }
    // dump_list(listHead);


    exit(0);
    
    //
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

    sp("[%s]\n", "helloworld");
    uint64_t ii = 0b000000000000;
    int i = 0;
    ii |= 0x1<<3;
    is_blocked(ii, 3);
    
    ii &= ~(0x1<<3);
    is_blocked(ii, 3);


    ii |= 0x1<<3;
    is_blocked(ii, 3);
    
    ii &= ~(0x1<<3);
    is_blocked(ii, 3);


    
    return 0;
}
