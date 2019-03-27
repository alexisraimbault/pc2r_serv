#ifndef GAME_HEADER
#define GAME_HEADER

typedef struct player Player;
typedef struct objectif Objectif;
typedef struct game Game;

Player * makePlayer(char* n);
Objectif * makeObjectif();
Game * makeGame();
void addPlayer(Game * game, Player * player);
void setObjectif(Game * game );

#endif
