#ifndef _REQUESTS_
#define _REQUESTS_

// functie ce returneaza un DELETE request
char* compute_delete_request(char* url, char* token);

// functie ce returneaza un GET request
char* compute_get_request(char* host, char* url, char* query_params,
    char* cookies, char* token, int cookies_count);

// functie ce returneaza un POST request
char* compute_post_request(char* host, char* url, char* content_type, char* body_data,
    int body_data_fields_count, char* cookies, char* token, int cookies_count);

#endif
