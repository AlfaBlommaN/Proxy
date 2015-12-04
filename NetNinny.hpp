//C++ headers
#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
//C-Headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>

#define BACKLOG 10   
#define FORBIDDEN "SpongeBob","Norrköping", "Britney Spears", "Paris Hilton","norrkoping"
#define HOST  "http://www.ida.liu.se/"
#define ERROR_GET "http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error1.html"
#define ERROR_URL "~TDTS04/labs/2011/ass2/error2.html"

#ifndef NETNINNY_H
#define NETNINNY_H
//For function declarations

bool is_bad_url(std::string& str);
bool is_done(char* buffer);
bool loop_all_bind_first(addrinfo*& servinfo,int& sockfd,addrinfo*& p);
int recv_all(const int sockfd,char buffer[BUFSIZ]);
int send_all(int sockfd,std::vector<std::string> all_segs);
std::string extract_url(const char* http_request);
std::string get_hostname(const char* http_request);
std::string get_ip(const char * http_request);
std::string recv_frsrvr2(const int sockfd,char buffer[BUFSIZ]);
std::string talkto_client(int& new_fd);
std::vector<std::string> cnct_to_ws(const char* http_request);
std::vector<std::string> recv_frsrvr(const int sockfd,char buffer[BUFSIZ]);
void *get_in_addr(sockaddr *sa);
void connection_close(std:: string& mod_http_req);
void init_cnction(std::string hostname,int& sockfd);
void init_hints(addrinfo& hints);
void init_ipv4sockaddrin(sockaddr_in& sin);
void init_ipv6sockaddrin(sockaddr_in6& sin6);
void main_accept_loop(sockaddr_storage& their_addr, int& new_fd,const int& sockfd);
void modget(std::string& str);
void print_stars(const size_t& stars);
void remove_hostname(std::string& modreq);
void sigchld_handler(int s);
#endif
