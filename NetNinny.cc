/*
** The humble beginnings of NetNinny
*/
#include "NetNinny.hpp"

int main()
{
  int rv;
  int sockfd,new_fd; //listen on sockfd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  struct sigaction sa;
  char PORT[20];

  init_hints(hints);
    
  std::cout << "Enter portnumber : ";
  std::cin >> PORT; 
  printf("\n");
  if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    std::cerr << "getaddrinfo :" <<  gai_strerror(rv) << std::endl;
    return 1;
  }
  else{
    std::cout << "getaddrinfo()  sucessfull\n";
  }

  if(loop_all_bind_first(servinfo,sockfd,p)){
    std::cout << "Bind sucessfull!\n";
  }
  else{
    std::cerr << "Bind failed..\n";
    return 2;
  }

  freeaddrinfo(servinfo); //Free memory.

  if (listen(sockfd, BACKLOG) == -1) {
    std::cerr <<  "listen error\n";
    exit(1);
  }
  else{
    std::cout << "Listen() call sucessfull..\n";
  }

  // reap all dead processes
  sa.sa_handler = sigchld_handler; 
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    std::cerr << "sigaction\n";
    exit(1);
  }

  std::cout << "server waiting for connection... ... ...\n";

  main_accept_loop(their_addr,new_fd,sockfd);

  return 0;
}

void print_stars(const size_t& stars)
{
  for(size_t i{0}; i < stars;++i){
    printf("*");
  } 
  printf("\n");
}
void sigchld_handler(int s = 0)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

