#include "parsing.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <climits>
#include <list>
#include <regex>
#include <string>
#include <unordered_set>

#include "common.h"
#include "err.h"

/* Additional functions */
bool check_seats(const std::string& input) {
  std::unordered_set<char> seats;
  for (int i = 5; i < input.size() - ENDING_LENGTH; i++) {
    if (!isdigit(input[i])) {
      seats.insert(input[i]);
    }
  }

  return seats.size() == NUMBER_OF_PLAYERS;
}

/* Functions parsing arguments the program is run with */

// Parses the parameters passed to the client. Exits the program if the required
// parameters are missing or parameters are invalid or incomplete. Saves the
// last instance of a parameter if it is repeated.
client_parameters client_parse_parameters(int argc, char* argv[]) {
  client_parameters params;

  bool host_set = false, port_set = false, seat_set = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        fatal(ERROR_MISSING_HOST);
      }

      const char* host = argv[++i];
      host_set = true;
      params.host = (char*)host;

      if (strlen(host) > HOST_NAME_MAX) {
        fatal(ERROR_HOST_TOO_LONG);
      }

      continue;
    }

    if (strcmp(argv[i], "-p") == 0) {
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        fatal(ERROR_MISSING_PORT);
      }

      params.port = read_port(argv[++i]);
      port_set = true;

      continue;
    }

    if (strcmp(argv[i], "-4") == 0 || strcmp(argv[i], "-6") == 0) {
      if (strcmp(argv[i], "-4") == 0) {
        params.protocol = AF_INET;
      } else {
        params.protocol = AF_INET6;
      }

      continue;
    }

    if (strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "-W") == 0 ||
        strcmp(argv[i], "-N") == 0 || strcmp(argv[i], "-S") == 0) {
      params.seat = argv[i][1];
      seat_set = true;
      continue;
    }

    if (strcmp(argv[i], "-a") == 0) {
      params.is_auto_player = true;
      continue;
    }

    fatal(ERROR_INVALID_PARAMETER, argv[i]);
  }

  if (!host_set) {
    fatal(ERROR_MISSING_HOST_PARAMETER);
  }

  if (!port_set) {
    fatal(ERROR_MISSING_PORT_PARAMETER);
  }

  if (!seat_set) {
    fatal(ERROR_MISSING_SEAT_PARAMETER);
  }

  return params;
}

// Parses the parameters passed to the server. Exits the program if the required
// parameters are missing or parameters are invalid or incomplete. Saves the
// last instance of a parameter if it is repeated.
server_parameters server_parse_parameters(int argc, char* argv[]) {
  server_parameters params;

  bool port_set = false, file_set = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0) {
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        fatal(ERROR_MISSING_PORT);
      }

      params.port = read_port(argv[++i]);
      port_set = true;

      continue;
    }

    if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        fatal(ERROR_MISSING_FILE);
      }

      params.file = argv[++i];
      file_set = true;

      continue;
    }

    if (strcmp(argv[i], "-t") == 0) {
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        fatal(ERROR_MISSING_TIMEOUT);
      }

      params.timeout = strtol(argv[++i], NULL, 10);

      if (params.timeout < 0) {
        fatal(ERROR_NEGATIVE_TIMEOUT, argv[i]);
      }
    }
  }

  if (!file_set) {
    fatal(ERROR_MISSING_FILE_PARAMETER);
  }

  return params;
}

/* Message parsing functions */

bool is_IAM(const std::string& input) {
  std::regex pattern(R"(IAM[NESW]\r\n)");
  return std::regex_match(input, pattern);
}

bool is_BUSY(const std::string& input) {
  std::regex pattern(R"(BUSY[NESW]{1,4}\r\n)");

  if (!std::regex_match(input, pattern)) {
    return false;
  }

  std::unordered_set<char> seats(input.begin() + 4,
                                 input.end() - ENDING_LENGTH);

  return seats.size() == input.size() - 6;
}

bool is_DEAL(const std::string& input) {
  std::regex pattern(
      R"(DEAL[1-7][NESW]((2|3|4|5|6|7|8|9|10|J|Q|K|A)[C|D|H|S]){13}\r\n)");

  if (!std::regex_match(input, pattern)) {
    return false;
  }

  std::list<std::string> cards = parse_cards(input.substr(6));
  std::unordered_set<std::string> card_set(cards.begin(), cards.end());

  return card_set.size() == cards.size();  // Check if there are no duplicates
}

bool is_TRICK_from_server(const std::string& input, int round) {
  std::regex pattern(
      R"(TRICK([1-9]|1[0-3])((2|3|4|5|6|7|8|9|10|J|Q|K|A)[C|D|H|S]){0,3}\r\n)");

  if (!std::regex_match(input, pattern)) {
    return false;
  }

  std::list<std::string> cards = parse_cards(input.substr(6));
  int length = 0;

  for (std::string card : cards) {
    length += card.size();
  }

  int round_number = stoi(input.substr(5, input.size() - 7 - length));
  return round_number == round;
}

bool is_any_TRICK(const std::string& input) {
  std::regex pattern(R"(TRICK.*\r\n)");
  return std::regex_match(input, pattern);
}

bool is_TRICK_from_client(const std::string& input) {
  std::regex pattern(
      R"(TRICK([1-9]|1[0-3])((2|3|4|5|6|7|8|9|10|J|Q|K|A)[CDHS]){1}\r\n)");
  return std::regex_match(input, pattern);
}

bool is_WRONG(const std::string& input, int round) {
  std::regex pattern(R"(WRONG([1-9]|1[0-3])\r\n)");
  return std::regex_match(input, pattern) &&
         stoi(input.substr(5, input.size() - 7)) == round;
}

bool is_TAKEN(const std::string& input, int round,
              std::unordered_set<std::string> CARDS_SET,
              std::string placed_card) {
  std::regex pattern(
      R"(TAKEN([1-9]|1[0-3])((2|3|4|5|6|7|8|9|10|J|Q|K|A)[C|D|H|S]){4}[NESW]\r\n)");

  if (!std::regex_match(input, pattern)) {
    return false;
  }

  std::list<std::string> cards = parse_cards(input);
  std::unordered_set<std::string> card_set(cards.begin(), cards.end());

  int mine = 0;
  for (std::string card : cards) {
    if (CARDS_SET.find(card) != CARDS_SET.end()) {
      mine++;
      if (placed_card != "" && card != placed_card) {
        return false;
      }
    }
  }

  if (mine != 1) {
    return false;
  }

  int length = 0;
  for (std::string card : cards) {
    length += card.size();
  }

  int round_number = stoi(input.substr(5, input.size() - 8 - length));
  return card_set.size() == cards.size() &&
         round_number == round;  // Check if there are no duplicates
}

bool is_SCORE(const std::string& input) {
  std::regex pattern(
      R"(SCORE([NESW])(\d+)([NESW])(\d+)([NESW])(\d+)([NESW])(\d+)\r\n)");
  return std::regex_match(input, pattern) && check_seats(input);
}

bool is_TOTAL(const std::string& input) {
  std::regex pattern(
      R"(TOTAL([NESW])(\d+)([NESW])(\d+)([NESW])(\d+)([NESW])(\d+)\r\n)");
  return std::regex_match(input, pattern) && check_seats(input);
}

bool is_card_placement(const std::string& input) {
  std::regex pattern(R"(!([2-9]|10|J|Q|K|A)[CDHS])");
  return std::regex_match(input, pattern);
}

bool is_tricks(const std::string& input) { return input == "tricks"; }

bool is_cards(const std::string& input) { return input == "cards"; }

std::list<std::string> parse_cards(const std::string& input) {
  std::list<std::string> cards;
  std::regex card_pattern(R"((2|3|4|5|6|7|8|9|10|J|Q|K|A)[C|D|H|S])");

  auto cards_begin =
      std::sregex_iterator(input.begin(), input.end(), card_pattern);
  auto cards_end = std::sregex_iterator();

  for (std::sregex_iterator i = cards_begin; i != cards_end; ++i) {
    std::string card = i->str();
    cards.push_back(card);
  }

  return cards;
}

SCORE parse_score(const std::string& message) {
  SCORE score;
  std::regex score_pattern(R"([NESW]\d+)");
  auto scores_begin =
      std::sregex_iterator(message.begin(), message.end(), score_pattern);
  auto scores_end = std::sregex_iterator();

  for (std::sregex_iterator i = scores_begin; i != scores_end; ++i) {
    std::string score_ = i->str();
    score.seats.push_back(score_[0]);
    score.scores.push_back(stoi(score_.substr(1)));
  }

  return score;
}

DEAL parse_deal(const std::string& input) {
  DEAL deal;
  std::list<std::string> cards = parse_cards(input.substr(6));

  deal.seat = input[5];
  deal.cards = cards;
  deal.round_type = stoi(input.substr(4, 1));

  return deal;
}

TRICK parse_trick_from_server(const std::string& input) {
  TRICK trick;
  std::list<std::string> cards = parse_cards(input);
  int64_t length = 0;

  for (std::string card : cards) {
    length += card.size();
  }

  trick.round = stoi(input.substr(5, input.size() - 7 - length));
  trick.cards = cards;

  return trick;
}

TAKEN parse_taken(const std::string& input) {
  TAKEN taken;
  std::list<std::string> cards = parse_cards(input.substr(5));
  int64_t length = 0;

  for (std::string card : cards) {
    length += card.size();
  }

  taken.seat = input[input.size() - 3];
  taken.cards = cards;
  taken.round = stoi(input.substr(5, input.size() - 8 - length));

  return taken;
}
