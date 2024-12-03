# Hearts Game - Server and Client
Class: Computer Networks

The goal of this task is to implement a server and client for the game of Hearts. The server conducts the game, while the clients represent the players.

## Game Rules

Hearts is played with four players using a standard 52-card deck. Players sit at the table in the following positions: N (North), E (East), S (South), W (West). The gameplay consists of rounds, with each player receiving 13 cards at the beginning of each round. Players only see their own cards. The game consists of 13 tricks.

1. In the first trick, a designated player leads by playing a chosen card.
2. The remaining players take turns playing one card each in a clockwise order.
3. Players must follow the suit of the leading card. If a player does not have a card in the required suit, they may play a card of any other suit.
4. The player who plays the highest card of the leading suit wins the trick and leads the next one.

### Card Ranking

- 2, 3, 4, …, 9, 10, Jack, Queen, King, Ace.

### Scoring

The objective is to collect as few points as possible. Scoring rules are as follows:

1. **Avoiding Tricks:** 1 point for each trick taken.
2. **Avoiding Hearts:** 1 point for each heart taken.
3. **Avoiding Queens:** 5 points for each Queen taken.
4. **Avoiding Kings and Jacks:** 2 points for each King or Jack taken.
5. **Avoiding the King of Hearts:** 18 points for taking it.
6. **Avoiding the 7th Trick:** 10 points for each of these tricks taken.
7. **Bandit:** Points for all of the above.

Rounds do not have to be played to completion—players may stop if it becomes clear that all points have been distributed.

## Server Invocation Parameters

Parameters for invoking the server can be provided in any order. If a parameter is given more than once, the first or last occurrence will take precedence.

- `-p <port>`: Specifies the port number on which the server will listen. This parameter is optional. If not provided or set to zero, the port number selection will be deferred to the bind function.
- `-f <file>`: Specifies the filename containing the game definition. This parameter is mandatory.
- `-t <timeout>`: Specifies the maximum time in seconds for server wait time. This must be a positive number. This parameter is optional. If not provided, the default timeout is 5 seconds.

## Client Invocation Parameters

Parameters for invoking the client can be provided in any order. If a parameter is given more than once or conflicting parameters are provided, the first or last occurrence will take precedence.

- `-h <host>`: Specifies the IP address or hostname of the server. This parameter is mandatory.
- `-p <port>`: Specifies the port number on which the server is listening. This parameter is mandatory.
- `-4`: Forces the use of IPv4 in communication with the server. This parameter is optional.
- `-6`: Forces the use of IPv6 in communication with the server. This parameter is optional.
- If neither `-4` nor `-6` is provided, the choice of IP protocol version will be deferred to the getaddrinfo function, with `ai_family = AF_UNSPEC`.
- `-N`: Specifies the position the client wants to occupy at the table. This parameter is mandatory.
- `-E`: Specifies the East position.
- `-S`: Specifies the South position.
- `-W`: Specifies the West position.
- `-a`: This optional parameter indicates that the client is an automated player. If not provided, the client acts as an intermediary between the server and the user-player.

## Communication Protocol

The server and client communicate using TCP. Messages are ASCII strings ending with the sequence `\r\n`. Besides this sequence, there are no other whitespace characters in the messages. Messages do not contain a terminal zero. Table positions are encoded with the letters N, E, S, or W. The type of round is encoded with a digit from 1 to 7. The trick number is encoded as a number from 1 to 13 written in base 10 without leading zeros. Card encoding includes the card value followed by the suit:

### Card Value Encoding

- 2, 3, 4, …, 9, 10, J, Q, K, A.

### Suit Encoding

- C – ♣ (clubs),
- D – ♦ (diamonds),
- H – ♥ (hearts),
- S – ♠ (spades).

### Communication Messages

The server and client send the following messages:

- **IAM<membership position>\r\n**  
  Message sent by the client to the server upon establishing a connection. It informs the server of the position the client wishes to occupy at the table. If the client does not send this message within the timeout period, the server closes the connection with the client. The server also treats any client that sends an invalid message after connection as non-compliant.

- **BUSY<list of occupied positions>\r\n**  
  Message sent by the server to the client if the chosen position at the table is already occupied. It informs the client which positions are currently occupied. After sending this message, the server closes the connection with the client. The server also treats any client that tries to connect to an ongoing game in this manner.

- **DEAL<round type><client's starting position for this round><list of cards>\r\n**  
  Message sent by the server to clients after four clients have gathered. It indicates the start of the round. The list contains the 13 cards that the client receives in this round.

- **TRICK<trick number><list of cards>\r\n**  
  Message sent by the server to the client requesting them to play a card on the table. The list includes zero to three cards currently on the table. If the client does not respond within the timeout period, the server will repeat the request. This message is also sent by the client to the server with the card they are playing (in which case the list contains one card).

- **WRONG<trick number>\r\n**  
  Message sent by the server to a client that sent an invalid message in response to a TRICK message. This message is also sent when the client tries to play a card without being prompted. The contents of the invalid message are ignored by the server.

- **TAKEN<trick number><list of cards><client's position taking the trick>\r\n**  
  Message sent by the server to clients. It informs which client took the trick. The list includes the four cards that make up the trick, in the order they were played.

- **SCORE<client's position><points><client's position><points><client's position><points><client's position><points>\r\n**  
  Message sent by the server to clients after the end of the round. It informs them of the scoring for this round.

- **TOTAL<client's position><points><client's position><points><client's position><points><client's position><points>\r\n**  
  Message sent by the server to clients after the end of the round. It informs them of the total scores in the game.

The client ignores invalid messages from the server.

### End of Game

After the game concludes, the server disconnects all clients and terminates its operation. Once the server disconnects, the client also terminates.

If a client disconnects during the game, the server pauses the game waiting for the client to reconnect to an empty position at the table. Once the client reconnects, the server provides the current state of the ongoing round. The server sends the cards that the client received in this round via the DEAL message. It also sends the previously played tricks using the TAKEN messages. After this, the server resumes the game and the exchange of TRICK messages.

## Functional Requirements

The programs should thoroughly check the validity of the invocation parameters. They should print understandable error messages to the standard output for diagnostics.

The server is assumed to be fair, while clients do not have to be. The server should meticulously verify that clients adhere to the game rules.

### Client Behavior

A heuristic strategy for playing the game must be implemented in the client.

Both the server and the client functioning as an automated player should print a game report to standard output.

The programs terminate with exit code 0 if the game concludes properly; otherwise, they terminate with exit code 1.

## Game Definition File Format

The file specified by the server contains a textual description of the game. It describes the rounds to be played in the following order:

<round type><client's position starting first in the round>\n <list of cards for client N>\n <list of cards for client E>\n <list of cards for client S>\n <list of cards for client W>\n


## User Communication with Client

The client acting as an intermediary provides a text-based user interface. The user interface should be intuitive. The client prints information for the user and requests card plays from the server. It reads from standard input the user's decisions and commands, such as displaying the cards in hand and tricks taken. The client should be able to respond to user commands at any time. Communication with the server must not block the user interface.

### Server Information Formatting

Messages from the server are formatted as follows:

- **BUSY<list of occupied places>**  
  Place busy, list of busy places received: `<list of occupied places>`.

- **DEAL<round type><client's starting position><list of cards>**  
  New deal `<round type>`: starting place `<client's starting position>`, your cards: `<list of cards>`.

- **WRONG<trick number>**  
  Wrong message received in trick `<trick number>`.

- **TAKEN<trick number><list of cards><client's position taking the trick>**  
  A trick `<trick number>` is taken by `<client's position taking the trick>`, cards `<list of cards>`.

- **SCORE<client's position><points><client's position><points><client's position><points><client's position><points>**  
  The scores are:
<client's position> | <points> <client's position> | <points> <client's position> | <points> <client's position> | <points>

- **TOTAL<client's position><points><client's position><points><client's position><points><client's position><points>**  
The total scores are:
<client's position> | <points> <client's position> | <points> <client's position> | <points> <client's position> | <points>


- **TRICK<trick number><list of cards>**  
Trick: (`<trick number>`) `<list of cards>`  
Available: `<list of cards remaining in hand>`.

### Card Selection and Commands

For the TRICK message, the user selects a card to play by typing an exclamation mark followed by the card code (e.g., `!10C`) and pressing enter. The user has the following commands at their disposal, which end with pressing enter:

- **cards**: Displays the list of cards in hand.
- **tricks**: Displays the list of tricks taken in the last round, in the order taken; each trick is a list of cards on a separate line.

All lists in messages for the user should be printed, separated by commas and spaces.

