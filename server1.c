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
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct{
	int userid;
	char phone[20];
	char name[20];
    char surname[20];
} user;

void receive_message(int client_sockfd, char* buffer) {
    memset(buffer, '\0', BUFFER_SIZE);
    read(client_sockfd, buffer, BUFFER_SIZE - 1);
    //printf("Client Side: %s\n", buffer);
}

int send_message(int sockfd, char* message) {
    //printf("This Side: %s\n", message);
    if(strlen(message) >= 1)
        send(sockfd, message, strlen(message), 0);
}

void displayContact(user* data, int numEntries) {
    int i;
    printf("| %-10s| %-20s| %-20s| %-20s\n", "User ID", "Phone", "Name", "Surname");
    printf("|──────────────────────────────────────────────────────\n");

    for (i = 0; i < numEntries; ++i) {
        printf("| %-10d| %-20s| %-20s| %-20s\n", data[i].userid, data[i].phone, data[i].name, data[i].surname);
        if (i < numEntries - 1) {
            printf("|───────────|─────────────────────|────────────────────\n");
        }
    }
    printf("|──────────────────────────────────────────────────────\n");
}

void getCurrentDateTime(char* dateTimeString, int maxLength) {
    time_t t;
    struct tm* tmInfo;

    // O anki zamanı al
    time(&t);
    tmInfo = localtime(&t);

    // Tarih ve saat bilgilerini biçimlendirip dateTimeString'e yerleştir
    snprintf(dateTimeString, maxLength, "%02d:%02d %02d-%02d-%04d",
             tmInfo->tm_hour, tmInfo->tm_min,
             tmInfo->tm_mday, tmInfo->tm_mon + 1, tmInfo->tm_year + 1900);
}

void writeStructToFile(char* filename, user* data, int numEntries) {
    FILE* file = fopen(filename, "w"); // Open the file for writing (overwrite mode)
    int i;
    

    if (file != NULL) {
        // Write the counter indicator
        fprintf(file, "%d\n", numEntries);

        // Write each user to the file
        for (i = 0; i < numEntries; ++i) {
            int result = fprintf(file, "%d, %s, %s, %s;\n", data[i].userid, data[i].phone, data[i].name, data[i].surname);

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
            fscanf(file, "%d, %19[^,], %19[^,], %19[^;\n]%*[; \t\n]",
                &data[i].userid, data[i].phone, data[i].name, data[i].surname);
        }

        fclose(file);
        return data;
    } else {
        *numEntries = 0;
        return NULL;
    }
}

void sendContact(int sockid){
    char buffer[BUFFER_SIZE];

    receive_message(sockid, buffer);

    char* user_id_str = buffer + strlen("/recvContact");
    int userid = atoi(user_id_str);

    char filename[40];
    if(userid != -1){
        sprintf(filename, "data/%d/contacts.txt", userid);
    }
    else{
        sprintf(filename, "users.txt");
    }
    
    size_t numEntries = 0;
    user* data = readStructFromFile(filename, &numEntries);

    printf("sending contacts\n");
    if(numEntries == 0){
        send_message(sockid, "Kayitli Kullanici Yok.");
        return;
    }
    else{
        snprintf(buffer, BUFFER_SIZE, "%d", (int)numEntries);
        send_message(sockid, buffer);
        printf("%s\n", buffer);
    }
    int i;

    receive_message(sockid, buffer);

    if(strcmp(buffer, "/ready") == 0){
        for(i=0; i<numEntries; i++){
            char buffer2[BUFFER_SIZE];
            memset(buffer, '\0', BUFFER_SIZE);
            memset(buffer2, '\0', BUFFER_SIZE);
            snprintf(buffer, BUFFER_SIZE, "%s, %s, %s, %d", data[i].name, data[i].surname, data[i].phone, data[i].userid);
            printf("%s, %s, %s, %d\n", data[i].name, data[i].surname, data[i].phone, data[i].userid);
            do{
                send_message(sockid, buffer);

                receive_message(sockid, buffer2);
            }while(strcmp(buffer2, "received") != 0);
            //printf("%s\n", buffer);
        }
    }

    return;
    
}

void appendMessage(const char *filename, const char *message) {
    FILE *file = fopen(filename, "a"); // Open file in append mode

    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%s\n", message); // Append the message to the file

    fclose(file); // Close the file
}

void initializeFileSystem(int userid) {
    // Create user directory
    char data_directory[] = "data";
    struct stat st_data = {0};
    if (stat(data_directory, &st_data) == -1) {
        if (mkdir(data_directory, 0777) != 0) {
            fprintf(stderr, "Error creating data directory\n");
            // Handle error as needed
        }
    }

    
    // Check if the directory already exists, if not, create it
    char user_directory[10];
    sprintf(user_directory, "data/%d", userid);
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
                snprintf(response, BUFFER_SIZE, "%s, %s, %s, %d", data[i].name, data[i].surname, data[i].phone, data[i].userid);
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
        memset(buffer, '\0', BUFFER_SIZE);
        receive_message(sockid, buffer);
        
        // Reallocate memory for one more user
        user* temp = realloc(data, (numEntries + 1) * sizeof(user));
        if (temp == NULL) {
            fprintf(stderr, "Error reallocating memory.\n");
            free(data);  // Free original data before exiting
            exit(EXIT_FAILURE);
        }
        data = temp;

        sscanf(buffer, "%[^,], %[^,], %[^,], %d", data[numEntries].name, data[numEntries].surname, data[numEntries].phone, &data[numEntries].userid);

        // Update the number of entries
        numEntries++;

        // Save the updated user data to the file
        writeStructToFile("users.txt", data, numEntries);
        initializeFileSystem(userid);

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "%d, %s, %s, %s",
                data[numEntries - 1].userid, data[numEntries - 1].name, data[numEntries - 1].surname, data[numEntries - 1].phone);
        send_message(sockid, response);
        printf("Registered %d\n", userid);
    }

    // Free allocated memory for user data
    free(data);
}

void addContact(int sockid, int userid){
    char buffer[BUFFER_SIZE];
    send_message(sockid, "/ok");
    sendContact(sockid);

    do{
        receive_message(sockid, buffer);
    }while (strlen(buffer) == 0);

    int addUser = atoi(buffer);

    // if client wants to add himself
    if(addUser == userid){
        send_message(sockid, "You can't add yourself\n");
        return;
    }

    char filename[40];
    sprintf(filename, "data/%d/contacts.txt", userid);

    size_t totalUsers = 0, numEntries = 0;
    int i=0, found=0;
    user* contactList = readStructFromFile("users.txt", &totalUsers);
    user* data = readStructFromFile(filename, &numEntries);
    user newUser;

    while(data != NULL && i < totalUsers){
        if(data[i].userid == addUser){
            send_message(sockid, "User already exists\n");
            return;
        }
        i++;
    }

    i=0;
    while(found == 0 && i < totalUsers){
        if(data != NULL && data[i].userid == addUser){
            return;
        }
        if(contactList[i].userid == addUser){
            found = 1;

            user* temp = realloc(data, (numEntries + 1) * sizeof(user));

            if (temp == NULL) {
                fprintf(stderr, "Error reallocating memory.\n");
                free(data);  // Free original data before exiting
                exit(EXIT_FAILURE);
            }
            data = temp;

            strcpy(data[numEntries].name, contactList[i].name);
            strcpy(data[numEntries].surname, contactList[i].surname);
            strcpy(data[numEntries].phone, contactList[i].phone);
            data[numEntries].userid = addUser;
            printf("%s, %s, %s, %d\n", data[numEntries].name, data[numEntries].surname, data[numEntries].phone, data[numEntries].userid);
            // Update the number of entries
            numEntries++;

            //displayContact(data, numEntries);

            // Save the updated user data to the file
            writeStructToFile(filename, data, numEntries);
        }
        i++;
    }
    if(found == 0)
        send_message(sockid, "Failed to add\n");
    else
        send_message(sockid, "Added to contact\n");
    return;
}

void deleteContact(int sockid, int userid){
    char buffer[BUFFER_SIZE];
    
    send_message(sockid, "/ok");
    sendContact(sockid);

    do{
        receive_message(sockid, buffer);
    }while (strlen(buffer) == 0);

    if(strcmp(buffer, "/empty") == 0)
        return;
    int addUser = atoi(buffer);

    // if client wants to add himself
    if(addUser == userid){
        send_message(sockid, "You can't delete yourself\n");
        return;
    }

    char filename[40];
    sprintf(filename, "data/%d/contacts.txt", userid);

    size_t numEntries = 0;
    int i=0;
    user* data = readStructFromFile(filename, &numEntries);
    user newUser;
    
    while(data != NULL && i < numEntries){
        if(data[i].userid == addUser){
            int j;
            numEntries--;
            if(numEntries != 0){
                for(j = i; j<numEntries; j++){
                    strcpy(data[j].name, data[j+1].name);
                    strcpy(data[j].surname, data[j+1].surname);
                    strcpy(data[j].phone, data[j+1].phone);
                    data[j].userid = data[j+1].userid;
                }
                user* temp = realloc(data, (numEntries) * sizeof(user));

                if (temp == NULL) {
                    fprintf(stderr, "Error reallocating memory.\n");
                    free(data);  // Free original data before exiting
                    exit(EXIT_FAILURE);
                }
                data = temp;

            writeStructToFile(filename, data, numEntries);
            }
            else{
                FILE* file = fopen(filename, "w"); // Open the file for writing (overwrite mode)
                if (file != NULL)
                    // Write the counter indicator
                    fprintf(file, "%s", "");
            }

            send_message(sockid, "Kullanici Silindi.\n");
            return;
        }
        i++;
    }
    send_message(sockid, "Kullanici Bulunamadi.\n");

}

void listContacts(int sockid){
    char buffer[BUFFER_SIZE];
    send_message(sockid, "/ok");
    sendContact(sockid);
    do {
        receive_message(sockid, buffer);
    }while(strcmp(buffer, "/done"));

}

void checkContact(int sockid, user source, int destid){
    char filename[20];
    sprintf(filename, "data/%d/contacts.txt", destid);
    size_t numEntries = 0;
    user* data = readStructFromFile(filename, &numEntries);

    int i, found;
    user newUser;

    while(data != NULL && i < numEntries){
        if(data[i].userid == source.userid){
            send_message(sockid, "Message Sent");
            return;
        }
        i++;
    }

    user* temp = realloc(data, (numEntries + 1) * sizeof(user));

    if (temp == NULL) {
        fprintf(stderr, "Error reallocating memory.\n");
        free(data);  // Free original data before exiting
        exit(EXIT_FAILURE);
    }
    data = temp;

    strcpy(data[numEntries].name, source.name);
    strcpy(data[numEntries].surname, source.surname);
    strcpy(data[numEntries].phone, source.phone);
    data[numEntries].userid = source.userid;
    printf("%s, %s, %s, %d\n", data[numEntries].name, data[numEntries].surname, data[numEntries].phone, data[numEntries].userid);
    // Update the number of entries
    numEntries++;

    // Save the updated user data to the file
    writeStructToFile(filename, data, numEntries);
    send_message(sockid, "You've added to %d's contact list \nMessage Sent");

    return;

}

void sendMessage(int sockid, int userid){
    char buffer[BUFFER_SIZE];
    send_message(sockid, "/ok");
    sendContact(sockid);

    receive_message(sockid, buffer);

    if(strcmp(buffer, "/emptyContact") == 0){
        return;
    }
    else{
        int i;
        size_t numEntries;
        char message[BUFFER_SIZE-100];
        int destid;
        char filename[10];
        char msg_directory[60];
        char date[20];

        char sourceMsg[BUFFER_SIZE];
        char destMsg[BUFFER_SIZE];

        sscanf(buffer, "%d, %[^\n]", &destid, message);

        user* data = readStructFromFile("users.txt", &numEntries);
        user destUser, sourceUser;
        for (i = 0; i < numEntries; i++){
            if(data[i].userid == userid){
                sourceUser = data[i];
            }
            else if(data[i].userid == destid){
                destUser = data[i];
            }
        }
        getCurrentDateTime(date, 20);

        // mesaj atan kullanıcının dosyasına sen ile başlayacak şekilde
        // yerleştirir.
        sprintf(sourceMsg, "%s | %-15s: %s", date, "Sen", message);
        sprintf(msg_directory, "data/%d/messages/%d.txt", userid, destid);
        appendMessage(msg_directory, sourceMsg);

        sprintf(destMsg, "%s | %-15s: %s", date, sourceUser.name, message);
        sprintf(msg_directory, "data/%d/messages/%d.txt", destid, userid);
        appendMessage(msg_directory, destMsg);

        sprintf(destMsg, "(Yeni Mesaj!) %s %s %d", sourceUser.name, sourceUser.surname, sourceUser.userid);
        sprintf(msg_directory, "data/%d/messages/messages.txt", destid);
        appendMessage(msg_directory, destMsg);

        // check if destination added source
        checkContact(sockid, sourceUser, destUser.userid);

    }
}

int getMessages(char* messageList, char* filename, int* chatIDs) {
    FILE* file = fopen(filename, "r");
    
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int lineNumber = 1;
    char dummy1[200];
    char dummy2[100];
    memset(dummy2, '\0', 100);
    memset(dummy1, '\0', 200);

    // Assuming each line in the file is a message
    while (fgets(dummy2, 100, file) != NULL) {
        // Add line number to the messageList
        chatIDs[lineNumber-1] = atoi(&dummy2[strlen(dummy2)-2]);
        sprintf(dummy1, "%d - %s", lineNumber, dummy2);
        strcat(messageList, dummy1);
        memset(dummy2, '\0', 200);

        lineNumber++;
    }

    fclose(file);
    return 0;
}

void checkMessages(int sockid, int userid){
    char buffer[BUFFER_SIZE];
    int status = 1;
    
    send_message(sockid, "/ok");

    char messageList[BUFFER_SIZE];
    memset(messageList, '\0', BUFFER_SIZE);
    char filename[40];
    int chatIDs[40];
    sprintf(filename, "data/%d/messages/messages.txt", userid);
    status = getMessages(messageList, filename, chatIDs);
    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "/ok") != 0);

    if(status){
        send_message(sockid, "/noFile");
        return;
    }

    send_message(sockid, messageList);

    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "") == 0);

    int chatid = atoi(buffer);

    printf("ID: %d\n", chatid);

    sprintf(filename, "data/%d/messages/%d.txt", userid, chatid);

    // yeni mesajları topla
    // client'a gönder

    FILE* file = fopen(filename, "r");
    
    char dummy[200];
    memset(dummy, '\0', 200);

    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "/start") != 0);
    printf("Got start\n");

    // Assuming each line in the file is a message
    while (fgets(dummy, 100, file) != NULL) {
        // Add line number to the messageList
        printf("%s", dummy);
        send_message(sockid, dummy);

        receive_message(sockid, buffer);
    }

    send_message(sockid, "/EOF");

    

    fclose(file);
    
}

void *handle_client(void *arg) {
    int sockid = *((int *)arg);
    char buffer[BUFFER_SIZE] = {0};
    int status = 1;


    receive_message(sockid, buffer);
    const char* user_id_str = buffer + strlen("/login"); // Skip the "/login" part
    int user_id = atoi(user_id_str);

    // Now you have the user ID, you can use it as needed
    printf("User %d is trying to log in.\n", user_id);
    logUser(sockid, user_id);
        
    printf("Login succes\n");
    while (status) {
        receive_message(sockid, buffer);
        
        if (strncmp(buffer, "/exit", strlen("/exit")) == 0){
            const char* user_id_str = buffer + strlen("/exit"); // Skip the "/exit" part
            int user_id = atoi(user_id_str);

            printf("User %d is logged out\n", user_id);
            status = 0;
        }
        else if (strcmp(buffer, "/addContact") == 0){
            addContact(sockid, user_id);
        }
        else if (strcmp(buffer, "/deleteContact") == 0){
            deleteContact(sockid, user_id);
        }
        else if (strcmp(buffer, "/listContacts") == 0){
            listContacts(sockid);
        }
        else if (strcmp(buffer, "/sendMessages") == 0){
            sendMessage(sockid, user_id);
        }
        else if (strcmp(buffer, "/checkMessages") == 0){
            checkMessages(sockid, user_id);
        }
        else if (strcmp(buffer, "/deleteMessage") == 0){
            
        }
        else if(strcmp(buffer, "") != 0)
            printf("%s\n", buffer);
    }

    // Close the socket and exit the thread
    close(sockid);
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