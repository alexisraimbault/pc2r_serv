/* Server program example for IPv4 */

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <windows.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "linked_list.h"
#include "thread_list.h"
#include "game.h"
#define MAP_SIZE 50


#define DEFAULT_PORT 45678
// default TCP socket type
#define DEFAULT_PROTO SOCK_STREAM
clock_t start;
List * player_list;
List_t * th_list;
pthread_t th;
int phase;
Game * game;
pthread_mutex_t mut;
typedef struct randCoords{
    double x;
    double y;
    struct randCoords * next;
}RandCoords;

RandCoords * makeRandCoord(){
    RandCoords * newrc = malloc(sizeof(RandCoords));
    newrc->x = 10;
    newrc->y = 10;
    newrc->next = NULL;
}
RandCoords * getRc(RandCoords * rc){
    RandCoords *tmp = rc;
    rc = tmp->next;
    return tmp;
}
RandCoords * rc;

void messageStart(void * args)
{
    SOCKET msgsock = (SOCKET)args;
    int retval1;
    char str[24] = "game starting...";
    retval1 = send(msgsock, str, 24, 0);
    if (retval1 == SOCKET_ERROR)
    {
         fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
    }
    else
         printf("Server: send() is OK in GAME START.\n");
}

void sendUpdates(void* args)
{
    srand(time(NULL));
    while(1){
        pthread_mutex_lock(&mut);
        char scores[((2*game->nbPlayers)+1)*24];
        memset(scores, '_', sizeof(scores));
        char xstr[10];
        memset(xstr, '\0', sizeof(xstr));
        char ystr[10];
        memset(ystr, '\0', sizeof(ystr));
        char scorestr[10];
        memset(scorestr, '\0', sizeof(scorestr));
        int tmpcpt;
        double xnum, ynum;
        int snum;
        int cpt = 0;
        int j,retval1,retval;
        char Buffer[128];
        scores[0] = 'U';
        scores[1] = 'P';
        scores[2] = 'D';
        scores[3] = 'A';
        scores[4] = 'T';
        scores[5] = 'E';
        scores[6] = '/';
        char nbPlay[(int)floor(log10((double)game->nbPlayers))+1];
        memset(nbPlay, '_', sizeof(nbPlay));
        sprintf(nbPlay, "%d", game->nbPlayers);
        int i;
        int l=0;
        while(l<sizeof(nbPlay))
        {
            scores[7+l]=nbPlay[l];
            l++;
        }
        scores[7+l] = '/';

        char x[5];
        memset(x, '\0', sizeof(x));
        char y[5];
        memset(y, '\0', sizeof(y));
        char s[5];
        memset(s, '\0', sizeof(s));
        int cptScore;
        if(game->players != NULL){
            i=1;
            Player * tmpPlayer = game->players;
            while(tmpPlayer != NULL){
                j=0;
                while(tmpPlayer->name[j]!='/')
                {
                    scores[(i*24)+j] = tmpPlayer->name[j];
                    scores[((game->nbPlayers)*24)+(i*24)+j] = tmpPlayer->name[j];
                    j++;
                }
                scores[(i*24)+j] = 'X';
                scores[((game->nbPlayers)*24)+(i*24)+j] = ':';
                j++;
                sprintf(x, "%f", tmpPlayer->x);
                sprintf(s, "%d", tmpPlayer->score);
                cptScore = j;
                for(int k=0; k<sizeof(s);k++)
                {
                    scores[((game->nbPlayers)*24)+(i*24)+cptScore] = s[k];

                    cptScore++;
                }
                printf("SCORES : %s\n", s);
                for(int k=0; k<sizeof(x);k++)
                {
                    scores[(i*24)+j] = x[k];
                j++;
                }
                scores[(i*24)+j] = 'Y';
                j++;
                sprintf(y, "%f", tmpPlayer->y);
                for(int k=0; k<sizeof(y);k++)
                {
                    scores[(i*24)+j] = y[k];
                j++;
                }
                scores[(i*24)+j] = '|';
                scores[((game->nbPlayers)*24)+(i*24)+cptScore] = '|';
                tmpPlayer = tmpPlayer->next;
                i++;
            }
        }
        if(game->players != NULL){
            Player * tmp = game->players;
            while(tmp != NULL){
                printf("sending : %s\n", scores);
                retval1 = send(tmp->sock, scores, sizeof(scores), 0);
                if (retval1 == SOCKET_ERROR)
                {
                     fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                }
                else
                     printf("Server: send() is OK IN UPDATE.\n");

                memset(Buffer, '\0', sizeof(Buffer));
                // In the case of SOCK_STREAM, the server can do recv() and send() on
                // the accepted socket and then close it.
                // However, for SOCK_DGRAM (UDP), the server will do recvfrom() and sendto()  in a loop.

                retval = recv(tmp->sock, Buffer, sizeof(Buffer), 0);
                if (retval == SOCKET_ERROR)
                      {
                    fprintf(stderr,"Server: recv() failed: error %d\n", WSAGetLastError());
                    closesocket(tmp->sock);
                }
               else
                    printf("Server: recv() is OK IN ECOUTE : %s .\n", Buffer);
                if (retval == 0)
                {
                    printf("Server: Client closed connection.\n");
                    closesocket(tmp->sock);

                }
                cpt=0;
                while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
                {
                    xstr[cpt] = Buffer[cpt];
                    cpt ++;
                }
                xnum = atof(xstr);

                cpt ++;
                tmpcpt = cpt;
                while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
                {
                    ystr[cpt - tmpcpt] = Buffer[cpt];
                    cpt++;
                }
                ynum = atof(ystr);
                cpt++;
                tmpcpt = cpt;
                while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
                {
                    scorestr[cpt - tmpcpt] = Buffer[cpt];
                    cpt++;
                }
                scorestr[cpt - tmpcpt] ='\0';
                printf("SCOR : %s\n", scorestr);
                snum = atoi(scorestr);

                printf("UPDATE VALUES : %f - %f -%d", xnum, ynum, snum);
                tmp->x = xnum;
                tmp->y = ynum;
                if(snum != tmp->score)
                {
                    char xobj[10];
                    memset(xobj, '\0', sizeof(xobj));
                    char yobj[10];
                    memset(yobj, '\0', sizeof(yobj));
                    char newObjMessage[48];
                    memset(newObjMessage, '_', sizeof(newObjMessage));
                    float xfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
                    float yfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
                    Objectif * obj = makeObjectif(xfobj,yfobj);
                    game->obj = obj;
                    printf("OBJECTIF RAND pre\n");
                    /*sprintf(xobj,"%f", xfobj);
                    sprintf(yobj,"%f", yfobj);*/
                    gcvt(xfobj, 6, xobj);
                    gcvt(yfobj, 6, yobj);
                    printf("OBJECTIF RAND %s - %s \n", xobj, yobj);
                    newObjMessage[0] = 'O';
                    newObjMessage[1] = 'B';
                    newObjMessage[2] = 'J';
                    newObjMessage[3] = 'E';
                    newObjMessage[4] = 'C';
                    newObjMessage[5] = 'T';
                    newObjMessage[6] = 'I';
                    newObjMessage[7] = 'V';
                    newObjMessage[8] = 'E';
                    newObjMessage[9] = '/';
                    newObjMessage[24] = 'x';
                    int l=25;
                    for(int k = 0; k<sizeof(xobj);k++)
                    {
                        newObjMessage[l]=xobj[k];
                        l++;
                    }
                    newObjMessage[l] = 'y';
                    l++;
                    for(int k = 0; k<sizeof(yobj); k++)
                    {
                        newObjMessage[l]=yobj[k];
                        l++;
                    }
                    newObjMessage[l]='/';
                    if(game->players != NULL){
                        Player * tmp2 = game->players;
                        while(tmp2 != NULL){
                            retval1 = send(tmp2->sock, newObjMessage, sizeof(newObjMessage), 0);
                            if (retval1 == SOCKET_ERROR)
                            {
                                 fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                            }
                            else
                                 printf("Server: send() is OK IN OBJECTIVE %s.\n", newObjMessage);
                            tmp2 = tmp2->next;
                        }
                    }
                    tmp->score = snum;
                }

                tmp = tmp->next;
            }
        }
        pthread_mutex_unlock(&mut);
        //Sleep(100);
    }
}


void ecoute(void * args)
{


    SOCKET msgsock = (SOCKET)args;
    char Buffer[128];
    int retval, retval1;
    int doOnce = 1;
    while(1){
        if(doOnce){
           //pthread_mutex_lock(&mut);
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
            }
           else
                //printf("Server: recv() is OKIN ECOUTE : %s .\n", Buffer);
            if (retval == 0)
                  {
                printf("Server: Client closed connection.\n");
                closesocket(msgsock);

            }
            //pthread_mutex_unlock(&mut);
            //pour gérer le retour a la ligne en fin d'entree clavier
            //Buffer[strlen(Buffer) - 1] = '\0';
            for(int j = 0; j < sizeof(Buffer) ; j++){
                if(Buffer[j] == '\n')
                    Buffer[j] = '\0';
            }
            int i=0;
            char cmd[24];
            while((i < sizeof(Buffer)) && (Buffer[i] != '/')){
                    cmd[i] = Buffer[i];
                i++;
            }

            char name[24];
            memset(name, '\0', sizeof(name));
            char msg[24];
            memset(msg, '\0', sizeof(msg));
            msg[0] = 'N';
            msg[1] = 'E';
            msg[2] = 'W';
            msg[3] = ' ';
            msg[4] = 'P';
            msg[5] = 'L';
            msg[6] = 'A';
            msg[7] = 'Y';
            msg[8] = 'E';
            msg[9] = 'R';
            msg[10] = ' ';



            int cpt = i+1;
            int test = strstr(cmd, "CONNECT");
            int testUpdate = strstr(cmd, "UPDATE");

            if(test != NULL)
            {
                printf("cmd : %s\n", cmd);

                while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
                {
                    msg[cpt-i+10] = Buffer[cpt];
                    name[cpt-i-1] = Buffer[cpt];
                    cpt++;
                }
                name[cpt-i-1] = Buffer[cpt];
                //mutex
                float randx = rc->x;
                float randy = rc->y;
                rc = rc->next;
                printf("rand : %f - %f\n"), randx, randy;
                Player *player = makePlayer(name, msgsock, randx,randy) ;
                if(game->players != NULL){
                    Player * tmp = game->players;
                    while(tmp != NULL){
                        retval1 = send(tmp->sock, msg, sizeof(msg), 0);
                        if (retval1 == SOCKET_ERROR)
                        {
                             fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                        }
                        else
                             printf("Server: send() is OK in WELCOME.\n");

                        tmp = tmp->next;
                    }
                }
                pthread_mutex_lock(&mut);
                addPlayer(game, player);
                pthread_mutex_unlock(&mut);
                char scores[(game->nbPlayers+1)*24];
                memset(scores, '_', sizeof(scores));
                int cpt = 0;
                int j;
                scores[0] = 'W';
                scores[1] = 'E';
                scores[2] = 'L';
                scores[3] = 'C';
                scores[4] = 'O';
                scores[5] = 'M';
                scores[6] = 'E';
                scores[7] = '/';
                char nbPlay[(int)floor(log10((double)game->nbPlayers))+1];
                memset(nbPlay, '_', sizeof(nbPlay));
                sprintf(nbPlay, "%d", game->nbPlayers);

                if(phase){
                    scores[8] = 'j';
                    scores[9] = 'e';
                    scores[10] = 'u';
                    scores[11] = '/';
                    int l=0;
                    while(l<sizeof(nbPlay))
                    {
                        scores[12+l]=nbPlay[l];
                        l++;
                    }
                    scores[12+l] = '/';
                }
                else{
                    scores[8] = 'a';
                    scores[9] = 't';
                    scores[10] = 't';
                    scores[11] = 'e';
                    scores[12] = 'n';
                    scores[13] = 't';
                    scores[14] = 'e';
                    scores[15] = '/';
                    int l=0;
                    while(l<sizeof(nbPlay))
                    {
                        scores[16+l]=nbPlay[l];
                        l++;
                    }
                    scores[16+l] = '/';
                }
                char x[5];
                memset(x, '\0', sizeof(x));
                char y[5];
                memset(y, '\0', sizeof(y));
                if(game->players != NULL){
                    i=1;
                    Player * tmpPlayer = game->players;
                    while(tmpPlayer != NULL){
                        j=0;
                        while(tmpPlayer->name[j]!='/')
                        {
                            scores[(i*24)+j] = tmpPlayer->name[j];
                            j++;
                        }
                        scores[(i*24)+j] = 'X';
                        j++;
                        sprintf(x, "%f", tmpPlayer->x);
                        for(int k=0; k<sizeof(x);k++)
                        {
                            scores[(i*24)+j] = x[k];
                        j++;
                        }
                        scores[(i*24)+j] = 'Y';
                        j++;
                        sprintf(y, "%f", tmpPlayer->y);
                        for(int k=0; k<sizeof(y);k++)
                        {
                            scores[(i*24)+j] = y[k];
                        j++;
                        }
                        scores[(i*24)+j] = '|';
                        tmpPlayer = tmpPlayer->next;
                        i++;
                    }
                }
                retval1 = send(msgsock, scores, sizeof(scores), 0);
                if (retval1 == SOCKET_ERROR)
                {
                     fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                }
                else
                     printf("Server: send() is OK.\n");
                char xobj[10];
                memset(xobj, '\0', sizeof(xobj));
                char yobj[10];
                memset(yobj, '\0', sizeof(yobj));
                char newObjMessage[48];
                memset(newObjMessage, '_', sizeof(newObjMessage));
                printf("OBJECTIF RAND pre\n");
                /*sprintf(xobj,"%f", xfobj);
                sprintf(yobj,"%f", yfobj);*/
                gcvt(game->obj->x, 6, xobj);
                gcvt(game->obj->y, 6, yobj);
                printf("OBJECTIF RAND %s - %s \n", xobj, yobj);
                newObjMessage[0] = 'O';
                newObjMessage[1] = 'B';
                newObjMessage[2] = 'J';
                newObjMessage[3] = 'E';
                newObjMessage[4] = 'C';
                newObjMessage[5] = 'T';
                newObjMessage[6] = 'I';
                newObjMessage[7] = 'V';
                newObjMessage[8] = 'E';
                newObjMessage[9] = '/';
                newObjMessage[24] = 'x';
                int l=25;
                for(int k = 0; k<sizeof(xobj);k++)
                {
                    newObjMessage[l]=xobj[k];
                    l++;
                }
                newObjMessage[l] = 'y';
                l++;
                for(int k = 0; k<sizeof(yobj); k++)
                {
                    newObjMessage[l]=yobj[k];
                    l++;
                }
                newObjMessage[l]='/';
                retval1 = send(msgsock, newObjMessage, sizeof(newObjMessage), 0);
            }
            //TODO : evaluer le message.
            if(testUpdate != NULL)
            {
                printf("UPDATE : %f", Buffer);
                /*
                while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
                {
                    cpt++;
                }
                cpt++;//passer le 'x'
                float nbx, nby;
                char tmpx[24];
                memset(tmpx, '\0', sizeof(tmpx));
                char tmpy[24];
                memset(tmpy, '\0', sizeof(tmpy));
                int tmpIndex = cpt;
                while((cpt < sizeof(Buffer)) && (Buffer[cpt] != 'y'))
                {
                    tmpx[cpt-tmpIndex] = Buffer[tmpIndex];
                    cpt++;
                }
                sscanf(tmpx, "%f", &nbx);
                cpt++; //passer le 'y'
                tmpIndex = cpt;
                while((cpt < sizeof(Buffer)) && (Buffer[cpt] != '/'))
                {
                    tmpy[cpt-tmpIndex] = Buffer[cpt];
                    cpt++;
                }
                sscanf(tmpy, "%f", &nby);
                tmp->x = nbx;
                tmp->y = nby;

                tmp = tmp->next;*/

            }
            /*char scores[(game->nbPlayers+1)*24];
            memset(scores, '_', sizeof(scores));
            cpt = 0;
            int j;
            scores[0] = 'U';
            scores[1] = 'P';
            scores[2] = 'D';
            scores[3] = 'A';
            scores[4] = 'T';
            scores[5] = 'E';
            scores[6] = '/';
            char nbPlay[(int)floor(log10((double)game->nbPlayers))+1];
            memset(nbPlay, '_', sizeof(nbPlay));
            sprintf(nbPlay, "%d", game->nbPlayers);
            int l=0;
            while(l<sizeof(nbPlay))
            {
                scores[7+l]=nbPlay[l];
                l++;
            }
            scores[7+l] = '/';

            char x[5];
            memset(x, '\0', sizeof(x));
            char y[5];
            memset(y, '\0', sizeof(y));
            if(game->players != NULL){
                i=1;
                Player * tmpPlayer = game->players;
                while(tmpPlayer != NULL){
                    j=0;
                    while(tmpPlayer->name[j]!='/')
                    {
                        scores[(i*24)+j] = tmpPlayer->name[j];
                        j++;
                    }
                    scores[(i*24)+j] = 'X';
                    j++;
                    sprintf(x, "%f", tmpPlayer->x);
                    for(int k=0; k<sizeof(x);k++)
                    {
                        scores[(i*24)+j] = x[k];
                    j++;
                    }
                    scores[(i*24)+j] = 'Y';
                    j++;
                    sprintf(y, "%f", tmpPlayer->y);
                    for(int k=0; k<sizeof(y);k++)
                    {
                        scores[(i*24)+j] = y[k];
                    j++;
                    }
                    scores[(i*24)+j] = '|';
                    tmpPlayer = tmpPlayer->next;
                    i++;
                }
            }
            if(game->players != NULL){
                Player * tmp = game->players;
                while(tmp != NULL){
                    retval1 = send(tmp->sock, scores, sizeof(scores), 0);
                    if (retval1 == SOCKET_ERROR)
                    {
                         fprintf(stderr,"Server: send() failed: error %d\n", WSAGetLastError());
                    }
                    else
                         printf("Server: send() is OK IN UPDATE.\n");
                    tmp = tmp->next;
                }
            }*/
            doOnce = 0;
        }
    }
}

void socketsListen(void * args)
{
    srand(time(NULL));
    SOCKET msgsock;
    SOCKET listen_socket = (SOCKET)args;
    struct sockaddr_in local, from;
    int fromlen;

    while(1)
    {
        RandCoords * newrc = makeRandCoord();
        newrc->x = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
        newrc->y = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
        newrc->next = rc;
        rc = newrc;

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
    srand(time(NULL));
    rc = makeRandCoord();
    pthread_t th, th1;
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
    game = makeGame();
    float xfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
    float yfobj = ((float)rand()/(float)(RAND_MAX))*MAP_SIZE;
    Objectif * obj = makeObjectif(xfobj,yfobj);
    game->obj = obj;
    th_list = makelist_t();
    player_list = makelist();
    phase = 0;
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
        while((clock()-start)/CLOCKS_PER_SEC < 5 ){
            Sleep(1000);
        }
        pthread_create (& th, NULL,sendUpdates, NULL);
        printf("game starting ...") ;
        printf("player count : %d\n", size(player_list));
        if(player_list->head != NULL){
            Node * tmp = player_list->head;
            while(tmp != NULL){
                pthread_create (& th, NULL,messageStart, (void*)tmp->data);
                tmp = tmp->next;
            }
        }
        phase = 1;

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
