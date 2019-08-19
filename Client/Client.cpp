#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <sstream>
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "Utils.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printUsage(true);
    }

    if (string(argv[1]) == "-s" || string(argv[1]) == "-S")
    {
        if (argc == 2)
        {
            handleSingleClient();
        }
        else if (string(argv[2]) == "-s")
        {
            testSingleCLient100("set");
        }
        else if (string(argv[2]) == "-g")
        {
            testSingleCLient100("get");
        }
        else if (string(argv[2]) == "-d")
        {
            testSingleCLient100("del");
        }
    }

    else if (string(argv[1]) == "-m" || string(argv[1]) == "-M")
    {
        if (string(argv[2]) == "-s")
        {
            testMultiClient("set");
        }
        else if (string(argv[2]) == "-g")
        {
            testMultiClient("get");
        }
        else if (string(argv[2]) == "-d")
        {
            testMultiClient("del");
        }
    }

    else if (string(argv[1]) == "-b" || string(argv[1]) == "-B")
    {
        int temp = atoi(argv[3]);
        if (string(argv[2]) == "-s")
        {
            benchMark("set", temp);
        }
        else if (string(argv[2]) == "-g")
        {
            benchMark("get", temp);
        }
        else if (string(argv[2]) == "-d")
        {
            benchMark("del", temp);
        }
    }

    else
    {
        cerr << "ERROR argument" << endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}