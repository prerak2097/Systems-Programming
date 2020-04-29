/* 
 * Assignment 0
 * Systems Programming Sping 2020
 * Zabir Rahman and Prerak Patel
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

typedef struct node {
    int num;
    char* str;
    struct node* next;
} node;

/*
 * global vars:
 * head     = pointer to the first node in the linked list 
 * isNum    = 1 means file is numeric
 * isStr    = 1 means file is alphabetic
 */ 

void* head = NULL;
int isNum = 0;
int isStr = 0;

int insertionSort(void* toSort, int(*comparator)(void*, void*));
int quickSort(void* toSort, int(*comparator)(void*, void*));
int stringComparator(void* firstt, void* secondd);
int numComparator(void* firstt, void* secondd);
void insertNode(char*);
void printList();
int split(node**,int,int,int(*comparator)(void*, void*));
void q_sort(node**, int,int, int(*comparator)(void*, void*));

int main(int argc, char** argv){
    /* 
     * variables:
     * str      = empty string for reading in words
     * buffer   = char variable for reading in each character of the file one by one
     * flag     = int for checking if file is empty or not
     * fd       = file descriptor
     */

    if(argc < 2) {
        char* errorMsg = "Fatal Error: Not enough arguments";
        printf("%s\n", errorMsg);
        return(-1);
    } else if(argv[1][1] != 'q' && argv[1][1] != 'i') {
        char* errorMsg = "Fatal Error: Invalid sort flag";
        printf("%s\n", errorMsg);
        return(-1);
    }
    
    char str[1023] = "";
    char buffer; 
    int flag;
    int fd = open(argv[2], O_RDONLY);
    if(fd < 0) {
        char* errorMsg = "Fatal Error: Invalid file";
        printf("%s\n", errorMsg);
        return(-1);
    }
    flag = read(fd, &buffer, 1) > 0;
    if(flag < 0){
        char* errorMsg = "Fatal Error: Invalid file";
        printf("%s\n", errorMsg);
        return(-1);
    } else if(flag == 0) {
        char* errorMsg = "Warning: File is empty";
        printf("%s\n", errorMsg);
    } else {
        do {
            if(isNum == 0 && isStr == 0) {          //check if file is numeric or alphabetic
                if(buffer >= 48 && buffer <= 57) { isNum = 1; } else 
                if(buffer >= 65 && buffer <= 90) { isStr = 1; } else 
                if(buffer >= 97 && buffer <= 122) { isStr = 1; } 
                else { continue; }
            }

            if(buffer == ',') {                     
                if(strlen(str) != 0 && !(isNum == 0 && isStr == 0)){
                    insertNode(str);
                    memset(str, 0, sizeof(str));        //empty out str so that we can store the next word in it
                }
            } else if(buffer == ' ' || buffer == '\n' || buffer == '\t') {
                continue;
            } else {
                strncat(str, &buffer, 1);           //concat current character to str
            }
        } while(flag = read(fd, &buffer, 1) > 0);   //run loop until file is empty
    } 

    insertNode(str);                            //create/insert node for last token in file

    if(argv[1][1] == 'i') {
        if(isStr == 1) {
            insertionSort(head, &stringComparator);
        } else if(isNum == 1) {
            insertionSort(head, &numComparator);
        }
        printList();
    } else if(argv[1][1] == 'q'){
        if(isStr == 1) {
            quickSort(head, &stringComparator);
        } else if(isNum == 1) {
            quickSort(head, &numComparator);
        }
    }
    
}

void insertNode(char* str){
    if(isStr == 1){                             //insert procedure for alphabetic file 
        node* newNode = (node*)malloc(sizeof(node));
        char* data = malloc((strlen(str)+1));
        if(newNode == NULL || data == NULL){
            printf("Error: malloc() unsuccessfull\n");
        }
        strcpy(data, str);                      //copy string into newly malloc'd memory to prevent overwritting 
        newNode->str = data;                    
        newNode->next = head;                   //insert at front of linked list
        head = newNode;                         //update head pointer 

    } else if(isNum == 1) {                    //insert procedure for numeric file
        node* newNode = (node*)malloc(sizeof(node));
        if(newNode == NULL){
            printf("Error: malloc() unsuccessfull\n");
        }
        int data = atoi(str);                  //convert string to int
        newNode->num = data;
        newNode->next = head;
        head = newNode;
    }
}

void printList() {
    struct node* temp = head;

    if(isStr == 1){
        while(temp != NULL) {
            node* next = temp->next;
            printf("%s\n", temp->str);
            free(temp);
            temp = next;
        }

    } else if(isNum == 1) {
        while(temp != NULL) {
            node* next = temp->next;
            printf("%d\n", temp->num);
            free(temp);
            temp = next;
        }
    }
}

int stringComparator(void* firstt, void* secondd) {
    node* firstNode = firstt;
	node* secondNode = secondd;
    char* first = firstNode->str;
    char* second = secondNode->str;

    while(*first == *second) {
        if(*first == '\0' || *second == '\0') { break ;}
        first++;
        second++;
    }

    if((*first == '\0' && *second != '\0') || *first < *second) {
        return 0; //first comes before second
    } else if((*second == '\0' && *first != '\0') || *first > *second) {
        return 1; //first comes after second
    }

    return -1; //an error has occured   
}

int numComparator(void* firstt, void* secondd) {
    node* firstNode = firstt;
	node* secondNode = secondd;
    int first = firstNode->num;
    int second = secondNode->num;
	
	if(first > second) {
		return 1; // first comes after second;
	}
	else if(first < second) {
		return 0; // first comes before second;
	}
    return -1;
}

int insertionSort(void* toSort, int(*comparator)(void*, void*)){
    node* sorted = NULL;
    node* curr = head;

    while(curr != NULL){
        node* next = curr->next;

        if(sorted == NULL || comparator(sorted, curr) == 1) {
            curr->next = sorted;
            sorted = curr;
        } else {
            node* temp = sorted;
            while(temp->next != NULL && comparator(temp->next, curr) == 0){
                temp = temp->next;
            }
            curr->next = temp->next;
            temp->next = curr;
        }

        curr = next;
    }

    head = sorted;

    return 1;
}

int quickSort(void* toSort, int(*comparator)(void*,void*)) {
	node* counter = toSort;
	if(counter == NULL || counter->next == NULL) {return 0;}
	int ctr= 0;
	while(counter!=NULL) {
		ctr++;
		counter = counter->next;
	}
	// counts # of nodes in LL
	node** hash_arr = malloc(ctr*sizeof(node*));
    if(hash_arr == NULL){
            printf("Error: malloc() unsuccessfull\n");
        }
	counter = toSort;
	node* prev = toSort;
	int i;
	for(i=0; i<ctr; i++) {
		hash_arr[i] = counter;
		counter = counter->next;
		prev->next = NULL;
		prev = counter; 
	}
	int jes = ctr;
	// linked list is unlinked, each element in the hash points to respective node on linked list
	q_sort(hash_arr, 0, ctr-1, comparator);	
	for(i=0; i<jes; i++) {
		if(isStr == 1){
            printf("%s\n", hash_arr[i]->str);
        } else {
            printf("%d\n", hash_arr[i]->num);
        }
	}
	for(i =0; i<jes; i++) {
		free(hash_arr[i]);
	}
    return 0;
}
int split(node** arr, int low, int high, int(*comparator)(void*, void*)) {
	int left = low+1;
	int right = high;
	node* pivot = arr[low];
	int x = 5000;
	while(x== 5000) {
		while(left<= right) {
			if(comparator(arr[left], pivot) == 0) {
				left++;	
			} else {
				break;
			}
		}
		while(right > left) {
			if(comparator(arr[right], pivot)== 0) {
				break;
			} else {
				right--;;
			}
	
		}

		if(left >= right) { break;}
		// swap left and right otherwise
		if(isNum==1) {
			int temp=  arr[left]->num;
			arr[left]->num = arr[right]->num;
			arr[right]->num = temp;
		}
		else {
			/*node*temp = arr[left];
			arr[left] = arr[right];
			arr[right] = temp;
			*/char* temp = arr[left]->str;
			arr[left]->str = arr[right]->str;
			arr[right]->str = temp;
		}
		left++;
		right--;
	}
	//swap pivot with left-1 pos
	if(isNum == 1) {
		arr[low] = arr[left-1];
		arr[left-1] = pivot;
	}else {
		arr[low] = arr[left-1];
		arr[left-1] = pivot;

	}
	// return split point;
	
    return left-1;
}
void q_sort(node** arr, int low, int high, int(*comparator)(void*,void*)) {
	
	if(high-low <= 0) { //there are feweer than 2 items 
		return;
	}
	int splitpoint = split(arr, low, high, comparator);
	q_sort(arr, low, splitpoint-1, comparator); //sort left subtree
	q_sort(arr, splitpoint+1, high, comparator); // sort right subtree
}
