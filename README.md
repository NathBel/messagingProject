# Messaging Project

This is a messaging project written in C that uses sockets and threads to enable real-time communication between clients. The project consists of two main components: a server and a client.

## Installation

To compile the project, open a terminal and run the following commands:

(`gcc server.c -o server -lpthread`)
(`gcc client.c -o client -lpthread`)

This will compile the server and client executables, which can be run using the following commands:

(`./server PORT`)
(`./client IP PORT`)

## Usage

To use the messaging system, start the server by running the server executable, and then start two or more client instances by running the client executable. The clients will connect to the server and can then communicate with each other in real time.

The client interface is designed to be user-friendly and intuitive, with options to create and join chat rooms, send and receive messages, and leave chat rooms. The server is multi-threaded, allowing it to handle multiple connections simultaneously.

## Licence
Project do in Polytech Montpellier by [THIRION Margaux](https://github.com/MargauxThirion), [COULON Thomas](https://github.com/thomasc21) and [BELLUS Nathan](https://github.com/NathBel)
