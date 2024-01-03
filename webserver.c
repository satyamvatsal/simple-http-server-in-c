#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILE_SIZE 8000
#define BACKLOG 16
#define BUFFER_SIZE 2048

// defining some functions
const char* getFileExtension(const char* filename);
void send_file(int client_fd,const char *filename);
void add_css_data(char css_resp[]);
void add_html_data(char http_header[]);

int main(int argc,char *argv[]) {

     if (argc < 2) {
        fprintf(stderr, "Usage: %s <PORT>\n", argv[0]);
        return 1; 
    }
    const short int PORT = atoi(argv[1]);

    char buffer[BUFFER_SIZE];

    char resp[FILE_SIZE] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n";
    char css_resp[FILE_SIZE];

    add_css_data(css_resp);
    add_html_data(resp);

    // create a socket
    int sock_fd = socket(AF_INET, SOCK_STREAM,0 );
    if (sock_fd == -1) {
        perror("webserver (socket)");
        return 1;
    }

    //creating an address to bind the socket to 
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //create a client address struct
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    //bind the socket address
    if( bind(sock_fd, (struct sockaddr *)&host_addr , host_addrlen) != 0){
        perror("webserver (bind)");
        return 1;
    }

    // listen for the incoming requests
    if(listen(sock_fd,BACKLOG) != 0 ) {
        perror("webserver (listen)");
        return 1;
    }
    printf("server is listening on port %hd\n",PORT);

    while(1){

        // accept the incoming connections
        int new_sock_fd = accept(sock_fd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (new_sock_fd < 0){
            perror("webserver (accept)");
            return 1;
        }

        //get client address
        int sockn = getpeername(new_sock_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("webserver (get client address)");
            return 1;
        }
        printf("%s client connected \n",inet_ntoa(client_addr.sin_addr));
        
        // read from the socket
        int valread = read(new_sock_fd,buffer,BUFFER_SIZE);
        if (valread < 0) {
            perror("webserver (read)");
            continue;
        }
        //read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s",method,uri,version);
        printf("[%s:%u] %s %s %s\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),
                                    method,uri,version);
        
        // seding data according to GET request

        if (strcmp(uri,"/") == 0) {
            int valwrite = write(new_sock_fd,resp,strlen(resp));
            if (valwrite < 0) {
                perror("webserver (write html)");
                continue;
            }
        }
        else if (strcmp(uri,"/styles.css") == 0) {
            int valwrite = write(new_sock_fd,css_resp,strlen(css_resp));
            if (valwrite < 0) {
                perror("webserver (write css)");
                continue;
            }
        }
        else if (strcmp(uri,"/script.js") == 0) {
            send_file(new_sock_fd,"script.js");
        }
        else {
            char filename[100] = "assets/";
            if (strcat(filename, uri) < 0) {
                printf("file name is too large\n");
            }
            send_file(new_sock_fd,filename);
        }
        close(new_sock_fd);
    }
    return 0;
}
const char* getFileExtension(const char* filename) {
    const char* dot = strrchr(filename, '.');

    if (dot != NULL && dot != filename) {
        
        return dot + 1;
    } else {
        return "";
    }
}
void send_file(int client_fd,const char *filename) {
    char buffer[BUFFER_SIZE];
    const char *extension = getFileExtension(filename);
    if (strcmp(extension,"pdf") == 0) {
        char http_header[] ="HTTP/1.0 200 OK\r\n"
                            "Content-Disposition: inline; filename=filename\r\n"
                            "Content-type: application/pdf\r\n";
        strcat(buffer,http_header);
    }
    FILE *file = fopen(filename,"r");

    if (file == NULL) {
        perror("websever (read file)");
        return;
    }
    
    size_t bytes_read;
    while((bytes_read = fread(buffer,1,sizeof(buffer),file) ) >0 ){
        if(write(client_fd,buffer,bytes_read) < 0) {
            perror("webserver (write file)");
            return;
        }
    }
    fclose(file);
}
void add_css_data(char css_resp[]) {
    FILE *css_data = fopen("style.css","r");
    char line[100];
    char response_data[FILE_SIZE];
    response_data[0] = '\0';
    while (fgets(line,100,css_data) != NULL)
    {
        strcat(response_data,line);
    }
    strcat(css_resp,response_data);
}

void add_html_data(char http_header[]){
    FILE *html_data = fopen("index.html", "r");
    char line[100];
    char response_data[FILE_SIZE];
    response_data[0] = '\0';
    while (fgets(line,100,html_data) != NULL) {
            strcat(response_data, line);
    }
    strcat(http_header,response_data);
}