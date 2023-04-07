#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int Connection ;

void* ClientHandler(void* args) {

    int msg_size;
    int name_size;

    while (true) {

        //получаем имя отправителя
        recv(Connection, (char*)&name_size, sizeof(int), 0);
        char* sender_name = new char[name_size + 1];
        sender_name[name_size] = '\0';
        recv(Connection, sender_name, name_size, 0);

        //получаем сообщение
        recv(Connection, (char*)&msg_size, sizeof(int), 0);
        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';
        recv(Connection, msg, msg_size, 0);

        std::cout << sender_name<< ": " << msg << std::endl;

        delete[] msg;
        delete[] sender_name;
    }
}

int main()
{
    //добавляем информацию об адресе сокета
    sockaddr_in addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //IP
    addr.sin_port = htons(1111); //port
    addr.sin_family = AF_INET;

    //создаем сокет для соединения с сервером
    Connection = socket(AF_INET, SOCK_STREAM, 0);

    if(Connection == -1){
        std::cout << "Error: failed create a socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    //пытаемся подключиться к серверу
    if (connect(Connection, (sockaddr*)&addr, sizeof(addr)) != 0) {
        std::cout << "Error: failed connect to server" << std::endl;
        return 1;
    }
    else {
        std::cout << "Connected" << std::endl;

        //получаем приветственное сообщение
        char hello_msg[256];
        recv(Connection, hello_msg, sizeof(hello_msg), 0);

        //вводим имя пользователя
        std::string user_name;
        std::cout << hello_msg;
        std::getline(std::cin, user_name);

        //отправляем имя на сервер
        int name_size = user_name.size();
        send(Connection, (char*)&name_size, sizeof(int), 0);
        send(Connection, user_name.c_str(), name_size, 0);

        //получаем информацию о пользователях, которые присоединились к чату
        int size_all_users;
        recv(Connection, (char*)&size_all_users, sizeof(int), 0);

        char* all_users = new char[size_all_users];
        recv(Connection, all_users, size_all_users, 0);
        
        std::cout << all_users << std::endl;

        delete[] all_users;

    }

    pthread_t tid;
    int err = pthread_create(&tid, NULL, ClientHandler, 0);

    std::string msg;

    while (true) {
        std::getline(std::cin, msg);

        int str_size = msg.size();
        send(Connection, (char*)&str_size, sizeof(int), 0);
        send(Connection, msg.c_str(), str_size, 0);

        usleep(10);
    }

    system("pause");
    return 0;

}
