#ifndef CLIENT_LIBRARY_H
#define CLIENT_LIBRARY_H

#include <cstdio>
#include <list>
#include <string>

#include "client_library.h"
#include "common.h"
#include "err.h"
#include "parsing.h"

/* Functions */
void send_IAM(int socket_fd, char seat);
void send_TRICK(int socket_fd, std::string card, int round,
                std::string& placed_card);
int create_socket(client_parameters params);
void print_DEAL_message(const DEAL& deal, std::string USER_INPUT);
void print_BUSY_message(const std::string& message);
void print_SCORES_message(const std::string& message);
void print_WRONG_message(const std::string& message);
void print_cards(std::list<std::string> cards_list, std::string USER_INPUT);
void print_tricks(std::list<std::string> TRICKS);
void print_invalid_input_message();
void print_you_cannot_place_card();

#endif