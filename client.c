#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define URL "3.8.116.10"
#define PORT 8080
#define PAYLOAD_TYPE "application/json"

// functie pentru a vedea daca un string
// este format din cifre pozitive sau nu
int isNumber(char s[20])
{
    int n;
    return (sscanf(s, "%d", &n) == 1);
}

int main(int argc, char* argv[])
{
    char* message;
    char* response;
    char command[20];
    char* cookie = malloc(1000 * sizeof(char));
    char* token = malloc(1000 * sizeof(char));
    char* token_copy = malloc(1000 * sizeof(char));
    bool logged = false;
    int sockfd;
    char delimitator_mesage[30] = "========Message========";
    char delimitator_response[30] = "========Response========";
    char delimitator_end[30] = "=====================";
    char temp;
   	bool enter_library_stop = false;

    // deschidem conexiunea
    sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);

    while (1) {

    	// citim comanda de la tastatura
        scanf("%s", command);

        // daca nu e nimeni logat si primim register
        if (logged == false && strcmp(command, "register") == 0) {

        	// creem json-ul ce va fi trimis
            JSON_Value* root_value_register = json_value_init_object();
            JSON_Object* root_object_register = json_value_get_object(root_value_register);
            char* payload_register = NULL;
            char username[40], password[100];
            printf("username=");
            scanf("%c", &temp);
            scanf("%[^\n]", username);
            printf("password=");
            scanf("%s", password);
            json_object_set_string(root_object_register, "username", username);
            json_object_set_string(root_object_register, "password", password);
            payload_register = json_serialize_to_string_pretty(root_value_register);

            // formam mesajul
            message = compute_post_request(URL, "/api/v1/tema/auth/register", PAYLOAD_TYPE,
            	payload_register, 2, NULL, NULL, 0);
            printf("\n%s\n%s", delimitator_mesage, message);

        register1:
        	// trimitem mesajul la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns de la server mai trimitem inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                goto register1;
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);
        }

         // daca nu e nimeni logat si primim login
        if (logged == false && strcmp(command, "login") == 0) {

        	// cream json-ul ce va fi trimis
            JSON_Value* root_value_login = json_value_init_object();
            JSON_Object* root_object_login = json_value_get_object(root_value_login);
            char* payload_login = NULL;
            char username[40], password[100];
            printf("username=");
            scanf("%c", &temp);
            scanf("%[^\n]", username);
            printf("password=");
            scanf("%s", password);
            json_object_set_string(root_object_login, "username", username);
            json_object_set_string(root_object_login, "password", password);
            payload_login = json_serialize_to_string_pretty(root_value_login);

            // formam mesajul
            message = compute_post_request(URL, "/api/v1/tema/auth/login", PAYLOAD_TYPE,
            	payload_login, 2, NULL, NULL, 0);
            printf("\n%s\n%s", delimitator_mesage, message);

        login1:
        	// trimitem mesajul
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns de la server mai trimitem inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                goto login1;
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);

            // retinem cookie-ul pentru a dovedi ca suntem autentificati
            char* pch = strstr(response, "Set-Cookie: ");
            if (pch != NULL) {
                char* pch1 = strstr(response, "; Path=/;");
                strcpy(pch1, "\0");
                strcpy(cookie, pch + 12);
                logged = true;
            }
        }

        // daca suntem logati si vrem acces la biblioteca
        if (enter_library_stop == false && logged == true
        	&& strcmp(command, "enter_library") == 0) {

        	enter_library_stop = true;

        	// formam mesajul
        	// cu cookie-ul care ne demostreaza autentificarea
            message = compute_get_request(URL, "/api/v1/tema/library/access", NULL, cookie,
            	NULL, 1);
            printf("\n%s\n%s", delimitator_mesage, message);

            //trimitem mesajul la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns mai trimitem inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);

            // retinem token-ul pentru a demostra accesul in biblioteca
            char* pch = strstr(response, "token");
            strcpy(token, pch + 8);
            char* pch1 = strstr(token, "}");
            strcpy(pch1 - 1, "\0");
            strcat(token_copy, "Bearer ");
            strcat(token_copy, token);
        }

        // daca suntem logati si vrem informatii despre carti
        if (logged == true && strcmp(command, "get_books") == 0) {
            
            // compunem mesajul cu dovada accesului in biblioteca
            message = compute_get_request(URL, "/api/v1/tema/library/books", NULL, NULL,
            	token_copy, 1);
            printf("\n%s\n%s", delimitator_mesage, message);

            //trimitem mesajul la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns mai trimitem mesajul inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);
        }

        // daca suntem logati si vrem informatii despre o carte
        if (logged == true && strcmp(command, "get_book") == 0) {
            char* book_id = malloc(100 * sizeof(char));

            // ne asiguram ca id-ul este un integer pozitiv
        read_again_book_id:
            printf("id=");
            scanf("%s", book_id);
            if (isNumber(book_id) == 0 || book_id[0] == '-') {
                printf("Please insert a positive integer\n");
                goto read_again_book_id;
            }

            // formam calea finala cu id-ul cartii
            char* path = malloc(200 * sizeof(char));
            strcat(path, "/api/v1/tema/library/books/");
            strcat(path, book_id);

            // formam mesajul cu dovada de acces in biblioteca
            message = compute_get_request(URL, path, NULL, NULL, token_copy, 1);
            printf("\n%s\n%s", delimitator_mesage, message);

            // trimitem mesajul la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns de la server, mai trimitem inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);
            free(book_id);
        	free(path);
        }

        // daca suntem logati si vrem sa adaugam o carte 
        if (logged == true && strcmp(command, "add_book") == 0) {
            
            // formam json-ul cu detaliile despre carte
            JSON_Value* root_value_add = json_value_init_object();
            JSON_Object* root_object_add = json_value_get_object(root_value_add);
            char* payload_add = NULL;
            char title[40], author[40], genre[20], publisher[40];
            char page_count[20];
            printf("title=");
            scanf("%c", &temp);
            scanf("%[^\n]", title);
            printf("author=");
            scanf("%c", &temp);
            scanf("%[^\n]", author);
            printf("genre=");
            scanf("%s", genre);

            // ne asiguram ca primim un integer pozitiv
        read_again_number:
            printf("page_count=");
            scanf("%s", page_count);
            if (isNumber(page_count) == 0 || page_count[0] == '-') {
                printf("Please insert a positive integer\n");
                goto read_again_number;
            }
            printf("publisher=");
            scanf("%c", &temp);
            scanf("%[^\n]", publisher);
            json_object_set_string(root_object_add, "title", title);
            json_object_set_string(root_object_add, "author", author);
            json_object_set_string(root_object_add, "genre", genre);
            json_object_set_string(root_object_add, "page_count", page_count);
            json_object_set_string(root_object_add, "publisher", publisher);
            payload_add = json_serialize_to_string_pretty(root_value_add);

            // formam mesajul cu dovada de acces in biblioteca
            message = compute_post_request(URL, "/api/v1/tema/library/books/", PAYLOAD_TYPE,
            	payload_add, 5, NULL, token_copy, 1);
            printf("\n%s\n%s", delimitator_mesage, message);

            // trimitem mesajul
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns de la server mai trimitem inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);
        }

        // daca suntem logati si vrem sa ne delogam
        if (logged == true && strcmp(command, "logout") == 0) {

        	enter_library_stop = false;

        	// formam mesajul cu dovada de autentificare
            message = compute_get_request(URL, "/api/v1/tema/auth/logout", NULL, cookie, NULL, 1);
            printf("\n%s\n%s", delimitator_mesage, message);

            //trimitem mesajul la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns mai trimitem inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);
            logged = false;

            // resetam stringurile pentru un viitor client
            cookie = calloc(1000, sizeof(char));
            token = calloc(1000, sizeof(char));
            token_copy = calloc(1000, sizeof(char));
        }

        // daca suntem logati si vrem sa stergem o carte
        if (logged == true && strcmp(command, "delete_book") == 0) {
            char* book_id_delete = malloc(100 * sizeof(char));

            // ne asiguram ca id-ul este un integer pozitiv
        read_again_book_id_delete:
            printf("id=");
            scanf("%s", book_id_delete);
            if (isNumber(book_id_delete) == 0 || book_id_delete[0] == '-') {
                printf("Please insert a positive integer\n");
                goto read_again_book_id_delete;
            }
            char* pathh = malloc(200 * sizeof(char));

            // formam url-ul cu id-ul cartii si dovada de
            // access in biblioteca
            strcat(pathh, "/api/v1/tema/library/books/");
            strcat(pathh, book_id_delete);
            message = compute_delete_request(pathh, token_copy);
            printf("\n%s\n%s", delimitator_mesage, message);

            // trimitem mesajul la server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // daca nu primim raspuns de la server mai trimitem inca o data
            if (strlen(response) == 0) {
                sockfd = open_connection(URL, PORT, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
            }
            printf("\n%s\n%s\n%s\n\n", delimitator_response, response, delimitator_end);
            free(book_id_delete);
        	free(pathh);
        }

        // daca primim comanda exit eliberam memoria si incheiem programul
        if (strcmp(command, "exit") == 0) {
        	free(cookie);
        	free(token);
        	free(token_copy);
            close_connection(sockfd);
            break;
        }
    }
    return 0;
}
