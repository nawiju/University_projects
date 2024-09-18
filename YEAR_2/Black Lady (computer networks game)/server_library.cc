#include "server_library.h"

/* Operators for the Card struct */
bool operator==(const Card& a, const Card& b) {
  try {
    if ((a.suit == b.suit) && (a.rank == b.rank)) {
      return true;
    }
  } catch (...) {
    return false;
  }

  return false;
}

bool operator!=(const Card& a, const Card& b) { return !(a == b); }

bool operator<(const Card& a, const Card& b) {
  if (((&a) == NULL) || ((&b) == NULL)) {
    return false;
  }

  if (a.suit == b.suit) {
    return a.rank < b.rank;
  }

  return false;
}

bool operator>(const Card& a, const Card& b) {
  if (((&a) == NULL) || ((&b) == NULL)) {
    return false;
  }

  if (a.suit == b.suit) {
    return a.rank > b.rank;
  }

  return false;
}

/* Other functions */
std::unordered_map<char, int> char_to_player_map = {
    {'N', N}, {'S', S}, {'E', E}, {'W', W}};

std::unordered_map<int, std::string> player_to_str_map = {
    {N, "N"}, {S, "S"}, {E, "E"}, {W, "W"}};

std::unordered_map<std::string, Rank> string_to_rank_map = {
    {"2", Rank::TWO},   {"3", Rank::THREE}, {"4", Rank::FOUR},
    {"5", Rank::FIVE},  {"6", Rank::SIX},   {"7", Rank::SEVEN},
    {"8", Rank::EIGHT}, {"9", Rank::NINE},  {"10", Rank::TEN},
    {"J", Rank::JACK},  {"Q", Rank::QUEEN}, {"K", Rank::KING},
    {"A", Rank::ACE}};

int char_to_player(char player) { return char_to_player_map[player]; }

std::string player_to_str(int player) { return player_to_str_map[player]; }

std::string rank_to_string(Rank& rank) {
  switch (rank) {
    case Rank::TWO:
      return "2";

    case Rank::THREE:
      return "3";

    case Rank::FOUR:
      return "4";

    case Rank::FIVE:
      return "5";

    case Rank::SIX:
      return "6";

    case Rank::SEVEN:
      return "7";

    case Rank::EIGHT:
      return "8";

    case Rank::NINE:
      return "9";

    case Rank::TEN:
      return "10";

    case Rank::JACK:
      return "J";

    case Rank::QUEEN:
      return "Q";

    case Rank::KING:
      return "K";

    case Rank::ACE:
      return "A";
  }

  return "";
}

Rank string_to_rank(const std::string& rank) {
  return string_to_rank_map[rank];
}

// Parses the information about the deal and creates a GAME struct. Prepares
// messages to send to the clients with the deal information.
void make_deal(GAME& game, std::vector<std::string>& game_definition,
               std::unordered_map<int, std::queue<std::pair<std::string, int>>>&
                   buffer_to_send) {
  std::string line = game_definition[0];
  game.rule = line[0] - '0';
  game.start_player = line[1];
  game.table.clear();
  game.in_hand.clear();
  game.tricks_taken.clear();
  game.current_player = char_to_player(line[1]);
  game.has_been_poked = false;
  game.started_poking = false;

  for (int i = 0; i < 4; i++) {
    std::list<std::string> cards = parse_cards(game_definition[i + 1]);
    game.in_hand.push_back({});
    for (auto card : cards) {
      Rank rank = string_to_rank(card.substr(0, card.size() - 1));
      game.in_hand[i].push_back(Card(card[card.size() - 1], rank));
    }

    std::string message = "DEAL" + std::to_string(game.rule) +
                          game.start_player + game_definition[i + 1] + ENDING;
    buffer_to_send[i].push({message, 0});  // DEAL
  }
}

// Creates a TRICK message to send to the clients.
std::string make_trick(GAME game) {
  std::string trick = "TRICK";
  trick += std::to_string(game.rule);

  for (Card card : game.table) {
    trick += card.suit + rank_to_string(card.rank);
  }

  trick += ENDING;
  return trick;
}

// Gives the number of the round in the deal.
std::string make_wrong(int trick) {
  return "WRONG" + std::to_string(trick) + ENDING;
}

// Returns a string with the taken message.
std::string calculate_score(GAME game, std::vector<std::vector<long long>>& points) {
  int start_player = game.current_player;
  Card max_card = game.table[0];
  int max_index = 0;
  std::string cards_taken = cards_to_string(game.table);

  for (int i = 1; i < game.table.size(); i++) {
    if (game.table[i] > max_card) {
      max_card = game.table[i];
      max_index = i;
    }
  }

  int taking_player = (start_player + max_index) % 4;

  if (game.rule == static_cast<int>(Rule::RULE_1) ||
      game.rule == static_cast<int>(Rule::RULE_7)) {
    points[taking_player][SCORE]++;
    points[taking_player][TOTAL_SCORE]++;
  }

  if (game.rule == static_cast<int>(Rule::RULE_2) ||
      game.rule == static_cast<int>(Rule::RULE_7)) {
    for (Card card : game.table) {
      if (card.suit == 'H') {
        points[taking_player][SCORE]++;
        points[taking_player][TOTAL_SCORE]++;
      }
    }
  }

  if (game.rule == static_cast<int>(Rule::RULE_3) ||
      game.rule == static_cast<int>(Rule::RULE_7)) {
    for (Card card : game.table) {
      if (card.rank == Rank::QUEEN) {
        points[taking_player][SCORE] += 5;
        points[taking_player][TOTAL_SCORE] += 5;
      }
    }
  }

  if (game.rule == static_cast<int>(Rule::RULE_4) ||
      game.rule == static_cast<int>(Rule::RULE_7)) {
    for (Card card : game.table) {
      if (card.rank == Rank::JACK || card.rank == Rank::KING) {
        points[taking_player][SCORE] += 2;
        points[taking_player][TOTAL_SCORE] += 2;
      }
    }
  }

  if (game.rule == static_cast<int>(Rule::RULE_5) ||
      game.rule == static_cast<int>(Rule::RULE_7)) {
    for (Card card : game.table) {
      if (card.rank == Rank::KING && card.suit == 'H') {
        points[taking_player][SCORE] += 18;
        points[taking_player][TOTAL_SCORE] += 18;
      }
    }
  }

  if (game.rule == static_cast<int>(Rule::RULE_6) ||
      game.rule == static_cast<int>(Rule::RULE_7)) {
    if (game.round == 7 || game.round == 13) {
      points[taking_player][SCORE] += 10;
      points[taking_player][TOTAL_SCORE] += 10;
    }
  }

  return "TAKEN" + std::to_string(game.round) + cards_taken +
         player_to_str(taking_player) + ENDING;
}

// Returns a string with the a string of cards.
std::string cards_to_string(std::vector<Card>& cards) {
  std::string cards_str = "";

  for (auto& card : cards) {
    cards_str += rank_to_string(card.rank);
    cards_str += card.suit;
  }

  return cards_str;
}