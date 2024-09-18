#include "client_library.h"
#include <unistd.h>
#include <cstdio>
#include <list>
#include <string>

#include "common.h"
#include "err.h"
#include "parsing.h"

std::string BEG_PRINT = "\r\033[K";

// Creates a socket and returns its file descriptor. Exits the program if the
// socket could not be created.
int create_socket(client_parameters params) {
  if (params.protocol == -1) {
    int socket_fd;
    std::pair<int, std::pair<struct sockaddr_in, struct sockaddr_in6>> result =
        get_server_address_unspecified(params.host, params.port);

    if (result.first == AF_INET) {  // IPv4
      socket_fd = socket(AF_INET, SOCK_STREAM, 0);
      struct sockaddr_in server_address = result.second.first;
      if (connect(socket_fd, (struct sockaddr*)&server_address,
                  sizeof(server_address)) == -1) {
        syserr(ERROR_CONNECT);
      }
    } else {  // IPv6
      socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
      struct sockaddr_in6 server_address = result.second.second;
      if (connect(socket_fd, (struct sockaddr*)&server_address,
                  sizeof(server_address)) == -1) {
        syserr(ERROR_CONNECT);
      }
    }

    return socket_fd;
  }

  int socket_fd = socket(params.protocol, SOCK_STREAM, 0);

  if (socket_fd < 0) {
    syserr(ERROR_CREATE_SOCKET);
  }

  if (params.protocol == AF_INET) {
    struct sockaddr_in server_address =
        get_server_address_IPv4(params.host, params.port);
    if (connect(socket_fd, (struct sockaddr*)&server_address,
                sizeof(server_address)) == -1) {
      syserr(ERROR_CONNECT);
    }
  } else if (params.protocol == AF_INET6) {
    struct sockaddr_in6 server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address = get_server_address_IPv6(params.host, params.port);

    if (connect(socket_fd, (struct sockaddr*)&server_address,
                sizeof(server_address)) == -1) {
      syserr(ERROR_CONNECT);
    }
  }

  return socket_fd;
}

// Sends an IAM message to the server. Exits the program if the message could
// not be sent.
void send_IAM(int socket_fd, char seat) {
  std::string message = "IAM";
  message += seat;
  message += ENDING;

  ssize_t written = writen(socket_fd, message.c_str(), message.size());
  if (!valid_write(written, message.size())) {
    syserr(ERROR_WRITE);
  }
}

// Sends a TRICK message to the server. Exits the program if the message could
// not be sent.
void send_TRICK(int socket_fd, std::string card, int round,
                std::string& placed_card) {
  std::string message = "TRICK";
  message += std::to_string(round);
  message += card;
  message += ENDING;
  placed_card = card;

  ssize_t written = writen(socket_fd, message.c_str(), message.size());
  if (!valid_write(written, message.size())) {
    syserr(ERROR_WRITE);
  }
}

// Prints the DEAL message for the human user.
void print_DEAL_message(const DEAL& deal, std::string USER_INPUT) {
  std::string cards_string = "";
  std::list<std::string> cards = deal.cards;

  for (auto it = cards.begin(); it != cards.end(); it++) {
    if (it != cards.begin() && it != cards.end()) {
      cards_string += ", ";
    }
    cards_string += *it;
  }

  std::string stdout_message = "New deal " + std::to_string(deal.round_type) +
                               ": staring place " + deal.seat +
                               ", your cards: " + cards_string + ".\n\n";

  printf("%s", BEG_PRINT.c_str());
  fflush(stdout);
  ssize_t written =
      write(STDOUT_FILENO, stdout_message.c_str(), stdout_message.size());
  fflush(stdout);
  printf("%s", USER_INPUT.c_str());
  fflush(stdout);

  if (!valid_write(written, stdout_message.size())) {
    syserr(ERROR_WRITE);
  }
}

void print_WRONG_message(const std::string& message) {
  int round_number = stoi(message.substr(5, message.size() - 7));
  std::string error_message = "Wrong message received in trick " +
                              message.substr(5, message.size() - 7) + ".\n\n";
  ssize_t written =
      write(STDOUT_FILENO, error_message.c_str(), error_message.size());
  fflush(stdout);

  if (!valid_write(written, error_message.size())) {
    syserr(ERROR_WRITE);
  }
}

// Prints the BUSY message for the human user.
void print_BUSY_message(const std::string& message) {
  std::string seats_string = "";
  for (size_t i = BUSY_START; i < message.size() - 2; i++) {
    if (i != BUSY_START) {
      seats_string += ", ";
    }
    seats_string += message[i];
  }

  std::string stdout_message =
      "Place busy, list of busy places received: " + seats_string + ".\n";
  ssize_t written =
      write(STDOUT_FILENO, stdout_message.c_str(), stdout_message.size());
  fflush(stdout);

  if (!valid_write(written, stdout_message.size())) {
    syserr(ERROR_WRITE);
  }
}

void print_invalid_input_message() {
  std::string error_message = "Invalid command\n\n";
  ssize_t written =
      write(STDOUT_FILENO, error_message.c_str(), error_message.size());
  fflush(stdout);

  if (!valid_write(written, error_message.size())) {
    syserr(ERROR_WRITE);
  }
}

void print_you_cannot_place_card() {
  std::string message = "You have not been asked to place a card\n\n";
  ssize_t written = write(STDOUT_FILENO, message.c_str(), message.size());
  fflush(stdout);

  if (!valid_write(written, message.size())) {
    syserr(ERROR_WRITE);
  }
}

// Prints the available cards for the human user.
void print_cards(std::list<std::string> cards_list, std::string USER_INPUT) {
  std::string cards_string = "Available: ";

  for (auto it = cards_list.begin(); it != cards_list.end(); it++) {
    if (it != cards_list.begin()) {
      cards_string += ", ";
    }
    cards_string += *it;
  }

  cards_string += "\n\n";

  printf("%s", BEG_PRINT.c_str());
  fflush(stdout);
  ssize_t written =
      write(STDOUT_FILENO, cards_string.c_str(), cards_string.size());
  fflush(stdout);
  printf("%s", USER_INPUT.c_str());
  fflush(stdout);

  if (!valid_write(written, cards_string.size())) {
    syserr(ERROR_WRITE);
  }
}

// Print the tricks played in the deal for the human user.
void print_tricks(std::list<std::string> tricks_list) {
  std::string tricks_string = "Tricks played:\n";

  for (auto it = tricks_list.begin(); it != tricks_list.end(); it++) {
    tricks_string += *it;
  }

  tricks_string += "\n";
  ssize_t written =
      write(STDOUT_FILENO, tricks_string.c_str(), tricks_string.size());
  fflush(stdout);

  if (!valid_write(written, tricks_string.size())) {
    syserr(ERROR_WRITE);
  }
}

// Prints the SCORE message for the human user.
void print_SCORES_message(const std::string& message) {
  SCORE score = parse_score(message);

  std::string scores_string = "";
  auto seats_it = score.seats.begin();
  auto scores_it = score.scores.begin();

  for (; seats_it != score.seats.end() && scores_it != score.scores.end();
       seats_it++, scores_it++) {
    char seat = *seats_it;
    scores_string += seat;
    scores_string += " | " + std::to_string(*scores_it) + "\n";
  }

  scores_string += "\n";

  ssize_t written =
      write(STDOUT_FILENO, scores_string.c_str(), scores_string.size());
  fflush(stdout);

  if (!valid_write(written, scores_string.size())) {
    syserr(ERROR_WRITE);
  }
}