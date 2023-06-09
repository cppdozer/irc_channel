#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

void displapScreen(){
    system("clear");
    printf( COLOR_BLUE " __      _____  __ _ _ __   ___  _ __  ___ \n"
           " \\ \\ /\\ / / _ \\/ _` | '_ \\ / _ \\| '_ \\/ __|\n"
           "  \\ V  V /  __/ (_| | |_) | (_) | | | \\__ \\\n"
           "   \\_/\\_/ \\___|\\__,_| .__/ \\___/|_| |_|___/\n"
           "                    |_|                    \n\n\n" COLOR_RESET);
}

void showCommands(){
    printf("logout            : sunucudan ayrılmak için kullanılır.\n"
           "open <room_name>  : yeni bir sohbet odası oluşturur.\n"
           "enter <room_name> : odaya bağlanmak için kullanılır.\n"
           "list rooms        : sunucudaki odaları listelemek için kullanılır.\n"
           "whereami          : kullanıcının içinde bulunduğu sohbet odasını yazdırır.\n"
           "login <nickname>  : belirtilen kullanıcı adı ile sunucuya bağlanılır.\n"
           "list users        : geçerli odadaki kullanıcıları listelemek için kullanılır.\n"
           "close <room_name> : belirtilen odayı kapatır ancak sadece odayı oluşturan kullanabilir.\n\n\n"
           "******************* ŞUANDA LOBİDESİN *******************\n"
           "status : guest\n\n"
           );
}

void str_overwrite_stdout() {
    printf("%s", "> ");
    fflush(stdout);
}

void createSocket(int* server_socket){
    *server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(*server_socket == -1){
        printf(COLOR_RED "\n[ERR] :Failed to create socket.\n" COLOR_RESET);
        exit(EXIT_FAILURE);
    } else {
        printf(COLOR_GREEN "\n[OK]: Socket created.\n" COLOR_RESET);
    }
}


void configureServerAddress(struct sockaddr_in* server_address){
    server_address->sin_family = AF_INET;
    server_address->sin_addr.s_addr = inet_addr(SERVER_IP);
    server_address->sin_port = htons(SERVER_PORT);
    printf(COLOR_GREEN "[OK]: Configuration have been done.\n" COLOR_RESET);
}

void connectServer(const int* server_socket,const struct sockaddr_in* server_address){
    if(connect(*server_socket,(struct sockaddr*)server_address, sizeof(*server_address)) == -1){
        printf(COLOR_RED "[ERR] :Failed to connect to server.\n");
        exit(EXIT_FAILURE);
    } else {
        printf(COLOR_GREEN "[OK]: Successfully connected to the server.\n" COLOR_RESET);
    }
}

void* send_message(void* arg) {
    char message[BUFFER_SIZE];
    int *sock = arg;
    while (1) {
        str_overwrite_stdout();
        fgets(message, BUFFER_SIZE, stdin);
        if (send(*sock, message, strlen(message), 0) < 0) {
            perror("Mesaj gönderilemedi");
            exit(1);
        }
    }
}



void* receive_message(void* arg) {
    char message[BUFFER_SIZE];
    int message_len;
    int *sock = arg;
    while (1) {
        message_len = recv(*sock, message, BUFFER_SIZE, 0);

        if (message_len <= 0) {
            perror(COLOR_RED "Çıkış yapılıyor..." COLOR_RESET);
            exit(1);
        } else if( message_len > 0){
            printf("%s",message);
            str_overwrite_stdout();
        } else {
            break;
        }
        memset(message, 0, sizeof(message));
    }

    return NULL;
}


int main(int args,char *argv[]){

    int server_socket;
    struct sockaddr_in server_address;
    pthread_t input_thread;
    pthread_t receive_thread;

    displapScreen();
    showCommands();

    createSocket(&server_socket);
    configureServerAddress(&server_address);
    connectServer(&server_socket,&server_address);

    printf("\n");

    pthread_create(&input_thread, NULL, send_message, &server_socket);
    pthread_create(&receive_thread, NULL, receive_message, &server_socket);

    pthread_join(input_thread,NULL);
    pthread_join(receive_thread,NULL);

    return 0;
}
