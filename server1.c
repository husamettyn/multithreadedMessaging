// gcc server1.c -o serv.out
// gcc client1.c -o cli.out

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <asm-generic/socket.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct{
	int userid;
	char phone[20];
	char name[40];
} user;

void receive_message(int client_sockfd, char* buffer, size_t buffsize) {
    bzero(buffer, buffsize);
    read(client_sockfd, buffer, buffsize - 1);
}

void send_message(int sockfd, char* message) {
    send(sockfd, message, strlen(message), 0);
}

void writeStructToFile(char* filename, user* data, int numEntries) {
    FILE* file = fopen(filename, "w"); // Open the file for writing (overwrite mode)
    int i;
    

    if (file != NULL) {
        // Write the counter indicator
        fprintf(file, "%d\n", numEntries);

        // Write each user to the file
        for (i = 0; i < numEntries; ++i) {
            int result = fprintf(file, "%d, %s, %s;\n", data[i].userid, data[i].phone, data[i].name);

            // Check if writing was successful
            if (result < 0) {
                fprintf(stderr, "Error writing data to file.\n");
                break;  // Exit loop if an error occurs
            }
        }

        fclose(file);
    } else {
        fprintf(stderr, "File Opening Error\n");
    }
}

user* readStructFromFile(char* filename, size_t* numEntries) {
    FILE* file = fopen(filename, "r"); // Open the file for reading in text mode
    int i;
    if (file != NULL) {
        // Read the counter indicator
        if (fscanf(file, "%lu", numEntries) != 1) {
            // Error handling for invalid or missing counter indicator
            fclose(file);
            return NULL;
        }

        // Skip the newline character after the counter indicator
        fgetc(file);

        // Allocate memory for the users
        user* data = (user*)malloc(*numEntries * sizeof(user));

        // Read each user from the file
        for (i = 0; i < *numEntries; ++i) {
            fscanf(file, "%d, %19[^,], %39[^;\n]%*[; \t\n]", &data[i].userid, data[i].phone, data[i].name);
        }

        fclose(file);
        return data;
    } else {
        *numEntries = 0;
        return NULL; 
    }
}

void listUsers(user* data, size_t numEntries) {
    printf("User ID\tPhone\t\t\tName\n");
    printf("------\t-----------------------\t----------------------------\n");

    for (size_t i = 0; i < numEntries; ++i) {
        printf("%d\t%s\t\t%s\n", data[i].userid, data[i].phone, data[i].name);
    }
}

void viewContactAll(int sockid){
    size_t numEntries = 0;
    user* data = readStructFromFile("users.txt", &numEntries);
    char buffer[BUFFER_SIZE];

    if(numEntries == 0)
        send_message(sockid, "Kayitli Kullanici Yok.");
    else{
        snprintf(buffer, BUFFER_SIZE, "%d", numEntries);
        send_message(sockid, buffer);
    }
    int i;

    
}

void initializeFileSystem(int userid) {
        // Create user directory
        char user_directory[10];
        sprintf(user_directory, "data/%d", userid);
        
        /*
        char user_directory[10];
        char messages_directory[40];
        char contacts_file[40];
        sprintf(user_directory, "data/%d", userid);
        sprintf(messages_directory, "%s/messages", user_directory);
        sprintf(contacts_file, "%s/contacts.txt", user_directory);
        */

        // Check if the directory already exists, if not, create it
        struct stat st = {0};
        if (stat(user_directory, &st) == -1) {
            if (mkdir(user_directory, 0777) != 0) {
                fprintf(stderr, "Error creating user directory for user_id %d\n", userid);
                // Handle error as needed
            }
        }

        // Create messages directory
        char messages_directory[40];
        sprintf(messages_directory, "%s/messages", user_directory);
        if (stat(messages_directory, &st) == -1) {
            if (mkdir(messages_directory, 0777) != 0) {
                fprintf(stderr, "Error creating messages directory for userid %d\n", userid);
                // Handle error as needed
            }
        }

        // Create contacts file
        char contacts_file[40];
        sprintf(contacts_file, "%s/contacts.txt", user_directory);
        FILE* file = fopen(contacts_file, "a");  // Open for append
        if (file == NULL) {
            fprintf(stderr, "Error creating contacts file for userid %d\n", userid);
            // Handle error as needed
        }
        fclose(file);
}

void logUser(int sockid, int userid) {
    
    size_t numEntries = 0;
    user* data = readStructFromFile("users.txt", &numEntries);
    char buffer[BUFFER_SIZE];

    // Check if there is any user with this ID
    int found = 0;
    int i = 0;
    if (data != NULL) {
        while (found == 0 && i < numEntries) {
            if (data[i].userid == userid) {
                // User found, send user info to the client
                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response), "%s, %s, %d",
                        data[i].name, data[i].phone, data[i].userid);
                send_message(sockid, response);
                found = 1;
                printf("Logged in %d\n", userid);
            }
            i++;
        }
    }

    // If the user is not found, ask the client to register
    if (!found) {
        send_message(sockid, "/register");
        bzero(buffer, BUFFER_SIZE);
        receive_message(sockid, buffer, BUFFER_SIZE);
        
        // Reallocate memory for one more user
        user* temp = realloc(data, (numEntries + 1) * sizeof(user));
        if (temp == NULL) {
            fprintf(stderr, "Error reallocating memory.\n");
            free(data);  // Free original data before exiting
            exit(EXIT_FAILURE);
        }
        data = temp;

        sscanf(buffer, "%[^,], %[^,], %d", data[numEntries].name, data[numEntries].phone, &data[numEntries].userid);

        // Update the number of entries
        numEntries++;

        // Save the updated user data to the file
        writeStructToFile("users.txt", data, numEntries);
        initializeFileSystem(userid);

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "%d, %s, %s",
                data[numEntries - 1].userid, data[numEntries - 1].name, data[numEntries - 1].phone);
        send_message(sockid, response);
        printf("Registered %d\n", userid);
    }

    // Free allocated memory for user data
    free(data);
}



void *handle_client(void *arg) {
    int socket_fd = *((int *)arg);
    char buffer[BUFFER_SIZE] = {0};
    int status = 1;

    while (status) {
        receive_message(socket_fd, buffer, BUFFER_SIZE);
        if (strncmp(buffer, "/login", strlen("/login")) == 0) {
            // Extract the user ID from the message
            const char* user_id_str = buffer + strlen("/login"); // Skip the "/login" part
            int user_id = atoi(user_id_str);

            // Now you have the user ID, you can use it as needed
            printf("User %d is trying to log in.\n", user_id);
            logUser(socket_fd, user_id);
        }
        else if (strncmp(buffer, "/exit", strlen("/exit")) == 0){
            const char* user_id_str = buffer + strlen("/exit"); // Skip the "/exit" part
            int user_id = atoi(user_id_str);

            printf("User %d is logged out\n", user_id);
            status = 0;
        }
    }

    // Close the socket and exit the thread
    close(socket_fd);
    pthread_exit(NULL);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server succesfully started on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create a new thread for each client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&new_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // Detach the thread to avoid memory leaks
        pthread_detach(thread_id);
    }

    // Closing the listening socket (Note: This part will not be reached in the current example)
    close(server_fd);
    return 0;
}

/*

snprintf(buffer, BUFFER_SIZE, "%s, %s, %d", name, phone, userid);
sscanf(buffer, "%[^,], %[^,], %d", myData.name, myData.phone, &myData.userid);

*/