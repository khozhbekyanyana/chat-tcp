
#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define NUMBER_OF_THREADS 100

// thread identifiers
pthread_t tid[NUMBER_OF_THREADS];

int Connections[100];
int Counter = 0;
std::vector <std::string> users;

//дескриптор текущего потока
struct thread_args {
    int index;
};

//принимаем и рассылаем сообщения
void* ClientHandler(void* args) {
    thread_args* arg = (thread_args*)args;
    int msg_size;

    while (true) {
        //принимаем размер сообщения
        int connected = recv(Connections[arg->index], (char*)&msg_size, sizeof(int), 0);

        if(!connected) {
            std::cout << "Client Disconnected\n";
            users[arg->index] = "DISC";
            close(Connections[arg->index]);
            return args;
        }

        char* msg = new char[msg_size + 1];
        msg[msg_size] = '\0';

        //принимаем сообщение пользователя
        recv(Connections[arg->index], msg, msg_size, 0);

        for (int i = 0; i < Counter; i++) {
            if (i == arg->index) continue;

            //отправляем имя отправителя
            int name_size  = sizeof(users[arg->index]);
            send(Connections[i], (char*)&name_size, sizeof(int), 0);           
            send(Connections[i], users[arg->index].c_str(), name_size, 0);

            //отправляем сообщение остальным клиентам
            send(Connections[i], (char*)&msg_size, sizeof(int), 0);
            send(Connections[i], msg, msg_size, 0);
        }

        delete[] msg;
    }
}

int main()
{

    //добавляем информацию об адресе сокета
    sockaddr_in addr;
    socklen_t sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //IP
    addr.sin_port = htons(1111); //port
    addr.sin_family = AF_INET;

    //создаем сокет
    int sListen = socket(AF_INET, SOCK_STREAM, 0);

    //привязываем адрес к сокету
    bind(sListen, (sockaddr*)&addr, sizeof(addr));

    //слушаем порт
    listen(sListen, SOMAXCONN); //SOMAXCONN - максимально допустимое число запросов

    //создаем новый сокет, чтобы удерживать соединение
    int newConnection;

    //ждем новых подключений
    for (int i = 0; i < 100; i++) {

        //принимаем новое подключение
        newConnection = accept(sListen, (sockaddr*)&addr, &sizeofaddr);

        if (newConnection == 0) {
            std::cout << "Error 2" << std::endl;
        }
        else {

            std::cout << "Client Connected!" << std::endl;
            char hello_msg[] = "Вас приветствуют в нашем ПОЧТИ анонимном чате знакомств. \nУкажите ваше имя: ";
            
            //отправляем приветственное сообщение
            send(newConnection, hello_msg, sizeof(hello_msg), 0);

            //получаем имя пользователя
            int size_name;
            recv(newConnection, (char*)&size_name, sizeof(int), 0);

            char* user_name = new char[size_name + 1];
            user_name[size_name] = '\0';
            recv(newConnection, user_name, size_name, 0);
            
            //строка, содержащая в себе всех online пользователей на текущий момент
            std::string all_users;

            if (users.empty()) {
                all_users = "Упс, никто не хочет играть с Вампусом\n";
            }
            else {
                all_users = "Сейчас не молЧАТ: ";
                for (auto name : users) {
                    if (name == "DISC") continue;
                    all_users += name + " ";
                }
            }

            //отправляем строку с текущими пользователями
            int size_all_users = all_users.length();
            send(newConnection, (char*)&size_all_users, sizeof(int), 0);
            send(newConnection, all_users.c_str(), size_all_users, 0);

            //добавляем нового пользователя в общий список
            users.push_back(user_name);

            //запоминаем соединение
            Connections[i] = newConnection;
            Counter++;

            //уведомляем остальных пользователей о новом участнике
            for (int i = 0; i < Counter; i++) {
                if (i == Counter - 1) continue;

                //отправляем имя отправителя
                int name_size = sizeof(user_name);
                send(Connections[i], (char*)&size_name, sizeof(int), 0);
                send(Connections[i], user_name, size_name, 0);

                //отправляем приветственное сообщение
                char msg_new_user[] = "Я в деле!";
                int msg_new_user_size = sizeof(msg_new_user);
                send(Connections[i], (char*)&msg_new_user_size, sizeof(int), 0);
                send(Connections[i], msg_new_user, sizeof(msg_new_user), 0);

            }


            //подготавливаем аргументы для создания потока
            thread_args* arg = new thread_args;
            arg->index = i;
            
            //создаем поток для принятия и рассылки сообщений текущего пользователя
            int err = pthread_create(&tid[Counter], NULL, ClientHandler, arg);
            if (err !=0 ) {
                std::cerr << "Can't create thread" << std::endl;
            }

        }
    }

    system("pause");
    return 0;
}

