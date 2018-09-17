#include <iostream>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <time.h>
#define RFC1123     "%a, %d %b %Y %H:%M:%S GMT" 
#define BACKLOG_MAX 5
#define PORT 8000

using namespace std;

void MontarHeader(string &header, int status, int conLen){
    header = "HTTP/1.1 " + to_string(status) + " OK\r\n";
    header += "Server: MeuServidor/1.0\r\n";
    header += "Content-Type: text/html\r\n";
    header += "Content-Length: " + to_string(conLen) + "\r\n";
    header += "Connection: close\r\n\r\n";
}

vector<string> split(string s, char t){
    vector<string> v;
    while(!s.empty()){
        size_t p = s.find(t);
        if(p != -1){
            v.push_back(s.substr(0, p));
            s.erase(s.begin(), s.begin()+p+1);
        }
        else{
            v.push_back(s);
            s.clear();
        }
    }
    return v;
}

int main(){
    string fname, buffer, header, resposta;
    int local_socket = 0, remote_socket = 0, remote_length = 0, message_length = 0, tam = 0;
    unsigned short local_port = PORT;
    unsigned short remote_port = 0;
    struct sockaddr_in local_address;
    struct sockaddr_in remote_address;
    time_t agora;
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0){
        cout<<"WSAStartup() failed"<<endl; // inicia o Winsock 2.0 (DLL), Only for Windows
        return 0;
    }
    local_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);// criando o socket local para o servidor
    if (local_socket == INVALID_SOCKET){
        WSACleanup();
        cout<<"socket() failed"<<endl;
        return false;
    }

    memset(&local_address, 0, sizeof(local_address)); // zera a estrutura local_address
    local_address.sin_family = AF_INET; // internet address family
    local_address.sin_port = htons(local_port);// porta local
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);  // endereco // inet_addr("127.0.0.1")
    // interligando o socket com o endereço (local)
    if (bind(local_socket, (struct sockaddr *) &local_address, sizeof(local_address)) == SOCKET_ERROR){
        WSACleanup();
        closesocket(local_socket);
        cout<<"bind() failed"<<endl;
        return false;
    }

    // coloca o socket para escutar as conexoes
    if (listen(local_socket, BACKLOG_MAX) == SOCKET_ERROR){
        WSACleanup();
        closesocket(local_socket);
        cout<<"listen() failed"<<endl;
        return false;
    }

    remote_length = sizeof(remote_address);

    cout<<"Aguardando Conexao..."<<endl;

    remote_socket = accept(local_socket, (struct sockaddr *) &remote_address, &remote_length);
    if(remote_socket == INVALID_SOCKET){
        WSACleanup();
        closesocket(local_socket);
        cout<<"accept() failed"<<endl;
        return false;
    }

    cout<<"Conexao estabelecida com "<< inet_ntoa(remote_address.sin_addr)<<endl;
    cout<<"Aguardando requisicoes..."<<endl;
    
    buffer.resize(1000);
    memset((char *) buffer.c_str(), 0, buffer.size()); // limpa o buffer

    // recebe a bloco do de requisição do navegador

    while(recv(remote_socket, (char *) buffer.c_str(), buffer.size(), 0) == SOCKET_ERROR);
    //cout<< "Requested: "<<buffer;
    vector<string> req = split(buffer, '\n');
    //for(string x:req) cout<<x<<endl;
    vector<string> metodo = split(req[0], ' ');
    //for(string x:metodo) cout<<x<<endl;
    fstream file;
    file.open(metodo[1].substr(1, metodo[1].size()), fstream::in);
    if(file.is_open()){
        cout<<metodo[1].substr(1, metodo[1].size());
        while(!file.eof()){
            string ax;
            file>>ax;
            resposta += ax;
        }
        cout<< resposta << endl;
        MontarHeader(header, 200, resposta.size());
        cout<<header<<endl;
    }
    else{
        resposta.clear();
        cout<<"Not Found"<<endl;
        MontarHeader(header, 404, 0);
    }
    while(send(remote_socket, (char *) header.c_str(), header.size(), 0) == SOCKET_ERROR) cout<<"Erro ao enviar bloco de resposta negativa\nTentando enviar novamente\n";
    Sleep(1000);
    while(send(remote_socket, (char *) resposta.c_str(), resposta.size(), 0) == SOCKET_ERROR) cout<<"Erro ao enviar bloco de resposta negativa\nTentando enviar novamente\n";
    WSACleanup();
    closesocket(remote_socket);
    closesocket(local_socket);
    return 0;
}