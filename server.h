#ifndef SERVER_H
#define SERVER_H

void copy_ident1(char client_handler_message[2048], char identifier[257]);

void copy_ident2(char ignore[260], char identifier[257]);

void copy_message1(char client_handler_message[2048], char client_message_converted[1790]);

void copy_name(char write_files[2048], char domain[2046], char * name);

void connect(char message[2048], char identifier[257], char domain[2046], char temp_domain[2046], char ident_wr[259], 
char ident_rd[259], char final_directory_wr[2048], char final_directory_rd[2048],  
int ident_count, int domain_count);

void say(char client_message_converted[1790], char client_message[2048], int say_count);

void set_up_receive_say(char client_handler_message[2048], char identifier[257], 
char client_message_converted[1790], char ignore[260]);

void domain_search(DIR * dom, struct dirent * entry, int len_ident, char post_fix[4], 
char ignore[260], char write_files[2049], char domain[2046], int rd_pipe_fd, char client_handler_message[2048]);

void say_cont(char saycont_message_converted[1789], char client_message[2048], int say_cont_count, char termination[1]);

void copy_message2(char client_handler_message[2048], char saycont_message_converted[1789]);

void copy_termination(char client_handler_message[2048], char termination[2]);

void set_up_recvcont(char client_handler_message[2048], char identifier[257], 
char saycont_message_converted[1789], char termination[1], char ignore[260]);

void death();

#endif