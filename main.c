/* Server program example for IPv4 */

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "linked_list.h"
#include "thread_list.h"
#include "game.h"



#define DEFAULT_PORT 45678
// default TCP socket type
#define DEFAULT_PROTO SOCK_STREAM
clock_t start;
List * player_list;
List_t * th_list;

void messageStart(void * args)
{
    SOCKET msgsock = (SOCKET)args;
    int retval1;
    retval1 = send(msgsock, "GAME STARTING", 12, 0);
    if (retval1 == SOCKET_ERROR)
    {
         fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
    }
    else
         printf("Server: send() is OK.\n");
}



void ecoute(void * args)
{
    SOCKET msgsock = (SOCKET)args;
    char Buffer[128];
    int retval, retval1;
    while(1)
   {
        //possibilité d'accepter de nouvelles connexions pendant 20 secs
        memset(Buffer, '\0', sizeof(Buffer));
        // In the case of SOCK_STREAM, the server can do recv() and send() on
        // the accepted socket and then close it.
        // However, for SOCK_DGRAM (UDP), the server will do recvfrom() and sendto()  in a loop.

        retval = recv(msgsock, Buffer, sizeof(Buffer), 0);
        if (retval == SOCKET_ERROR)
              {
            fprintf(stderr,"Server: recv() failed: error %d\n", WSAGetLastError());
            closesocket(msgsock);
            continue;
        }
       else
            printf("Server: recv() is OK.\n");
        if (retval == 0)
              {
            printf("Server: Client closed connection.\n");
            closesocket(msgsock);
            continue;
        }
        Buffer[strlen(Buffer) - 1] = '\0';
        for(int j = 0; j < sizeof(Buffer) ; j++){
            if(Buffer[j] == '\n')
                Buffer[j] = '\0';
            else
                Buffer[j] = toupper(Buffer[j]);
        }
        printf("Server: Received %d bytes, data \"%s\" from client, buff size : %d \n", retval, Buffer,sizeof(Buffer));

        //TODO : evaluer le message.
        retval1 = send(msgsock, Buffer, retval, 0);
          if (retval1 == SOCKET_ERROR)
          {
                 fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
            }
          else
                 printf("Server: send() is OK.\n");
    }
}

void socketsListen(void * args)
{
    SOCKET msgsock;
    SOCKET listen_socket = (SOCKET)args;
    struct sockaddr_in local, from;
    int fromlen;
    pthread_t th;
    while(1)
    {
        printf("elapsed time : %d\n", (clock()-start)/ CLOCKS_PER_SEC);
        fromlen =sizeof(from);
        msgsock = accept(listen_socket, (struct sockaddr*)&from, &fromlen);
        if (msgsock == INVALID_SOCKET)
       {
            fprintf(stderr,"Server: accept() error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }
       else
       {
          printf("Server: accept() is OK.\n");
          clock_t start = clock();
          add(msgsock, player_list);

          pthread_create (& th, NULL,ecoute, (void*)msgsock);
          add_t(th, th_list);
          printf("Server: accepted connection from %s, port %d\n", inet_ntoa(from.sin_addr), htons(from.sin_port)) ;
          printf("player count : %d\n", size(player_list));
       }
    }
}

int main(int argc, char **argv)

{
    pthread_t th;
    char Buffer[128];
    char BufferSend[128];
    char *ip_address= NULL;
    unsigned short port=DEFAULT_PORT;
    int retval;
    int fromlen;
    int socket_type = DEFAULT_PROTO;
    struct sockaddr_in local, from;
    WSADATA wsaData;
    SOCKET listen_socket, msgsock;

    th_list = makelist_t();
    player_list = makelist();

    /* Parse arguments, if there are arguments supplied */
    // Request Winsock version 2.2

    if ((retval = WSAStartup(0x202, &wsaData)) != 0)
    {
        fprintf(stderr,"Server: WSAStartup() failed with error %d\n", retval);
        WSACleanup();
        return -1;
    }
    else
        printf("Server: WSAStartup() is OK.\n");
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = (!ip_address) ? INADDR_ANY:inet_addr(ip_address);

    /* Port MUST be in Network Byte Order */
    local.sin_port = htons(port);


    // TCP socket
    listen_socket = socket(AF_INET, socket_type,0);

    if (listen_socket == INVALID_SOCKET){
        fprintf(stderr,"Server: socket() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    else
       printf("Server: socket() is OK.\n");



    // bind() associates a local address and port combination with the socket just created.

    // This is most useful when the application is a

    // server that has a well-known port that clients know about in advance.
    if (bind(listen_socket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
       {
        fprintf(stderr,"Server: bind() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    else
        printf("Server: bind() is OK.\n");

     // So far, everything we did was applicable to TCP as well as UDP.

     // However, there are certain steps that do not work when the server is

     // using UDP. We cannot listen() on a UDP socket.
    if (listen(listen_socket,5) == SOCKET_ERROR)
          {
        fprintf(stderr,"Server: listen() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
   else
        printf("Server: listen() is OK.\n");

    printf("Server: I'm listening and waiting connection\non port %d, protocol %s\n", port, (socket_type == SOCK_STREAM)?"TCP":"UDP");
    // accept() doesn't make sense on UDP, since we do not listen()
        fromlen =sizeof(from);
        msgsock = accept(listen_socket, (struct sockaddr*)&from, &fromlen);
        if (msgsock == INVALID_SOCKET)
        {
            fprintf(stderr,"Server: accept() error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }
        else
        {
            printf("Server: accept() is OK.\n");
            printf("Server: accepted connection from %s, port %d\n", inet_ntoa(from.sin_addr), htons(from.sin_port)) ;
        }

        start = clock();
        add(msgsock, player_list);
        pthread_create (& th, NULL,ecoute, (void*)msgsock);
        add_t(th, th_list);
        printf("starting countdown...");
        printf("start time : %d\n", start/ CLOCKS_PER_SEC);
        pthread_create (& th, NULL,socketsListen, (void*)listen_socket);
        while((clock()-start)/CLOCKS_PER_SEC < 20 ){
            Sleep(1000);
        }
        printf("game starting ...") ;
        printf("player count : %d\n", size(player_list));
        if(player_list->head != NULL){
            Node * tmp = player_list->head;
            while(tmp != NULL){
                pthread_create (& th, NULL,messageStart, (void*)tmp->data);
                tmp = tmp->next;
            }

        }

        if(th_list->head != NULL){
            Node_t * tmp_t = th_list->head;
            while(tmp_t != NULL){
                pthread_join (tmp_t->data, NULL);
                tmp_t = tmp_t->next;
            }
        }

       /*
       while(1)

       {

        //possibilité d'accepter de nouvelles connexions pendant 20 secs

        memset(Buffer, '\0', sizeof(Buffer));

        // In the case of SOCK_STREAM, the server can do recv() and send() on

        // the accepted socket and then close it.

        // However, for SOCK_DGRAM (UDP), the server will do recvfrom() and sendto()  in a loop.

        if (socket_type != SOCK_DGRAM)

            retval = recv(msgsock, Buffer, sizeof(Buffer), 0);



       else

       {

            retval = recvfrom(msgsock,Buffer, sizeof(Buffer), 0, (struct sockaddr *)&from, &fromlen);

            printf("Server: Received datagram from %s\n", inet_ntoa(from.sin_addr));

        }



        if (retval == SOCKET_ERROR)

              {

            fprintf(stderr,"Server: recv() failed: error %d\n", WSAGetLastError());

            closesocket(msgsock);

            continue;

        }

       else

            printf("Server: recv() is OK.\n");



        if (retval == 0)

              {

            printf("Server: Client closed connection.\n");

            closesocket(msgsock);

            continue;

        }
        Buffer[strlen(Buffer) - 1] = '\0';
        for(int j = 0; j < sizeof(Buffer) ; j++){
            if(Buffer[j] == '\n')
                Buffer[j] = '\0';
            else
                Buffer[j] = toupper(Buffer[j]);
}
        printf("Server: Received %d bytes, data \"%s\" from client, buff size : %d \n", retval, Buffer,sizeof(Buffer));


        printf("Server: Echoing the same data back to client...\n");

        if (socket_type != SOCK_DGRAM)

            retval1 = send(msgsock, Buffer, retval, 0);

        else

            retval1 = sendto(msgsock, Buffer, sizeof(Buffer), 0, (struct sockaddr *)&from, fromlen);



              if (retval1 == SOCKET_ERROR)

              {

                     fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());

               }

              else

                     printf("Server: send() is OK.\n");



        /*if (socket_type != SOCK_DGRAM)

       {

            printf("Server: I'm waiting more connection, try running the client\n");

            printf("Server: program from the same computer or other computer...\n");

            closesocket(msgsock);

        }

        else

            printf("Server: UDP server looping back for more requests\n");

        continue;*//*

    }*/

       return 0;

}
