#ifndef GAME_HEADER
#define GAME_HEADER

typedef struct player Player;
typedef struct objectif Objectif;
typedef struct game Game;

struct player {
  char * name;
  SOCKET sock;
  float dir, x, y, vx, vy;
  int score;
  struct player * next;
};

struct objectif {
  float x, y;
};

struct game {
  int nbPlayers;
  Player * players;
  Objectif * obj;
  int x, y;
};

Player * makePlayer(char* n, SOCKET s, float x, float y);
Objectif * makeObjectif(float xrand, float yrand);
Game * makeGame();
void addPlayer(Game * game, Player * player);

#endif
