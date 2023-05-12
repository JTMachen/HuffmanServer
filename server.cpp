#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

using namespace std;

class HuffmanTreeNode {
    public:
        char data;
        int freq;
        HuffmanTreeNode* left;
        HuffmanTreeNode* right;
        HuffmanTreeNode(char character, int frequency) {
            data = character;
            freq = frequency;
            left = right = nullptr;
        }
};
vector<HuffmanTreeNode*> generateTree(vector<HuffmanTreeNode*> nodes) {
    while (nodes.size() != 1) {
        HuffmanTreeNode *tempNode1 = nodes[0];
        HuffmanTreeNode *tempNode2 = nodes[1];
        nodes.erase(nodes.begin());
        nodes.erase(nodes.begin());
        HuffmanTreeNode *newNode = new HuffmanTreeNode('$', tempNode1->freq + tempNode2->freq);
        newNode->left = tempNode1;
        newNode->right = tempNode2;

        for (int i = 0; i < nodes.size() + 1; ++i) {
            if (newNode->freq <= nodes[i]->freq) {
                nodes.insert(nodes.begin() + i, newNode);
                break;
            }
            else if (i == nodes.size()) {
                nodes.insert(nodes.begin() + i, newNode);
                break;
            }
        }
    }
    return nodes;
}
void printCodes(HuffmanTreeNode* root, string huffCode) {
    if (root->left == nullptr) {
        cout << "Symbol: " << root->data << ", Frequency: " << root->freq << ", Code: " << huffCode << endl;
    }
    else {
        printCodes(root->left, huffCode += "0");
        huffCode = huffCode.substr(0,huffCode.size() - 1);
        printCodes(root->right, huffCode += "1");
    }
}
char returnChar(string huffCode, HuffmanTreeNode* root, char value) {
    if (root->left == nullptr) {
        value = root->data;
        return value;
    }
    else {
        if (huffCode.substr(0,1) == "0") {
            value = returnChar(huffCode.substr(1), root->left, value);
        }
        else {
            value = returnChar(huffCode.substr(1), root->right, value);
        }
    }
    return value;
}

int main(int argc, char *argv[]) {
    char character, data[50];
    int freq[50], frequency, size;
    size = 0;
    string tempLine;
    while (getline(cin,tempLine)) {
        data[size] = tempLine[0];
        freq[size] = int(tempLine[2]) - 48;
        size ++;
    }

    int tempInt;
    char tempChar;
    for (int i = 0; i < size; ++i) {
        for(int j = i + 1; j < size; ++j) {
            if (freq[i] > freq[j] || (freq[i] == freq[j] && data[i] > data[j])) {
                tempInt = freq[i];
                freq[i] = freq[j];
                freq[j] = tempInt;
                tempChar = data[i];
                data[i] = data[j];
                data[j] = tempChar;
            }
        }
    }
    vector<HuffmanTreeNode*> nodes;
    for (int i = 0; i < size; ++i) {
        HuffmanTreeNode* newNode = new HuffmanTreeNode(data[i], freq[i]);
        nodes.push_back(newNode);
    }
    nodes = generateTree(nodes);
    string arr[100];
    string huffCodeTree = "";
    printCodes(nodes[0], huffCodeTree);

    int sockfd, newsockfd,portno, clilen;
    char outChar;
    sockaddr_in servAddr, cliAddr;
    int n;
    if (argc < 2) {
        cout << "ERROR, no port provided" << endl;
        return -1;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout <<"ERROR opening socket" << endl;
        return -1;
    }
    bzero((char *)&servAddr, sizeof(servAddr));
    portno = atoi(argv[1]);
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(portno);
    if (bind(sockfd, (sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        cout << "ERROR on binding" << endl;
        return -1;
    }
    listen(sockfd, size);
    clilen = sizeof(cliAddr);
    int pid;
    for(int p = 0; p < size; ++p){
        pid = fork();
        if(pid == 0) {
            newsockfd = accept(sockfd, (sockaddr *)&cliAddr, (socklen_t *)&clilen);
            if (newsockfd < 0) {
                cout << "ERROR on accept" << endl;
                return -1;
            }
            char huffCode[256];
            n = read(newsockfd, huffCode, 256);
            if (n < 0) {
                cout << "ERROR reading from socket" << endl;
                return -1;
            }
            int huffCodeSize = 0;
            for(int i = 0; i < 256; ++i) {
                if (huffCode[i] != '\0') {
                    huffCodeSize++;
                }
            }
            char newChar[1];
            string tempCode = "";
            for(int i = 0; i < huffCodeSize - 1; ++i) {
                tempCode = tempCode + huffCode[i];
            }
            newChar[0] = returnChar((tempCode), nodes[0], '3');
            if (n < 0) {
                cout << "ERROR writing to socket" << endl;
                return -1;
            }
            send(newsockfd, newChar, 1, 0);
            _exit(0);
        }
    }
    close(newsockfd);
    _exit(0);
    return 0;
}