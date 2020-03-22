# instant-messenger
### Messenger application in C++

##### How to run the application
1. First you have to build the project. In terminal open `instant-messenger` folder and run:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

2. To run chat application first you need to start the server.
In terminal open `instant-messenger` folder and run command with a chosen port number:
```
$ server/bin/Server <portNumber>
```

3. In new terminal open `instant-messenger` folder.
Then start the client and give it IP address (IPv4) and port number of the server:
```
$ client/bin/Client <ipAdress> <portNumber>
```

4. To test if the chat is working, start another client the same way.