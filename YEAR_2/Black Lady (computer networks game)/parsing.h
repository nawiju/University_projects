#ifndef PARSING_H
#define PARSING_H

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

/* Constants */
#define BUSY_OFFSET 6
#define BUSY_START 4

#define DEAL_START 4

#define TRICK_OFFSET 5
#define TRICK_LENGTH 8

#define ENDING_LENGTH 2
#define NUMBER_OF_PLAYERS 4

/* Structures */

// Structure used to store the parameters passed to the client
typedef struct client_parameters {
  char* host;
  uint16_t port;
  int protocol = -1;
  char seat;
  bool is_auto_player = false;
} client_parameters;

// Structure used to store the parameters passed to the server
typedef struct server_parameters {
  uint16_t port = -1;
  char* file;
  int64_t timeout = 5;
} server_parameters;

typedef struct TRICK {
  int round;
  std::list<std::string> cards;
} TRICK;

typedef struct DEAL {
  char seat;
  int round_type;
  std::list<std::string> cards;
} DEAL;

typedef struct TAKEN {
  char seat;
  int round;
  std::list<std::string> cards;
} TAKEN;

typedef struct SCORE {
  std::list<char> seats;
  std::list<int> scores;
} SCORE;

/* Functions */

client_parameters client_parse_parameters(int argc, char* argv[]);
server_parameters server_parse_parameters(int argc, char* argv[]);
bool is_IAM(const std::string& input);
bool is_BUSY(const std::string& input);
bool is_TRICK_from_server(const std::string& input, int round);
bool is_TRICK_from_client(const std::string& input);
bool is_any_TRICK(const std::string& input);
bool is_DEAL(const std::string& input);
bool is_WRONG(const std::string& input, int round);
bool is_TAKEN(const std::string& input, int round,
              std::unordered_set<std::string> CARDS_SET,
              std::string placed_card);
bool is_SCORE(const std::string& input);
bool is_TOTAL(const std::string& input);
bool is_card_placement(const std::string& input);
bool is_cards(const std::string& input);
bool is_tricks(const std::string& input);
std::list<std::string> parse_cards(const std::string& input);
SCORE parse_score(const std::string& message);
DEAL parse_deal(const std::string& input);
TRICK parse_trick_from_server(const std::string& input);
TAKEN parse_taken(const std::string& input);

#endif