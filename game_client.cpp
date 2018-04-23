#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <vector>
#include <fstream>
#include <ncurses.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
using namespace std;

struct sockaddr_in stSockAddr;
char buffer[255];

typedef struct _win_border_struct {
        chtype  ls, rs, ts, bs,
                tl, tr, bl, br;
                                      }WIN_BORDER;
typedef struct _WIN_struct {
        int startx, starty;
        int height, width;
        WIN_BORDER border;
                                      }WIN;

                                      WIN win;
                                      WIN win2;
void init_win_params(WIN &p_win);
void print_win_params(WIN &p_win);
void create_box(WIN &win, bool flag);
void bullet(WIN &p_win)
{
}

void __format_msg(string & str_buff, int nroBytes){ // esta funcion añade la cantidad "nroBytes" al inicio de la cadena str_buff
    str_buff = string((nroBytes-to_string(str_buff.size()).length()),'0') + to_string(str_buff.size())+str_buff;
}

void __format_size(string & str_buff, int nroBytes){
  while(str_buff.size() < nroBytes){
    str_buff.insert(0,"0");
  }

}

int write_move(WIN & w){
   	int ch;
    create_box(win2, TRUE);
    // while((ch = getch()) != 27)
    if((ch = getch()) !=  KEY_F(1))
    {
        switch(ch)
        {   case KEY_LEFT:
                    create_box(w, FALSE);
                    --w.startx;
                    create_box(w, TRUE);
                    break;
            case KEY_RIGHT:
                    create_box(w, FALSE);
                    ++w.startx;
                    create_box(w, TRUE);
                    break;
            case KEY_UP:
                    create_box(w, FALSE);
                    --w.starty;
                    create_box(w, TRUE);
                    break;
            case KEY_DOWN:
                    create_box(w, FALSE);
                    ++w.starty;
                    create_box(w, TRUE);
                    break;

        }
        return ch;
    }

    //endwin();

}
void read_move(WIN & w, int ch){
    create_box(win, TRUE);
    // while((ch = getch()) != 27)
        switch(ch)
        {   case KEY_LEFT:
                    create_box(w, FALSE);
                    --w.startx;
                    create_box(w, TRUE);
                    break;
            case KEY_RIGHT:
                    create_box(w, FALSE);
                    ++w.startx;
                    create_box(w, TRUE);
                    break;
            case KEY_UP:
                    create_box(w, FALSE);
                    --w.starty;
                    create_box(w, TRUE);
                    break;
            case KEY_DOWN:
                    create_box(w, FALSE);
                    ++w.starty;
                    create_box(w, TRUE);
                    break;

        }
    //endwin();

}
void __read_thread(int SocketFD, char *buffer){
    int n;
    for (;;){
        bzero(buffer, 100);
        //string tmp ="";
        do{
            n = read(SocketFD, buffer, 4);
            //string __size_general_msg(buffer);
        } while (n == 0);

        read_move(win2,atoi(buffer));

    }
    shutdown(SocketFD, SHUT_RDWR);close(SocketFD);
}

void __write_thread(int SocketFD){
    do{
        string __msg="0000G";
        // string __input;
        // cin>>__input;
        // __msg +=__input;
        int ch = write_move(win);
        string movement = to_string(ch);
        __format_size(movement,4);
        __msg += movement;
        write(SocketFD, __msg.c_str(), __msg.size());
    }while (1);
    shutdown(SocketFD, SHUT_RDWR);    close(SocketFD);
}

void init_curses(){
  initscr();                      /* Start curses mode            */
  start_color();                  /* Start the color functionality */
  cbreak();                       /* Line buffering disabled, Pass on
                                   * everty thing to me           */
  keypad(stdscr, TRUE);           /* I need that nifty F1         */
  noecho();
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  /* Initialize the window parameters */
  init_win_params(win);
  print_win_params(win);
  /* Initialize the window parameters */
  init_win_params(win2);
  print_win_params(win2);
  attron(COLOR_PAIR(1));
  printw("Press F1 to exit");
  refresh();
  attroff(COLOR_PAIR(1));
  create_box(win, TRUE);
--win2.startx;
--win2.starty;
  create_box(win2, TRUE);
}
int main(int argc, char* argv[]){
    int Res, SocketFD;
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == SocketFD){
        perror("cannot create socket");
        exit(EXIT_FAILURE);
      }
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    if(argc > 2){
      stSockAddr.sin_port = htons(atoi(argv[1]));
      Res = inet_pton(AF_INET, argv[2], &stSockAddr.sin_addr);
      init_curses();
      //thread(&init_curses).detach();
      thread(__read_thread,SocketFD, buffer).detach();
      thread(__write_thread,SocketFD).detach();
    }
    else{
      cout << "missing port or ip arguments";return 0;}
    if (0 > Res){
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);}
    else if (0 == Res){
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);}
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);}
    int n = read(SocketFD, buffer,sizeof(buffer));
    printf("%s\n", buffer);


    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
}

void init_win_params(WIN &p_win)
{
        p_win.height = 8;
        p_win.width = 16;
        p_win.starty = (LINES - p_win.height)/2;
        p_win.startx = (COLS - p_win.width)/2;
        p_win.border.ls = '|';
        p_win.border.rs = '|';
        p_win.border.ts = '-';
        p_win.border.bs = '-';
        p_win.border.tl = '+';
        p_win.border.tr = '+';
        p_win.border.bl = '+';
        p_win.border.br = '+';
}
void print_win_params(WIN &p_win)
{
}
void create_box(WIN &p_win, bool flag)
{       int i, j;
        int x, y, w, h;
        x = p_win.startx;
        y = p_win.starty;
        w = p_win.width;
        h = p_win.height;
        if(flag == TRUE)
        {
                move( y+0,x ); addstr("  ##        ##  ");
                move( y+1,x ); addstr("    #      #    ");
                move( y+2,x ); addstr("  ############  ");
                move( y+3,x ); addstr(" ###  ####  ### ");
                move( y+4,x ); addstr("################");
                move( y+5,x ); addstr("# ############ #");
                move( y+6,x ); addstr("# #          # #");
                move( y+7,x ); addstr("   ##      ##   ");
        }
        else
               for(j = y; j <= y + h; ++j)
                        for(i = x; i <= x + w; ++i)
                                mvaddch(j, i, ' ');
        refresh();
}
