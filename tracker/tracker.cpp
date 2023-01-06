#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <bits/stdc++.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;
struct client
{
    string uname, passowrd, ip;
    int port;
    bool islogin;
    map<string, int> group; // gid,1 if owner
    vector<string>uploaded_files;
    vector<string>downloaded_files;
};
struct file{
    string filename,filepath,filesize,uname,gid,sha;
};
map<string, client> clients;               // uname, details
map<int, string> client_port;              // port number,uname
map<string, string> group_owner;           // group id and owner uname
map<string, vector<string>> user_in_group; // group id and list of uname present in group
map<string, vector<file>> file_details_in_group;  
map<string, vector<pair<string, string>>> clients_having_file;
string findcommand(string input)
{
    int index = input.find_first_of("|");
    input = input.substr(0, index);
    cout << input << input.length() << "\n";
    return input;
}
void *clientHandler(void *arg)
{
    cout << "[+] Connection Request Accepted"
         << "\n";
    int client_socket = *(int *)arg;
    char client_request[8192];
    bzero(client_request, 8192);
    string response;
    struct client c;
    recv(client_socket, &client_request, sizeof(client_request), 0);
    cout << "client Request: " << client_request << "\n";
    string input = client_request;
    int index = input.find_first_of("|");
    string cmd = input.substr(0, index);
    cout<<"command is "<<cmd<<"\n";
    string next;
    if (strcmp(cmd.c_str(), "create_user") == 0)
    {
        // cout<<"here";
        index = input.find_first_of("|");
        // cout<<index<<"\n";
        int index2 = input.find_last_of("|");
        int index3 = input.find(",");
        // cout<<index2<<"\n";
        string uname = input.substr(index + 1, index2 - index - 1);
        string pass = input.substr(index2 + 1, index3 - index2 - 1);
        index2 = input.find(",");
        index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << uname << "\n"
             << pass << "\n"
             << ip << "\n"
             << port << "\n";
        c.uname = uname;
        c.passowrd = pass;
        c.islogin = false;
        c.ip = ip;
        c.port = port;
        if (clients.find(c.uname) != clients.end())
        { //|| client_port.find(c.port)!=client_port.end()){//can add if that port is already exist or not
            response = "Account exist already!!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else
        {
            clients[c.uname] = c;
            client_port[c.port] = c.uname;
            response = "User created successfully!!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
    }

    else if (strcmp(cmd.c_str(), "login") == 0)
    {
        // cout<<"here";
        index = input.find_first_of("|");
        // cout<<index<<"\n";
        int index2 = input.find_last_of("|");
        int index3 = input.find(",");
        // cout<<index2<<"\n";
        string uname = input.substr(index + 1, index2 - index - 1);
        string pass = input.substr(index2 + 1, index3 - index2 - 1);
        index2 = input.find(",");
        index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << uname << "\n"
             << pass << "\n"
             << ip << "\n"
             << port << "\n";

        if (clients.find(uname) != clients.end())
        { //|| client_port.find(c.port)!=client_port.end()){//can add if that port is already exist or not
            if (clients[uname].islogin == true)
            {
                response = "Already Logged in!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
            else
            {
                if (clients[uname].passowrd == pass)
                {
                    clients[uname].islogin = true;
                    response = "Login successfully!!";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else
                {
                    response = "Password don't match";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
            }
        }
        else
        {
            response = "User doesn't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
    }
    else if (strcmp(cmd.c_str(), "create_group") == 0)
    {

        index = input.find_first_of("|");
        // cout<<index<<"\n";
        // int index2=input.find_last_of("|");
        int index3 = input.find(",");
        // cout<<index2<<"\n";
        string gid = input.substr(index + 1, index3 - index - 1);
        // string pass=input.substr(index2+1,index3-index2-1);
        int index2 = input.find(",");
        index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << gid << "\n"
             << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        cout<<uname<<"\n";
        if (clients.find(uname) == clients.end())
        {
            response = "User doesn't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else
        {
            if (clients[uname].islogin == true)
            {
                if (group_owner.find(gid) != group_owner.end())
                {
                    response = "group with " + gid + " already exists!!";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else
                {
                    group_owner[gid] = uname;
                    clients[uname].group[gid] = 1;
                    user_in_group[gid].push_back(uname);
                    response = "Group created successfully!!";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
            }
            else
            {
                response = "Login to create group!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
        }
    }
    else if (strcmp(cmd.c_str(), "join_group") == 0)
    {
        index = input.find_first_of("|");
        // cout<<index<<"\n";
        // int index2=input.find_last_of("|");
        int flag=0;
        int index3 = input.find(",");
        // cout<<index2<<"\n";
        string gid = input.substr(index + 1, index3 - index - 1);
        // string pass=input.substr(index2+1,index3-index2-1);
        int index2 = input.find(",");
        index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << gid << "\n"
             << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        if (clients.find(uname) == clients.end())
        {
            response = "0|User doesn't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else
        {
            if (clients[uname].islogin == true)
            {
                if (group_owner.find(gid) == group_owner.end())
                {
                    response = "0|Group id doesn't exists!!";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else
                {
                    string owner_uname = group_owner[gid];
                    string ownerip = clients[owner_uname].ip;
                    int owner_port = clients[owner_uname].port;
                    string res="1|"+uname+","+ownerip+":"+to_string(owner_port); 
                    cout<<res<<"\n";
                    send(client_socket,res.c_str(),sizeof(res),0);
                 }
            }
            else
            {
                response = "0|Login to join group!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
        }
    }
    else if (strcmp(cmd.c_str(), "leave_group") == 0)
    {
        index = input.find_first_of("|");
        // cout<<index<<"\n";
        // int index2=input.find_last_of("|");
        int index3 = input.find(",");
        // cout<<index2<<"\n";
        string gid = input.substr(index + 1, index3 - index - 1);
        // string pass=input.substr(index2+1,index3-index2-1);
        int index2 = input.find(",");
        index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << gid << "\n"
             << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        if(clients.find(uname)==clients.end()){
            response = "User doesn't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else{
            if(clients[uname].islogin==true){
                if(group_owner.find(gid)!=group_owner.end()){
                    string owner=group_owner[gid];
                    if(strcmp(owner.c_str(),uname.c_str())==0){
                        cout<<"Deleting Owner.........";
                        group_owner.erase(gid);//delete group
                        user_in_group.erase(gid);//delete members of group
                        for(auto c:clients){
                            if(((c.second).group).find(gid)!=((c.second).group).end()){
                                 c.second.group.erase(gid);
                            }
                        }
                    }
                    else{
                        bool flag=false;
                        int i=0;
                        for(auto u:user_in_group[gid]){
                            if(u==uname){
                                flag=true;
                                break;
                            }
                            i++;
                        }
                        if(flag==true){
                            user_in_group[gid].erase(user_in_group[gid].begin()+i);
                        }
                        else{
                            response="User not in this group";
                            send(client_socket, response.c_str(), sizeof(response), 0);

                        }
                        clients[uname].group.erase(gid);
                    }
                    response="Left the group\n";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else{
                    response="group id don't exist\n";
                    send(client_socket, response.c_str(), sizeof(response), 0);

                }
            }
            else{
                response="Login to leave the group\n";
                    send(client_socket, response.c_str(), sizeof(response), 0);
            }

            
        }

    }
    else if (strcmp(cmd.c_str(), "list_requests") == 0)
    {
        //for(auto r: reques)
    }
    else if (strcmp(cmd.c_str(), "accept_request") == 0)
    {

        index = input.find_first_of("|");
        // cout<<index<<"\n";
        int index2 = input.find_last_of("|");
        int index3 = input.find(",");
        // cout<<index2<<"\n";
        string gid = input.substr(index + 1, index2 - index - 1);
        string uname = input.substr(index2 + 1, index3 - index2 - 1);
        index2 = input.find(",");
        index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        string owner=client_port[port];
        cout << gid << "\n"
             << uname << "\n"
             << ip << "\n"
             << port << "\n";
        response = "";
        if (clients.find(owner) == clients.end())
        {
            response = "User doesn't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else
        {
            if (clients[owner].islogin == true)
            {
                if (group_owner.find(gid) != group_owner.end())
                {
                    user_in_group[gid].push_back(uname);
                    clients[uname].group[gid] = 0;
                    response = "true";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else
                {
                    response = "Group id doesn't exist !!";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
            }
            else
            {
                response = "Login first !!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
        }
    }
    else if (strcmp(cmd.c_str(),"list_groups") == 0)
    {
        int index2 = input.find(",");
        int index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        response = "";
        if (clients.find(uname) == clients.end())
        {
            response = "User id don't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else
        {
            if (clients[uname].islogin == true)
            {
                if (group_owner.size() == 0)
                {
                    response = "No group exist!!";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else
                {
                    for (auto g : group_owner)
                    {
                        response += g.first;
                        response += "\n";
                    }
                    cout<<"group lists";
                    cout<<response<<"\n";
                    send(client_socket, response.c_str(), sizeof(response), 0);
                }
            }
            else
            {
                response = "You are not login!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
        }
    }
    else if (strcmp(cmd.c_str(), "list_files") == 0)
    {
        string listfiles="";
        index = input.find_first_of("|");
        // cout<<index<<"\n";
        // int index2=input.find_last_of("|");
        int index3 = input.find(",");
        // cout<<index2<<"\n";
        string gid = input.substr(index + 1, index3 - index - 1);
        // string pass=input.substr(index2+1,index3-index2-1);
        int index2 = input.find(",");
        index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << gid << "\n"
             << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        
        if(clients.find(uname)==clients.end()){
        response = "User id don't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else{
            if(clients[uname].islogin==true){
                if(group_owner.find(gid)!=group_owner.end()){
                 for(auto f:file_details_in_group[gid]){
                    listfiles+=f.filename;
                    listfiles+="\n";
                 }
            send(client_socket, listfiles.c_str(), sizeof(listfiles), 0);
                }
                else{
                    response = "group don't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
                }

            }
            else{
                response = "Login first!!";
            send(client_socket, response.c_str(), sizeof(response), 0);
            }

        }

    }
    else if (strcmp(cmd.c_str(), "upload_file") == 0)
    {
        //"upload_file|"+fpath+"|"+gid+"|"+to_string(fsize)+"|"+sha+ "," + detail;
       cout<<input<<"\n";
       int index2 = input.find(",");
        int index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        cout<<uname<<"\n";
        index3=input.find_last_of("|");
        string sha=input.substr(index3+1,index2-index3-1);//sha
        input=input.substr(0,index3);
        index3=input.find_last_of("|");
        string fsize=input.substr(index3+1);
        input=input.substr(0,index3);
        index3=input.find_last_of("|");
        string gid=input.substr(index3+1);
        input=input.substr(0,index3);
        index3=input.find_last_of("|");
        string fpath=input.substr(index3+1);
        input=input.substr(0,index3);
       // cout<<ip<<"\n"<<port<<"\n"<<uname<<"\n"<<sha<<"\n"<<fsize<<"\n"<<gid<<"\n"<<fpath<<"\n";
        string fname=fpath.substr(fpath.find_last_of("/")+1);
        cout<<fname<<"\n";
        if (clients.find(uname) == clients.end())
        {
            response = "User id don't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else
        {
            if (clients[uname].islogin == false)
            {
                response = "Not Logged in!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
            else
            {
                if(group_owner.find(gid)==group_owner.end()){
                    response = "group id not exist!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else{
                    if(find(user_in_group[gid].begin(),user_in_group[gid].end(),uname)==user_in_group[gid].end()){
                        response = "kindly raise request to join the group!!";
                        send(client_socket, response.c_str(), sizeof(response), 0);
                    }
                    else{
                        struct file file_to_upload;
                        file_to_upload.filepath=fpath;
                        file_to_upload.filesize=fsize;
                        file_to_upload.filename=fname;
                        file_to_upload.gid=gid;
                        file_to_upload.sha=sha;
                        file_to_upload.uname=uname;
                        int found=0;
                        file_details_in_group[gid].push_back(file_to_upload);
                        for(auto i:clients_having_file[sha]){
                            if(i.first==uname){
                                found=1;
                        response = "file already present!!";
                        send(client_socket, response.c_str(), sizeof(response), 0);
                        break;
                            }
                        }
                        if(found==0){
                            clients_having_file[sha].push_back(make_pair(uname,gid));
                            response = "file added successfully!!";
                            send(client_socket, response.c_str(), sizeof(response), 0);

                        }



                    }
                }
            }





        }
    }
    else if (strcmp(cmd.c_str(), "download_file") == 0)
    {
       //download_file|gid|fname|dpath,127.0.0.1:4444
       int index2 = input.find(",");
        int index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        index3=input.find_last_of("|");
        string dpath=input.substr(index3+1,index2-index3-1);
        input=input.substr(0,index3);
        index3=input.find_last_of("|");
        string fname=input.substr(index3+1);
        input=input.substr(0,index3);
        index3=input.find_last_of("|");
        string gid=input.substr(index3+1);
        cout<<gid<<" "<<fname<<" "<<dpath<<"\n";
        string fpath,fsha,fsize;
        if(file_details_in_group.find(gid)==file_details_in_group.end()){

        }
        else{
            for(auto f:file_details_in_group[gid]){
                if(f.filename==fname){
                 fpath=f.filepath;
                 fsize=f.filesize;
                 fsha=f.sha;
                }
            }
        }
        cout<<fpath<<" "<<fsize<<" "<<fsha<<"\n";
        if(clients.find(uname)==clients.end()){
            response = "User id don't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
       
        }
        else{
            if (clients[uname].islogin == false)
            {
                response = "Not Logged in!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
            else{
                if(group_owner.find(gid)==group_owner.end()){
                    response = "group id not exist!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else{
                    if(find(user_in_group[gid].begin(),user_in_group[gid].end(),uname)==user_in_group[gid].end()){
                        response = "kindly raise request to join the group!!";
                        send(client_socket, response.c_str(), sizeof(response), 0);
                    }
                    else{
                        for(auto f:clients_having_file[fsha]){
                            if(f.first!=uname){
                                if(clients[f.first].islogin){
                                    response=response+to_string(clients[f.first].port)+":";
                                }
                            }
                        }
                        response=response+(fsha+":"+fsize+":"+fpath+":"+ip);
                        char resp[8192];
                        strcpy(resp,response.c_str());
                        cout<<response<<" to be sent\n size is : "<<sizeof(resp)<<"\n";
                        send(client_socket,resp,sizeof(resp),0);
                    }

                }

            }

        }



    }
    else if (strcmp(cmd.c_str(), "logout") == 0)
    {
        int index2 = input.find(",");
        int index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout << ip << "\n"
             << port << "\n";
        string uname = client_port[port];
        if (clients.find(uname) == clients.end())
        {
            response = "User id don't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else
        {
            if (clients[uname].islogin == false)
            {
                response = "Not Logged in!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
            else
            {
                clients[uname].islogin = false;
                response = "Logout successfully!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
        }
    }
    else if (strcmp(cmd.c_str(), "show_downloads") == 0)
    {
       
    }
    else if (strcmp(cmd.c_str(), "stop_share") == 0)
    {
       //stop_share|gid|fname,127.0.0.1:4000

       int index2 = input.find(",");
        int index3 = input.find(":");
        string ip = input.substr(index2 + 1, index3 - index2 - 1);
        int port = stoi(input.substr(index3 + 1, input.length()));
        cout 
             << ip << "\n"
             << port << "\n";
        string uname = client_port[port];

        index3=input.find_last_of("|");
        string fname=input.substr(index3+1,index2-index3-1);
        input=input.substr(0,index3);
        index3=input.find_last_of("|");
        string gid=input.substr(index3+1);
        cout<<fname<<"\n"<<gid<<"\n";
        string sha;

        if(file_details_in_group.find(gid)!=file_details_in_group.end()){
            for(auto f:file_details_in_group[gid]){
                if(f.filename==fname){
                    sha=f.sha;
                }
            }
        }
      
      if(clients.find(uname)==clients.end()){
         response = "User id don't exist !!";
            send(client_socket, response.c_str(), sizeof(response), 0);
        }
        else{
            if(clients[uname].islogin==false){
                response = "Not Logged in!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
            }
            else{
                if(group_owner.find(gid)==group_owner.end()){
                    response = "group id not exist!!";
                send(client_socket, response.c_str(), sizeof(response), 0);
                }
                else{
                    if(find(user_in_group[gid].begin(),user_in_group[gid].end(),uname)==user_in_group[gid].end()){
                        response = "kindly raise request to join the group!!";
                        send(client_socket, response.c_str(), sizeof(response), 0);
                    }
                    else{
                        int count=0,found=0;
                        for(auto f:clients_having_file[sha]){
                            if(f.first==uname){
                                clients_having_file[sha].erase(clients_having_file[sha].begin()+count);
                                cout<<"stopped sharing\n";
                                response="stop";
                                found=1;
                                break;
                            }
                            count++;
                        }
                        if(found==0){
                            cout<<"File not found to stop sharing\n";
                            response="no";
                        }
                        send(client_socket, response.c_str(), sizeof(response), 0);

                    }
            }
        
      }



    }
    }

    else
    {
        string msg = "not valid";
        send(client_socket, msg.c_str(), sizeof(msg), 0);
    }

    return nullptr;
}
void *tracker_quit(void *arg)
{
    string q;
    cin >> q;
    if (q == "quit")
        exit(0);
    return NULL;
}
int main(int argc, char **args)
{
    cout << "tracker running\n";
    if (argc != 3)
    {
        cout << "you have not provided correct info\n";
        exit(0);
    }
    pthread_t tid1;
    pthread_create(&tid1, NULL, tracker_quit, NULL);
    fstream tracker_info;
    string line;
    int port_no;
    tracker_info.open(args[1], ios::in);
    if (!tracker_info.is_open())
    {
        cout << "Tracker info file not opening\n";
        exit(0);
    }
    getline(tracker_info, line);
    // cout<<args[2]<<strcmp(args[2],"1")<<"\n";
    if (strcmp(args[2], "1") == 0)
    {
        int index = line.find(':');
        // cout<<index<<"\n";
        // cout<<line.substr(index+1,line.length()).c_str()<<"\n";
        port_no = atoi(line.substr(index + 1, line.length()).c_str());
    }
    else
    {
        getline(tracker_info, line);
        int index = line.find(':');
        port_no = atoi(line.substr(index + 1, line.length()).c_str());
    }
    // cout<<port_no<<"\n";

    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_no);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int bind_status = bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    listen(server_socket, 5);

    cout << "[+] Server is LISTENING at port " << port_no << "\n";

    struct sockaddr_in client_address;
    pthread_t thread_id;

    int client_socket;
    int len_client = sizeof(client_address);
    while ((client_socket = accept(server_socket, (sockaddr *)&client_address, (socklen_t *)(&len_client))) >= 0)
    {
        cout << "[+] Connection recieved from" << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << "\n";
        pthread_create(&thread_id, NULL, clientHandler, (void *)&client_socket);
    }

    shutdown(server_socket, SHUT_RDWR);
    return 0;
}