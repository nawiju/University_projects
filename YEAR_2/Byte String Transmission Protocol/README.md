# Byte Stream Transmission Protocol (BSTP)

Welcome to the Byte Stream Transmission Protocol (BSTP) repository! This project implements a protocol for transmitting byte streams between clients and servers over TCP or UDP connections. The protocol includes provisions for connection establishment, data transmission, and termination, with support for both reliable (TCP) and unreliable (UDP) communication.

## Overview

BSTP is designed to facilitate efficient and reliable transmission of byte streams between clients and servers. It offers flexibility by supporting both TCP and UDP as underlying transport layer protocols. Additionally, UDP connections can optionally implement a simple retransmission mechanism to enhance reliability.

## Features

- **Connection Establishment**: Clients initiate connections to servers using the `CONN` packet. Servers respond with either a `CONACC` packet to accept the connection or a `CONRJT` packet to reject it.
- **Data Transmission**: Byte streams are transmitted in packets of variable length using the `DATA` packet type. Servers acknowledge received data with an `ACC` packet or reject it with an `RJT` packet.
- **Connection Termination**: Upon receiving all data, servers send an `RCVD` packet to confirm reception before proceeding to handle subsequent connections. TCP clients close connections after sending all data and receiving an `RCVD` confirmation, while UDP clients terminate immediately after receiving the `RCVD` packet.

## Implementation Details

This repository contains two main programs:

1. **Server (`ppcbs`)**: The server program accepts connections from clients, handles data transmission according to the BSTP protocol, and prints received byte streams to standard output. It supports both TCP and UDP protocols.
   
2. **Client (`ppcbc`)**: The client program establishes connections to servers, transmits byte streams following the BSTP protocol, and terminates upon successful transmission. It supports TCP, UDP, and UDP with retransmission protocols.

## Usage

### Server

To run the server, use the following command:

`./ppcbs <protocol> <port>`

- `<protocol>`: Specifies the protocol to use (`tcp` or `udp`).
- `<port>`: Specifies the port number on which the server should listen.

### Client

To run the client, use the following command:

`./ppcbc <protocol> <server_address> <port>`

- `<protocol>`: Specifies the protocol to use (`tcp`, `udp`, or `udpr` for UDP with retransmission).
- `<server_address>`: Specifies the address or hostname of the server.
- `<port>`: Specifies the port number on which the server is listening.


