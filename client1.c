// Client side C/C++ program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <asm-generic/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_USER 50

// FLAGS
#define LOGIN 0
#define SEND 1
#define RECEIVE 2
#define DELETEUSER 4
#define CHECKMESSAGES 5

typedef struct{
	int userid;
	char phone[20];
	char name[40];
} user;

void receive_message(int client_sockfd, char* buffer, size_t buffsize) {
    bzero(buffer, BUFFER_SIZE);
    read(client_sockfd, buffer, buffsize - 1);
    //printf("Server Side: %s\n", buffer);
}

void send_message(int sockfd, char* message) {
    //printf("Client Side: %s\n", message);
    send(sockfd, message, strlen(message), 0);
}

void display_menu(user myUser) {

    system("clear");
    printf(" User Info\n");
    printf(" %d - %s - %s\n\n", myUser.userid, myUser.name, myUser.phone);
    printf("- 1. Display contacts\n");
    printf("- 2. Add contact\n");
    printf("- 3. Delete contact\n");
    printf("- 4. Check messages\n");
    printf("- 5. Send message\n");
    printf("- 6. Delete message\n");
    printf("- 7. Logout\n");
    printf("Enter your choice: ");
}

void init_main(user myData, int sockid){
    char buffer[BUFFER_SIZE];

    while (1){
        bzero(buffer, BUFFER_SIZE);
        int choice;

        display_menu(myData);
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                // Display contacts
                break;
            case 2:
                // Add contact
                break;
            case 3:
                // Delete contact
                break;
            case 4:
                // Check messages
                break;
            case 5:
                // Send message
                break;
            case 6:
                // Delete message
                break;
            case 7:
                snprintf(buffer, sizeof(buffer), "/exit %d", myData.userid);
                send_message(sockid, buffer);
                exit(0);
                break;
            default:
                printf("Invalid choice");
        }
    }
    
}

void login_to_server(int sockid, int userid){
    
    // Allocate memory for the user data
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);

    snprintf(buffer, sizeof(buffer), "/login %d", userid);

    // Send the login message to the server
    send_message(sockid, buffer);

    bzero(buffer, BUFFER_SIZE);
    receive_message(sockid, buffer, BUFFER_SIZE);

    if (strcmp(buffer, "/register") == 0){
        char name[40];
        char phone[20];

        printf("Your Name: ");
        scanf(" %[^\n]", name);
        getchar();

        printf("Your Phone: ");
        scanf("%s", phone);
        getchar();

        char userString[BUFFER_SIZE];
        snprintf(userString, sizeof(userString), "%s, %s, %d", name, phone, userid);

        // Send the user data to the server
        send_message(sockid, userString);

        receive_message(sockid, buffer, BUFFER_SIZE);
        login_to_server(sockid, userid);
    }
    else{
        user myData;
        sscanf(buffer, "%[^,], %[^,], %d", myData.name, myData.phone, &myData.userid);
        printf("%s, %s, %d\n", myData.name, myData.phone, myData.userid);
        init_main(myData, sockid);
    }
}

void connect_server(int user_id){
    int status, valread, sockid;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    if ((status = connect(sockid, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
		printf("\nConnection Failed \n");
		return;
	} 
	else {
		printf("Connection Successful. Server address: %s, Port: %d\n",
		inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
	}

    printf("Logging in...\n");
    login_to_server(sockid, user_id);

    close(sockid);
}

int main(int argc, char* argv[]){

    if (argc < 2) {
        fprintf(stderr,"usage %s userid\n", argv[0]);
        exit(0);
    }
    

    connect_server(atoi(argv[1]));
	

    return 0;
}
