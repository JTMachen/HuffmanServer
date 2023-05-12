#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

using namespace std;

struct finalString {
    string final = "";
    char neededChar;
    int size, pos[50], sockfd;
    string huffCode;
    int argc; 
    char **argv;
};


void *addToString (void* void_ptr) {
    finalString *tree_ptr = (finalString *)void_ptr;
    int sockfd, portno, readChar;
    sockaddr_in servAddr;
    hostent *server;
    if (tree_ptr->argc < 3) {
        cerr << "Usage " << tree_ptr->argv[0] << " hostname port" << endl;
    }
    portno = atoi(tree_ptr->argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "ERROR openeing socket" << endl;
    }
    server = gethostbyname(tree_ptr->argv[1]);
    if (server == NULL) {
        cerr << "ERROR, no such host" << endl;
    }
    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&servAddr.sin_addr.s_addr, server->h_length);
    servAddr.sin_port = htons(portno);
    if (connect(sockfd,(const sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        cerr << "ERROR connecting" << endl;
    }
    int n;
    char huffCode[256];
    strcpy(huffCode, tree_ptr->huffCode.c_str());
    n = write(sockfd, huffCode, 8);
    if (n < 0) {
        cerr << "ERROR writing to socket";
    }
    send(sockfd, huffCode, 256, 0);
    char newChar[1];
    n = read(sockfd, newChar, 1);
    if (n < 0) {
        cerr << "HEAROJ ERROR reading from socket\n";
        return nullptr;
    }
    tree_ptr->neededChar = newChar[0];
    for(int i = 0; i < tree_ptr->size; ++i) {
        if(i > 0 && tree_ptr->pos[i] == 0) {
            break;
        }
        tree_ptr->final[tree_ptr->pos[i]] = tree_ptr->neededChar;
    }
    close(sockfd);
    return nullptr;
}

int main(int argc, char *argv[]) {
    string input;
    string huffCode, codes[50];
    string ints;
    int pos[50][50];
    int counts[50], totalCount = 0, stringLength;
    int position;
    while (getline(cin,input)) {
        codes[totalCount] = input.substr(0,input.find(' '));
        input.erase(0,input.find(' ') + 1);
        size_t tempCount = 0;
        while ((position = input.find(' ')) != string::npos) {
            pos[totalCount][tempCount] = stoi(input.substr(0,input.find(' ')));
            input.erase(0,input.find(' ') + 1);
            tempCount++;
        }
        pos[totalCount][tempCount] = stoi(input);
        tempCount ++;
        counts[totalCount] = tempCount;
        stringLength += tempCount;
        totalCount++;
    }


    
    // Create pthread, run through the shit, and compile into one string
    string final = "";
    finalString * arg = new finalString[totalCount];
    pthread_t *tid = new pthread_t[totalCount];
    for(int i = 0; i < totalCount; ++i) {
        arg[i].huffCode = codes[i];
        arg[i].argc = argc;
        arg[i].argv = argv;
        for (int j = 0; j < counts[i]; ++j) {
            arg[i].pos[j] = pos[i][j];
        }
        for(int k = 0; k < stringLength; ++ k) {
            arg[i].final += '\0';
        }
        arg[i].size = counts[i];
        if(pthread_create(&tid[i],NULL,addToString,&arg[i])) {
            cout << "Error creating thread" << endl;
            return -1;
        }
    }
    
    for(int i = 0; i < totalCount; ++i) {
        pthread_join(tid[i],NULL);
    }
    final = arg[0].final;
    for(int i = 0; i < totalCount; ++i) {
        string tempString = arg[i].final;
        for(int j = 0; j < tempString.size(); ++j) {
            if(tempString[j] != '\0') {
                final[j] = tempString[j];
            }
        }
    }
    cout << "Original message: " << final << endl;
    
    return 0;
}