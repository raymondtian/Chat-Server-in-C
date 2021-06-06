#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "server.h"

//*****************************HELPER FUNCTIONS*****************************//

// Own implementation to copy identifier into client handler message
void copy_ident1(char client_handler_message[MAX_MSG], char identifier[MAX_IDENT + 1]) {
    for (int i = 0; i < MAX_IDENT; i ++) {
        client_handler_message[i + 2] = identifier[i];
    }
}

// Own implementation to copy client message into client handler message
void copy_message1(char client_handler_message[MAX_MSG], char client_message_converted[MAX_CLIENT_MSG]) {
    for (int i = 0; i < MAX_CLIENT_MSG; i ++) {
        client_handler_message[i + 2 + 256] = client_message_converted[i];
    }
}

// Own implementation to copy identifier into ignore client
void copy_ident2(char ignore[MAX_IDENT + 4], char identifier[MAX_IDENT + 1]) {
    for (int i = 0; i < MAX_IDENT; i ++) {
        ignore[i] = identifier[i];
    }
    strcat(ignore, "_RD");
}

// Own implementation to set up relevant messages and file names for RECEIVE say
void set_up_receive_say(char client_handler_message[MAX_MSG], char identifier[MAX_IDENT + 1], 
char client_message_converted[MAX_MSG], char ignore[MAX_IDENT + 4]) {
     // Setting up client handler's message
    client_handler_message[0] = 3; // type for RECEIVE
    copy_ident1(client_handler_message, identifier);
    copy_message1(client_handler_message, client_message_converted);

    // Setting up the client that we won't relay the RECEIVE message to
    copy_ident2(ignore, identifier);
}

// Own implementation to parse the client message to say
void say(char client_message_converted[MAX_CLIENT_MSG], char client_message[MAX_MSG], int say_count) {
    for (int i = 0; i < MAX_CLIENT_MSG; i ++) {
        client_message_converted[i] = client_message[i + 2];
            if (client_message_converted[i] != 0) {
                say_count ++;
            }
    }
    client_message_converted[say_count + 1] = '\0';
}

// Own implementation to parse the gevent file to connect
void connect(char message[MAX_MSG], char identifier[MAX_IDENT + 1], char domain[MAX_DOMAIN + 256], char temp_domain[MAX_DOMAIN + 256], char ident_wr[MAX_IDENT + 3], 
char ident_rd[MAX_IDENT + 3], char final_directory_wr[MAX_NAME], char final_directory_rd[MAX_NAME],  
int ident_count, int domain_count) {
    for (int i = 0; i < MAX_IDENT; i ++) {
        identifier[i] = message[i + 2];
        if (identifier[i] != 0) {
            ident_count ++;
        }
    }
    identifier[ident_count + 1] = '\0';

    memset(ident_wr, '\0', MAX_IDENT + 3);
    memset(ident_rd, '\0', MAX_IDENT + 3);
    strcpy(ident_wr, identifier);
    strcpy(ident_rd, identifier);
    
    for (int j = 0; j < MAX_DOMAIN; j ++) {
        domain[j] = message[j + 2 + 256];
        if (domain[j] != 0) {
            domain_count ++;
        }
    }
    domain[domain_count + 1] = '\0';
    strcpy(temp_domain, domain);
    strcat(temp_domain, "/"); // comp2017/

    memset(final_directory_wr, '\0', MAX_NAME);
    strcat(final_directory_wr, temp_domain); // comp2017/
    strcat(final_directory_wr, ident_wr); // comp2017/spectre
    strcat(final_directory_wr, "_WR"); // comp2017/spectre_WR

    memset(final_directory_rd, '\0', MAX_NAME);
    strcat(final_directory_rd, temp_domain); // comp2017/
    strcat(final_directory_rd, ident_rd); // comp2017/spectre
    strcat(final_directory_rd, "_RD"); // comp2017/spectre_RD
}

// Own implementation to copy domain into files to write to
void copy_name(char write_files[MAX_MSG], char domain[MAX_DOMAIN + 256], char * name) {
    for (int i = 0; i < MAX_DOMAIN; i ++) {
        write_files[i] = domain[i];
    }
    strcat(write_files, "/");
    strcat(write_files, name);
}

// Own implementation to traverse through domain to find client to relay RECEIVE message to
void domain_search(DIR * dom, struct dirent * entry, int len_ident, char post_fix[MAX_POST_FIX], 
char ignore[MAX_IDENT + 4], char write_files[MAX_NAME + 1], char domain[MAX_DOMAIN + 256], int rd_pipe_fd, char client_handler_message[MAX_MSG]) {
    while ((entry = readdir(dom))) {
        // Setting a variable for the respective file's name
        char * name = entry->d_name;
        len_ident = strlen(name);
        if (len_ident > 3) {
            for (int j = 0; j < 3; j ++) {
                // Getting the post fix - _RD
                // _RD because client handler writes to the RD_pipe
                post_fix[j] = name[j + len_ident - 3]; 
            }
        }
        // If we have found a client that's not the original client
        if (strcmp(post_fix, "_RD") == 0 && strcmp(ignore, name) != 0) {
                copy_name(write_files, domain, name);
                rd_pipe_fd = open(write_files, O_WRONLY);

                // Client handler will relay the message
                write(rd_pipe_fd, client_handler_message, MAX_MSG);
                close(rd_pipe_fd);
        }
    }
}

// Own implementation to parse the client message to saycont
void say_cont(char saycont_message_converted[MAX_CLIENT_MSG - 1], char client_message[MAX_MSG], int say_cont_count, char termination[MAX_TERMINATION]) {
    for (int i = 0; i < MAX_CLIENT_MSG - 1; i ++) {
        saycont_message_converted[i] = client_message[i + 2];
        if (saycont_message_converted[i] != 0) {
            say_cont_count ++;
        }
    }
    termination[0] = client_message[2047];
    saycont_message_converted[say_cont_count + 1] = '\0';
}

// Own implementation to copy client message into client handler message
void copy_message2(char client_handler_message[MAX_MSG], char saycont_message_converted[MAX_CLIENT_MSG - 1]) {
    for (int i = 0; i < MAX_CLIENT_MSG - 1; i ++) {
        client_handler_message[i + 2 + 256] = saycont_message_converted[i];
    }
}

// Own implementation to copy termination into client handler message
void copy_termination(char client_handler_message[MAX_MSG], char termination[MAX_TERMINATION + 1]) {
    client_handler_message[2 + 256 + 1789] = termination[0];
}

// Own implementation to set up relevant messages and file names for RECEIVE for saycont
void set_up_recvcont(char client_handler_message[MAX_MSG], char identifier[MAX_IDENT + 1], 
char saycont_message_converted[MAX_CLIENT_MSG - 1], char termination[MAX_TERMINATION], char ignore[MAX_IDENT + 4]) {
    // Setting up client handler's message
    client_handler_message[0] = 4; // type for RECVCONT
    copy_ident1(client_handler_message, identifier);
    copy_message2(client_handler_message, saycont_message_converted);
    copy_termination(client_handler_message, termination);

    // Setting up the client that we won't relay the RECEIVE message to
    copy_ident2(ignore, identifier);
}

// Function to handle SIGUSR1 signalling death of child process :(
void death(){
    wait(0);
}

//*****************************MAIN FUNCTION*****************************//

int main(int argc, char** argv) {

    //************************GLOBAL PROCESS VARIABLES************************//

    // Global process fifo and fd
    int gevent, gevent_fd;

    // Message in global process
    char message[MAX_MSG];
    // Getting rid of all junk in message
    memset(message, '\0', sizeof(message));
    // Identifier
    char identifier[MAX_IDENT + 1];
    // Getting rid of all junk in message
    memset(identifier, '\0', sizeof(identifier));
    // Domain: max size of domain is 2048 - type - min size of ident
    char domain[MAX_DOMAIN + 256];
    // Getting rid of all junk in message
    memset(domain, '\0', sizeof(domain));
    // Domain to copy strings
    char temp_domain[MAX_DOMAIN + 256];
    // Getting rid of all junk in message
    memset(temp_domain, '\0', sizeof(temp_domain));
    // WR identifier
    char ident_wr[MAX_IDENT + 3];
    // Getting rid of all junk in message
    memset(ident_wr, '\0', sizeof(ident_wr));
    // RD identifier
    char ident_rd[MAX_IDENT + 3];
    // Getting rid of all junk in message
    memset(ident_rd, '\0', sizeof(ident_rd));
    // Final WR directory
    char final_directory_wr[MAX_NAME];
    // Getting rid of all junk in message
    memset(final_directory_wr, '\0', sizeof(final_directory_wr));
    // Final RD directory
    char final_directory_rd[MAX_NAME];
    // Getting rid of all junk in message
    memset(final_directory_rd, '\0', sizeof(final_directory_rd));

    // Type for global process (going to be CONNECT - ie: 0)
    int global_process_type;

    // RD and WR pipes
    int rd_pipe, wr_pipe;
    // RD and WR pipe fd's
    int rd_pipe_fd, wr_pipe_fd;

    // Process id
    int pid;

    // Directory
    DIR * dir1;

    // Count for how long identifier is
    int ident_count = 0;
    // Count for how long domain is
    int domain_count = 0;

    //PID for gevent
    int gprocess_id = getpid();

    //************************CLIENT HANDLER VARIABLES************************//

    // Char array for message that client will send to client handler (msg in WR pipe)
    char client_message[MAX_MSG];
    // Getting rid of all junk in message
    memset(client_message, '\0', sizeof(client_message));
    // Converted char array for message that client will send to client handler
    char client_message_converted[MAX_CLIENT_MSG];
    // Getting rid of all junk in message 
    memset(client_message_converted, '\0', sizeof(client_message_converted));
    // Char array for message that client handler will relay to all other clients
    char client_handler_message[MAX_MSG];
    // Getting rid of all junk in message
    memset(client_handler_message, '\0', sizeof(client_handler_message));
    // Char array for files that will be written to
    char write_files[MAX_MSG + 1];
    // Getting rid of all junk in message
    memset(write_files, '\0', sizeof(write_files));   
    // Char array for converted saycont message
    char saycont_message_converted[MAX_CLIENT_MSG - 1];
    // Getting rid of all junk in message
    memset(saycont_message_converted, '\0', sizeof(saycont_message_converted));   
    // _WR or _RD
    char post_fix[MAX_POST_FIX];
    memset(post_fix, '\0', sizeof(post_fix));
    // Client to ignore (ignore the client that sends the msg)
    char ignore[MAX_IDENT + 4];
    // Getting rid of all junk in message
    memset(ignore, '\0', sizeof(ignore));
    // Termination
    char termination[MAX_TERMINATION + 1];
    memset(termination, '\0', sizeof(termination));

    // Type for clients (1-7)
    int client_type;

    // Count for how long say message is
    int say_count = 0;
    // Length of identifier
    int len_ident = 0;
    // Count for saycont message
    int say_cont_count = 0;

    //************************MAIN CODE************************//

    // Creating global event pipe
    gevent = mkfifo("gevent", 0755);
    if (gevent == -1 && errno != EEXIST) {
        fprintf(stderr, "Could not create global pipe, error code is %d\n", errno);
    }

    // Global Process
    while (1) {
        // Setting up

        // Opening gevent pipe
        gevent_fd = open("gevent", O_RDONLY);
        if (gevent_fd == -1) {
            fprintf(stderr, "Could not open pipe, error code is %d\n", errno);
        }

        // Reading fifo file and transferring its contents into char array message
        if (read(gevent_fd, &message, 2048) == -1) {
            fprintf(stderr, "Could not read from server, error code is %d\n", errno);
        }

        // Retrieving type - CONNECT
        // Message breakdown
        // 00 (2 bytes) 000000000000... (256 bytes) 000000000000... (1790 bytes), total 2048 bytes
        // Type will be the 0th index
        global_process_type = (int) message[0];

        // Reading time

        if (global_process_type == 0) { // We have read CONNECT
            // Parsing gevent file to find connect message, identifier and domain
            connect(message, identifier, domain, temp_domain, ident_wr, 
            ident_rd, final_directory_wr, final_directory_rd, 
            ident_count, domain_count);
            
            dir1 = opendir(domain);

            // Checking if domain exists
            if (dir1 == NULL) {
                mkdir(domain, 0777);
            }

            // Creating WR pipe
            wr_pipe = mkfifo(final_directory_wr, S_IRWXU | S_IRWXG);
            if (wr_pipe == -1 && errno != EEXIST) {
                fprintf(stderr, "Could not create WR pipe, error code is %d\n", errno);
            }

            wr_pipe_fd = open(final_directory_wr, O_RDWR);
            if (wr_pipe_fd == -1) {
                fprintf(stderr, "Could not open WR pipe, error code is %d\n", errno);
            }
            close(wr_pipe_fd);

            // Creating RD pipe
            rd_pipe = mkfifo(final_directory_rd, S_IRWXU | S_IRWXG);
            if (rd_pipe == -1 && errno != EEXIST) {
                fprintf(stderr, "Could not create RD pipe, error code is %d\n", errno);
            }

            rd_pipe_fd = open(final_directory_rd, O_RDWR);
            if (rd_pipe_fd == -1) {
                fprintf(stderr, "Could not open RD pipe, error code is %d\n", errno);
            }
            close(rd_pipe_fd);

            closedir(dir1);

            pid = fork();
            if (pid == 0) { // Child process
                close(gevent_fd);
                break;
            }
            else { // Parent process
                signal(SIGUSR1, death);
            }
        }
        close(gevent_fd);
    }

    // Client handler
    while (1) {
        // Setting up

        wr_pipe_fd = open(final_directory_wr, O_RDONLY);
        if (wr_pipe_fd == -1) {
            fprintf(stderr, "Could not open WR pipe, error code is %d\n", errno);
        }

        if (read(wr_pipe_fd, &client_message, 2048) == -1) {
            fprintf(stderr, "Could not read from WR pipe, error code is %d\n", errno);
        }

        client_type = (int) client_message[0];

        // Reading time!!

        if (client_type == 1) { // We have read SAY

            // Parsing client's message to retrieve 'message'
            say(client_message_converted, client_message, say_count);

            // Setting up client handler's message and client that we won't relay RECEIVE message to
            set_up_receive_say(client_handler_message, identifier, client_message_converted, ignore);

            DIR * dom;
            struct dirent * entry = 0;
            dom = opendir(domain);

            if (dom == NULL) {
                fprintf(stderr, "Domain %s doesn't exist, error code is %d\n", domain, errno);
            }

            // Traversing through domain to find clients that
            // the client handler will relay the RECEIVE message to
            domain_search(dom, entry, len_ident, post_fix, ignore, write_files, 
            domain, rd_pipe_fd, client_handler_message);
        }

        else if (client_type == 7) { // We have read DISCONNECT
            // Deleting WR & RD pipe filenames from the filesystem
            unlink(final_directory_rd);
            unlink(final_directory_wr);

            // Closing WR & RD pipe file descriptors
            close(wr_pipe_fd);
            close(rd_pipe_fd);
            
            // Removing file
            if (remove(final_directory_rd) > 0) {
                fprintf(stderr, "Removing %s was unsuccessful, error code is %d\n", final_directory_rd, errno);
            }

            // Removing file
            if (remove(final_directory_wr) > 0) {
                fprintf(stderr, "Removing %s was unsuccessful, error code is %d\n", final_directory_wr, errno);
            }

            kill(gprocess_id, SIGUSR1);
            break;
        }

        else if (client_type == 2) { // We have read SAYCONT

            // Parsing client's message to retrieve 'message'
            say_cont(saycont_message_converted, client_message, say_cont_count, termination);

            // Setting up client handler's message and client that we won't relay RECVCONT message to
            set_up_recvcont(client_handler_message, identifier, 
            saycont_message_converted, termination, ignore);

            DIR * dom;
            struct dirent * entry = 0;
            dom = opendir(domain);

            if (dom == NULL) {
                fprintf(stderr, "Domain %s doesn't exist, error code is %d\n", domain, errno);
            }

            // Traversing through domain to find clients that
            // the client handler will relay the RECVCONT message to
            domain_search(dom, entry, len_ident, post_fix, ignore, write_files, 
            domain, rd_pipe_fd, client_handler_message);
        }
        close(wr_pipe_fd);
    }
}