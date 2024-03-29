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

typedef struct{
	int userid;
	char phone[20];
	char name[20];
    char surname[20];
} user;

// Function prototypes
void receive_message(int client_sockfd, char* buffer);
void send_message(int sockfd, char* message);
void display_menu(user myUser);
void displayContact(user* data, int numEntries);
user* recvContact(int sockid, int* num, int userid);
void addContact(int sockid);
void deleteContact(int sockid, int userid);
void listContacts(int sockid, int userid);
void sendMessage(int sockid, int userid);
void checkMessages(int sockid, int userid);
void init_main(user myUser, int sockid);
void login_to_server(int sockid, int userid);
void connect_server(int user_id);

// Main function: Entry point of the program
int main(int argc, char* argv[]){

    // Check command line arguments
    if (argc < 2) {
        fprintf(stderr,"usage %s userid\n", argv[0]);
        exit(0); // Exit if not enough arguments
    }

    // Connect to server with user ID
    connect_server(atoi(argv[1]));
    
    return 0; // End of program
}

void receive_message(int client_sockfd, char* buffer) {
    memset(buffer, '\0', BUFFER_SIZE);
    read(client_sockfd, buffer, BUFFER_SIZE - 1);
    //printf("Server Side: %s\n", buffer);
}

void send_message(int sockfd, char* message) {
    //printf("This Side: %s\n", message);
    send(sockfd, message, strlen(message), 0);
}

void display_menu(user myUser) {
    int i;
    for (i = 0; i < 3; ++i) {
        usleep(500000);  // Sleep for 0.5 seconds (500,000 microseconds)
        printf("#-#-#-");
        fflush(stdout);  // Flush the output buffer
    }
    system("clear");
    printf(" Welcome %s %s\n", myUser.name, myUser.surname);
    printf(" %d - %s\n\n", myUser.userid, myUser.phone);
    printf("- 1. List contacts\n");
    printf("- 2. Add contact\n");
    printf("- 3. Delete contact\n");
    printf("- 4. Check messages\n");
    printf("- 5. Send message\n");
    printf("- 6. Delete message (Not Working)\n");
    printf("- 0. Logout\n");
    printf("Enter your choice: ");
}

void displayContact(user* data, int numEntries) {
    int i;
    printf("| %-10s| %-20s| %-20s| %-20s\n", "User ID", "Phone", "Name", "Surname");
    printf("|───────────────────────────────────────────────────────────────────────────\n");

    for (i = 0; i < numEntries; ++i) {
        printf("| %-10d| %-20s| %-20s| %-20s\n", data[i].userid, data[i].phone, data[i].name, data[i].surname);
        if (i < numEntries - 1) {
            printf("|───────────|─────────────────────|─────────────────────|───────────────────\n");
        }
    }
    printf("|───────────────────────────────────────────────────────────────────────────\n");
}

user* recvContact(int sockid, int* num, int userid){
    
    int numEntries;
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    sprintf(buffer, "/recvContact %d", userid);
    send_message(sockid, buffer);

    receive_message(sockid, buffer);
    
    user* data;

    if(strcmp(buffer, "Kayitli Kullanici Yok.") == 0){
        printf("Kayitli Kullanici Yok.\n");
        return NULL;
    }
    else{
        numEntries = atoi(buffer);
        data = (user*)malloc(numEntries * sizeof(user));
        *num = numEntries;
    }

    sprintf(buffer, "/ready");
    send_message(sockid, buffer);
    int i;
    for(i=0; i<numEntries; i++){
        memset(buffer, '\0', BUFFER_SIZE);
        receive_message(sockid, buffer);
        sscanf(buffer, "%[^,], %[^,], %[^,], %d", data[i].name, data[i].surname, data[i].phone, &data[i].userid);
        //printf("%s, %s %d\n", data[i].name, data[i].phone, data[i].userid);
        send_message(sockid, "received");
    }
    
    return data;
}

void addContact(int sockid){
    char buffer[BUFFER_SIZE];

    int numEntries = 0;
    snprintf(buffer, sizeof(buffer), "%s", "/addContact");
    send_message(sockid, buffer);

    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "/ok") != 0);

    user* allContacts = recvContact(sockid, &numEntries, -1);
    
    if(allContacts != NULL){
        displayContact(allContacts, numEntries);
    }
    else{
        printf("\nErisim Hatasi.\n\n");
        fflush(stdout);
    }

    printf("%s\nID of User: ", "Add Contact");
    scanf("%s", buffer);

    send_message(sockid, buffer);

    receive_message(sockid, buffer);
    printf("%s", buffer);

}

void deleteContact(int sockid, int userid){
    char buffer[BUFFER_SIZE];

    int numEntries = 0;
    snprintf(buffer, BUFFER_SIZE, "%s", "/deleteContact");
    send_message(sockid, buffer);

    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "/ok") != 0);

    user* allContacts = recvContact(sockid, &numEntries, userid);
    
    if(allContacts != NULL){
        displayContact(allContacts, numEntries);
    }
    else{
        printf("\nRehberiniz bos.\n\n");
        memset(buffer, '\0', BUFFER_SIZE);
        snprintf(buffer, BUFFER_SIZE, "%s", "/empty");
        send_message(sockid, buffer);
        return;
    }

    printf("%s\nID of User: ", "Delete Contact");
    scanf("%s", buffer);

    send_message(sockid, buffer);

    receive_message(sockid, buffer);
    printf("%s", buffer);

}

void listContacts(int sockid, int userid){
    char buffer[BUFFER_SIZE];

    int numEntries = 0;
    snprintf(buffer, sizeof(buffer), "/listContacts");
    send_message(sockid, buffer);

    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "/ok") != 0);

    user* myContacts = recvContact(sockid, &numEntries, userid);
    
    if(myContacts != NULL){
        displayContact(myContacts, numEntries);
        printf("\nDevam Etmek Icin Bir Tusa Basiniz. ");
        getchar();
        getchar();
    }
    else{
        printf("\nRehberiniz bos.\n\n");
        fflush(stdout);
    }

    send_message(sockid, "/done");
}

void sendMessage(int sockid, int userid){
    char buffer[BUFFER_SIZE];

    int numEntries = 0;
    snprintf(buffer, sizeof(buffer), "/sendMessages");
    send_message(sockid, buffer);

    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "/ok") != 0);

    user* myContacts = recvContact(sockid, &numEntries, userid);
    
    if(myContacts != NULL){
        displayContact(myContacts, numEntries);
        memset(buffer, '\0', BUFFER_SIZE);
        printf("\nLutfen mesajinizi giriniz.\n(Ornek Mesaj: \"3, Cuma gunu musait misin?\")\n>> ");
        scanf(" %[^\n]", buffer);

        // burada da o kişi rehberde değilse atamamamız lazım onun kontrolü önemli.

        send_message(sockid, buffer);
    }
    else{
        printf("\nRehberiniz bos. Lutfen rehberinize kisi ekleyiniz.\n\n");
        send_message(sockid, "/emptyContact");
        return;
    }

    
    receive_message(sockid, buffer);
 
    printf("%s\n", buffer);
}

void checkMessages(int sockid, int userid){
    char buffer[BUFFER_SIZE];

    int numEntries = 0;
    snprintf(buffer, sizeof(buffer), "/checkMessages");
    send_message(sockid, buffer);

    do{
        receive_message(sockid, buffer);
    }while (strcmp(buffer, "/ok") != 0);

    int choice;
    
    send_message(sockid, "/ok");

    receive_message(sockid, buffer);

    if(strcmp(buffer, "/noFile") == 0){
        printf("Henuz mesajiniz yok.\n");
        return;
    }

    printf("Message List\n%s\n\nMesajini goruntulemek istediginiz kisinin ID'sini yaziniz: ", buffer);
    scanf("%d", &choice);
    
    sprintf(buffer, "%d", choice);
    send_message(sockid, buffer);
    send_message(sockid, "/start");

    printf("\nMesajlar:\n");
    memset(buffer, '\0', BUFFER_SIZE);
    int status = 1;
    do{
        receive_message(sockid, buffer);
        if(strcmp(buffer, "/EOF") == 0)
            status = 0;
        else
            printf("%s", buffer);

        send_message(sockid, "/c");
    }while(status);

    printf("\nDevam etmek icin herhangi bir tusa basiniz ");
    getchar();
    getchar();
}

void init_main(user myUser, int sockid){
    //int myNumEntries = 0;
    //user* myContacts = recvContact(sockid, &myNumEntries, myUser.userid);

    while (1){
        char buffer[BUFFER_SIZE];

        int choice;

        display_menu(myUser);
        
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // Display contacts
                    listContacts(sockid, myUser.userid);
                break;
            case 2:
                // add contact
                   addContact(sockid);
                break;
            case 3:
                // Delete contact
                   deleteContact(sockid, myUser.userid);
                break;
            case 4:
                // Check messages
                    checkMessages(sockid, myUser.userid);
                break;
            case 5:
                // Send message
                    sendMessage(sockid, myUser.userid);
                break;
            case 6:
                // Delete message
                break;
            case 0:
                snprintf(buffer, sizeof(buffer), "/exit %d", myUser.userid);
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
    memset(buffer, '\0', BUFFER_SIZE);

    snprintf(buffer, sizeof(buffer), "/login %d", userid);

    // Send the login message to the server
    send_message(sockid, buffer);

    memset(buffer, '\0', BUFFER_SIZE);
    receive_message(sockid, buffer);

    if (strcmp(buffer, "/register") == 0){
        user myData;
        char name[20];
        char surname[20];
        char phone[20];

        printf("Your Name: ");
        scanf(" %[^\n]", myData.name);
        getchar();

        printf("Your Surname: ");
        scanf(" %[^\n]", myData.surname);
        getchar();

        printf("Your Phone: ");
        scanf("%s", myData.phone);
        getchar();
        myData.userid = userid;

        char userString[BUFFER_SIZE];
        snprintf(userString, sizeof(userString), "%s, %s, %s, %d", myData.name, myData.surname, myData.phone, userid);

        // Send the user data to the server
        send_message(sockid, userString);

        init_main(myData, sockid);
    }
    else{
        user myData;
        sscanf(buffer, "%[^,], %[^,], %[^,], %d", myData.name, myData.surname, myData.phone, &myData.userid);
        
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
