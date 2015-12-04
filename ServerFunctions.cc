#include "NetNinny.hpp"
#include <errno.h>
/* The hearth of the server
   it accepts vaild connection on the correct port and then forwards
   the connections to the client part of the server, when the client part is done
   with the data it will resend the requests back to the browser.
*/
void main_accept_loop(sockaddr_storage& their_addr,
		      int& new_fd,const int& sockfd)
{
  socklen_t sin_size{0}; //socklen_t is a 32 bit unsigned
  std::string proxy_buffer; //buffer for server data.
  char connection_ip[INET6_ADDRSTRLEN]; //INET_6ADDRSTRLEN is always the longest possible		
  std::vector<std::string> all_segs;
  while(true){
    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd,(sockaddr *)&their_addr,&sin_size);

    if(new_fd == -1){
      std::cerr << "Accept() failed.. \n";
      continue;
    }

    inet_ntop(their_addr.ss_family,
	      get_in_addr((sockaddr *)&their_addr), //theiraddr must be either a ipv_adder or 1pv_6addr
	      connection_ip, sizeof(connection_ip));

    std::cout << "server: got connection from: " << connection_ip << '\n'; 
    //Handle Children
    if (fork() == 0) { // this is the child process, cp always returns 0;
      close(sockfd); 
      proxy_buffer = talkto_client(new_fd); //proxy_buffer contains the request
      //Forward the request to the internetserver.
      all_segs = cnct_to_ws(proxy_buffer.data()); 
      //forward data back to the client e,g the browser            
      if (send_all(new_fd,all_segs) < 0) {
	  	std::cerr << "Failed to send to client" << std::endl;
	  }

      close(new_fd);    
      exit(0); 
    }
    close(new_fd); //done and done.
  }
  return;
}

/*
  The function will loop the linked list of addrinfo
  and bind to the first possible, then it will be decided if 
  they should be accepted or not! 
*/

bool loop_all_bind_first(addrinfo*& servinfo,int& sockfd,addrinfo*& p)
{
  const int yes = 1; //listen on sock_fd
  
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      std::cerr << "server socket probs" << std::endl;
      continue;
    }

     if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		 sizeof(int)) == -1) {
      std::cerr << "setsockopt not good\n";
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      std::cerr << "server: bind";
      continue;
    }

    break;
  }

  if (p == NULL){
    std::cerr <<  "server: failed to bind\n";
    return false;
  }

  return true;
}

/*recv data from the client
  We make sure the recv all data
  using our recv_all function
*/
int recv_all(const int sockfd,char buffer[BUFSIZ])
{
  int len_recv{-1},total_data_recv{0};
  bool not_all_recieved{true};
  
  while(not_all_recieved || len_recv == 0 ){ //Kolla senare
    len_recv = recv(sockfd, buffer, BUFSIZ-1, 0); // <---
    total_data_recv += len_recv;
    buffer[len_recv] = '\0';
      
    if (len_recv < 0) {
      std:: cerr << "Error retrieving data\n";
      return -1;
    }
    else if(is_done(buffer)){
      not_all_recieved = false;
    }
  }
  std::cout << "Sucessfully recieved: " << total_data_recv << " bytes \n";
  print_stars(20);
  return 0;
}

bool is_done(char* buffer)
{
  int cntws{0};

  if((strlen(buffer) -1) > 4){
    for(size_t i{strlen(buffer)-1},j{0}; j <  4; --i,++j){
      if(isspace(buffer[i])){
	++cntws;
      }
      if((!isspace(buffer[i])) && cntws > 0){
	cntws = 0;
      }
    }
  }
  
  if(cntws == 4){
    return true;
  }

  return false;
}

/*
  Talk to the client new_fd contains the filedescriptor
  for the new socket.
  return the data as a string.
  However if the URL is bad we are going to prepair a redirection 
*/
std::string talkto_client(int& new_fd)
{
  char buffer[BUFSIZ];
  int sockfd {new_fd}; //clients socket file descriptor        
  std::vector<std::string> forbidden{FORBIDDEN}; //vector containing the forbidden words

  recv_all(sockfd,buffer); //<--- headern från browsern.
  std::string buffer_str = buffer;  //The buffer is now contained in a std::string
  printf("HTTP HEADER WILL NOW FOLLOW\n%s",buffer_str.data());
  if(is_bad_url(buffer_str)){
    modget(buffer_str);
  }

  return buffer_str;
}

/*
  inits hints with neccesary data 
*/
void init_hints(addrinfo& hints)
{
  memset(&hints, 0, sizeof hints); //init hints
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM; //TCP
  hints.ai_flags = AI_PASSIVE; // use my IP <--
}
/*
  Returns the Adress family.
*/
void *get_in_addr(sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((sockaddr_in*)sa)->sin_addr);
  }
  else{
    return &(((sockaddr_in6*)sa)->sin6_addr);
  }
}

int send_all(int sockfd, std::vector<std::string> all_segs)
{
  std::string response;
  for(auto seg: all_segs){
    response += seg;
  }

 int total_sent{0},left_to_send{int(response.size())},sent{0},len{int(response.size())}; //Observe danger
 bool error {false};


  while(total_sent < len){

    if((sent = send(sockfd, response.c_str()+total_sent, left_to_send,0))  < 0) {
      std::cerr << "# Failed when trying to send data, errno: " << strerror(errno) << std::endl;
      error = true;
      break;
    }
    total_sent += sent;
    left_to_send -= sent;
  }

  if(error){
    exit(1);
  }

  std::cout << "# Number of bytes sent: " << total_sent << std::endl;

  close(sockfd);
  print_stars(20);
  return 0;
}

bool is_bad_url(std::string& str)
{
  std::vector <std::string> forbidden {FORBIDDEN};
  for(auto i : forbidden){
    if(str.find(i) != std::string::npos){
      return true;
    }
    continue;
  }
  return false;
}

/* 
Change URL to the redirect trafic. 
SO if we encounter one of the words 
we will avoid visiting the website. 
 */

void modget(std::string& str)
{
  size_t found1,found2; 
  found1 = str.find("http");
  found2 = str.find("/r/n");
  str.replace(found1,found2,ERROR_GET);
  found1 = str.find("Host:");
  found2 = str.find("/r/n");
  str.replace(found1+4,found2,HOST);

  printf("%s", str.data());
}



