#include <sys/time.h>
#include <netinet/in.h>
#include <chrono>
#include "BTree.h"
#include <sstream>
#include "ThreadPool.h"
#include "Connection.h"
#include <semaphore.h>
#include <signal.h>

#define TRUE 1
#define FALSE 0
#define LENGTH 512
#define DATA_FILE "./datafile.dat"
#define NUM_THREAD 16

BTree *tree;
// pthread_mutex_t mtx;
// pthread_cond_t readersQ, writersQ;
// int readers = 0, writers = 0, active_writers = 0;
sem_t sem, wrt;

void holdConnection(int nsockfd, char *request);
void signalExe(int num);

int main(int argc, char *argv[])
{
   signal(SIGINT, signalExe);
   int i, len, rc, on = 1;
   int sockfd, maxSd, nsockfd;
   int descReady, endServer = FALSE;
   int closeConn;
   char buffer[900];

   struct timeval timeout;
   fd_set masterSet, workingSet;
   tree = BTreeInit(DATA_FILE, true);
   ThreadPool pool(NUM_THREAD);

   // typedef void *(*THREADFUNCPTR)(void *); /* type of function to pass to thread */
   //BTreeClose(tree);

   sockfd = getConnectionServer();

   /* Initialize the master fd_set */
   FD_ZERO(&masterSet);
   maxSd = sockfd;
   FD_SET(sockfd, &masterSet);

   do
   {
      /* Copy master fdset to working fdset */
      memcpy(&workingSet, &masterSet, sizeof(masterSet));

      /* Call select() & wait for timeout*/
      //cout << "Waiting on select() ... " << endl;
      rc = select(maxSd + 1, &workingSet, NULL, NULL, NULL);
      if (rc < 0)
      {
         perror("ERROR: select() failed");

         break;
      }
      if (rc == 0)
      {
         cout << "[SERVER]: select() timeout, end program." << endl;
      }
      /* 1 or more fd are readable, determine which ones */
      descReady = rc;

      for (i = 0; i <= maxSd && descReady > 0; i++)
      {
         /* Check if this fd isready (isset) */
         if (FD_ISSET(i, &workingSet))
         {
            /* Process 1 of descReady*/
            descReady -= 1;

            if (i == sockfd)
            {
               cout << "[SERVER]: Listening socket is readable" << endl;

               /* Accept all incoming connections are queued up on sockfd before loop back then call select */
               do
               {
                  nsockfd = accept(sockfd, NULL, NULL);
                  if (nsockfd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("ERROR: accept() failed");
                        endServer = true;
                     }
                     break;
                  }
                  /* Add new connection to master fdset*/
                  cout << "[SERVER]: New incoming connection " << nsockfd << endl;
                  FD_SET(nsockfd, &masterSet);
                  if (nsockfd > maxSd)
                     maxSd = nsockfd;

               } while (nsockfd != -1);
            } /* if not sockfd*/
            else
            {
               closeConn = false;
               rc = recv(i, buffer, sizeof(char) * 900, 0);
               if (rc < 0)
               {
                  if (errno != EWOULDBLOCK)
                  {
                     perror("  recv() failed");
                     closeConn = TRUE;
                  }
               }

               if (rc == 0)
               {
                  cout << "[SERVER]:_Connection closed: " << i << endl;
                  closeConn = TRUE;
               }

               char *des = (char *)buffer;

               pool.enqueue([i, des] {
                  holdConnection(i, des);
               });

               /**
               * If closeConn, clean up this active  connection.  This clean up process
               * includes removing the fd from master fdset and determining the new max
               * fd value base on the bits that are still turned on in the master fdset
               */
               if (closeConn)
               {
                  //BTreeClose(tree);
                  close(i);
                  FD_CLR(i, &masterSet);
                  if (i == maxSd)
                  {
                     while (FD_ISSET(maxSd, &masterSet) == FALSE)
                        maxSd -= 1;
                  }
               }
            } /* End of existing connection is readable */
         }    /* End of if (FD_ISSET(i, &working_set)) */
      }       /* End of loop through selectable descriptors */

   } while (endServer == FALSE);

   for (i = 0; i <= maxSd; ++i)
   {
      if (FD_ISSET(i, &masterSet))
         close(i);
   }
   BTreeClose(tree);
   sem_destroy(&wrt);
   sem_destroy(&sem);
   return 0;
}

void holdConnection(int nsockfd, char *request)
{
   sem_init(&sem, 0, 1);
   sem_init(&wrt, 0, 1);
   int rc = 0, closeConn;
   char sendBuff[LENGTH];

   string cmd, key, val;

   cout << "[SERVER]: client " << nsockfd << " requests: " << request << endl;

   stringstream ss(request);

   ss >> cmd >> key >> val;
   /* convert string to char * */
   char *charKey = new char[key.length() + 1];
   strcpy(charKey, key.c_str());
   char *charVal = new char[val.length() + 1];
   strcpy(charVal, val.c_str());

   char *result = new char[LENGTH];

   if (cmd == "set")
   {
      sem_wait(&wrt);
      set(tree, charKey, charVal);
      strcpy(result, "success");
      sem_post(&wrt);
   }
   if (cmd == "get")
   {
      sem_wait(&sem);
      rc++;
      if (rc == 1)
         sem_wait(&wrt);
      sem_post(&sem);

      result = get(tree, charKey);

      sem_wait(&sem);
      rc--;
      if (rc == 0)
         sem_post(&wrt);
      sem_post(&sem);

      // sem_wait(&wrt);
      // result = get(tree, charKey);
      // sem_post(&wrt);
   }
   if (cmd == "del")
   {
      sem_wait(&wrt);

      removeFromTree(tree, charKey);
      strcpy(result, "success");

      sem_post(&wrt);
   }

   send(nsockfd, result, sizeof(result), 0);
}

void signalExe(int num)
{
   cout << "[SERVER]: terminated! " << endl;
   BTreeClose(tree);
   exit(num);
}