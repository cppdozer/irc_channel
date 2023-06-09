# Chat Application

This repository contains a client-server chat application implemented in C. The application allows multiple clients to connect to a server and communicate with each other through chat rooms.

## Features

- Login with a username
- Create and join chat rooms
- List available chat rooms
- List users in a chat room
- Send and receive messages within a chat room
- Close a chat room (only available to the creator)
- Logout from the server

## Prerequisites

- C compiler
- POSIX threads library

## Usage

1. Clone the repository:

   ```shell
   git clone <repository-url>
   ```
   
```shell 
  gcc client.c -o client -lpthread
  gcc server.c -o server -lpthread
  ```

Start the server:

```shell
./server
```

Start the client(s):

```shell
./client
```

Use the available commands to interact with the chat application. Type help to see the list of commands.

Command List
- login <username>: Login with the specified username.
- logout: Logout from the server.
- list_rooms: List available chat rooms.
- list_users: List users in the current chat room.
- open <room_name>: Create a new chat room.
- close <room_name>: Close the specified chat room (only available to the creator).
- enter <room_name>: Join the specified chat room.
- whereami: Display the current chat room.
  
###License
This project is licensed under the MIT License.
coded by cppdozer.
