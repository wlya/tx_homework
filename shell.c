#include "stdio.h"
#include "stdint.h"
#include "base.h"



void func111(char *str)
{
    printf("this is func  %s\n", str);
}

void str_to_upper(unsigned char *s)
{
    while(*s){
        *s = toupper(*(unsigned char*)s);
        s++;
    }
}
void func_AUTHOR(char* str_in, char* str_out){
    vlog("I, %s, have read and understood the course academic integrity policy.\n", "tanxi");
}
void func_IP(char* str_in, char* str_out){
    char myip[IP_ADDR_MAX_LEN];
    get_my_public_ip(myip);
    strncpy(myself_info, myip, IP_ADDR_MAX_LEN);
    vlog("IP:%s\n", myip);
}
void func_PORT(char* str_in, char* str_out){
    vlog("PORT:%d\n", LISTEN_PORT);
}
void func_LIST(char* str_in, char* str_out){
    int online_cout = 0;
    char* str_out_head = str_out;
    char* str_out_copy = str_out+8;
    for(int i = 0; i < MAX_CLIENTS_LIMIT; i++){
        if (clients[i].in_use == IN_USE){
            memcpy(str_out_copy+(sizeof(VClient)*online_cout), clients[i], sizeof(VClient));
            online_cout++;
        }
    }
    *(char*)str_out_head = (char)online_cout;
}
void run_command(unsigned char *str){
    Command cmds[] = {
        {"AUTHOR",  COMMON, func_AUTHOR, "ID func is test ID"},
        {"IP",      COMMON, func_IP,    "LIST description is list users,...."},
        {"PORT",    COMMON, func_PORT,    "HELP print all help messages description is whicnn,...."},
        {"LIST",    COMMON, func111,    "HELP print all help messages description is whicnn,...."},

        {"STATISTICS",  SERVER, func111,    ""},
        {"BLOCKED",     SERVER, func111,    ""},

        {"LOGIN",       CLIENT, func111, ""},
        {"REFRESH",     CLIENT, func111, ""},
        {"SEND",        CLIENT, func111, ""},
        {"BROADCAST",   CLIENT, func111, ""},
        {"BLOCK",       CLIENT, func111, ""},
        {"UNBLOCK",     CLIENT, func111, ""},
        {"LOGOUT",      CLIENT, func111, ""},
        {"EXIT",        CLIENT, func111, ""},
    };
    
    // str_to_upper(str);
    for (int i = 0; i < sizeof(cmds) / sizeof(Command); i++)
    {
        if (strstr(str, cmds[i].cmd) == str){
            if( cmds[i].type == CUR_MODE || cmds[i].type == COMMON){
                cmds[i].func( str + strlen(cmds[i].cmd) );
                break;
            }else{
                loge("You are in xxx mode, but run yyy's command");
            }
            
        }
    }
    return 0;
}


int main(int argc, char const *argv[]){
    char buff[300];
    scanf("%s", buff);
    run_command(buff);
}
