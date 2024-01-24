# Multithreaded Messaging

## Introduction
This is a multithreaded messaging project written in C which works in local network. This project leverages multithreading to handle multiple client connections seamlessly.

## Features
With MultithreadedMessaging, you can:
- **Login/Register:** Access your messaging account.
- **Manage Contacts:** Add and view contacts in your personal list.
- **Instant Messaging:** Send and receive messages with anyone in your contact list.

## File Structure
**The project is organized as follows:**
  - `server.out`: The server executable.
  - `client.out`: The client executable.
  - `users.txt`: Stores all registered users in the system.
  - `data/`: The main directory for user data.
    - `user_idX/`: Unique user directory (X represents the user ID)
      - `contacts.txt`: User's contact list.
      - `messages/`: Contains message data.
        - `user_idY.txt`: Messages exchanged with user Y.
        - `messages.txt`: Stores new and old message notifications.

## Getting Started
To start using MultithreadedMessaging: (You can use the `compile` file. Be careful with file names, this is a Linux guideline)
1. Compile the source code to generate `server.out` and `client.out`. 
2. Run `server.out` to start the server.
3. From another terminal execute `client.out X` on client machines to connect to the server. (X represents user IDs)
4. Follow the on-screen prompts to register or log in.
5. Begin messaging!

## Contributing
Contributions to MultithreadedMessaging are welcome! Whether it's feature requests, bug reports, or code contributions, your input is valuable. Please read our contributing guidelines for more information.

---
