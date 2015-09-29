# TTP3121---Project
TTP3121 - Project
# TCP-Project

Team Name: TheImp

Student Name:
 
Tan Ka Leong
Lim Kok Keong

Student ID: 

1131121895
1121118986

************************************************************

Development of File Repository System using Client-Server TCP/IP Model

File repository system is used for user file storage. So user is character Client, will connect to the file server using socket connection while server have create the port. However, server should be able to handle multiple client connection requests as many as the same time. Then client can send file to the server, and server will automatically store the file in specific user’s directory. In additional, client could also perform other tasks, e.g. create, copy, and delete directory. Client could also request for files to be downloaded from the server. During the data transmission, the program should be clear of any interruption. 

************************************************************

User manual:

1. Compile client.c and server.c (gcc filename.c -o filename)
2. Run both files on different terminal/pc (server.c must be run before client.c)
3. Choose the option 1.create 2.Download 3.Send 4.Delete 5.Exit (server.c can view client.c function)

For example:

Terminal 1 is to create server and create port:
./server portNumber


Terminal 2 is to create client to access to server port:
 ./client serverHostName portNumber



