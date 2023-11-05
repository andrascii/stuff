#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    std::cerr << "Failed to create socket" << std::endl;
    return 1;
  }

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(12345); // Укажите нужный порт сервера здесь
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Укажите IP-адрес сервера здесь

  if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Failed to connect to server" << std::endl;
    return 1;
  }

  // Отправка сообщения на сервер
  std::string message = "Hello, server!";
  if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
    std::cerr << "Failed to send message to server" << std::endl;
    return 1;
  }

  // Получение ответа от сервера
  char buffer[4096];
  memset(buffer, 0, sizeof(buffer));
  if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
    std::cerr << "Failed to receive response from server" << std::endl;
    return 1;
  }

  // Вывод ответа от сервера
  std::cout << "Server response: " << buffer << std::endl;

  // Закрытие сокета
  close(clientSocket);

  return 0;
}