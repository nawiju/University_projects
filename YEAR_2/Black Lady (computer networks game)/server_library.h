#ifndef SERVER_LIBRARY_H
#define SERVER_LIBRARY_H

#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "err.h"
#include "parsing.h"

/* Constants */
#define SCORE 0
#define TOTAL_SCORE 1

#define N 0
#define E 1
#define S 2
#define W 3

/* Structures */
enum class Rule {
  RULE_1 = 1,  // Do not take tricks, for each trick taken 1 point
  RULE_2 = 2,  // Do not take hearts, for each heart taken you get 1 point
  RULE_3 = 3,  // Do not take queens, for each queen taken you get 5 points
  RULE_4 = 4,  // Do not take lords (jacks and kings), for each lord taken you
               // get 2 points
  RULE_5 = 5,  // You get 18 points for taking the king of hearts
  RULE_6 = 6,  // Do not take the seventh and last trick, for taking each of
               // these tricks you get 10 points
  RULE_7 = 7   // Points are given for everything mentioned above
};

enum class Rank {
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  FIVE = 5,
  SIX = 6,
  SEVEN = 7,
  EIGHT = 8,
  NINE = 9,
  TEN = 10,
  JACK = 11,
  QUEEN = 12,
  KING = 13,
  ACE = 14
};

typedef struct Card {
  Card(char s, Rank r) : suit(s), rank(r) {}
  char suit;
  Rank rank;
} Card;

typedef struct GAME {
  int round;       // Number of the round [1, 13]
  long long deal;  // Number of the deal
  int rule;
  char start_player;
  int current_player;       // Who is playing now
  bool has_been_poked;      // Has the current player been poked
  bool started_poking;      // Has the poking started
  std::vector<Card> table;  // Cards on the table
  std::vector<std::vector<Card>> in_hand;
  std::vector<std::string>
      tricks_taken;  // Cards taken by the players in this round in the correct
                     // format and order
} GAME;

/* Functions */
bool operator==(const Card& a, const Card& b);
bool operator!=(const Card& a, const Card& b);
int char_to_player(char player);
std::string player_to_str(int player);
std::string rank_to_string(Rank& rank);
Rank string_to_rank(const std::string& rank);
void make_deal(GAME& game, std::vector<std::string>& game_definition,
               std::unordered_map<int, std::queue<std::pair<std::string, int>>>&
                   buffer_to_send);
std::string make_trick(GAME round);
std::string make_wrong(int trick);
std::string calculate_score(GAME game, std::vector<std::vector<long long>>& points);
std::string cards_to_string(std::vector<Card>& cards);

#endif