#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

int connectServer(int port)
{
    int fd;
    struct sockaddr_in server_address;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("Error in connecting to server\n");
    }

    return fd;
}

void sendRequest(int fd, char *request)
{
    send(fd, request, strlen(request), 0);
}

void write_request_to_client_file(int req, int fd)
{
    int received_k;
    recv(fd, &received_k, sizeof(int), 0);
    int i = 1;
    char buff[1024];

    FILE *file = fopen("received_request.txt", "r");
    while (fgets(buff, sizeof(buff), file) != NULL)
    {

        if (i == req)
        {
            char client_filename[20];
            sprintf(client_filename, "request_%d.txt", received_k);
            // printf("\n\n*******************************************************************\n\n");
            FILE *file = fopen(client_filename, "a");
            if (file == NULL)
            {
                printf("Error opening the file.\n");
                exit(1);
            }
            fprintf(file, "%s\n", buff);
            fclose(file);
        }
        i++;
    }
    fclose(file);

    printf("\nYour request written to client file.\n");
}

void handel_received_request(int fd)
{
    int req;
    printf("Chose the request you want to process: ");
    scanf("%d", &req);
    write_request_to_client_file(req, fd);
}

void receiveRequest(int fd)
{
    char file_data[1024];
    int i = 1;
    printf("\nReceived request from server: \n");

    char buff[1024];
    FILE *file = fopen("received_request.txt", "r");
    if (file == NULL)
    {
        printf("Error opening the file.\n");
        exit(1);
    }
    while (fgets(buff, sizeof(buff), file) != NULL)
    {
        printf("%d. %s", i, buff);
        i++;
    }
    fclose(file);

    handel_received_request(fd);
}

int select_option(int option)
{
    printf("\n1. Send request \n");
    printf("2. Receive request from server\n");
    printf("3. Exit\n");
    printf("\nPlease select an option: ");
    scanf("%d", &option);
    return option;
}

void handel_option(int option, int fd)
{
    char buff[1024] = {0};

    if (option == 1)
    {
        printf("Enter your request: ");
        scanf("%s", buff);
        sendRequest(fd, buff);
    }

    else if (option == 2)
    {
        receiveRequest(fd);
    }
    else if (option == 3)
    {
        printf("Exiting...\n");
        close(fd);
        exit(0);
    }
    else
    {
        printf("Invalid option!\n");
        select_option(option);
    }
}

void show_req_controller()
{
    int i = 1;
    printf("\nAvailable requests: \n");

    char buff[1024];
    FILE *file = fopen("received_request.txt", "r");
    if (file == NULL)
    {
        printf("Error opening the file.\n");
        exit(1);
    }
    while (fgets(buff, sizeof(buff), file) != NULL)
    {
        printf("%d. %s", i, buff);
        i++;
    }
    fclose(file);
}

void write_request_to_controller_file(int req, int k)
{
    int i = 1;
    char buff[1024];

    FILE *input_file = fopen("received_request.txt", "r");
    while (fgets(buff, sizeof(buff), input_file) != NULL)
    {
        if (i == req)
        {
            char cont_filename[20];
            sprintf(cont_filename, "Controller_%d.txt", k);
            FILE *output_file = fopen(cont_filename, "a");
            if (output_file == NULL)
            {
                printf("Error opening the output file.\n");
                exit(1);
            }
            fprintf(output_file, "%s\n", buff);
            fclose(output_file);
        }
        i++;
    }

    fclose(input_file);

    printf("\nYour request written to controller file.\n");
}

void handel_req_controller(int fd)
{
    int req;
    int received_k;
    recv(fd, &received_k, sizeof(int), 0);
    printf("\nChose the request you want to process: ");
    scanf("%d", &req);
    write_request_to_controller_file(req, received_k);
}

int starting(int option)
{
    printf("\n1. Receive existing requests\n2. Exit\nPlease select an option: ");
    scanf("%d", &option);
    return option;
}

void select_type(int fd)
{
    int type;
    int option;
    int option2;
    printf("1. Client\n2. Spector\nPlease select your type: ");
    scanf("%d", &type);
    if (type == 1)
    {
        while (1)
        {
            option = select_option(option);
            handel_option(option, fd);
        }
    }
    else if (type == 2)
    {
        while (1)
        {
            option2 = starting(option2);
            if (option2 == 1)
            {
                show_req_controller();
                handel_req_controller(fd);
            }
            else if (option2 == 2)
            {
                printf("Exiting...\n");
                close(fd);
                break;
            }
            else
            {
                printf("Invalid option!\n");
                starting(option2);
            }
        }
    }
    else
    {
        printf("Invalid type!\n");
        select_type(fd);
    }
}

int main(int argc, char const *argv[])
{
    int fd;
    char buff[1024] = {0};
    fd = connectServer(8080);
    select_type(fd);
    return 0;
}