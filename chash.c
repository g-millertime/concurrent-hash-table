#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

typedef struct hash_struct {
	uint32_t hash;
	char name[50];
	uint32_t salary;
	struct hash_struct *next;
} hashRecord;

// Jenkins' One at a Time Hashing Algorithm
uint32_t gethash(const uint8_t* key) {
  size_t length = sizeof(key)/sizeof(uint8_t);
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

// Globals
hashRecord* list = NULL; // List of records
time_t now;		// Current epoch time
size_t la = 0;	// Number of lock acquisitions
size_t lr = 0;	// Number of lock releases
	

hashRecord* search(uint8_t* key) {
	// Compute Hash
	uint32_t hash = gethash(key); 

	// TODO Acquire Reader Lock
	now = time(0);
	printf("%ld: READ LOCK ACQUIRED\n", now);
	la++;

	// Search list for key-data pair
	hashRecord* cur = list;
	while (cur) {
		if (hash == cur->hash) return cur;
		cur = cur->next;
	}

	// TODO Release Reader Lock
	now = time(0);
	printf("%ld: READ LOCK RELEASED\n", now);
	lr++;
	
	// No search found
	now = time(0);
	printf("%ld: SEARCH NOT FOUND\n", now);
	return NULL;
}

void insert(uint8_t* key, hashRecord* values) {
	// Compute Hash
	values->hash = gethash(key); 

	// TODO Acquire Write Lock
	now = time(0);
	printf("%ld: WRITE LOCK ACQUIRED\n", now);
	la++;
	
	// Search for existing record
	// TODO Hash Table Lookup: Currently as a linked list
	hashRecord* cur = list;
	while (cur) {
		if (values->hash == cur->hash) {
			// Update record
			strcpy(cur->name, values->name);
			cur->salary = values->salary;
			return;
		}
		cur = cur->next;
	}

	// No record found, create new and add to head list
	values->next = list;
	list = values;
	
	// TODO Release Writer Lock
	now = time(0);
	printf("%ld: WRITE LOCK RELEASED\n", now);
	lr++;
	
	return;
}


void delete(uint8_t* key) {
	// Compute Hash
	uint32_t hash = gethash(key); 

	/// TODO Acquire Write Lock
	now = time(0);
	printf("%ld: WRITE LOCK ACQUIRED\n", now);
	la++;
	
	// Search for existing record
	hashRecord* prev = NULL;
	hashRecord* cur = list;
	while (cur) {
		if (hash == cur->hash) {
			// TODO Hash Table: This is removal from linked list
			if (prev == NULL) list = cur->next; // If first element, mark new head
			else prev->next = cur->next;
			free(cur);
			return;
		}
	}

	
	// TODO Release Writer Lock
	now = time(0);
	printf("%ld: WRITE LOCK RELEASED\n", now);
	lr++;

	// No record found
	return;
}

int main(int argc, char** args) {
	FILE * commands = fopen(args[1], "r");	
	// Parse command.txt
	char* line = "";
	size_t len = 0;
	
	ssize_t read = 2;	
	while ((read = getline(&line, &len, commands)) > 1) {
		
		char* cmd = strdup(strtok(line, ",\n"));
		char* name = strdup(strtok(NULL, ",\n"));
		size_t salary = atoi(strdup((strtok(NULL, ",\n"))));

		// Debug: Print line
		// printf("%s %s %ld\n", cmd, name, salary);		
		
		if (strcmp(cmd, "insert") == 0) {
			hashRecord* values = (hashRecord*) malloc(sizeof(hashRecord));
			values->hash = 0; 
			strcpy(values->name, name); 
			values->salary = salary;
			values->next = NULL;
			
			insert(name, values);	
		}
		
		else if (strcmp(cmd, "search") == 0) {
			search(name);
		}
		
		else if (strcmp(cmd, "delete") == 0) {
			delete(name);
		}

		else if (strcmp(cmd, "threads") == 0) {
			printf("Running %d threads\n", atoi(name));	// This is horrible naming convention but it works
		}
		
		else if (strcmp(cmd, "print") == 0) {
			// TODO if necessary
		}

		else {
			fprintf(stderr, "Invalid command\n");
			return 1;
		}
	}

	fclose(commands);
	
	return 0;
}
