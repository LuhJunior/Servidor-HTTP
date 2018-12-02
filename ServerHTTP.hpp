#ifndef SERVIDOR_HTTP
#define SERVIDOR_HTTP
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

class Tools{
public:
    static string convExt(string);
    static vector<string> split(string, char);
    template<class T> static T Error(string, T);
    template<class T> static T ErrorW(string, T);
    template<class T> static T ErrorS(string, T, int);
};

class Response{
private:
    string header, status, server, content_type, content_length, connection, buffer, date;
public:
    Response(){
        header = status = server = content_type = content_length = connection = buffer = "";
    }
    string getStatus(){
        return this->status;
    }
    string getServer(){
        return this->server;
    }
    string getContentType(){
        return this->content_type;
    }
    string getContentLength(){
        return this->content_length;
    }
    string getConnection(){
        return this->connection;
    }
    string getDate(){
        return this->date;
    }
    void setStatus(string status){
        this->status = status;
    }
    void setServer(string server){
        this->server = server;
    }
    void setContentType(string content_type){
        this->content_type = content_type;
    }
    void setContentLength(int content_length){
        this->content_length = to_string(content_length);
    }
    void setConnection(string connection){
        this->connection = connection;
    }
    void setDate(string date){
        this->date = date;
    }
    bool Send(string, int);
    void makeHeader();
};

class Request{
private:
    string header;
public:
    Request(){
        this->header = "";
    }
    string getHeader(){
        return this->header;
    }
    string getMethod();
    string getRequestFileName();
    bool receiveRequest(int);
};

class http{
private:
    int local_socket, remote_socket;
    unsigned short local_port, remote_port;
    struct sockaddr_in local_address, remote_address;
    WSADATA wsa_data;
public:
    http(){
        local_socket = remote_socket = 0;
        local_port = remote_port = 0;
        memset(&local_address, 0, sizeof(local_address));
        memset(&remote_address, 0, sizeof(remote_address));
    }
    int getLocalPort(){
        return this->local_port;
    }
    int getRemotePort(){
        return this->remote_port;
    }
    void setLocalPort(int port){
        this->local_port = port;
    }
    void setRemotePort(int port){
        this->remote_port = port;
    }
    bool Listen(int);
    bool creatServer();
    void end();
};

#endif