/*
 *  mfind.c
 * 	David Töyrä (dv15dta)
 *  Systemnära programmering HT18
 *  2019-01-14
 */
#include "mfind.h"

/**
 * This is mfind, a program that can find files similiar to the unix
 * program find. It takes two optional flags [-p] for threads and [-t]
 * for type. If not option is given it will assume 1 thread and search for a 
 * regular file. Then it takes at least one start directory and the name to search for.
 */
int main (int argc, char *argv[])
{
	if(argc < 3)
	{
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	}
	status = SEARCH;
	findInfo *comInput = malloc(sizeof(findInfo));
	if(comInput == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	comInput->dirCounter = 0;
	comInput->searchDir = list_makeList();
	comInput->resultDir = list_makeList();
	list_setMemHandler(comInput->searchDir, free);
	list_setMemHandler(comInput->resultDir, free);
	comInput->sleepThreads = 0;
	comInput->threads = 1;
	comInput->type = 0;
	
	//Parse command line arguments
	int errorValue = parseCommands(argc, argv, comInput);
	
	pthread_mutex_init(&resultList, NULL);
	pthread_mutex_init(&searchList, NULL);
	pthread_cond_init(&waitForWork, NULL);
	pthread_t threads[comInput->threads];
	threads[0] = pthread_self();
	
	//Start threads
	for(int i = 1; i < comInput->threads; i++)
	{
		if(pthread_create(&threads[i], NULL, &getDirectory, comInput))
		{
			perror("pthread\n");
            exit(EXIT_FAILURE);
		}	
	}
	
	//Start searching in main thread.
	getDirectory(comInput);

	//Wait for the threads to finish
    for(int i = 1; i < comInput->threads; i++)
	{
        if(pthread_join(threads[i], NULL))
		{
            perror("pthread_join");
        }
    }
   
	//Print out search directories
	while(!list_isEmpty(comInput->resultDir))
	{
		printf("%s\n", (char*)list_inspect(list_first(comInput->resultDir)));
		list_remove(comInput->resultDir, list_first(comInput->resultDir));
	}

	//Destroy mutexes and free allocations
	pthread_mutex_destroy(&resultList);
    pthread_mutex_destroy(&searchList);
    pthread_cond_destroy(&waitForWork);
	list_free(comInput->searchDir);
	list_free(comInput->resultDir);
	free(comInput);
	return errorValue;
	
}

/**
 * Remove directory from the search list.
 * Locks mutex while removing.
 * @param searchDir - list of directories
 * @param pos - position to be removed
 * @return 
 */
int removeDir(list *searchDir, listPosition pos)
{
	pthread_mutex_lock(&searchList);
	list_remove(searchDir, pos);
	pthread_mutex_unlock(&searchList);
	
	return 0;
}

/**
 * Insert the matching file into the list.
 * Locks the result mutex while inserting.
 * @param resultDir - the list
 * @param path - name of the path
 * @return 
 */
int insertDir(list *resultDir, char *path)
{
	char *pathCopy = strdup(path);
	pthread_mutex_lock(&resultList);
	listPosition pos = list_end(resultDir);
	list_insert(resultDir,pathCopy, pos);
	pthread_mutex_unlock(&resultList);
	return 0;
}

/**
 * Insert directory to the search list. 
 * Locks the mutex while inserting and broadcasts to other threads when done.
 * @param searchDir - the list
 * @param path - name of the path
 * @return 
 */
int insertSearchDir(list *searchDir, char *path)
{
	char *pathCopy = strdup(path);
	pthread_mutex_lock(&searchList);
	listPosition pos = list_end(searchDir);
	list_insert(searchDir,pathCopy, pos);
	pthread_cond_broadcast(&waitForWork);
	pthread_mutex_unlock(&searchList);
	
	return 0;
}

/**
 * Loops through all the files in the directory and add matches to list. 
 * All list insertions is done with a mutex lock.
 * @param dirInfo - struct with lists and parsed arguments
 * @param currentPath - path to the directory being searched
 * @return -1 on any error, 0 on otherwise
 */
int traverseDirectory(findInfo *dirInfo, char *currentPath )
{
		struct stat statbuf;
	    struct dirent *dirp;
	    DIR *dp;

		//Check for access
	    if(access(currentPath, R_OK|X_OK) < 0)
		{
	        fprintf(stderr, "mfind: '%s': permission denied\n", currentPath);
	        perror("access");
	        return 1;
	    }
	
	    //Attempt to read
	    if((dp = opendir(currentPath))==NULL)
		{
	        fprintf(stderr, "can't read directory\n");
	        return 1;
	    } 
	
	   //Loop through every file in the directory
	    while ((dirp= readdir(dp)) != NULL)
		{
	        if (strcmp(dirp->d_name, ".") == 0  || strcmp(dirp->d_name, "..") == 0)
			{
				continue;
			}
	
	        size_t pathLength = strlen(currentPath);
	        size_t fileLength = strlen(dirp->d_name);
	        int multiplier = 1;
	        while((pathLength+fileLength+2) > (pathmax * multiplier))
			{
				multiplier ++;
	        }
	        char* fullpath = calloc(pathmax * multiplier, sizeof(char));
	        if(fullpath == NULL)
	        {
	        	perror("calloc");
	        	exit(EXIT_FAILURE);
			}
	        strcpy(fullpath, currentPath);
	        
	        //Add a '/' to the end of the path if it doesn't have one.
	        if(fullpath[pathLength-1] != '/')
	        {
	        	fullpath[pathLength++] = '/';
			}
	        strcpy(&fullpath[pathLength], dirp->d_name);
	
	        bool matched = false;
	
	        if(strcmp(dirp->d_name, dirInfo->filename)==0)
			{
	            matched = true;
	        };
	
	        if(lstat(fullpath, &statbuf)<0)
			{
	            perror("lstat");
	            continue;
	        };
	
	        //Directory match
	        if (S_ISDIR(statbuf.st_mode) == 1)
			{
	            if(matched && (dirInfo->type == 100 || dirInfo->type == 0))
				{
			
	                insertDir(dirInfo->resultDir,  fullpath);
	            }
	            
	            insertSearchDir(dirInfo->searchDir,  fullpath);
	        }
	
	
			 //File match
	         else if(S_ISREG(statbuf.st_mode) ==1)
			{
	            if(matched && (dirInfo->type == 102 || dirInfo->type == 0))
				{
	               insertDir(dirInfo->resultDir,  fullpath);
	            }
	        } 
			
			//Link match
			else if(S_ISLNK(statbuf.st_mode) == 1)
			{
	            if(matched && (dirInfo->type == 108 || dirInfo->type == 0))
	            {
	                insertDir(dirInfo->resultDir,  fullpath);
	            }
	        }
	        free(fullpath);
	    }
	    if (closedir(dp) < 0)
		{
	        perror("closedir");
	    }
	    
	    return 0;
	
}

/**
 * Goes through every directory in the list and look for matching files.
 * @param findArguments - struct that contains find information
 */
void *getDirectory(void *findArguments)
{
	
	findInfo* directories = findArguments;
	int callToOpenDir = 0;
	while(status == SEARCH)
	{
	
		//Locks the search mutex while collecting a new directory
		pthread_mutex_lock(&searchList);
		if(list_isEmpty(directories->searchDir))
		{
			//If all other threads are waiting
			if((directories->sleepThreads >= directories->threads-1))
			{
				//printf("Thread: %ld complete read%d\n",(int long)pthread_self(), callToOpenDir);
				status = COMPLETE;
				pthread_cond_broadcast(&waitForWork);
			}
			
			//Wait for other threads to find a new directory.
			while(list_isEmpty(directories->searchDir) && status == SEARCH)
			{
				directories->sleepThreads+=1;
				pthread_cond_wait(&waitForWork,&searchList);
				directories->sleepThreads-=1;
			}
				pthread_mutex_unlock(&searchList);
				continue;
		}
		listPosition currentPosition;
		char currentPath[1024];
	    
		//Get a new directory
	    if(!list_isEmpty(directories->searchDir))
	    {	
	    	
	     	currentPosition = list_first(directories->searchDir);
	    	strcpy(currentPath,list_inspect(currentPosition));
	    	list_remove(directories->searchDir, currentPosition);
	    	directories->dirCounter++;
	    	callToOpenDir++;
		}
	    pthread_mutex_unlock(&searchList);
	    
	    //Search through the entire directory
		traverseDirectory(directories, currentPath);
	}
	printf("Thread: %ld Reads: %d\n",(int long)pthread_self(), callToOpenDir);
	return 0;
}	



/**
 * Check the validity of the command line arguments and add them to the
 * structure.
 * @param argc
 * @param argv
 * @param input - struct that holds find information
 * @return 0 on valid input, -1 on error.
 */
int parseCommands(int argc, char *argv[], findInfo *input)
{
	struct stat statbuf;
	int c;
	int returnValue = 0;

	//Goes through the options -t and -p
	
	while((c = getopt(argc, argv, "t:p:")) != -1)
	{
		
		switch (c)
		{
			//if the argument with -t is longer than 1 or invalid print out error.
			case 't':
			if(strlen(optarg)!=1 || (optarg[0] != 'l' && optarg[0] != 'f' && optarg[0] != 'd'))
			{
				fprintf(stderr, "Wrong argument for -t\n");
				exit(EXIT_FAILURE);
			}
			input->type = *optarg;
			break;
			
			// check if p is a positive number
			case 'p':
			input->threads = atoi(optarg);
			if(input->threads < 1)
			{
				fprintf(stderr, "Wrong arguments for -p must be positive integer\n");
				exit(EXIT_FAILURE);
			}
			break;
			
			default:
			{
				fprintf(stderr, "Wrong option argument, correct is [-p nrthr] [-t type]\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	//If there are no arguments left after the options.
	if(optind+2 > argc)
	{
		fprintf(stderr, "Too few arguments\n");
		exit(EXIT_FAILURE);
	}
	//Loop through all start directory arguments and check if they are valid
	for(int dirArguments = argc-2; dirArguments >= optind; dirArguments--)
	{
		if (stat(argv[dirArguments], &statbuf) < 0)
		{
			fprintf(stderr, "lstat: %s", argv[dirArguments]);
			perror("");
			returnValue = 1;
			continue;
		}
		if (S_ISDIR(statbuf.st_mode) == 0)
		{
			fprintf(stderr,"%s is not or does not link to a directory\n", argv[dirArguments]);
			returnValue = 1;
			continue;
		}
		
		//Insert directory to the list
		char *arg = malloc((strlen(argv[dirArguments])+1) * sizeof(char));
        strncpy(arg, argv[dirArguments], strlen(argv[dirArguments])+1);

		list_insert(input->searchDir, arg, list_first(input->searchDir));
	}
	input->filename = argv[argc-1];
	return returnValue;
}
	
	

