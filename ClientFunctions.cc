#include "NetNinny.hpp"
/*
  Function to talk to the webserver
  it shall return data and forward to browser.
  The function shall act as the client part 
  for the proxyserver. 
*/
std::vector<std::string> cnct_to_ws(const char* http_request) //<---
{
  int sockfd{0}, nByte{0}, total{0};
  char BUF[BUFSIZ]; //Hur göra..
  
  std::string mod_http_req{http_request};
  std::string hostname = get_hostname(http_request);
  std::vector<std::string> response;

  init_cnction(hostname,sockfd);

  //Remove hostname
  remove_hostname(mod_http_req);
  connection_close(mod_http_req);
  mod_http_req += "\r\n\r\n";  //Fix the carriage return.

  print_stars(20);
  // printf("The get look like this\n %s",mod_http_req.data());

  if(((nByte = send(sockfd,mod_http_req.data(),strlen(mod_http_req.data()), 0)) <=  0)){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(nByte));
  }  

  std::cout << nByte << " sent in GET to server\n";
  print_stars(20);  
  response = recv_frsrvr(sockfd,BUF); //recv from server.3  //close(sockfd);
  //response2 = recv_frsrvr2(sockfd,BUF,total); // All data gets init.
  return response;                      
}
/*
  This shall return the canonnical hostname
*/
std::string get_hostname(const char* http_request)
{
  
  char tmp[BUFSIZ];
  std::string sneaky; 
  std::string hostname{http_request};
  size_t found1 = hostname.find("Host:") + 6;
  size_t found2 = hostname.find("\r\n",found1);
  
  hostname.copy(tmp,found2-found1,found1); 
  sneaky = tmp;
  
  return sneaky;
}


void remove_hostname(std::string& modreq)
{  
  size_t found1 = modreq.find("http:");
  size_t found2 = modreq.find("/",found1+8);	
  modreq.erase(found1,found2-4);
  //printf("%s",modreq.data());
}

void connection_close(std::string& modreq)
{
  std::string str2("keep-alive");
  modreq.replace(modreq.find(str2),modreq.length(),"close");
  //printf("%s\n",modreq.data());
}

/*
  Recv data from server buffsiz is 8192 bytes.
*/

std::vector<std::string> recv_frsrvr(const int sockfd,char buffer[BUFSIZ])
{
  std::string data_seg; 
  std::vector<std::string> all_segs;
  int len_rcv{-1},total_data_rcv{0};
  int len = 0;
  int rv;

  while((len_rcv = recv(sockfd,buffer,BUFSIZ-1,0)) > 0){ //8192
    total_data_rcv += len_rcv;

    for(int i = 0; i < len_rcv;++i){
      data_seg += buffer[i];
    }

    memset(&buffer[0], 0, sizeof(buffer));
    buffer[0] = '\0';

    //kolla segment
    all_segs.push_back(data_seg);
    data_seg.clear();
  }

  printf("\nRecieved %d bytes from server \n",total_data_rcv);
  print_stars(20);
  int lens = 0;

  std::cout << "Sanitycheck length of str: \n";
  for(auto i :all_segs){
    std::cout << i.length() << std::endl;
    lens = 0;
  } 

  print_stars(20);
  return all_segs;
}

/*
  Connects to the remote server
 */
void init_cnction(std::string hostname,int& sockfd)
{

  char ip_adress[INET6_ADDRSTRLEN];
  int rv;
  int  numbytes;  
  struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname.data(), "http", &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to connect\n");
        exit(2);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	      ip_adress, sizeof(ip_adress));
    printf("client: connecting to %s \n", ip_adress);

    freeaddrinfo(res); 

  return;
}


