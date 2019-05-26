#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

typedef struct findInfo
{
	char type;
	int threads;
	char *filename;
	list *searchDir;
	int dirCounter;
	list *resultDir;
	int sleepThreads;
} findInfo;


static long pathmax = 4096;
pthread_mutex_t resultList;
pthread_mutex_t searchList;
pthread_cond_t waitForWork;
int status;
#define SEARCH 1
#define COMPLETE 0

/**
 * Remove directory from the search list.
 * Locks mutex while removing.
 * @param searchDir - list of directories
 * @param pos - position to be removed
 * @return 
 */
int removeDir(list *searchDir, listPosition pos);

/**
 * Insert the matching file into the list.
 * Locks the result mutex while inserting.
 * @param resultDir - the list
 * @param path - name of the path
 * @return 
 */
int insertDir(list *resultDir, char *path);

/**
 * Insert directory to the search list. 
 * Locks the mutex while inserting and broadcasts to other threads when done.
 * @param searchDir - the list
 * @param path - name of the path
 * @return 
 */
int insertSearchDir(list *searchDir, char *path);

/**
 * Loops through all the files in the directory and add matches to list. 
 * All list insertions is done with a mutex lock.
 * @param dirInfo - struct with lists and parsed arguments
 * @param currentPath - path to the directory being searched
 * @return -1 on any error, 0 on otherwise
 */
int traverseDirectory(findInfo *dirInfo, char *currentPath );

/**
 * Goes through every directory in the list and look for matching files.
 * @param findArguments - struct that contains find information
 */
void *getDirectory(void *findArguments);

/**
 * Check the validity of the command line arguments and add them to the
 * structure.
 * @param argc
 * @param argv
 * @param input - struct that holds find information
 * @return 0 on valid input, -1 on error.
 */
int parseCommands(int argc, char *argv[], findInfo *input);




