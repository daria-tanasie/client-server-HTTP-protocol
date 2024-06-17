#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    char host[] = "34.246.184.49";
    char reg[] = "/api/v1/tema/auth/register";
    char out[] = "GET /api/v1/tema/auth/logout";
    char json_app[] = "application/json";
    char log[] = "/api/v1/tema/auth/login";
    char acc[] = "/api/v1/tema/library/access";
    char books[] = "/api/v1/tema/library/books";
    char cookie[50] = {0}, token[300] = {0}, error[100] = {0};
    char user[30] = {0}, pass[30] = {0}, title[100] = {0}, author[100] = {0},
            genre[100] = {0}, publisher[100] = {0}, page_count[100] = {0};
    int logged_in = 0, in_library = 0;


    while (1) {
        char command[20] = {0};
        fgets(command, 20, stdin);

        if (!strncmp(command,"register", 8)) {

            fprintf(stdout, "username=");
            fgets(user, 20, stdin);
            user[strlen(user) - 1] = '\0';
            fprintf(stdout, "password=");
            fgets(pass, 20, stdin);
            pass[strlen(pass) - 1] = '\0';

            if (strchr(user, ' ')) {
                printf("ERROR! No spaces in username!\n");
                continue;
            }

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *serialized_string = NULL;
            json_object_set_string(object, "username", user);
            json_object_set_string(object, "password", pass);
            serialized_string = json_serialize_to_string_pretty(value);

            message = compute_post_request(host, reg, json_app,
                    &serialized_string, 1, NULL);
            sockfd = open_connection(host, 8080, PF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);   
            response = receive_from_server(sockfd);

            if (strstr(response, "error")) {
                strcpy(error, strstr(response, "error"));
                error[strlen(error) - 2] = '\0';
                strcpy(error, error + 8);
                printf("ERROR! %s\n", error);
            } else {
                printf("SUCCESS! Registered!\n");
            }

            json_free_serialized_string(serialized_string);
            json_value_free(value);
            close_connection(sockfd);

        } else if (!strncmp(command, "login", 5)) {
            fprintf(stdout, "username=");
            fgets(user, 20, stdin);
            user[strlen(user) - 1] = '\0';
            fprintf(stdout, "password=");
            fgets(pass, 20, stdin);
            pass[strlen(pass) - 1] = '\0';

            if (strchr(user, ' ')) {
                printf("ERROR! No spaces in username!\n");
                logged_in = 0;
                continue;
            }

            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            char *serialized_string = NULL;
            json_object_set_string(object, "username", user);
            json_object_set_string(object, "password", pass);
            serialized_string = json_serialize_to_string_pretty(value);

            message = compute_post_request(host, log, json_app,
                    &serialized_string, 1, NULL);
            sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);   
            response = receive_from_server(sockfd);

            if (strstr(response, "error")) {
                strcpy(error, strstr(response, "error"));
                error[strlen(error) - 2] = '\0';
                strcpy(error, error + 8);
                printf("ERROR! %s\n", error);
                logged_in = 0;
            } else {
                strcpy(cookie, strstr(response, "connect.sid"));

                for (int i = 0; i < strlen(cookie); i++) {
                    if (cookie[i] == ';' && cookie[i + 1] != '\0') {
                        cookie[i + 1] = '\0';
                        break;
                    }
                }

                logged_in = 1;
                printf("SUCCESS! Cookie: %s\n", cookie);
            }

            json_free_serialized_string(serialized_string);
            json_value_free(value);
            close_connection(sockfd);
        } else if (!strncmp(command, "enter_library", 13)) {
            if (!logged_in) {
                printf("ERROR! Not logged in!\n");
                continue;
            } else {
                message = compute_get_request(host, acc, NULL, cookie, 1);
                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "error")) {
                    strcpy(error, strstr(response, "error"));
                    error[strlen(error) - 2] = '\0';
                    strcpy(error, error + 8);
                    printf("ERROR! %s\n", error);
                } else {
                    char *cpy = strstr(response, "token");
                    strcpy(token, cpy + 8);
                    token[strlen(token) - 2] =  '\0';
                    printf("SUCCESS! Cookie: %s\n", cookie);
                    in_library = 1;
                }
            }
            close_connection(sockfd);
        } else if (!strncmp(command, "get_books", 9)) {
            if (!in_library) {
                printf("ERROR! Not in library!\n");
                continue;
            } else {
                message = compute_get_request(host, books, token, NULL, 1);
                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);   
                response = receive_from_server(sockfd);

                if (strstr(response, "error")) {
                    strcpy(error, strstr(response, "error"));
                    error[strlen(error) - 2] = '\0';
                    strcpy(error, error + 8);
                    printf("ERROR! %s\n", error);
                } else {
                    char library_books[500];
                    if (strstr(response, "[{")) {
                    strcpy(library_books, strstr(response, "[{"));
                    strcpy(library_books, library_books);
                    library_books[strlen(library_books)] = '\0';
                    printf("%s\n", library_books);
                    }
                }
            }
            close_connection(sockfd);
        } else if (!strncmp(command, "add_book", 8)) {
            if (!in_library) {
                printf("ERROR! No access to library!\n");
                continue;
            } else {
                fprintf(stdout, "title=");
                fgets(title, 100, stdin);
                title[strlen(title) - 1] = '\0';
                fprintf(stdout, "author=");
                fgets(author, 100, stdin);
                author[strlen(author) - 1] = '\0';
                fprintf(stdout, "genre=");
                fgets(genre, 100, stdin);
                genre[strlen(genre) - 1] = '\0';
                fprintf(stdout, "publisher=");
                fgets(publisher, 100, stdin);
                publisher[strlen(publisher) - 1] = '\0';
                fprintf(stdout, "page_count=");
                fgets(page_count, 100, stdin);
                page_count[strlen(page_count) - 1] = '\0';

                if (strlen(title) < 1 || strlen(author) < 1
                    || strlen(genre) < 1 || strlen(publisher) < 1
                    || strlen(page_count) < 1) {
                    printf("ERROR! All fields should be completed!\n");
                    continue;
                }

                if (page_count[0] < '0' || page_count[0] > '9') {
                    printf("ERROR! page_count should be an integer and positive!\n");
                    continue;
                }

                JSON_Value *value = json_value_init_object();
                JSON_Object *object = json_value_get_object(value);
                char *serialized_string = NULL;
                json_object_set_string(object, "title", title);
                json_object_set_string(object, "author", author);
                json_object_set_string(object, "genre", genre);
                json_object_set_string(object, "publisher", publisher);
                json_object_set_string(object, "page_count", page_count);
                serialized_string = json_serialize_to_string_pretty(value);

                message = compute_post_request(host, books, json_app,
                        &serialized_string, 1, token);

                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);   
                response = receive_from_server(sockfd);

                if (strstr(response, "error")) {
                    strcpy(error, strstr(response, "error"));
                    error[strlen(error) - 2] = '\0';
                    strcpy(error, error + 8);
                    printf("ERROR! %s\n", error);
                } else {
                    printf("SUCCESS! Book %s added!\n", title);
                }

                json_free_serialized_string(serialized_string);
                json_value_free(value);
            }
            close_connection(sockfd);
        } else if (!strncmp(command, "delete_book", 11)) {
            if (!in_library) {
                printf("ERROR! Not connected to library!\n");
                continue;
            } else {
                char id[10] = {0};
                char link[20] = {0};

                fprintf(stdout, "id=");
                fgets(id, 10, stdin);
                id[strlen(id)] = '\0';
                strcpy(link, books);
                strcat(link, "/");
                strcat(link, id);
                link[strlen(link)-1] = '\0';

                message = compute_delete_request(host, link, token);
                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);   
                response = receive_from_server(sockfd);

                if (strstr(response, "error")) {
                    strcpy(error, strstr(response, "error"));
                    error[strlen(error) - 2] = '\0';
                    strcpy(error, error + 8);
                    printf("ERROR! %s\n", error);
                } else {
                    printf("SUCCESS! Book deleted!\n");
                }
            }
            close_connection(sockfd);
        } else if (!strncmp(command, "get_book", 8)) {
            if (!in_library) {
                printf("ERROR! Not connected to library!\n");
                continue;
            } else {
                char id[10] = {0};
                char link[50] = {0};

                fprintf(stdout, "id=");
                fgets(id, 10, stdin);
                strcpy(link, books);
                strcat(link, "/");
                strcat(link, id);
                link[strlen(link)-1] = '\0';

                message = compute_get_request(host, link, token, NULL, 1);
                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);   
                response = receive_from_server(sockfd);

                if (strstr(response, "error")) {
                    strcpy(error, strstr(response, "error"));
                    error[strlen(error) - 2] = '\0';
                    strcpy(error, error + 8);
                    printf("ERROR! %s\n", error);
                } else {
                    char library_book[100];
                    if (strstr(response, "{")) {
                        strcpy(library_book, strstr(response, "{"));
                        printf("%s\n", library_book);
                    }
                }
            }

            close_connection(sockfd);
        } else if (!strncmp(command, "logout", 6)) {
            if (!logged_in) {
                printf("ERROR! Not logged in!\n");
                continue;
            } else {
                message = compute_get_request(host, out, NULL, cookie, 1);
                sockfd = open_connection(host, 8080, AF_INET, SOCK_STREAM, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                if (strstr(response, "error")) {
                    strcpy(error, strstr(response, "error"));
                    error[strlen(error) - 2] = '\0';
                    strcpy(error, error + 8);
                    printf("ERROR! %s\n", error);
                } else {
                   memset(token, 0, sizeof(token));
                   memset(cookie, 0, sizeof(cookie));
                   printf("SUCCESS! Logged out!\n");
                   logged_in = 0;
                   in_library = 0;
                }
            }
            close_connection(sockfd);
        } else if (!strncmp(command, "exit", 4)) {
            return 0;
        }
    }

    return 0;
}
