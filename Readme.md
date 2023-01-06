# client-tracker
File sharing system having one tracker only running on port 9002.
tracker_info.txt contains ip:port of two trackers (9002,9003)

## steps/Commands
- cd tracker
- compile the tracker using the following commnad - 
-       g++ tracker.cpp -o tracker -lcrypto -lpthread

- run the tracker using following command -
-      ./tracker tracker_info.txt 1 (here 1 define first tracker address)
- cd client
- compile the client using the following command - 
-       g++ client.cpp -o client -lcrypto -lpthread

- run the client using following command-
-       ./client 127.0.0.1:port_number


- Now register yourself as a peer using following command-
-       create_user user_name password

- Now login with your credentials using the following command-
-       login user_name password

- Now either create a new group or join a pre-existing group

- To create a new group use the following command-
-       create_group group_id

- Or to join pre-existing group first fetch the list of all the available group using the following command-
-       list_groups

-   now select the group id of the group you want to join and join that group using the following command-
-       join_group group_id

- once the join_group command is executed then the request will be sent to creater of the group.
- After receving the request admin of the group wil check the available request using the following command and this command returns the user_name_of_requesting_peer
-       list_requests group_id

-   To accept any request from all the available request use the following command-
-       accept_request group_id user_name_of_requesting_peer

-  To upload file in a group use the following command-
-       upload_file <file_path> <group_id>

-  To get the list of all available files in a group
-       list_files <group_id>

-   To download file from a group-
-       download_file <group_id> <file_name> <destination_path>

-   To see the status of dowload whether it is completed or not use the following command
-       show_downloads

-  To stop sharing a file use following command-
-       stop_share <group_id> <file_name>

-  To  leave a group use the following command -
-       leave_group group_id

- To quit the tracker use the follwoing command-
    quit

## Example commands
- - eg:/client 127.0.0.1:4444
       /client 127.0.0.1:5555
- -   create_user abc abc
      create_user def def
      login abc abc
      create_group g1
      list_groups
      join_group g1
      list_requests g1
      accept_request g1 abd
      upload_file 1.png g1
      list_files g1
      download_file g1 1.png ../tracker/2.png
      show_downloads
      stop_share g1 1.png
      leave_group g1
      quit





      
