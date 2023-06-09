#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 20
#define MAX_ROOMS   5
#define BUFFER_SIZE 1024

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

// Structure to hold client information

struct client_t{
    int socket;
    struct sockaddr_in address;
    char* username;
    int logged;
    char* createdRooms;
    char* whereami;
    pthread_t thread;
};

char** rooms;
int logoutFlag = 0;
struct client_t* clients; // Array of clients
pthread_mutex_t clients_mutex; // Mutex to protect clients array

void login(char* username,const int* client_index){
    if(clients[*client_index].logged == 0){
        char response[BUFFER_SIZE];
        clients[*client_index].username = username;
        clients[*client_index].logged   = 1;

        sprintf(response,COLOR_GREEN "Sisteme %s olarak başarıyla giriş yaptınız.\n" COLOR_RESET,username);

        write(clients[*client_index].socket,response, strlen(response));
    } else {
        char response[BUFFER_SIZE];

        sprintf(response,COLOR_YELLOW "Sisteme %s olarak kayıtlısınız.\n" COLOR_RESET,clients[*client_index].username);
        write(clients[*client_index].socket,response, strlen(response));
    }
}

void logout(){
   logoutFlag = 1;
}

void whereami(const int* client_index){
    char response[BUFFER_SIZE];
    sprintf(response,COLOR_GREEN "Şuanda %s adlı yerdesiniz.\n" COLOR_RESET, clients[*client_index].whereami);

    write(clients[*client_index].socket,response, strlen(response));
}


void list_rooms(const int* client_index) {
    char response[BUFFER_SIZE] = "Mevcut oda listesi:\n";
    char temp_str[BUFFER_SIZE];

    for (int i = 0; i < MAX_ROOMS; i++) {
        if (rooms[i] != NULL) {
            snprintf(temp_str, BUFFER_SIZE, "Room: %s\n", rooms[i]);
            strcat(response, temp_str);
        }
    }

    write(clients[*client_index].socket, response, strlen(response));
}

void open_rooms(const int* client_index,char* room_name){

    char response[BUFFER_SIZE];
    char* warning_msg = "Maksimum oda sayısına ulaşıldığından dolayı yeni oda açılmasına izin verilmiyor.\n";


    int room_counter = 0;
    for (int i = 0; i < MAX_ROOMS; ++i) {
        if(rooms[i] != NULL){
            room_counter++;
        }
    }

    if(room_counter == MAX_ROOMS){
            write(clients[*client_index].socket,warning_msg, strlen(warning_msg));
    }
    else {
        rooms[room_counter] = room_name;
        clients[*client_index].createdRooms = room_name;
        clients[*client_index].whereami = room_name;
        sprintf(response,COLOR_GREEN "%s adlı oda başarıyla oluşturuldu.\n" COLOR_RESET,room_name);
        write(clients[*client_index].socket,response, strlen(response));
    }
}

void list_users(const int* client_index){
    char response[BUFFER_SIZE] = "Mevcut odadaki kullanıcı listesi:\n";
    char *room_name = clients[*client_index].whereami;
    strcat(room_name,"\0");
    char temp_str[BUFFER_SIZE];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i].username != NULL){
            if(strcmp(clients[i].whereami,room_name) == 0){
                snprintf(temp_str, sizeof(temp_str), "User: %s\n", clients[i].username);
                strcat(response, temp_str);
            }
        }
    }
    write(clients[*client_index].socket, response, strlen(response));
}

void close_room(const int* client_index,char* room_name){

    char* warning_msg = "Oluşturmadığınız odayı kapatamazsınız.\n";
    char  temp_str[BUFFER_SIZE];
    printf("oda adı : %s\n",room_name);

    if(clients[*client_index].createdRooms == NULL) {
        write(clients[*client_index].socket,warning_msg, strlen(warning_msg));
    }
    else if(strcmp(clients[*client_index].createdRooms,room_name) == 0){


        for (int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i].socket != -1 && strcmp(clients[i].whereami,room_name) == 0){
                clients[i].whereami = "lobby";
                char* message = "Mevcut oda kurucu tarafından kapatılmıştır lobiye aktarılıyorsunuz.\n";
                write(clients[i].socket,message, strlen(message));
            }
        }

        for (int i = 0; i < MAX_ROOMS; i++) {
            if(rooms[i] != NULL && strcmp(rooms[i],room_name) == 0){
                rooms[i] = NULL;
                clients[*client_index].whereami = "lobby";
                clients[*client_index].createdRooms = NULL;
            }
        }

        snprintf(temp_str, sizeof(temp_str), COLOR_GREEN "%s adlı oda kapatıldı.\n" COLOR_RESET, room_name);
        write(clients[*client_index].socket,temp_str,strlen(temp_str));

    } else {
        write(clients[*client_index].socket,warning_msg, strlen(warning_msg));
    }
}

void enter_room(const int* client_index,char* room_name){
    char  temp_str[BUFFER_SIZE];
    for (int i = 0; i < MAX_ROOMS; ++i) {
        if(rooms[i] != NULL){
            if(strcmp(rooms[i],room_name) == 0){
                clients[*client_index].whereami = room_name;

                snprintf(temp_str, sizeof(temp_str), COLOR_GREEN "%s adlı odaya geçiş yapıldı.\n" COLOR_RESET, room_name);
                write(clients[*client_index].socket,temp_str, strlen(temp_str));
            }
        }
    }
}

int classify(char* token,char *data,int client_index){

    if(strcmp(token,"login\0") == 0){
        login(data,&client_index);
        return 1;
    } else if(strcmp(token,"logout\0") == 0){
        logout();
        return 1;
    } else if(strcmp(token,"list_rooms\0") == 0){
        list_rooms(&client_index);
        return 1;
    } else if(strcmp(token,"list_users\0") == 0){
        list_users(&client_index);
        return 1;
    } else if(strcmp(token,"enter\0") == 0){
        enter_room(&client_index,data);
        return 1;
    } else if(strcmp(token,"whereami\0") == 0){
        whereami(&client_index);
        return 1;
    } else if(strcmp(token,"open\0") == 0){
        open_rooms(&client_index,data);
        return 1;
    } else if(strcmp(token,"close\0") == 0){
        close_room(&client_index,data);
        return 1;
    } else {
        return 0;
    }

}


void *handle_client(void *arg) {
    int client_index = *((int *) arg);
    int client_socket = clients[client_index].socket;
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int result;

    while (1) {

        if(logoutFlag == 1){
            break;
        }

        bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read > 0) {
            printf("Received from client %d: %s", client_index, buffer);


                char* token = NULL;

                char* buffer_copy = strdup(buffer); // Make a copy of the buffer
                char* buffer_copy_2 = strdup(buffer);

                buffer_copy[strcspn(buffer_copy, "\n")] = '\0';
                buffer_copy_2[strcspn(buffer_copy_2, "\n")] = '\0';

                token = strtok(buffer_copy, " "); // Tokenize the copy

                char *data = NULL;
                char *afterwhitespace = NULL;

                afterwhitespace = strchr(buffer_copy_2, ' '); // Daha önce tanımlanan 'afterwhitespace' işaretçisini güncelliyoruz

                if (afterwhitespace != NULL) {
                    data = strdup(afterwhitespace + 1); // Bellek tahsisi yapıyoruz
                    data[strcspn(data,"\n")] = '\0';
                }

                if (token != NULL) {
                    result = classify(token, data,client_index);
                }

            // Broadcast the received message to all other clients which is in same room
            char temp_str[BUFFER_SIZE];
            pthread_mutex_lock(&clients_mutex);
            if(result == 0) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].whereami != NULL) {
                        if (i != client_index && strcmp(clients[i].whereami, clients[client_index].whereami) == 0) {

                            snprintf(temp_str, sizeof(temp_str), COLOR_BLUE "%s: " COLOR_RESET,
                                     clients[client_index].username);
                            strcat(temp_str, buffer);
                            write(clients[i].socket, temp_str, strlen(temp_str));
                        }
                    }
                }
            }
            pthread_mutex_unlock(&clients_mutex);

            bzero(temp_str,BUFFER_SIZE);
            bzero(buffer, BUFFER_SIZE);
            free(buffer_copy_2);
            free(buffer_copy); // Free the allocated copy
        }
    }

    // Client disconnected
    printf("Client %d disconnected\n", client_index);

    // Clean up client resources
    close(client_socket);

    pthread_mutex_lock(&clients_mutex);

    clients[client_index].socket = -1;
    clients[client_index].logged = 0;
    clients[client_index].username = NULL;
    clients[client_index].whereami = NULL;
    clients[client_index].createdRooms = NULL;

    pthread_mutex_unlock(&clients_mutex);
    logoutFlag = 0;
    return NULL;
}



int main() {

    clients = (struct client_t*) malloc(MAX_CLIENTS * sizeof(struct client_t));
    rooms   = (char**) malloc(MAX_ROOMS * sizeof(char*));

    rooms[0] = "lobby\0";

    for (int i = 1; i < MAX_ROOMS; i++) {
        rooms[i] = NULL;
    }

    int server_socket;
    int client_index = 0;
    struct sockaddr_in server_address;

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;
        clients[i].whereami = NULL;
        clients[i].logged = 0;
        clients[i].username = NULL;
        clients[i].createdRooms = NULL;
    }

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create server socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(12345);

    // Bind server socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("Failed to bind server socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Failed to listen for connections");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Listening for connections...\n");

    // Initialize the clients mutex
    if (pthread_mutex_init(&clients_mutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int client_socket;
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        // Accept a new connection
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_length);
        if (client_socket == -1) {
            perror("Failed to accept connection");
            exit(EXIT_FAILURE);
        }

        printf("New connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Find a free slot in the clients array
        pthread_mutex_lock(&clients_mutex);
        while (clients[client_index].socket != -1) {
            client_index = (client_index + 1) % MAX_CLIENTS;
        }

        // Initialize the client structure
        clients[client_index].socket   = client_socket;
        clients[client_index].address  = client_address;
        clients[client_index].username = "guest";
        clients[client_index].logged   = 0;
        clients[client_index].whereami = "lobby";
        clients[client_index].address  = client_address;

        // Create a new thread to handle the client
        if (pthread_create(&clients[client_index].thread, NULL, handle_client, &client_index) != 0) {
            perror("Failed to create client thread");
            exit(EXIT_FAILURE);
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    // Close the server socket
    close(server_socket);

    // Clean up the clients mutex
    pthread_mutex_destroy(&clients_mutex);

    return 0;
}
