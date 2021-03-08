#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "base.h"

#ifdef DEBUG
int main(int argc, char* argv[]){
    LISTEN_PORT = 2020;
    server_main(argc, argv);
    return 0;
}
#endif



int server_main(int argc, char *argv[])
{
    memset(clients, 0, sizeof(clients));
    /* master file descriptor list */
    fd_set master;
    /* temp file descriptor list for select() */
    fd_set read_fds;
    /* server address */
    struct sockaddr_in serveraddr;
    /* client address */
    struct sockaddr_in clientaddr;
    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener;
    /* newly accept()ed socket descriptor */
    int newfd;
    /* buffer for client data */
    char buf[1024];
    int nbytes;
    /* for setsockopt() SO_REUSEADDR, below */
    int yes = 1;
    int addrlen;
    int i, j;
    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    /* get the listener */
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Server-socket() error lol!");
        /*just exit lol!*/
        exit(1);
    }
    printf("Server-socket() is OK...\n");
    /*"address already in use" error message */
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }
    printf("Server-setsockopt() is OK...\n");
    /* bind */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(LISTEN_PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);
    if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("Server-bind() error lol!");
        exit(1);
    }
    printf("Server-bind() is OK...\n");
    /* listen */
    if (listen(listener, 10) == -1)
    {
        perror("Server-listen() error lol!");
        exit(1);
    }
    printf("Server-listen() is OK...\n");
    /* add the listener to the master set */
    FD_SET(0, &master);
    FD_SET(listener, &master);
    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one*/
    /* loop */
    for (;;)
    {
        /* copy it */
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Server-select() error lol!");
            exit(1);
        }
        logd("Server-select() is OK...\n");
        /*run through the existing connections looking for data to be read*/
        // vhit: stdin input handler
        if (FD_ISSET(0, &read_fds))
        {
            char cmd_buffer[512];
            nbytes = read(0, cmd_buffer, sizeof(cmd_buffer));
            run_command(cmd_buffer);
            // if(nbytes>0){
            //     write(0, cmd_buffer, strnlen(cmd_buffer, 200));
            //     // printf("ffffffffff: %d, [%s]\n", nbytes, ss);
            // }
        }
        for (i = 1; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            { /* we got one... */
                if (i == listener)
                {
                    /* handle new connections */
                    addrlen = sizeof(clientaddr);
                    if ((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
                    {
                        perror("Server-accept() error lol!");
                    }
                    else
                    {
                        logd("IP address is: %s\n", inet_ntoa(clientaddr.sin_addr));
                        logd("port is: %d\n", (int) ntohs(clientaddr.sin_port));
               VClient        logd("Server-accept() is OK...\n");

                        //vhit: init clinet info
                        clients[newfd].in_use = IN_USE;
                        clients[newfd].fd = newfd;
                        strncpy(clients[newfd].ip_addr, inet_ntoa(clientaddr.sin_addr), IP_ADDR_MAX_LEN);
                        strncpy(clients[newfd].hostname, inet_ntoa(clientaddr.sin_addr), IP_ADDR_MAX_LEN);
                        clients[newfd].num_msg_sent = 0;
                        clients[newfd].num_msg_rcv =  0;
                        
                        FD_SET(newfd, &master); /* add to master set */
                        
                        if (newfd > fdmax)
                        { /* keep track of the maximum */
                            fdmax = newfd;
                        }
                        
                        printf("%s: New connection from %s on socket %d\n", argv[0], inet_ntoa(clientaddr.sin_addr), newfd);
                    }
                }
                else
                {
                    /* handle data from a client */
                    if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
                    {
                        //vhit: erase logouted client msg
                        memset(&clients[i], 0 , sizeof(clients[i]));
                        /* got error or connection closed by client */
                        if (nbytes == 0)
                        {
                            /* connection closed */
                            printf("%s: socket %d hung up\n", argv[0], i);
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                        }
                        else
                        {
                            /* close it... */
                            perror("recv() error lol!");
                        }
                        close(i);
                        /* remove from master set */
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        /* we got some data from a client*/
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        logd("msg from:client %d, send>>>%s\n", i, buf);
                        // logd("msg from:%s, to:%s\n[msg]:%s\n", );
                        for (j = 1; j <= fdmax; j++)
                        {
                            /* send to everyone! */
                            if (FD_ISSET(j, &master))
                            {
                                /* except the listener and ourselves */
                                if (j != listener && j != i)
                                {
                                    if (send(j, buf, nbytes, 0) == -1)
                                        logd("send() error lol! j = %d ", j);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}