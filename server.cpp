
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <map>
#include <chrono>


using namespace std;

int size_4 = 4, SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);char buffer[9999];int n;
struct sockaddr_in stSockAddr;
map<string,int> __socket_cl;


void __format_size(string & str_buff, int nroBytes){
  while(str_buff.size() < nroBytes){
    str_buff.insert(0,"0");
  }

}
void __format_msg(string & str_buff, int nroBytes){
  str_buff = string((nroBytes-to_string(str_buff.size()).length()),'0') + to_string(str_buff.size())+str_buff;
}

bool __search_user(string st){ //find  a  nickname is equal to st

  map<string, int>::iterator it;
  for (it = __socket_cl.begin(); it != __socket_cl.end(); it++)
      if(it->first == st){
          return true;
      }
  return false;
}

void __getname_by_socket(int id,string & st){//return nickanme found their number socket

  map<string, int>::iterator it;
  for (it = __socket_cl.begin(); it != __socket_cl.end(); it++)
      if(it->second == id){
          st=it->first;
      }
}
void __write_f_mssg(int ConnectFD, string mssg,string act){
  if (act!="F") {
      mssg = "R" +mssg;
      __format_msg(mssg,4);
      cout << mssg<<endl;
      write(ConnectFD, mssg.c_str(), mssg.size());
  }
}


void __read_mssg(int ConnectFD, char *buffer){
  int n;
  string response="";
  for (;;){
      bzero(buffer, 9999);
      do{
          n = read(ConnectFD, buffer, 4);
          string r_filesize(buffer);
          cout << r_filesize<<endl;

          int size_txt=atoi(buffer);bzero(buffer, 4);
          n = read(ConnectFD, buffer, 1);

          string __option(buffer);
          cout << __option<<endl;

          if (__option == "P"){
              string list_users="\nLista de usuarios conectados:\n";
              map<string, int>::iterator user;
              for (user = __socket_cl.begin(); user != __socket_cl.end(); user++){
                  list_users+=" * "+user->first + '\n';
              }
              //cout<<"Lista de usuarios conectados:\n"<<list_users<<endl; // print has all __socket_cl
              cout << "Print all users"<<endl;
              __write_f_mssg(ConnectFD,list_users,__option);
          }

          else if (__option == "L"){//protocolo for Login
              n = read(ConnectFD, buffer, size_txt);
              if(__search_user(string(buffer)) == true){    string err="nickname already exists, enter other\n";__write_f_mssg(ConnectFD,err.c_str(),__option);}
              else{
                  __socket_cl[buffer] = ConnectFD;cout << "Login: " << buffer << endl;
              }
          }

          else if (__option == "C"){
              string protocol ="";
              string __sender = "";
              __getname_by_socket(ConnectFD,__sender);
              n = read(ConnectFD, buffer, 2);
              protocol+=buffer;
              int size_othername=atoi(buffer);
              bzero(buffer, 2);
              n = read(ConnectFD, buffer, size_othername);
              protocol+=buffer;
              string __receptor(buffer);
              bzero(buffer, size_othername);
              if(__search_user(__receptor)==false){
                  string err = "nickname not found, enter other\n";
                  __write_f_mssg(ConnectFD, err.c_str(), __option);
                  break;
              }
              int __total_length= size_txt-3-__receptor.size();n = read(ConnectFD, buffer, __total_length);
              protocol+=buffer;
              string msg(buffer);
              bzero(buffer,__total_length);

              int otherConnectFD = __socket_cl.find(__receptor)->second; //finding socket number the other client for send to mssg
              if (otherConnectFD < 0){    perror("error in nickname");}
              msg = __sender+">>"+msg;    cout << protocol<<endl;    __write_f_mssg(otherConnectFD, msg, __option);
          }
          else if (__option == "F"){
            response += r_filesize;
            string __sender = "";
             __getname_by_socket(ConnectFD,__sender);

            response +="D";
            string __sender_nick_size = to_string(__sender.size());
            __format_size(__sender_nick_size,2);
            response += __sender_nick_size;
            response += __sender;
            int filename_size = size_txt;
            //string nickname_size = "",nick_name = "",file_name = "",file_size = "",file = "";
            //get  RECEPTOR NICKNAME
            n = read(ConnectFD, buffer, 2);
            int nickname_sz = atoi(buffer);
            n = read(ConnectFD,buffer,nickname_sz);
            string __receptor(buffer);
            if(__search_user(__receptor)==false){
                string err = "nickname not found, enter other\n";
                __write_f_mssg(ConnectFD, err.c_str(), __option);
                break;
            }
            n = read(ConnectFD,buffer,filename_size);
            string filename(buffer);
            response += filename;
            n = read(ConnectFD,buffer,4);
            int filesz = atoi(buffer);
            string r_filesz = to_string(filesz);
            __format_size(r_filesz,4);
            response += r_filesz;

            n = read(ConnectFD, buffer, filesz);
            string __file="";
            for(int i=0; i<filesz ; i++){
              __file += buffer[i];
            }
            response += __file;
            cout << response<<endl;
            //cout <<  __file;
            int otherConnectFD = __socket_cl.find(__receptor)->second;
            if (otherConnectFD < 0){    perror("error in nickname");}
            write(otherConnectFD, response.c_str(), response.size());
          }

          else if (__option == "G"){
            n = read(ConnectFD,buffer,4);
            cout << buffer<<endl;
            string ms(buffer);
            bzero(buffer,4);

            if(ConnectFD == 4) write(5,ms.c_str(),ms.size());
            if(ConnectFD == 5)  write(4,ms.c_str(),ms.size());
          }
          else if (__option == "E"){
              cout<<"El socketFD es: "<<ConnectFD<<endl;
              map<string, int>::iterator user;
              for (user = __socket_cl.begin(); user != __socket_cl.end(); user++){
                  if (user->second == ConnectFD){
                      __socket_cl.erase(user);
                      cout<<"Cliente eliminado exitosamente!!"<<endl;
                      return;
                  }
              }
          }
          else{
            //cout << "input incorrect !! \n";
            break;
          }
      } while (n == 0);
  }
}

void __conection_threads(int each_clientFD) {
  if (0 > each_clientFD){    perror("error accept failed");    close(SocketFD);    exit(EXIT_FAILURE);}
  //string first_greeting = "\n\nWelcome to Chuju Chat!!";
  //write(each_clientFD, first_greeting.c_str(), first_greeting.size());
}
int main(int argc, char* argv[]){
  if(-1 == SocketFD){    perror("can not create socket");    exit(EXIT_FAILURE);}
  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
  stSockAddr.sin_family = AF_INET;
  if(argc > 1){ stSockAddr.sin_port = htons(atoi(argv[1]));}
  else{    cout << "missing port argument";    return 0;}
  stSockAddr.sin_addr.s_addr = INADDR_ANY;
  if(-1 == ::bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){    perror("error bind failed");    close(SocketFD);    exit(EXIT_FAILURE);}
  if(-1 == listen(SocketFD, 10)){    perror("error listen failed");    close(SocketFD);    exit(EXIT_FAILURE);}

  while(true){
      int each_clientFD = accept(SocketFD, NULL, NULL);
      cout<<"[[[[[ Ingreso un nuevo cliente: "<<each_clientFD<<" ]]]]]"<<endl;
      thread(__conection_threads,each_clientFD).detach();
      thread(__read_mssg, each_clientFD,buffer).detach();
  }
  shutdown(SocketFD, SHUT_RDWR);    close(SocketFD);
  return 0;
}
