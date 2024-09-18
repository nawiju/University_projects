#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <list>
#include <ostream>
#include <string>
#include <unordered_set>
#include <vector>

#include "client_library.h"
#include "common.h"
#include "err.h"
#include "parsing.h"

/* Constants */
#define CONNECTIONS 2
#define BLOCK_FD -1
#define MAX_MESSAGE_SIZE \
  1000  // Maximum size of a message received from the server that will be read

#define client 0
#define server 1

#define WAITING_ON_DEAL 0
#define WAITING_ON_TRICK 1
#define WAITING_ON_SCORE 2
#define WAITING_ON_DEAL_OR_BUSY 3

#define DELETE 127
#define BACKSPACE 8

#define SERVER_FD 0
#define USER_FD 1

/* Global variables */
client_parameters PARAMS;
NODE SERVER;
NODE CLIENT;
int SOCKET_FD;
int stage = WAITING_ON_DEAL_OR_BUSY;  // Stage of the game

struct pollfd POLL_DESCRIPTORS[CONNECTIONS];

char BUFFER[BUFFER_SIZE];
size_t BUFFER_INDEX = 0;

bool FINISHED = false;

std::list<std::string> CARDS;
std::unordered_set<std::string> CARDS_SET;
std::list<std::string> TRICKS;

std::string MESSAGE = "";
std::string USER_INPUT = "";
std::string BEG_PRINTF = "\r\033[K";

int CURRENT_ROUND = 1;
std::string placed_card = "";


/* Additional functions */
void print(std::string message) {
  printf("%s", BEG_PRINTF.c_str());
  printf("%s", message.c_str());
  printf("%s", USER_INPUT.c_str());
  fflush(stdout);
}

void print_and_flush(std::string message) {
  printf("%s", message.c_str());
  fflush(stdout);
}

// Prints log message to the standard output.
static void print_log_message(const std::string& message, int who) {
  std::string log_message = "";
  if (PARAMS.is_auto_player) {
    if (who == client) {
      log_message = "[" + CLIENT.address + ":" + std::to_string(CLIENT.port) +
                    "," + SERVER.address + ":" + std::to_string(SERVER.port) +
                    "," + get_current_time().first + "] " + message;
    } else {
      log_message = "[" + SERVER.address + ":" + std::to_string(SERVER.port) +
                    "," + CLIENT.address + ":" + std::to_string(CLIENT.port) +
                    "," + get_current_time().first + "] " + message;
    }

    print_and_flush(BEG_PRINTF);
    print_log((char*)log_message.c_str(), log_message.size(), true, BUFFER,
              &BUFFER_INDEX);
    fflush(stdout);
    print_and_flush(USER_INPUT);
  }
}

// Resets the terminal to canonical mode and enables echo.
void reset_terminal() {
  struct termios tty;
  if (tcgetattr(POLL_DESCRIPTORS[USER_FD].fd, &tty) == 0) {
    tty.c_lflag |= (ICANON | ECHO);  // Restore canonical mode and echo
    tcsetattr(POLL_DESCRIPTORS[USER_FD].fd, TCSANOW, &tty);
  }
}

// Flushes the log buffer and prints the log to the standard output.
static void flush_log() {
  if (PARAMS.is_auto_player) {
    print_and_flush(BEG_PRINTF);
    print_log((char*)"", 0, true, BUFFER, &BUFFER_INDEX);
    fflush(stdout);
    print_and_flush(USER_INPUT);
  }
}

// Ends the connection with the server. Closes the socket and resets the
// terminal.
void end_connection() {
  flush_log();
  reset_terminal();
  close(SOCKET_FD);
}

// Waits for an event on the poll descriptors. Exits the program if the poll
// call fails.
static void poll_descriptors() {
  int ret = poll(POLL_DESCRIPTORS, CONNECTIONS,
                 -1);  // Wait indefinitely for an event

  if (FINISHED) {  // SIGINT received. Exit the program.
    end_connection();
    exit(FAILURE);
  }

  if (POLL_DESCRIPTORS[SERVER_FD].revents & POLLHUP) {
    end_connection();
    exit(SUCCESS);
  }

  if (ret == -1) {
    end_connection();
    syserr(ERROR_POLL);
  }
}

// Sets the file descriptor to non-blocking mode.
// Sets the terminal to non-canonical mode and disables echo.
void setNonBlocking(int fd) {
  // Get current terminal attributes
  struct termios tty;
  if (tcgetattr(fd, &tty) != 0) {
    perror("tcgetattr");
    return;
  }

  // Modify the attributes to set non-blocking mode
  tty.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
  }
}

// Handles the SIGINT signal.
static void catch_int(int sig) {
  FINISHED = true;
  reset_terminal();
  flush_log();

  if (POLL_DESCRIPTORS[SERVER_FD].fd != BLOCK_FD) {
    close(POLL_DESCRIPTORS[SERVER_FD].fd);
  }

  POLL_DESCRIPTORS[SERVER_FD].fd = BLOCK_FD;

  if (POLL_DESCRIPTORS[USER_FD].fd != BLOCK_FD) {
    close(POLL_DESCRIPTORS[USER_FD].fd);
  }

  POLL_DESCRIPTORS[USER_FD].fd = BLOCK_FD;
  close(SOCKET_FD);
  exit(FAILURE);
}

// Receives a message from the server. Exits the program if the message could
// not be received.
static bool receive_message(int socket_fd, bool end_in_success = false) {
  char byte;
  int total_bytes_read = 0;
  bool reached_end = false;

  // Read the message byte by byte until the end of the message is reached or
  // the maximum message size is exceeded. If the maximum message size is
  // exceeded, the message is invalid.
  while (total_bytes_read < MAX_MESSAGE_SIZE) {
    ssize_t bytes_read = read(socket_fd, &byte, 1);

    if (bytes_read == -1) {
      end_connection();
      syserr(ERROR_RECEIVE_MESSAGE);  // Error receiving the message
    } else if (bytes_read == 0) {
      end_connection();
      if (end_in_success) {
        exit(SUCCESS);  // Server closed the connection after a successful round
      } else {
        exit(FAILURE);  // Server closed the connection
      }
    }

    MESSAGE += byte;
    total_bytes_read++;

    if (MESSAGE.size() >= 2 &&
        MESSAGE.substr(MESSAGE.size() - 2, 2) == ENDING) {
      reached_end = true;
      return true;
    }
  }

  if (!reached_end) {
    MESSAGE += ENDING;
  }

  return false;
}

/* Functions helping with communication with the server and user */
static void on_BUSY(std::string message) {
  flush_log();
  if (!PARAMS.is_auto_player) {
    print_BUSY_message(message);
  }

  end_connection();
  exit(FAILURE);
}

static void on_DEAL(std::string message) {
  DEAL deal = parse_deal(message);
  CARDS = deal.cards;

  for (auto it = CARDS.begin(); it != CARDS.end(); it++) {
    CARDS_SET.insert(*it);
  }

  if (!PARAMS.is_auto_player) {
    print_DEAL_message(deal, USER_INPUT);
  }
}

static void on_TOTAL(std::string message) {
  if (!PARAMS.is_auto_player) {
    std::string scores_start = "The total scores are:\n";
    printf("%s", BEG_PRINTF.c_str());
    ssize_t written =
        write(STDOUT_FILENO, scores_start.c_str(), scores_start.size());
    fflush(stdout);
    printf("%s", USER_INPUT.c_str());

    if (!valid_write(written, scores_start.size())) {
      end_connection();
      syserr(ERROR_WRITE);
    }

    print_SCORES_message(message);
  }
}

static void on_SCORE(std::string message) {
  if (!PARAMS.is_auto_player) {
    std::string scores_start = "The scores are:\n";

    print_and_flush(BEG_PRINTF);
    ssize_t written =
        write(STDOUT_FILENO, scores_start.c_str(), scores_start.size());
    fflush(stdout);
    print_and_flush(USER_INPUT);

    if (!valid_write(written, scores_start.size())) {
      end_connection();
      syserr(ERROR_WRITE);
    }

    print_SCORES_message(message);
  }
}

static void on_WRONG(std::string message) {
  if (!PARAMS.is_auto_player) {
    print_WRONG_message(message);
  }
}

static bool handle_user_input(bool& can_place_card, int round) {
  char c;

  if (read(POLL_DESCRIPTORS[USER_FD].fd, &c, 1) == -1) {
    end_connection();
    syserr(ERROR_READ);
  }

  std::string user_input = "";

  if (c == '\n') {
    printf("\n");
    fflush(stdout);
    user_input = USER_INPUT;
    USER_INPUT = "";
  } else if ((int)c == DELETE || (int)c == BACKSPACE) {
    if (USER_INPUT.size() > 0) {
      USER_INPUT.pop_back();
      printf("\033[1D\033[1P");
    }

    fflush(stdout);
    return false;
  } else {
    // Only allow alphanumeric characters and '!' character. Ignore other
    // characters.
    if (std::isalpha(c) || std::isdigit(c) || c == '!') {
      USER_INPUT += c;
      printf("%c", c);
      fflush(stdout);
    }

    return false;
  }

  if (is_cards(user_input)) {
    print_cards(CARDS, USER_INPUT);
  } else if (is_tricks(user_input)) {
    print_tricks(TRICKS);
  } else if (is_card_placement(user_input)) {
    if (can_place_card) {
      std::string card = user_input.substr(1, user_input.size() - 1);
      send_TRICK(POLL_DESCRIPTORS[SERVER_FD].fd, card, round, placed_card);
      can_place_card = false;
      return true;
    } else {
      print_you_cannot_place_card();
    }
  } else {
    print_invalid_input_message();
  }

  return false;
}

// Sends the first card of the same suit as the first card in the trick, if
// possible. Otherwise, sends the last card in the list. This function is only
// called when the player is an automatic player.
static void send_card(std::string message, int round) {
  std::list<std::string> cards_list = parse_cards(message);
  bool chosen_card = false;
  std::string current_card = "";

  if (cards_list.size() > 0) {
    for (auto it = CARDS.begin(); it != CARDS.end(); it++) {
      std::string card = *it;

      if (card[card.length() - 1] ==
          cards_list.front()[cards_list.front().length() - 1]) {
        current_card = card;
        chosen_card = true;
        break;
      }
    }
  }

  if (!chosen_card) {
    current_card = CARDS.back();
  }

  std::string trick_message =
      "TRICK" + std::to_string(round) + current_card + ENDING;
  send_TRICK(POLL_DESCRIPTORS[SERVER_FD].fd, current_card, round, placed_card);

  print_log_message(trick_message, client);
}

static void prompt_user(std::string message, int round) {
  TRICK trick = parse_trick_from_server(message);
  std::list<std::string> cards_list = trick.cards;
  int trick_round = trick.round;

  std::string message_for_user =
      "Trick: (" + std::to_string(trick_round) + ") ";

  for (auto it = cards_list.begin(); it != cards_list.end(); it++) {
    if (it != cards_list.begin() && it != cards_list.end()) {
      message_for_user += ", ";
    }
    message_for_user += *it;
  }

  message_for_user += "\n";

  print_and_flush(BEG_PRINTF);
  ssize_t written =
      write(STDOUT_FILENO, message_for_user.c_str(), message_for_user.size());
  fflush(stdout);
  print_and_flush(USER_INPUT);

  if (!valid_write(written, message_for_user.size())) {
    reset_terminal();
    flush_log();
    syserr(ERROR_WRITE);
  }

  print_cards(CARDS, USER_INPUT);
}

static void handle_trick(std::string server_message, int round,
                         bool& can_place_card) {
  if (PARAMS.is_auto_player) {
    send_card(server_message, round);
    can_place_card = false;
  } else {
    prompt_user(server_message, round);
  }
}

static void on_TAKEN(std::string message) {
  TAKEN taken = parse_taken(message);
  std::string cards_string = "";
  std::string seat = "";
  seat += taken.seat;
  std::string round_string = std::to_string(taken.round);

  if (CURRENT_ROUND != taken.round) {
    end_connection();
    exit(FAILURE);
  }

  CURRENT_ROUND = taken.round + 1;

  for (auto it = taken.cards.begin(); it != taken.cards.end(); it++) {
    if (it != taken.cards.begin() && it != taken.cards.end()) {
      cards_string += ", ";
    }
    cards_string += *it;

    if (CARDS_SET.find(*it) != CARDS_SET.end()) {
      CARDS_SET.erase(*it);
      CARDS.remove(*it);
    }
  }

  if (!PARAMS.is_auto_player) {
    std::string trick_log = "A trick " + round_string + " is taken by " + seat +
                            ", cards " + cards_string + ".\n\n";
    print(trick_log);
    TRICKS.push_back(trick_log);
  }
}

/* Functions responsible for playing the game of Black Lady */
static void play_TRICK(std::string server_message, int round) {
  bool can_place_card = true;  // Only true after receiving a TRICK message and
                               // false after placing a TRICK

  handle_trick(server_message, round, can_place_card);

  while (true) {
    poll_descriptors();

    if (POLL_DESCRIPTORS[SERVER_FD].revents & POLLIN) {  // Read from the server
      bool received_correctly = receive_message(POLL_DESCRIPTORS[SERVER_FD].fd);
      if (!received_correctly) {  // The message was not received correctly;
                                  // ignore it
        continue;
      }

      std::string server_message = MESSAGE;
      MESSAGE = "";
      print_log_message(server_message, server);

      if (is_TAKEN(server_message, round, CARDS_SET, placed_card)) {
        on_TAKEN(server_message);
        return;
      } else if (is_WRONG(server_message, round)) {
        on_WRONG(server_message);
      } else if (is_TRICK_from_server(server_message, round)) {
        handle_trick(server_message, round, can_place_card);
        can_place_card = true;
      } else {
        continue;  // Ignore other messages; they are not valid at this stage
      }
    }

    if (!PARAMS.is_auto_player &&
        POLL_DESCRIPTORS[USER_FD].revents & POLLIN) {  // Read from stdin
      bool placed = handle_user_input(can_place_card, CURRENT_ROUND);

      if (placed) {
        can_place_card = false;
      }
    }
  }
}

// Here the only valid messages from the server are DEAL and TOTAL. Other
// messages are ignored. This function handles the game after the first deal.
static void play_game() {
  bool score_received = false;
  bool total_received = false;

  while (true) {
    poll_descriptors();

    if (POLL_DESCRIPTORS[SERVER_FD].revents & POLLIN) {  // Read from the server
      bool received_correctly = receive_message(POLL_DESCRIPTORS[SERVER_FD].fd);

      if (!received_correctly) {  // The message was not received correctly;
                                  // ignore it
        continue;
      }

      std::string server_message = MESSAGE;
      MESSAGE = "";
      print_log_message(server_message, server);

      if (stage == WAITING_ON_DEAL_OR_BUSY) {
        if (is_BUSY(server_message)) {
          on_BUSY(server_message);
        } else if (is_DEAL(server_message)) {
          on_DEAL(server_message);
          stage = WAITING_ON_TRICK;
          CURRENT_ROUND = 1;
        } else {
          continue;  // Ignore other messages; they are not valid at this stage
        }
      } else if (stage == WAITING_ON_DEAL) {
        if (is_DEAL(server_message)) {
          on_DEAL(server_message);
          stage = WAITING_ON_TRICK;
          CURRENT_ROUND = 1;
        } else if (is_TOTAL(server_message) && score_received) {
          // Receiving a TOTAL message is only valid if a SCORE message had been
          // received.
          if (!PARAMS.is_auto_player) {
            on_TOTAL(server_message);
          }

          return;  // The game has ended
        } else {
          continue;  // Ignore other messages; they are not valid at this stage
        }
      } else if (stage == WAITING_ON_TRICK) {
        score_received = false;
        if (is_TRICK_from_server(server_message, CURRENT_ROUND)) {
          play_TRICK(server_message, CURRENT_ROUND);
        } else if (is_TAKEN(server_message, CURRENT_ROUND, CARDS_SET,
                            placed_card)) {
          on_TAKEN(server_message);
        } else {
          continue;  // Ignore other messages; they are not valid at this stage
        }

        if (CURRENT_ROUND == ROUNDS + 1) {
          stage = WAITING_ON_SCORE;
        }
      } else if (stage == WAITING_ON_SCORE) {
        if (is_SCORE(server_message) && !score_received) {
          on_SCORE(server_message);
          score_received = true;
          stage = WAITING_ON_DEAL;
        } else if (is_TOTAL(server_message) && !total_received) {
          on_TOTAL(server_message);
          total_received = true;
        } else {
          continue;  // Ignore other messages; they are not valid at this stage
        }

        if (score_received && total_received) {
          return;  // The game has ended
        }
      }
    }

    if (!PARAMS.is_auto_player &&
        POLL_DESCRIPTORS[USER_FD].revents & POLLIN) {  // Read from stdin
      bool can_place_card = false;
      handle_user_input(can_place_card, CURRENT_ROUND);
    }
  }
}

int main(int argc, char* argv[]) {
  // Parse the command line arguments and create the socket
  PARAMS = client_parse_parameters(argc, argv);
  SOCKET_FD = create_socket(PARAMS);

  // Handle SIGINT
  install_signal_handler(SIGINT, catch_int, SA_RESTART);

  // Set the server and client addresses and ports based on the socket file
  // descriptor
  set_peer_address(SOCKET_FD, SERVER);
  set_own_address(SOCKET_FD, CLIENT);

  // Send the IAM message
  std::string message = "IAM" + std::string(1, PARAMS.seat) + ENDING;
  send_IAM(SOCKET_FD, PARAMS.seat);

  print_log_message(message, client);

  // Set up the poll descriptors
  POLL_DESCRIPTORS[SERVER_FD].fd = SOCKET_FD;
  POLL_DESCRIPTORS[SERVER_FD].events = POLLIN;

  if (PARAMS.is_auto_player) {
    POLL_DESCRIPTORS[USER_FD].fd =
        BLOCK_FD;  // Block reading from stdin if the player is automatic
  } else {
    POLL_DESCRIPTORS[USER_FD].fd = STDIN_FILENO;
    POLL_DESCRIPTORS[USER_FD].events = POLLIN;
    POLL_DESCRIPTORS[USER_FD].revents = 0;
    setNonBlocking(POLL_DESCRIPTORS[USER_FD].fd);
  }

  play_game();
  end_connection();

  return SUCCESS;
}