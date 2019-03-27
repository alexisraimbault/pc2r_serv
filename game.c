#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"
#define MAP_SIZE 50

struct player {
  char * name;
  float dir, x, y, vx, vy;
  int score;
  struct player * next;
};

struct objectif {
  float x, y;
};

struct game {
  Player * players;
  Objectif * obj;
  int x, y;
};

Player * makePlayer(char * n)
{
  srand(time(NULL));
  Player * newPlayer = malloc(sizeof(Player));
  if (!newPlayer)
    return NULL;
  newPlayer->name = n;
  newPlayer->dir = 0;
  newPlayer->x = rand()/RAND_MAX*MAP_SIZE;
  newPlayer->y = rand()/RAND_MAX*MAP_SIZE;
  newPlayer->vx = 0;
  newPlayer->vy = 0;
  newPlayer->next = NULL;
  newPlayer->score = 0;
  return newPlayer;
}

Objectif * makeObjectif()
{
  srand(time(NULL));
  Objectif * newObj = malloc(sizeof(Objectif));
  if (!newObj)
    return NULL;
  newObj->x = rand()/RAND_MAX*MAP_SIZE;
  newObj->y = rand()/RAND_MAX*MAP_SIZE;
  return newObj;
}

Game * makeGame()
{
  Game * newGame = malloc(sizeof(Game));
  if (!newGame)
    return NULL;
  newGame->x = MAP_SIZE;
  newGame->y = MAP_SIZE;
  newGame->players = NULL;
  newGame->obj = NULL;
  return newGame;
}

void addPlayer(Game * game, Player * player)
{
  if(game->players == NULL){
    game->players = player;
  }else{
    Player * tmp = game->players;
    while(tmp->next != NULL)
      tmp = tmp->next;
    tmp->next = player;
  }
}

void setObjectif(Game * game )
{
  game->obj = makeObjectif();
}
