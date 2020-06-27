#include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char* compute_delete_request(char* url, char* token)
{
    char* message = calloc(BUFLEN, sizeof(char));
    char* line = calloc(LINELEN, sizeof(char));

    // adaugam numele metodei
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    // adaugam autorizatia de acces in biblioteca
    sprintf(line, "Authorization: %s", token);
    compute_message(message, line);
    
    compute_message(message, "");
    return message;
}

char* compute_get_request(char* host, char* url, char* query_params,
    char* cookies, char* token, int cookies_count)
{
    char* message = calloc(BUFLEN, sizeof(char));
    char* line = calloc(LINELEN, sizeof(char));

    // scriem numele metodei, URL-ul, request params(daca exista)
    // si tipul protocolului
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    }
    else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // adaugam hostu-ul
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // adaugam cookies daca avem
    if (cookies != NULL) {
        sprintf(line, "Cookie: %s;", cookies);
        compute_message(message, line);
    }

    // adaugam token daca avem
    if (token != NULL) {
        sprintf(line, "Authorization: %s", token);
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}

char* compute_post_request(char* host, char* url, char* content_type, char* body_data,
    int body_data_fields_count, char* cookies, char* token, int cookies_count)
{
    char* message = calloc(BUFLEN, sizeof(char));
    char* line = calloc(LINELEN, sizeof(char));
    char* body_data_buffer = calloc(LINELEN, sizeof(char));

    // scriem numele metodei, URL-ul, protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // adaugam host-ul
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // adaugam headere pt Content-Type, Content Length
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(body_data));
    compute_message(message, line);

    // adaugam cookies daca avem
    if (cookies != NULL) {
        sprintf(line, "Cookie: %s", cookies);
        compute_message(message, line);
    }

    // adaugam token daca avem
    if (token != NULL) {
        sprintf(line, "Authorization: %s", token);
        compute_message(message, line);
    }

    compute_message(message, "");
    memcpy(body_data_buffer, body_data, strlen(body_data));

    // adaugam payload data ce va fi trimis
    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}
