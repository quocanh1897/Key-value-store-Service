#ifndef _UTILSCLIENT_
#define _UTILSCLIENT_

#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <sstream>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <vector>
#include <time.h>

#define PORT 7779
#define IP "127.0.0.1"
#define LENGTH 512
#define NUM_THREAD 4
#define NUM_TEST 20
using namespace std;

int checkErr(int n, char *err, string succ);
int getConnection();
void printUsage();
void handleSingleClient();
void error(const char *msg);
void *threadExcute(void *paramInp);
void testMultiClient(string command);
void benchMark(string command, int num);
void testSingleCLient100(string command);

int checkErr(int n, char *err, string succ)
{
    if (n < 0)
    {
        perror(err);
        exit(1);
    }
    else
    {
        cout << succ;
    }
    return n;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void printUsage(bool error)
{
    cerr << "usage: ./Client <-S|-M> [<-s|-g|-d>]" << endl
         << "------- Listing options -------" << endl
         << "  -S: Single client test" << endl
         << "\t-S <-s|-g|-d>: Single client test specific command" << endl
         << "  In single client test: " << endl
         << "\tset <key> <value>" << endl
         << "\tget <key>" << endl
         << "\tdel <key>" << endl
         << "\tquit/exit" << endl
         << "  -M <-s|-g|-d>: Multiple client test specific command" << endl
         << "  -B <-s|-g|-d> <N>: Measure command's time with N keys" << endl
         << "\t-s: For testing SET command" << endl
         << "\t-g: For testing GET command" << endl
         << "\t-d: For testing DEL command" << endl;
    if (error)
        exit(EXIT_FAILURE);
}

int getConnection()
{
    int sockfd;
    struct sockaddr_in server_addr;

    /* Get the Socket file descriptor */
    ;
    checkErr((sockfd = socket(AF_INET, SOCK_STREAM, 0)),
             "ERROR: Failed to obtain Socket Descriptor!",
             "[CLIENT]: Obtained sockfd ");
    cout << sockfd << endl;

    /* Fill the socket address struct */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr); /*convert IPv4 into sin_addr*/
    memset(&server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    /* Try to connect the server */
    checkErr(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)),
             "ERROR: Failed to connect to the host!",
             "[Client] Connected to server at port ");
    cout << PORT << endl;
    return sockfd;
}

void handleSingleClient()
{
    printUsage(false);
    int sockfd = getConnection();
    char receiveBuff[LENGTH];

    while (1)
    {
        string sline;
        string cmd, key, val;

        cout << "\n\n(cmd): ";
        getline(cin, sline);
        stringstream ss(sline);
        ss >> cmd >> key >> val;

        if (cmd != "set" && cmd != "get" && cmd != "del" && cmd != "quit" && cmd != "exit")
        {
            cout << "\'" << cmd << "\'"
                 << " not found, please follow the ";
            printUsage(false);
            continue;
        }
        if (cmd == "quit" || cmd == "exit")
        {
            break;
        }
        /* assign string to char* to send */
        char *sendBuff = new char[sline.length() + 1];
        strcpy(sendBuff, sline.c_str());

        /* send buffer to server */
        send(sockfd, sendBuff, sizeof(cmd), 0);

        /* allocate new buffer*/
        memset(&receiveBuff, '\0', LENGTH);
        char *prbuf = receiveBuff;
        /* receive buffer from server */
        recv(sockfd, receiveBuff, LENGTH, 0);
        cout << "=> " << receiveBuff;
    }

    close(sockfd);
    cout << "[Client] Connection closed.\n";
}

/* Request 100 times command to server*/
void testSingleCLient100(string command)
{

    int sockfd = getConnection();
    char receiveBuff[LENGTH];

    for (int i = 0; i < NUM_TEST; i++)
    {
        string sendBuff = command;
        if (command == "get")
        {
            sendBuff += " key-" + to_string(i);
        }
        else if (command == "set")
        {
            sendBuff += " key-" + to_string(i) + " value-" + to_string(i);
        }
        else if (command == "del")
        {
            sendBuff += " key-" + to_string(i);
        }

        /* assign string to char* to send */
        char *tempBuff = new char[sendBuff.length() + 1];
        strcpy(tempBuff, sendBuff.c_str());

        cout << "\n(" << sockfd << ") send : " << tempBuff << endl;

        /* send buffer to server */
        send(sockfd, tempBuff, sizeof(sendBuff), 0);

        /* allocate new buffer*/
        memset(&receiveBuff, '\0', LENGTH);

        /* receive buffer from server */
        recv(sockfd, receiveBuff, LENGTH, 0);
        cout << "=> (" << i << ") " << receiveBuff << endl;
    }

    close(sockfd);
    cout << "[Client] Connection closed.\n";
}

struct param
{
    string command;
};

void *threadExcute(void *paramInp)
{
    param *p = (param *)paramInp;
    string cmd = p->command;
    testSingleCLient100(cmd);
}

void testMultiClient(string command)
{
    pthread_t task[NUM_THREAD];
    param pr[NUM_THREAD];
    for (int i = 0; i < NUM_THREAD; i++)
    {
        pr[i].command = command;
        pthread_create(&task[i], nullptr, &threadExcute, &pr[i]);
    }

    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(task[i], nullptr);
    }
}

void benchMark(string command, int num)
{
    int sockfd = getConnection();
    char receiveBuff[LENGTH];

    for (int i = 0; i < num; i++)
    {
        string sendBuff = "";
        sendBuff += "set key-" + to_string(i) + " value-" + to_string(i);

        /* assign string to char* to send */
        char *tempBuff = new char[sendBuff.length() + 1];
        strcpy(tempBuff, sendBuff.c_str());

        // cout << "\n(" << sockfd << ") send : " << tempBuff << endl;

        /* send buffer to server */
        send(sockfd, tempBuff, sizeof(sendBuff), 0);

        /* allocate new buffer*/
        memset(&receiveBuff, '\0', LENGTH);

        /* receive buffer from server */
        recv(sockfd, receiveBuff, LENGTH, 0);
        // cout << "=> (" << i << ") " << receiveBuff << endl;
    }
    cout << "\nBUILD done" << endl;
    string sb = command;
    if (command == "get")
    {
        sb += " key-" + to_string(rand() % (num + 1));
    }
    else if (command == "set")
    {
        sb += " key-" + to_string(rand() % (num + 1)) + " value-" + to_string(rand() % (num + 1));
    }
    else if (command == "del")
    {
        sb += " key-" + to_string(rand() % (num + 1));
    }

    /* assign string to char* to send */
    char *tempBuff = new char[sb.length() + 1];
    strcpy(tempBuff, sb.c_str());

    cout << "\n(" << sockfd << ") send : " << tempBuff << endl;

    clock_t start = clock();
    /* send buffer to server */
    send(sockfd, tempBuff, sizeof(sb), 0);

    /* allocate new buffer*/
    memset(&receiveBuff, '\0', LENGTH);

    /* receive buffer from server */
    recv(sockfd, receiveBuff, LENGTH, 0);
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    cout << "=>  " << receiveBuff << endl;
    cout << "TIME: " << end - start << "ms\n";
    close(sockfd);

    cout << "[Client] Connection closed.\n";
}

#endif