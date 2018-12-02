#include <iostream>
#include "ServerHTTP.hpp"
clock_t inicio, fim;
using namespace std;

string Tools::convExt(string ext){
    if(ext == ".html") return "text/html";
    else if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    else if (ext == ".gif") return "image/gif";
    else if (ext == ".png") return "image/png";
    else if (ext == ".css") return "text/css";
    else if (ext == ".avi") return "video/x-msvideo";
    else if (ext == ".mpeg" || ext == ".mpg") return "video/mpeg";
    else if (ext == ".mp3") return "audio/mpeg";
    return nullptr;
}

vector<string> Tools::split(string s, char t){
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

template<class T>
T Tools::Error(string msg, T r){
    cerr<<msg<<endl;
    return r;
}

template<class T>
T Tools::ErrorW(string msg, T r){
    cerr<<msg<<endl;
    WSACleanup();
    return r;
}

template<class T>
T Tools::ErrorS(string msg, T r, int s){
    cerr<<msg<<endl;
    WSACleanup();
    closesocket(s);
    return r;
}

bool Request::receiveRequest(int remote_socket){
    this->header.resize(1000);
    // recebe a bloco do de requisição do navegador
    while(recv(remote_socket, (char *) this->header.c_str(), header.size(), 0) == SOCKET_ERROR);
    return true;
}

string Request::getMethod(){
    vector<string> head = Tools::split(this->header, '\n');
    vector<string> metodo = Tools::split(head[0], ' ');
    return head[0];
}

string Request::getRequestFileName(){
    vector<string> head = Tools::split(this->header, '\n');
    vector<string> metodo = Tools::split(head[0], ' ');
    return metodo[1].substr(1, metodo[1].size());
}

void Response::makeHeader(){
    this->header.append("HTTP/1.1 " + this->getStatus() + "\r\n");
    this->header.append("Server: " + this->getServer() +  "\r\n");
    this->header.append("Date: " + this->getDate() +  "\r\n");
    this->header.append("Content-Type: " + this->getContentType() +  "\r\n");
    this->header.append("Content-Length: " + this->getContentLength() +  "\r\n");
    this->header.append("Connection: " + this->getConnection() +  "\r\n");
    this->header.append("\r\n");
}

bool Response::Send(string fname, int remote_socket){
    time_t agora;
    fstream f;
    f.open(fname, fstream::in | fstream::binary);
    if(f.is_open()){
        cout<<fname<<endl;
        f.seekg(0, fstream::end);
        int tam = f.tellg();
        f.seekg(0, fstream::beg);
        this->buffer.resize(tam);
        f.read((char *) this->buffer.c_str(), tam);
        //cout<< this->buffer << endl;
        vector<string> ext = Tools::split(fname, '.');
        agora = time(NULL);
        string data;
        data.resize(128);
        strftime((char *) data.c_str(), data.size(), RFC1123, gmtime(&agora));	
        this->setStatus("200 OK");
        this->setServer("ODYDN SERVER");
        this->setContentType(Tools::convExt('.' + ext[1]));
        this->setContentLength(tam);
        this->setConnection("close");
        this->setDate(data); 
    }
    else{
        f.open("NotFound.html", fstream::in | fstream::binary);
        if(f.is_open()){
            f.seekg(0, fstream::end);
            int tam = f.tellg();
            f.seekg(0, fstream::beg);
            this->buffer.resize(tam);
            f.read((char *) this->buffer.c_str(), tam);
            vector<string> ext = Tools::split(fname, '.');
            agora = time(NULL);
            string data;
            data.resize(128);
            strftime((char *) data.c_str(), data.size(), RFC1123, gmtime(&agora));
            this->setStatus("404");
            this->setServer("ODYDN SERVER");
            this->setContentType("text/html");
            this->setContentLength(tam);
            this->setConnection("close");
            this->setDate(data);
        }
        else{
            this->header.clear();
        }
    }
    this->makeHeader();
    //enviando cabeçalho
    while(send(remote_socket, (char *) this->header.c_str(), this->header.size(), 0) == SOCKET_ERROR) cout<<"Erro ao enviar resposta\nTentando enviar novamente\n";
    Sleep(100);
    //enviando arquivo pedido
    //for(int i=0; i<this->buffer.size(); i += 8000){
        //string ax = this->buffer.substr(0, 8000);
        //send(remote_socket, (char *) ax.c_str(), 3000/*this->buffer.size()*/, 0);
    while(send(remote_socket, (char *) this->buffer.c_str(), this->buffer.size(), 0) == SOCKET_ERROR) cout<<"Erro ao enviar resposta\nTentando enviar novamente\n";
    //}
    return true;
}

bool http::creatServer(){
    inicio = clock();
    // inicia o Winsock 2.0 (DLL), Only for Windows
    if (WSAStartup(MAKEWORD(2, 0), &this->wsa_data) != 0) return Tools::Error("WSAStartup() failed", false);
    // criando o socket local para o servidor
    this->local_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->local_socket == INVALID_SOCKET) return Tools::ErrorW("socket() failed", false);

    this->local_address.sin_family = AF_INET; // internet address family
    this->local_address.sin_port = htons(this->local_port);// porta local
    this->local_address.sin_addr.s_addr = htonl(INADDR_ANY);  // endereco // inet_addr("127.0.0.1")

    // interligando o socket com o endereço (local)
    if (bind(local_socket, (struct sockaddr *) &local_address, sizeof(local_address)) == SOCKET_ERROR)
        return Tools::ErrorS("bind() failed", false, this->local_socket);

    return true;
}

bool http::Listen(int port){
    Request req;
    Response res;
    int remote_length = 0;
    this->local_port = port;
    this->creatServer();
    // coloca o socket para escutar as conexoes
    if (listen(this->local_socket, BACKLOG_MAX) == SOCKET_ERROR) return Tools::ErrorS("listen() failed", false, this->local_socket);
    
    remote_length = sizeof(remote_address);
    //while(true){
        cout<<"Aguardando Conexao..."<<endl;
        fim = clock();
        this->remote_socket = accept(local_socket, (struct sockaddr *) &remote_address, &remote_length);
        if(this->remote_socket == INVALID_SOCKET) return Tools::ErrorS("accept() failed", false, this->local_socket);
        fim = clock();
        cout<<"Conexao estabelecida com "<< inet_ntoa(remote_address.sin_addr)<<endl;
        cout<<"Aguardando requisicoes..."<<endl;
        if(!req.receiveRequest(this->remote_socket)) return Tools::ErrorS("Falha ao receber a requisicao", false, this->local_socket);
        if(!res.Send(req.getRequestFileName(), this->remote_socket))  return Tools::ErrorS("Falha ao responder a requisicao", false, this->local_socket);
    //}
    return true;
}

void http::end(){
    WSACleanup();
    closesocket(this->remote_socket);
    closesocket(this->local_socket);
}