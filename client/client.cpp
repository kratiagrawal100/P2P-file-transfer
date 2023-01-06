#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <iostream>
#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <ctype.h>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAXSIZE 8192
#define CHUNK_SIZE 512
#define port_tracker 9002
using namespace std;
map<string, vector<string>> join_requests;
map<string, vector<string>> files_completed;
map<string, vector<string>> files_downloading;

void *client_thread_handler(void *param)
{
    // cout << "in client handler\n";
    int server_socket = *(int *)param;
    char request[MAXSIZE];
    int n;
    bzero(request, MAXSIZE);
    recv(server_socket, request, sizeof(request), 0);
    string req = request;
    int index = req.find_first_of("|"); // join_request|uname|gid;
    string cmd = req.substr(0, index);
    cout << request << "\n"
         << cmd << "\n";
    if (strcmp(cmd.c_str(), "join_group") == 0)
    {
        // cout << "here in join requests\n";
        int flag = 0;
        int index2 = req.find_last_of("|");
        string uname = req.substr(index + 1, index2 - index - 1);
        string gid = req.substr(index2 + 1);
        for (auto r : join_requests[gid])
        {
            if (strcmp(r.c_str(), uname.c_str()) == 0)
                flag = 1;
        }
        if (flag == 0)
            join_requests[gid].push_back(uname);
        string response = "Response received";
        send(server_socket, response.c_str(), sizeof(response), 0);
    }
    else if (strcmp(cmd.c_str(), "download_file") == 0)
    {
        // download_file|1.mp4|dpath|9467288;

        cout << "request recieved for downloading .....\n";
        int index = req.find_first_of("|");
        int index2 = req.find_last_of("|");
        int fsize = stoi(req.substr(index2 + 1));
        req=req.substr(0,index2);
        index2 = req.find_last_of("|");
        string fpath = req.substr(index + 1, index2 - index - 1);
        string dpath=req.substr(index2+1);
        req.substr(0,index2);
        cout << fpath << "\n"
             << fsize << "\n";
        string line;
        ifstream in(fpath);
        in.seekg(0, ios::end);
        int ssize = in.tellg();
        in.seekg(0);
        cout<<ssize<<"\n";
        char *buffer = new char[ssize];
        in.read(buffer, ssize);
        ofstream f("dummy.png");
        f.write(buffer, fsize);
        f.close();
        in.close();
        cout<<"Response sent";
        //cout<<response.length();
        send(server_socket,buffer,ssize, 0);
        //send(server_socket,response.c_str(),ssize, 0);
    }
    return nullptr;
}

void *server_thread_handler(void *param)
{
    //cout << param << "\n";
    char *client = (char *)param;
   // cout << "type:" << client << "\n";
    string client_address = client;
    //cout << "here" << client_address << "\n";
    int index = client_address.find(":");
    string ip = client_address.substr(0, index);
    string port = client_address.substr(index + 1);
    int port_no = stoi(port);
   // cout << port_no << "\n";

    //------------------------------------
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_no);
    server_address.sin_addr.s_addr = inet_addr(ip.c_str());

    int bind_status = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    if (listen(server_socket, 5) < 0)
    {
        perror("Not Listening");
        exit(1);
    }
    else
    {
        cout << "[+] Running..." << endl;
    }
    struct sockaddr_in client_add;
    pthread_t thread_id;

    int client_socket;
    int len_client = sizeof(client_add);
    while ((client_socket = accept(server_socket, (sockaddr *)&client_add, (socklen_t *)(&len_client))) >= 0)
    {
        cout << "[+] Connection recieved from " << inet_ntoa(client_add.sin_addr) << ":" << ntohs(client_add.sin_port) << "\n";
        pthread_create(&thread_id, NULL, client_thread_handler, (void *)&client_socket);
    }

    shutdown(server_socket, SHUT_RDWR);
    return nullptr;
}
void printmsg(string msg, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());

    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, msg.c_str(), sizeof(msg), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}
void create_user(string input, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}
void login(string input, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}

void create_group(string input, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}

void join_group(string input, string detail)
{
    // input:join_group|gid
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    string response_t;
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    response_t = response;
    cout << response_t.substr(2) << "\n"; // uname,ip:port  ---------->owner of group from recieved from tracker

    int flag = stoi(response_t.substr(0, 1));
    if (flag)
    {
        response_t = response_t.substr(2);
        index = response_t.find(",");
        int index1 = response_t.find(":");
        string uname = response_t.substr(0, index);
        string ip_owner = response_t.substr(index + 1, index1 - index - 1);
        int port_owner = stoi(response_t.substr(index1 + 1));
        cout << uname << "\n " << ip_owner << "\n " << port_owner << "\n";
        shutdown(server_socket, SHUT_RDWR);

        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error\n");
            return;
        }

        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port_owner);
        server_address.sin_addr.s_addr = inet_addr(ip_owner.c_str());

        cout << "Connecting " << ip_owner << " : " << port_owner << " for group joining request\n";

        if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("\nConnection Failed \n");
            return;
        }

        string newinput = "";
        int i = input.find("|");
        int j = input.find(",");

        string gid = input.substr(i + 1, j - i - 1);
        newinput = "join_group|" + uname + "|" + gid;
        cout << newinput << "\n";
        send(server_socket, newinput.c_str(), sizeof(newinput), 0);
        string req = "request sent to owner";
        cout << req << "\n";
        char newresponse[256];
        bzero(newresponse, 256);
        recv(server_socket, &newresponse, sizeof(newresponse), 0);
        cout << newresponse << "\n";
        shutdown(server_socket, SHUT_RDWR);
    }
    else
    {
        shutdown(server_socket, SHUT_RDWR);
    }
}

void leave_group(string gid, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    string input = "leave_group|" + gid + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << "\n";
    shutdown(server_socket, SHUT_RDWR);
}

void list_requests(string gid, string detail)
{

    for (auto r : join_requests[gid])
    {
        cout << r << "\n";
    }
}

void accept_requests(string gid, string uname, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    string input = "";
    input = "accept_request|" + gid + "|" + uname + "," + detail;
    cout << input << "\n";
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << "checking response" << response << "\n";
    if (strcmp(response, "true") == 0)
    {
        cout << "Accepted successfully!!\n";
        int pos = 0;
        for (auto r : join_requests[gid])
        {
            if (strcmp(uname.c_str(), r.c_str()) == 0)
            {
                join_requests[gid].erase(join_requests[gid].begin() + pos);
                break;
            }
            pos++;
        }
    }
    else
    {
        cout << response << "\n";
    }

    shutdown(server_socket, SHUT_RDWR);
}

void list_groups(string input, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}

void list_files(string input, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}
string SHA_file(string fpath, unsigned long long int fsize)
{
    char *file_buffer;
    unsigned char *result = new unsigned char[20];
    int fd = open(fpath.c_str(), O_RDONLY);
    file_buffer = (char *)mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);
    SHA1((unsigned char *)file_buffer, fsize, result);
    munmap(file_buffer, fsize);
    close(fd);
    char *sha1hash = (char *)malloc(sizeof(char) * 41);
    sha1hash[41] = '\0';
    int i;
    for (i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        sprintf(&sha1hash[i * 2], "%02x", result[i]);
    }
    string calculated_hash(sha1hash);
    return calculated_hash;
}

void upload_file(string fpath, string gid, string detail)
{
    cout << fpath;
    ifstream fin(fpath, ios::binary);
    fin.seekg(0, ios::end);
    int fsize = fin.tellg();
    cout << "\nsize of file is: " << fsize << " bytes \n";
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    string sha = SHA_file(fpath, fsize);
    string input = "upload_file|" + fpath + "|" + gid + "|" + to_string(fsize) + "|" + sha + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}

/*void *chunk_client(void *param)
{
char *msg=(char *)param;
string request(msg);
cout<<msg<<"\n";
vector<string> data;
stringstream info(request);
string line;
while(getline(info,line,":")){
    data.push_back(line);
}
int len=data.size();
string ip=data[0];
string port=data[1];
string chunk_no=data[2];
string fpath=data[3];
string dpath=data[4];

string crequest="download_file:"+chunk_no+":"fpath;
char req_client[8192];
strcpy(req_client,crequest.c_str());

int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return NULL;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip_address.c_str());

    char response[8192] = {0};

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return NULL;
    }

    send(server_socket, req_client, sizeof(req_client), 0);

    bzero(response, 8192);
    recv(server_socket, &response, sizeof(response), 0);
    cout <<response << endl;

    string buffer(response);
    int pos = buffer.find(":");
    int val = stoi(buffer.substr(0, pos));
    string chunk_data = buffer.substr(pos + 1);
    cout << "CHUNK data to be written to file" << chunk_data << endl;
    fstream dfpr;
    dfpr.open(dpath, ios::out | ios::binary | ios::in);
    dfpr.seekg(stoi(chunk_no) * CHUNK_SIZE, dfpr.beg);
    dfpr.write(chunk_data.c_str(), val);
    dfpr.close();

    shutdown(server_socket, SHUT_RDWR);

    return NULL;


}*/

int download_file(string gid, string fname, string dpath, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    string input = "download_file|" + gid + "|" + fname + "|" + dpath + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return 0;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return 0;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[8192];
    bzero(response, 8192);
    // string response;
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << "\n";
    string response_t;
    response_t = response;

    vector<string> data;
    string info(response);
    stringstream dinfo(info);
    string s;
    cout<<"---------------data--------------------\n";
    while (getline(dinfo, s, ':'))
        {data.push_back(s);
        //cout<<s<<"\n";
        }

    int len = data.size();
    int requestor = len - 4;

    cout << requestor << " requestors\n";
    if (requestor == 0)
    {
        cout << "No one is online\n";
        shutdown(server_socket, SHUT_RDWR);
        return 0;
    }
    else
    {
        string ip_owner = data[len - 1];
        string fpath = data[len - 2];
        string filesize = data[len - 3];
        string isha = data[len - 4];
        int port_owner = stoi(data[len - 5]);
        cout << port_owner << "\n"<<filesize<<"\n";

        shutdown(server_socket, SHUT_RDWR);

        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error\n");
            return 0;
        }

        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port_owner);
        server_address.sin_addr.s_addr = inet_addr(ip_owner.c_str());

        cout << "Connecting " << ip_owner << " : " << port_owner << " downloading request\n";

        if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            printf("\nConnection Failed \n");
            return 0;
        }

        string newinput = "";
        int i = input.find("|");
        int j = input.find(",");

        string gid = input.substr(i + 1, j - i - 1);
        newinput = "download_file|" + fpath + "|"+ dpath+"|"+ filesize;
        cout << newinput << "\n";
        send(server_socket, newinput.c_str(), newinput.length(), 0);
        string req = "request sent to owner";
        cout << req << "\n";
        int fsize = stoi(filesize);
        //cout<<filesize<<"\n"<<"now fsize from string to integer "<<fsize<<"\n";
        char *newresponse=new char[fsize];
        bzero(newresponse, fsize);
       // cout<<"size of new response"<<sizeof(newresponse)<<"\n";
        recv(server_socket, newresponse, fsize,256);
       // cout<<newresponse<<"\n"<<sizeof(newresponse)<<"\n";
        ofstream f(dpath);
        f.write(newresponse, fsize);
        f.close();
        shutdown(server_socket, SHUT_RDWR);

        ifstream ofile(dpath, ios::binary);
        ofile.seekg(0, ios::end);
        int osize = ofile.tellg();
        cout << "output file size " << osize << "\n";
        string osha = SHA_file(dpath, osize);

        if (strcmp(osha.c_str(), isha.c_str()) == 0)
        {
            cout << "File Downloaded successfully!!\n";
            return 1;
        }
        else
        {
            cout << "Error"
                 << "\n";
            return 0;
        }
    }
//--------------------------------------------------------------------------------------
    // string line;
    // ifstream in(fpath);

    // // Output file stream object to
    // // write to file2.txt
    // ofstream f(dpath);

    // // Reading file.txt completely using
    // // END OF FILE eof() method
    // in.seekg(0,ios::end);
    // int ssize=in.tellg();
    // in.seekg(0);
    // char *buffer=new char[ssize];
    // in.read(buffer,ssize);
    // cout<<buffer<<"\n"<<sizeof(buffer);
    // f.write(buffer,ssize);
    // in.close();
    // f.close();
    /* int chunks_qty=ceil(stoi(fsize)/float(CHUNK_SIZE));

     pthread_t tid[chunks_qty];

     for(int i=0;i<chunks_qty;i++){
         string request="127.0.0.1:"+data[i%requestor]+":"+to_string(i)+":"+fpath+":"+dpath;

         cout<<"Request after downloding "<<request<<"\n";
         pthread_create(&tid[i],NULL,chunk_client,(void *)request);
         pthread_join(tid[i],NULL);
     }*/

     //-----------------------------------------------------------------------------------------------------------------
        // ifstream ofile(dpath,ios::binary);
        // ofile.seekg(0,ios::end);
        // int osize=ofile.tellg();
        // cout<<"file size"<<osize<<"\n";
        // string osha = SHA_file(dpath, osize);
        //  if (strcmp(osha.c_str(), isha.c_str()) == 0)
        // {
        //     cout << "File Downloaded successfully!!";
        //     return 1;
        // }
        // else
        // {
        //     cout << "Error"
        //          << "\n";
        //     return 0;
        // }
}


void logout(string input, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}

void show_downloads(string input, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    input = input + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[256];
    bzero(response, 256);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << endl;
    shutdown(server_socket, SHUT_RDWR);
}

void stop_share(string gid, string fname, string detail)
{
    int index = detail.find(":");
    string ip = detail.substr(0, index);
    string port = detail.substr(index + 1);
    int port_no = atoi(port.c_str());
    string input = "stop_share|" + gid + "|" + fname + "," + detail;
    char request[8192];
    strcpy(request, input.c_str());
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error\n");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_tracker);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(server_socket, request, sizeof(request), 0);
    char response[8192];
    bzero(response, 8192);
    recv(server_socket, &response, sizeof(response), 0);
    cout << response << "\n";
    if (strcmp(response, "stop") == 0)
    {
        cout << "stopped sharing " << fname << "\n";
    }
    else
    {
        cout << "unable to stop sharing/file not found\n";
    }
    shutdown(server_socket, SHUT_RDWR);
}
int main(int argc, char **args)
{
    string client_details = args[1];
    // cout<<args[1];
    cout << "client details : " << client_details << "\n";
    // cout<<client_details.c_str();
    pthread_t client_server;
    pthread_create(&client_server, NULL, server_thread_handler, (void *)client_details.c_str());
    while (1)
    {
        string input, cmd;
        cin >> input;
        if (strcmp(input.c_str(), "create_user") == 0)
        {
            //cout << "valid command " << input << "\n";
            string userid, password;
            cin >> userid >> password;
            create_user(input + "|" + userid + "|" + password, client_details);
        }
        else if (strcmp(input.c_str(), "login") == 0)
        {
           // cout << "valid command" << cmd << "\n";
            string userid, password;
            cin >> userid >> password;
            login(input + "|" + userid + "|" + password, client_details);
        }
        else if (strcmp(input.c_str(), "create_group") == 0)
        {
          //  cout << "valid command" << cmd << "\n";
            string gid;
            cin >> gid;
            create_group(input + "|" + gid, client_details);
        }
        else if (strcmp(input.c_str(), "join_group") == 0)
        {
            //cout << "valid command" << cmd << "\n";
            string gid;
            cin >> gid;
            join_group(input + "|" + gid, client_details);
        }
        else if (strcmp(input.c_str(), "leave_group") == 0)
        {
           // cout << "valid command" << cmd << "\n";
            string gid;
            cin >> gid;
            leave_group(gid, client_details);
        }
        else if (strcmp(input.c_str(), "list_requests") == 0)
        {
            //cout << "valid command" << cmd << "\n";
            string gid;
            cin >> gid;
            list_requests(gid, client_details);
        }
        else if (strcmp(input.c_str(), "accept_request") == 0)
        {
            //cout << "valid command" << cmd << "\n";
            string gid, uname;
            cin >> gid >> uname;
            accept_requests(gid, uname, client_details);
        }
        else if (strcmp(input.c_str(), "list_groups") == 0)
        {
           // cout << "valid command" << cmd << "\n";
            list_groups(input + "|", client_details);
        }
        else if (strcmp(input.c_str(), "list_files") == 0)
        {
           // cout << "valid command" << cmd << "\n";
            string gid;
            cin >> gid;
            list_files(input + "|" + gid, client_details);
        }
        else if (strcmp(input.c_str(), "upload_file") == 0)
        {
            //cout << "valid command" << cmd << "\n";
            string fpath, gid;
            cin >> fpath >> gid;
            upload_file(fpath, gid, client_details);
        }
        else if (strcmp(input.c_str(), "download_file") == 0)
        {
           // cout << "valid command" << cmd << "\n";
            string gid, fname, dpath;
            cin >> gid >> fname >> dpath;
            files_downloading[gid].push_back(dpath);
            int x = download_file(gid, fname, dpath, client_details);
            if (x)
            {
                int count = 0;
                // cout<<"here\n";
                for (auto f : files_downloading[gid])
                {
                    if (f == dpath)
                    {
                        files_downloading[gid].erase(files_downloading[gid].begin() + count);
                        files_completed[gid].push_back(dpath);
                        break;
                    }
                    count++;
                }
              //  cout<<"Downloaded\n";
               // upload_file(dpath, gid, client_details);
            }
            else
            {
                cout << "Already downloaded\n";
            }
        }
        else if (strcmp(input.c_str(), "logout") == 0)
        {
           // cout << "valid command" << cmd << "\n";
            logout(input + "|", client_details);
        }
        else if (strcmp(input.c_str(), "show_downloads") == 0)
        {
           // cout << "valid command" << cmd << "\n";
            for (auto f : files_downloading)
            {
                for (auto c : f.second)
                {
                    cout << "[D]\t" << f.first << "\t" << c << "\n";
                }
            }
            for (auto f : files_completed)
            {
                for (auto c : f.second)
                {
                    cout << "[C]\t" << f.first << "\t" << c << "\n";
                }
            }
        }
        else if (strcmp(input.c_str(), "stop_share") == 0)
        {
            //cout << "valid command" << cmd << "\n";
            string gid, fname;
            cin >> gid >> fname;
            stop_share(gid, fname, client_details);
        }
        else
        {
            cout << "invalid command\n";
        }
    }
    return 0;
}