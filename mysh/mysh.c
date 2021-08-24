// Michael Laager - COP 4600 - Fall 2020

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <limits.h>

#define HISTORY_SIZE 10
#define ARGS_MAX 100


char **history;


typedef struct Node
{
	int pid;
	struct Node *next;
} Node;

typedef struct ProcessHistory
{
	struct Node *first;
	int size;
} ProcessHistory;



Node *newNode(int pid);
ProcessHistory *freeProcessHistory(ProcessHistory *ph);
void clearProcessHistory(ProcessHistory *ph);
void removeFromProcessHistory(ProcessHistory *ph, int pid);
void saveToProcessHistory(ProcessHistory *ph, int pid);
void moveToDir(char **args, int args_cnt);
void whereAmI(void);
void startProgram(char **args, ProcessHistory *ph, int bg);
void exterminateProgram(char **args, ProcessHistory *ph);
void printHistory(void);
void clearHistory(void);
void historyCommand(char **args, int args_cnt);
int repeat(char **args, ProcessHistory *ph, int args_cnt);
void saveToHistory(char **str);
int performCommand(char **str, ProcessHistory *ph, int args_cnt);
int takeCommand(ProcessHistory *ph);

// Creates a new node for process history
Node *newNode (int pid)
{
	Node *n = malloc(sizeof(Node));
	if (n == NULL)
		return NULL;
	n->pid = pid;
	n->next = NULL;
	return n;
}

// Frees the process history
ProcessHistory *freeProcessHistory (ProcessHistory *ph)
{
	clearProcessHistory(ph);	

	free(ph);

	return NULL;
}

// Removes the process history
void clearProcessHistory (ProcessHistory *ph)
{
	Node *tmp, *tmp2;
	
	if (ph == NULL)
		return;
	tmp = ph->first;

	while (tmp != NULL)
	{
		tmp2 = tmp;
		tmp = tmp->next;
		free(tmp2);
	}

	ph->first = NULL;
	ph->size = 0;
	
	return;
}

// Creates a new process history
ProcessHistory *createProcessHistory (void)
{
	ProcessHistory *ph;
	ph =  malloc(sizeof(ProcessHistory));
	if (ph == NULL)
		return NULL;
	ph->first = NULL;
	return ph;
}

// Saves an entry to the process history
void saveToProcessHistory (ProcessHistory *ph, int pid)
{
	Node *n;

	if (ph == NULL)
		return;

	n = newNode(pid);
	if (n == NULL)
		return;

	n->next = ph->first;
	ph->first = n;

	ph->size = ph->size + 1;
	
	return;
}

// Removes an entry from the process history
void removeFromProcessHistory (ProcessHistory *ph, int pid)
{
	Node *tmp, *tmp2;
	
	if (ph == NULL)
		return;
	
	tmp = ph->first;
	while (tmp != NULL)
	{
		if (tmp->pid == pid)
		{
			if (tmp == ph->first)
			{
				ph->first = tmp->next;
				tmp2 = tmp;
				tmp = tmp->next;
				free(tmp2);
				tmp2 = NULL;
			}
			else
			{
				tmp2->next = tmp->next;
				free(tmp);
				tmp = tmp2->next;
			}
			
			ph->size = ph->size - 1;
		}
		else
		{
			tmp2 = tmp;
			tmp = tmp->next;
		}
		
	}

	return;
}

// Prints out the process history
void printProcessHistory (ProcessHistory *ph)
{
	Node *tmp;

	if (ph == NULL || ph->first == NULL)
		return;
	
	tmp = ph->first;
	printf("%d", tmp->pid);
	tmp = tmp->next;

	while (tmp != NULL)
	{
		printf(", %d", tmp->pid);
		tmp = tmp->next;
	}

	return;
}

// Changes the current working directory
void moveToDir (char **args, int args_cnt)
{
	if (args == NULL || args[0] == NULL || strcmp(args[0], "") == 0)
		printf("Argument required for movetodir.\n");
	else if (chdir(args[0]) != 0)
		printf("'%s' is not a valid directory.\n", args[0]);

	return;
}

// Gets the current working directory
void whereAmI (void)
{
	char cwd[PATH_MAX];

	// Gets and prints the current working directory
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		printf("Could not find where you are.\n");
	else
		printf("%s\n", cwd);
	return;
}

// Starts a program with arguments args, and if it is in background bg
void startProgram (char **args, ProcessHistory *ph, int bg)
{
	pid_t rc;
	int status;

	if (args == NULL || args[0] == NULL)
	{
		printf("'start' requires an argument.\n");
		return;
	}

	rc = fork();
	if (rc < 0)
		printf("Could not fork into program '%s'.\n", args[0]);
	else if (rc == 0)
	{
		if (execvp(args[0], args) == -1)
		{
			printf("The program terminated with an error. (is '%s' the proper executable?)\n", args[0]);
			exit(1);
		}
	}
	else
	{
		if (!bg)
			waitpid(rc, &status, 0);
		else
		{
			printf("PID: %d\n", (int)rc);
			saveToProcessHistory(ph, (int)rc);
		}
	}

	return;
}

// Kills a process
void exterminateProgram (char **args, ProcessHistory *ph)
{
	if (args == NULL || args[0] == NULL)
		printf("'exterminate' requires an argument.\n");
	else if (kill(atoi(args[0]), SIGKILL) == -1)
		printf("Could not terminate process with id '%s'.\n", args[0]);
	else
	{
		removeFromProcessHistory(ph, atoi(args[0]));
		printf("Process with id '%s' was terminated successfully.\n", args[0]);
	}
	return;
}

// Kills all processes
void exterminateAll (ProcessHistory *ph)
{
	Node *tmp;

	if (ph == NULL || ph->first == NULL)
	{
		printf("There are no processes to kill.\n");
		return;
	}

	printf("Murdering %d process%s: ", ph->size, (ph->size == 1) ? "" : "es");
	printProcessHistory(ph);
	printf("\n");

	tmp = ph->first;
	while (tmp != NULL)
	{
		kill(tmp->pid, SIGKILL);
		tmp = tmp->next;
	}

	clearProcessHistory(ph);
	return;
}

// Prints out the contents of the history
void printHistory (void)
{
	int i;

	for (i = HISTORY_SIZE-1; i >= 0; i--)
	{
		if (history[i] == NULL)
			continue;
		printf("[%d] %s\n", i+1, history[i]);
	}

	return;
}

// Removes all entries from history
void clearHistory (void)
{
	int i;

	for (i = 0; i < HISTORY_SIZE; i++)
	{
		if (history[i] != NULL)
			free(history[i]);
		history[i] = NULL;
	}

	return;
}

// Controls history-related commands
void historyCommand (char **args, int args_cnt)
{
	if (args == NULL || args[0] == NULL)
		printHistory();
	else if (strcmp(args[0], "-c") == 0)
		clearHistory();
	else
		printf("Invalid argument '%s' for history.\n", args[0]);

	return;
}

// Will perform the same command n times
int repeat (char **args, ProcessHistory *ph, int args_cnt)
{
	int i, n, newlen = 1, retval;
	char *new_str, *tmp;

	// Argument count check
	if (args == NULL || args_cnt < 2 || args[0] == NULL || args[1] == NULL)
	{
		printf("'repeat' requies at least 2 arguments.\n");
		return 0;
	}

	// Reads number of repeats
	sscanf(args[0], "%d", &n);

	// Creates new string for commands
	for (i = 1; i < args_cnt; i++)
	{
		if (args[i] == NULL)
			break;
		newlen += strlen(args[i]) + 1;
	}

	new_str = malloc(sizeof(char) * newlen + 1);
	if (new_str == NULL)
	{
		printf("Memory allocation error (newstr).\n");
		return -1;
	}
	new_str[0] = '\0';


	for (i = 1; i < args_cnt; i++)
	{
		if (args[i] == NULL)
			break;
		strcat(new_str, args[i]);
		strcat(new_str, " ");
	}

	// Repeats new command on subset of the arguments array
	for (i = 0; i < n; i++)
	{
		// A copy needs to be made of new string everytime, as strtok is destructive
		tmp = malloc(sizeof(char) * newlen + 1);
		strcpy(tmp, new_str);
		retval = performCommand(&tmp, ph, args_cnt-2);
		free(tmp);
		if (retval != 0)
		{
			free(new_str);
			return retval;
		}
	}

	free(new_str);
	return 0;
}

// Saves the previous comamand to the history
void saveToHistory (char **str)
{
	int i;
	char *tmp = NULL;

	// Advances every string in array by 1
	for (i = HISTORY_SIZE-1; i > 0; i--)
	{
		if (history[i-1] == NULL)
			continue;
		if (history[i] != NULL)
			free(history[i]);
		history[i] = malloc(sizeof(char) * strlen(history[i-1]) + 1);
		strcpy(history[i], history[i-1]);
	}

	// Replaces the most recent history value with the new one
	if (history[0] != NULL)
		free(history[0]);
	history[0] = malloc(sizeof(char) * strlen(*str) + 1);
	strcpy(history[0], *str);

	return;

}

// Processes user command to be run
int performCommand (char **str, ProcessHistory *ph, int args_cnt)
{
	int i, retval = 0;
	char *token;
	char **args;

	// Gets the original command
	token = strtok(*str, " ");
	if (token == NULL)
		return 0;

	if (args_cnt < 1)
		args = NULL;
	else
	{
		// Allocate memory for additional arguments
		args = malloc(args_cnt * sizeof(char *));
		if (args == NULL)
		{
			printf("Memory allocation error (args).\n");
			return -1;
		}

		// Saves each argument of the command
		for (i = 0; i <= args_cnt; i++)
		{
			if (i == args_cnt)
			{
				if (strtok(NULL, " ") != NULL)
				{
					printf("Too many arguments for command. Max is %d.\n", args_cnt);
					free(args);
					return 0;
				}
			}
			else
			{
				args[i] = strtok(NULL, " ");
				if (args[i] == NULL)
					break;
			}
		}
	}

	// Commands
	//
	// Change directory
	if (strcmp(token, "movetodir") == 0)
		moveToDir(args, args_cnt);
	// Gives current directory
	else if (strcmp(token, "whereami") == 0)
		whereAmI();
	// Prints command history
	else if (strcmp(token, "history") == 0)
		historyCommand(args, args_cnt);
	// Executes a program with arguments
	else if (strcmp(token, "start") == 0)
		startProgram(args, ph, 0);
	// Executes a program in background with arguments
	else if (strcmp(token, "background") == 0)
		startProgram(args, ph, 1);
	// Terminates a program with argument PID
	else if (strcmp(token, "exterminate") == 0)
		exterminateProgram(args, ph);
	// Terminates all processes started by mysh
	else if (strcmp(token, "exterminateall") == 0)
		exterminateAll(ph);
	// Repeats any command
	else if (strcmp(token, "repeat") == 0)
		retval = repeat(args, ph, args_cnt);
	// Stops execution of mysh
	else if (strcmp(token, "byebye") == 0)
		retval = 1;
	// Invalid command
	else
		printf("'%s' is not a valid command.\n", token);

	free(args);
	return retval;
}

// Will read user input for nnext command
int takeCommand (ProcessHistory *ph)
{
	size_t n = 0;
	char *str, *token;

	printf("# ");

	// Gets the command and removes trailing newline
	getline(&str, &n, stdin);
	n = strlen(str);
	if (n > 0)
		str[n-1] = '\0';

	// Tests if string is empty
	if (strcmp(str,"") == 0)
		return 0;

	// Saves command to history
	saveToHistory(&str);

	// Performs the command and returns the status
	return performCommand(&str, ph, ARGS_MAX);
}

// Creates and runs the shell in a loop until abort signal
int main (void)
{
	int i, command_return = 0;
	ProcessHistory *ph;

	history = malloc(HISTORY_SIZE * sizeof(char *));
	if (history == NULL)
	{
		printf("History array could not be allocated.\n");
		return 1;
	}
	for (i = 0; i < HISTORY_SIZE; i++)
		history[i] = NULL;	

	// Allocated space for process history
	ph = createProcessHistory();
	if (ph == NULL)
	{
		printf("Process history array could not be allocated.\n");
		return 1;
	}

	// Will perform commands until status is unexpected
	do
	{
		command_return = takeCommand(ph);
	}
	while (command_return == 0);


	// Kills all child processes
	//exterminateAll(ph);

	// Deallocate history array
	for (i = 0; i < HISTORY_SIZE; i++)
		free(history[i]);
	free(history);

	// Deallocate process history
	ph = freeProcessHistory(ph);

	return 0;
}
