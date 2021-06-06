#ifndef SERVER_H
#define SERVER_H

#define MAX_MSG 2048
#define MAX_NAME 2048
#define MAX_CLIENT_MSG 1790
#define MAX_DOMAIN 1790
#define MAX_IDENT 256
#define MAX_TERMINATION 1
#define MAX_POST_FIX 4

void copy_ident1(char client_handler_message[MAX_MSG], char identifier[MAX_IDENT + 1]);

void copy_ident2(char ignore[MAX_IDENT + 4], char identifier[MAX_IDENT + 1]);

void copy_message1(char client_handler_message[MAX_MSG], char client_message_converted[MAX_CLIENT_MSG]);

void copy_name(char write_files[MAX_MSG], char domain[MAX_DOMAIN + 256], char * name);

void connect(char message[MAX_MSG], char identifier[MAX_IDENT + 1], char domain[MAX_DOMAIN + 256], char temp_domain[MAX_DOMAIN + 256], char ident_wr[MAX_IDENT + 3], 
char ident_rd[MAX_IDENT + 3], char final_directory_wr[MAX_NAME], char final_directory_rd[MAX_NAME],  
int ident_count, int domain_count);

void say(char client_message_converted[MAX_MSG], char client_message[MAX_MSG], int say_count);

void set_up_receive_say(char client_handler_message[MAX_MSG], char identifier[MAX_IDENT + 1], 
char client_message_converted[MAX_MSG], char ignore[MAX_IDENT + 4]);

void domain_search(DIR * dom, struct dirent * entry, int len_ident, char post_fix[MAX_POST_FIX], 
char ignore[MAX_IDENT + 4], char write_files[MAX_NAME + 1], char domain[MAX_DOMAIN + 256], int rd_pipe_fd, char client_handler_message[MAX_MSG]);

void say_cont(char saycont_message_converted[MAX_CLIENT_MSG - 1], char client_message[MAX_MSG], int say_cont_count, char termination[MAX_TERMINATION]);

void copy_message2(char client_handler_message[MAX_MSG], char saycont_message_converted[MAX_CLIENT_MSG - 1]);

void copy_termination(char client_handler_message[MAX_MSG], char termination[MAX_TERMINATION + 1]);

void set_up_recvcont(char client_handler_message[MAX_MSG], char identifier[MAX_IDENT + 1], 
char saycont_message_converted[MAX_CLIENT_MSG - 1], char termination[MAX_TERMINATION], char ignore[MAX_IDENT + 4]);

void death();

#endif