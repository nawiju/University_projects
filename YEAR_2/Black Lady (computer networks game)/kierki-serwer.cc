#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <climits>
#include <fstream>
#include <iostream>
#include <queue>
#include <regex>
#include <vector>

#include "common.h"
#include "err.h"
#include "parsing.h"
#include "server_library.h"

/* Defines */
#define MAX_CLIENTS 4
#define QUEUE_LENGTH 4
#define MAIN_SOCKET 0
#define PREGAME 69
#define PLAY_GAME 420
#define PAUSED_GAME 2137
#define NEVER LLONG_MAX
#define MILLISECONDS 1000

/* Comparators */
struct CMP_PQ {
  bool operator()(std::pair<int, long long> const& p1,
                  std::pair<int, long long> const& p2) {
    return p1.second > p2.second;
  }
};

/* Global variables */
server_parameters PARAMS;
NODE SERVER;
int SOCKET_FD;
bool FINISHED = false;

char BUFFER[BUFFER_SIZE];
size_t BUFFER_INDEX = 0;
long long poke_timeout = NEVER;

std::vector<std::vector<long long>> POINTS = {
    {0, 0}, {0, 0}, {0, 0}, {0, 0}};  // {current score, total score}
std::vector<struct pollfd> POLL_DESCRIPTORS(MAX_CLIENTS +
                                            1);  // +1 for the main socket

std::string
    MESSAGES_BUFFER[MAX_CLIENTS];  // What the server read in from each client

std::vector<std::vector<std::string>> GAMEPLAY =
    {};  // Parsed information about the game play
GAME CURRENT_DEAL;

int CLIENTS_COUNT = 0;
std::unordered_map<int, std::string>
    buffer_received;  // Buffer for the messages that were received from the
                      // clients
std::unordered_map<int, std::queue<std::pair<std::string, int>>>
    buffer_to_send;  // Buffer for the messages that are to be sent to the
                     // clients
char tmp_buffer[BUFFER_SIZE];

std::priority_queue<std::pair<int, long long>,
                    std::vector<std::pair<int, long long>>, CMP_PQ>
    timeouts;  // Client, time in milliseconds
std::unordered_map<int, long long> timeouts_help;

std::unordered_map<int, int>
    id_mapper;  // Maps the pollID to the index in the POLL_DESCRIPTORS

std::unordered_map<int, NODE>
    clients_info;  // Maps the pollID to the client's address and port
std::vector<bool> gotta_catch_up = {true, true, true, true};

/* Functions */
static long long parse_file(const std::string& file_name) {
  std::ifstream file(file_name);
  if (!file.is_open()) {
    fatal(ERROR_OPEN_FILE);
  }

  std::string line;
  int count = 0;

  while (std::getline(file, line)) {
    count++;
    std::vector<std::string> cards_of_clients;
    cards_of_clients.push_back(line);

    for (int i = 0; i < MAX_CLIENTS; i++) {
      std::getline(file, line);
      cards_of_clients.push_back(line);
    }
    GAMEPLAY.push_back(cards_of_clients);
  }

  file.close();
  return count;
}

static void set_up_server() {
  // Create a socket
  if ((SOCKET_FD = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
    syserr(ERROR_CREATE_SOCKET);
  }

  int optval = 1;
  if (setsockopt(SOCKET_FD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
      0) {
    syserr(ERROR_SETSOCKOPT);
  }

  if (setsockopt(SOCKET_FD, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) <
      0) {
    syserr(ERROR_SETSOCKOPT);
  }

  // Bind the socket to a concrete address.
  struct sockaddr_in6 server_address;
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin6_family = AF_INET6;
  server_address.sin6_addr = in6addr_any;

  if (PARAMS.port != -1) {
    server_address.sin6_port = htons(PARAMS.port);
  }

  if (bind(SOCKET_FD, (struct sockaddr*)&server_address,
           (socklen_t)sizeof server_address) < 0) {
    syserr(ERROR_BIND);
  }

  // Switch the socket to listening.
  if (listen(SOCKET_FD, QUEUE_LENGTH) < 0) {
    syserr(ERROR_LISTEN);
  }

  // Set necessary parameters
  set_own_address(SOCKET_FD, SERVER);

  // The "main" socket will be at index 0
  POLL_DESCRIPTORS[MAIN_SOCKET].fd = SOCKET_FD;
  POLL_DESCRIPTORS[MAIN_SOCKET].events = POLLIN;

  for (int i = 1; i <= MAX_CLIENTS; ++i) {
    POLL_DESCRIPTORS[i].fd = -1;
    POLL_DESCRIPTORS[i].events = POLLIN | POLLOUT;
  }
}

// Disconnects the client with the given pollID, closes the socket and removes
// all the data associated with the client.
void disconnect(int pollID) {
  int idx = id_mapper[pollID];
  close(POLL_DESCRIPTORS[idx].fd);
  POLL_DESCRIPTORS[idx].fd = -1;
  timeouts_help.erase(pollID);
  buffer_received.erase(pollID);
  buffer_to_send.erase(pollID);
  clients_info.erase(pollID);

  if (!(1 <= idx && idx <= MAX_CLIENTS)) {
    id_mapper[POLL_DESCRIPTORS.back().fd] = idx;
    std::swap(POLL_DESCRIPTORS[idx], POLL_DESCRIPTORS.back());
    POLL_DESCRIPTORS.pop_back();
  } else {
    CLIENTS_COUNT--;
    gotta_catch_up[idx - 1] = true;
  }

  if (idx - 1 == CURRENT_DEAL.current_player) {
    CURRENT_DEAL.started_poking = false;
  }
}

// Handles the SIGINT signal.
static void catch_int(int sig) {
  FINISHED = true;

  for (int i = 0; i < POLL_DESCRIPTORS.size(); ++i) {
    if (POLL_DESCRIPTORS[i].fd != -1) {
      disconnect(POLL_DESCRIPTORS[i].fd);
    }
  }

  close(SOCKET_FD);
  exit(SUCCESS);
}

void reset_revents() {
  for (int i = 0; i < MAX_CLIENTS + 1; ++i) {
    POLL_DESCRIPTORS[i].revents = 0;
  }
}

// Returns the current timeout in milliseconds.
long long calculate_new_timeout() {
  std::pair<std::string, long long> current_time = get_current_time();
  long long timeout = -1;

  // Close the clients that did not return in time. Also handles the timeout
  // case
  while (!timeouts.empty() && timeouts.top().second <= current_time.second) {
    int idx = timeouts.top().first;
    int actual_idx = id_mapper[idx]; // Index in the POLL_DESCRIPTORS vector
    long long tm = timeouts.top().second;
    timeouts.pop();

    if (timeouts_help.find(idx) == timeouts_help.end()) {
      continue;
    } else if (timeouts_help[idx] != tm) {
      continue;
    } else if (1 <= actual_idx && actual_idx <= MAX_CLIENTS) {
      continue;
    }

    disconnect(idx);
  }

  if (!timeouts.empty()) {
    timeout = timeouts.top().second - current_time.second;
  }

  return timeout;
}

// Establishes a new connection with a client and creates all the necessary data
// structures for the client.
void establish_new_connection() {
  if (POLL_DESCRIPTORS[MAIN_SOCKET].revents & POLLIN) {
    struct sockaddr_storage client_addr;
    unsigned int addr_len = sizeof(client_addr);
    int client_fd =
        accept(SOCKET_FD, (struct sockaddr*)&client_addr, &addr_len);

    if (client_fd < 0) {
      syserr(ERROR_ACCEPT);
    }

    if (fcntl(client_fd, F_SETFL, O_NONBLOCK)) {
      syserr(ERROR_FCNTL);
    }

    if (client_addr.ss_family == AF_INET) {
      // Accept a connection from a client over IPv4.
      struct sockaddr_in* client_addr_v4 = (struct sockaddr_in*)&client_addr;
      char client_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client_addr_v4->sin_addr, client_ip,
                sizeof(client_ip));
      clients_info[client_fd] = {ntohs(client_addr_v4->sin_port), client_ip};
    } else if (client_addr.ss_family == AF_INET6) {
      // Accept a connection from a client over IPv6.
      struct sockaddr_in6* client_addr_v6 = (struct sockaddr_in6*)&client_addr;
      char client_ip[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &client_addr_v6->sin6_addr, client_ip,
                sizeof(client_ip));
      clients_info[client_fd] = {ntohs(client_addr_v6->sin6_port), client_ip};
    }

    // Add the client to the poll descriptors and create the necessary data
    // structures for the client.
    POLL_DESCRIPTORS.push_back({client_fd, POLLIN});
    std::pair<std::string, long long> timeout_pair = get_current_time();
    id_mapper[client_fd] = POLL_DESCRIPTORS.size() - 1;
    timeouts.push(
        {client_fd, timeout_pair.second + PARAMS.timeout * MILLISECONDS});
    timeouts_help[client_fd] =
        timeout_pair.second + PARAMS.timeout * MILLISECONDS;
    buffer_received[client_fd] = "";
    buffer_to_send[client_fd] = {};
  }
}

// Reads the message from the client and handles the case when the client
// disconnected. Returns true if the client disconnected or an error occurred.
bool read_from_client_poll(int& i, int idx) {
  if ((POLL_DESCRIPTORS[i].revents & (POLLIN | POLLERR)) != 0) {
    // Ready to read.
    ssize_t received_bytes = read(idx, tmp_buffer, BUFFER_SIZE);
    std::pair<std::string, long long> timeout_pair = get_current_time();
    timeouts_help[idx] = timeout_pair.second + PARAMS.timeout * MILLISECONDS;

    if (received_bytes < 0) {
      // Error while reading.
      disconnect(idx);
      i--;
      return true;
    } else if (received_bytes == 0) {
      // Client disconnected.
      disconnect(idx);
      i--;
      return true;
    } else {
      // Read the message from the client.
      for (int j = 0; j < received_bytes; j++) {
        buffer_received[idx] += tmp_buffer[j];
      }
      POLL_DESCRIPTORS[i].events =
          POLLIN | POLLOUT;  // Switch from reading to writing.
    }
  }

  return false;
}

// Prepares a response to the client. Returns true if the client disconnected or
// an error occurred.
bool prepare_message_pregame(int& i, std::string& client_message,
                             std::string& message, int idx) {
  if (is_IAM(client_message)) {
    if (1 <= i && i <= MAX_CLIENTS) {
      // The client is already connected. This is an error. Disconnect the
      // client.
      disconnect(idx);
      i--;
      return true;
    } else {
      char seat = client_message[3];
      std::string NESW = "NESW";
      std::string busy = "BUSY";

      // Check if the seat is already taken.
      for (int j = 0; j < MAX_CLIENTS; j++) {
        if (POLL_DESCRIPTORS[j + 1].fd != -1) {
          busy += NESW[j];
        }
      }

      // Give the client their seat if it is available or prepare a BUSY
      // message.
      for (int j = 0; j < MAX_CLIENTS; j++) {
        if (POLL_DESCRIPTORS[j + 1].fd == -1 && seat == NESW[j]) {
          int idx2 = id_mapper[idx];
          std::swap(POLL_DESCRIPTORS[idx2], POLL_DESCRIPTORS[j + 1]);
          std::swap(POLL_DESCRIPTORS[idx2], POLL_DESCRIPTORS.back());
          id_mapper[idx] = j + 1;
          POLL_DESCRIPTORS.pop_back();
          CLIENTS_COUNT++;
        } else if (seat == NESW[j]) {
          message = busy + ENDING;
        }
      }
    }
  } else {
    if (1 <= i && i <= MAX_CLIENTS && is_TRICK_from_client(client_message)) {
      // The client sent a TRICK message without being asked. Send a WRONG
      // message.
      message = "WRONG1" + ENDING;
    } else {
      // The client sent an invalid message. Disconnect the client.
      disconnect(idx);
      i--;
      return true;
    }
  }

  return false;
}

// Returns true if the client has the card in their hand.
bool client_has_card(Card card, int client) {
  for (Card c : CURRENT_DEAL.in_hand[client]) {
    if (card == c) {
      return true;
    }
  }

  return false;
}

// Makes a move in the game. Removes the card from the client's hand and adds it
// to the table.
void make_move(int client, Card card) {
  CURRENT_DEAL.table.push_back(card);
  auto it = CURRENT_DEAL.in_hand[client].begin();

  while (*it != card) {
    it++;
  }

  CURRENT_DEAL.in_hand[client].erase(it);
  CURRENT_DEAL.current_player = (client + 1) % MAX_CLIENTS;

  if (CURRENT_DEAL.table.size() == MAX_CLIENTS) {
    // The trick is over. Calculate the score and prepare the message.
    std::string message = calculate_score(CURRENT_DEAL, POINTS);
    CURRENT_DEAL.tricks_taken.push_back(message);
    CURRENT_DEAL.round++;
    CURRENT_DEAL.table.clear();
    CURRENT_DEAL.current_player = char_to_player(message[message.size() - 3]);

    for (int i = 1; i <= MAX_CLIENTS; i++) {
      buffer_to_send[POLL_DESCRIPTORS[i].fd].push({message, 0});  // Send SCORE
    }
  }
}

// Verifies whether the move is legal. Returns true if the move is legal.
bool is_move_legal(Card card, int player) {
  if (CURRENT_DEAL.table.size() == 0) {
    return true;
  }

  char suit = CURRENT_DEAL.table[0].suit;
  if (card.suit == suit) {
    return true;
  }

  for (Card c : CURRENT_DEAL.in_hand[player]) {
    if (c.suit == suit) {
      return false;
    }
  }

  return true;
}

// Prepares a SCORE or TOTAL message to send to the clients. Resets the score if
// the message is SCORE.
void prepare_score_or_total(int score_type) {
  std::string message = "SCORE";

  if (score_type == TOTAL_SCORE) {
    message = "TOTAL";
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    message += player_to_str(i) + std::to_string(POINTS[i][score_type]);
  }

  if (score_type == SCORE) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      POINTS[i][SCORE] = 0;
    }
  }

  message += ENDING;

  for (int i = 1; i <= MAX_CLIENTS; i++) {
    buffer_to_send[POLL_DESCRIPTORS[i].fd].push(
        {message, 0});  // put in send_buffer SCORE or TOTAL
  }
}

// Sends the DEAL message and the tricks taken to a client that has rejoined.
static bool catch_up_client(int i) {
  int idx = POLL_DESCRIPTORS[i].fd;
  long long deal = CURRENT_DEAL.deal;
  std::string st_player = "";
  st_player += CURRENT_DEAL.start_player;
  std::string message = "DEAL" + std::to_string(CURRENT_DEAL.rule) + st_player +
                        GAMEPLAY[deal][i] + ENDING;
  buffer_to_send[idx].push({message, 0});

  for (std::string trick : CURRENT_DEAL.tricks_taken) {
    buffer_to_send[idx].push({trick, 0});
  }

  return false;
}

// Sends the TRICK message to the client that needs to make a move.
static void poke(int i) {
  int idx = POLL_DESCRIPTORS[i].fd;
  long long deal = CURRENT_DEAL.deal;

  if (CURRENT_DEAL.round > ROUNDS) {
    return;
  }

  std::string message = "TRICK" + std::to_string(CURRENT_DEAL.round) +
                        cards_to_string(CURRENT_DEAL.table) + ENDING;
  buffer_to_send[idx].push({message, 0});
}

// Prepares response messages for the clients when the game is paused. Sends the
// messages to the clients when the game is unpaused.
bool prepare_message_paused_game(int& i, std::string& client_message,
                                 std::string& message, int idx) {
  if (is_IAM(client_message) && i >= MAX_CLIENTS + 1) {
    return prepare_message_pregame(i, client_message, message, idx);
  } else if (is_TRICK_from_client(client_message) && 1 <= i &&
             i <= MAX_CLIENTS) {
    ssize_t message_size = client_message.size();

    int round = 0;
    char suit = client_message[message_size - 3];
    std::string value = client_message.substr(message_size - 4, 1);

    // Check if the client has sent a card with a value of 10.
    if (value == "0") {
      value = client_message.substr(message_size - TRICK_OFFSET, 2);
    }

    Rank rank = string_to_rank(value);

    round = std::stoi(client_message.substr(
        TRICK_OFFSET, message_size - TRICK_LENGTH - value.size()));

    if ((i - 1) == CURRENT_DEAL.current_player && CURRENT_DEAL.has_been_poked) {
      if (poke_timeout == NEVER || poke_timeout < get_current_time().second) {
        return false;
      }

      if (round == CURRENT_DEAL.round &&
          client_has_card(Card(suit, rank), CURRENT_DEAL.current_player) &&
          is_move_legal(Card(suit, rank), CURRENT_DEAL.current_player)) {
        make_move(i - 1, Card(suit, rank));
      } else {
        message = "WRONG" +
                  std::to_string(std::min(ROUNDS, CURRENT_DEAL.round)) + ENDING;
      }
      CURRENT_DEAL.has_been_poked = false;
      return false;
    }

    message =
        "WRONG" + std::to_string(std::min(ROUNDS, CURRENT_DEAL.round)) + ENDING;
    return false;
  } else {
    // Disconnect the client if they sent an invalid message.
    disconnect(idx);
    i--;
    return true;
  }

  return false;
}

// Prepares response messages for the clients when the game is in progress.
bool prepare_message_play_game(int& i, std::string& client_message,
                               std::string& message, int idx) {
  if (is_IAM(client_message) && i >= MAX_CLIENTS + 1) {
    return prepare_message_pregame(i, client_message, message, idx);
  } else if (is_TRICK_from_client(client_message) && 1 <= i &&
             i <= MAX_CLIENTS) {
    ssize_t message_size = client_message.size();

    int round = 0;
    char suit = client_message[message_size - 3];
    std::string value = client_message.substr(message_size - 4, 1);

    if (value == "0") {
      value = client_message.substr(message_size - TRICK_OFFSET, 2);
    }

    Rank rank = string_to_rank(value);
    round = std::stoi(client_message.substr(
        TRICK_OFFSET, message_size - TRICK_LENGTH - value.size()));

    if ((i - 1) == CURRENT_DEAL.current_player && CURRENT_DEAL.has_been_poked) {
      if (round == CURRENT_DEAL.round &&
          client_has_card(Card(suit, rank), CURRENT_DEAL.current_player) &&
          is_move_legal(Card(suit, rank), CURRENT_DEAL.current_player)) {
        make_move(i - 1, Card(suit, rank));
      } else {
        message = "WRONG" +
                  std::to_string(std::min(ROUNDS, CURRENT_DEAL.round)) + ENDING;
      }
      CURRENT_DEAL.has_been_poked = false;
      return false;
    }

    message =
        "WRONG" + std::to_string(std::min(ROUNDS, CURRENT_DEAL.round)) + ENDING;
    return false;
  } else {
    disconnect(idx);
    i--;
    return true;
  }

  return false;
}

// Prepares the message to send to the client. Returns true if the client
// disconnected or an error occurred.
bool prepare_message(int& i, int stage, int idx) {
  if ((POLL_DESCRIPTORS[i].revents & POLLOUT) != 0 &&
      buffer_received[idx].size() > 0) {
    // Ready to write.
    std::string client_message = buffer_received[idx];
    size_t length_of_message = client_message.find(ENDING) + ENDING.size();

    if (length_of_message == std::string::npos) {
      return true;
    }

    client_message = client_message.substr(0, length_of_message);
    buffer_received[idx] = buffer_received[idx].substr(length_of_message);

    std::string log_message = "[" + clients_info[idx].address + ":" +
                              std::to_string(clients_info[idx].port) + "," +
                              SERVER.address + ":" +
                              std::to_string(SERVER.port) + "," +
                              get_current_time().first + "] " + client_message;
    print_log((char*)log_message.c_str(), log_message.size(), true, BUFFER,
              &BUFFER_INDEX);

    std::string message = "";

    if (stage == PREGAME) {
      if (prepare_message_pregame(i, client_message, message, idx)) {
        return true;
      }
    } else if (stage == PLAY_GAME) {
      if (prepare_message_play_game(i, client_message, message, idx)) {
        return true;
      }
    } else if (stage == PAUSED_GAME) {
      if (prepare_message_paused_game(i, client_message, message, idx)) {
        return true;
      }
    }

    if (message.size() == 0) {
      return true;
    }

    buffer_to_send[idx].push({message, 0});  // most types of msg
  }

  return false;
}

// Sends the message to the client. Returns true if the client disconnected or
// an error occurred.
bool send_message(int& i, int idx) {
  if ((POLL_DESCRIPTORS[i].revents & POLLOUT) != 0 &&
      buffer_to_send[idx].size() > 0) {
    std::string full_message = buffer_to_send[idx].front().first;
    ssize_t sent = buffer_to_send[idx].front().second;

    std::string message = full_message.substr(sent, full_message.size() - sent);

    ssize_t sent_bytes = write(idx, message.c_str(), message.size());

    std::string log_message = "[" + SERVER.address + ":" +
                              std::to_string(SERVER.port) + "," +
                              clients_info[idx].address + ":" +
                              std::to_string(clients_info[idx].port) + "," +
                              get_current_time().first + "] " + message;
    print_log((char*)log_message.c_str(), log_message.size(), true, BUFFER,
              &BUFFER_INDEX);

    if (sent_bytes < 0) {
      error("error when writing message to connection %d (errno %d, %s)\n", i,
            errno, strerror(errno));
      disconnect(idx);
      i--;
      return true;
    } else {
      buffer_to_send[idx].front().second += sent_bytes;

      if (buffer_to_send[idx].front().second ==
          buffer_to_send[idx].front().first.size()) {
        buffer_to_send[idx].pop();

        if (is_any_TRICK(full_message) &&
            is_TRICK_from_server(full_message, CURRENT_DEAL.round)) {
          CURRENT_DEAL.has_been_poked = true;
          CURRENT_DEAL.started_poking = false;
          poke_timeout =
              get_current_time().second + PARAMS.timeout * MILLISECONDS;
        }

        if (is_BUSY(full_message)) {
          disconnect(idx);
          i--;
          return true;
        }
      }
    }
    POLL_DESCRIPTORS[i].events = POLLIN | POLLOUT;
  }

  return false;
}

// Waits for the clients to return after the game has been paused or to come to
// a game.
void wait_for_clients(int stage) {
  while (CLIENTS_COUNT != MAX_CLIENTS) {
    reset_revents();
    int timeout = calculate_new_timeout();
    int poll_status =
        poll(POLL_DESCRIPTORS.data(), POLL_DESCRIPTORS.size(), timeout);

    if (poll_status == -1) {
      if (errno == EINTR) {
        error(ERROR_EINTR);
      } else {
        syserr(ERROR_POLL);
      }

      continue;
    } else if (poll_status == 0) {
      continue;
    }

    // Establishing new connection
    establish_new_connection();

    // Managing existing clients
    for (int i = 1; i < POLL_DESCRIPTORS.size(); ++i) {
      int idx = POLL_DESCRIPTORS[i].fd;

      if (idx == -1) {
        continue;
      }

      if (stage == PREGAME) {
        if (read_from_client_poll(i, idx) || prepare_message(i, PREGAME, idx) ||
            send_message(i, idx)) {
          continue;
        }
      } else if (stage == PAUSED_GAME) {
        if (read_from_client_poll(i, idx) ||
            prepare_message(i, PAUSED_GAME, idx) ||
            (i >= MAX_CLIENTS + 1 && send_message(i, idx))) {
          continue;
        }
      }
    }
  }
}

bool not_sent_message() {
  bool not_sent = false;

  for (int i = 1; i < POLL_DESCRIPTORS.size(); ++i) {
    if (POLL_DESCRIPTORS[i].fd == -1) {
      continue;
    }

    if (buffer_to_send[POLL_DESCRIPTORS[i].fd].size() > 0) {
      not_sent = true;
      break;
    }
  }

  return not_sent;
}

void play_game() {
  for (int i = 1; i < MAX_CLIENTS + 1; i++) {
    if (gotta_catch_up[i - 1]) {
      catch_up_client(i);
      gotta_catch_up[i - 1] = false;
    }
  }

  while (CLIENTS_COUNT == MAX_CLIENTS) {
    reset_revents();

    long long timeout = poke_timeout - get_current_time().second;
    long long cnt = calculate_new_timeout();

    if (poke_timeout != NEVER && (timeout < cnt || cnt == -1)) {
      if (timeout <= 0) {
        CURRENT_DEAL.has_been_poked = false;
        poke_timeout = NEVER;
      }
    } else {
      timeout = cnt;
    }

    int poll_status =
        poll(POLL_DESCRIPTORS.data(), POLL_DESCRIPTORS.size(), timeout);

    if (poll_status == -1) {
      if (errno == EINTR) {
        error(ERROR_EINTR);
      } else {
        syserr(ERROR_POLL);
      }

      continue;
    } else if (poll_status == 0) {
      continue;
    }

    // Establishing new connection
    establish_new_connection();

    // do we have to poke?
    if (!CURRENT_DEAL.has_been_poked && !CURRENT_DEAL.started_poking) {
      poke(CURRENT_DEAL.current_player + 1);
      CURRENT_DEAL.started_poking = true;
    }

    // Managing existing clients
    for (int i = 1; i < POLL_DESCRIPTORS.size(); ++i) {
      int idx = POLL_DESCRIPTORS[i].fd;

      if (idx == -1) {
        continue;
      }

      if (read_from_client_poll(i, idx) || prepare_message(i, PLAY_GAME, idx) ||
          send_message(i, idx)) {
        continue;
      }
    }

    if (not_sent_message()) {
      continue;
    }

    break;
  }
}

// Manages the gameplay of the deal
static void play_deal(long long deal) {
  CURRENT_DEAL.round = 1;
  CURRENT_DEAL.deal = deal;
  make_deal(CURRENT_DEAL, GAMEPLAY[deal], buffer_to_send);

  while (CURRENT_DEAL.round <= ROUNDS) {
    wait_for_clients(PAUSED_GAME);
    play_game();
  }

  prepare_score_or_total(SCORE);
}

int main(int argc, char* argv[]) {
  PARAMS = server_parse_parameters(argc, argv);
  set_up_server();
  long long number_of_deals = parse_file(PARAMS.file);
  install_signal_handler(SIGINT, catch_int, SA_RESTART);

  wait_for_clients(PREGAME);

  for (long long i = 0; i < number_of_deals; i++) {
    play_deal(i);
    for (int i = 1; i < MAX_CLIENTS + 1; i++) {
      gotta_catch_up[i - 1] = true;
    }
  }

  prepare_score_or_total(TOTAL_SCORE);

  while (not_sent_message()) {
    wait_for_clients(PAUSED_GAME);

    for (int i = 1; i < POLL_DESCRIPTORS.size(); ++i) {
      int idx = POLL_DESCRIPTORS[i].fd;

      if (idx == -1) {
        continue;
      }
      send_message(i, idx);
    }
  }

  close(SOCKET_FD);

  return 0;
}
