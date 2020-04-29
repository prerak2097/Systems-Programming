/*
 * Assignment 1
 * Systems Programming Spring 2020
 * Zabir Rahman and Prerak Patel
 *
 */

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<dirent.h>

typedef struct node {
    char* key;
    int freq;
    struct node* left;
    struct node* right;
    int height;
    char* encoding;
} node;

typedef struct minheap {
    int size;
    int capacity;
    struct node** array;
} minheap;

void freeTree(node*);
node* readIntoAVL(int, node*);
node* recursiveRead(char*, node*);
void recursiveCD(char*, char*, node*);

/* AVL */
node* searchKey(node*, char*);
node* searchEncoding(node*, char*);
node* newNode(char*);
node* insertNode(node*, char*);
node* rightRotate(node*);
node* leftRotate(node*);
int getBalance(node*);
int getHeight(node*);
int getMax(int, int);
void inorder();
void preorder();
void levelorder(node*);
void printLevel(node*, int);
void AVLToLL(node*, node*);
void AVLToArr(node*, node**, int);
int countAVLNodes(node*, int);
void printLL(node*);
void printArr(node**, int);

/* MINHEAP */
minheap* initMinheap(int, node**);
void heapify(minheap*, int);
node* getMin(minheap*);
void insertMinHeap(minheap*, node*);
void printHeap(minheap*);

/* HUFFMAN */
node* buildHufTree(int, node**);
void printCodes(node*, char*);
void buildCodebook(node*, char*, int);
node* buildCodebookAVL(int);
void decompress(int, int, node*);
void compress(int, int, node*);

int main(int argc, char** argv){
    char* flag;
    char* codebook;
    char* file;
    int recursive = 0;

    if(argc < 3) {
        printf("Fatal Error: Insufficient args\n");
        return -1;
    } else if(argc > 5) {
        printf("Fatal Error: Too Many args\n");
        return -1;
    }

    if (strcmp(argv[1], "-b") != 0 && strcmp(argv[1], "-c") != 0 &&
        strcmp(argv[1], "-d") != 0 && strcmp(argv[1], "-R") != 0) {
            printf("Fatal Error: Invalid FLag\n");
            return -1;
    }

    flag = argv[1];

    //build no recursion
    if(argc == 3){
        flag = argv[1];
        file = argv[2];
    }
    //build w/recursion or c/d no recursion
    else if(argc == 4){
        if(strcmp(argv[1], "-R") == 0) {
            recursive = 1;
            flag = argv[2];
            file = argv[3];
        } else if(strcmp(argv[2], "-R") == 0) {
            recursive = 1;
            flag = argv[1];
            file = argv[3];
        } else {
            flag = argv[1];
            file = argv[2];
            codebook = argv[3];
        }
    }
    //c/d with recursion
    else if(argc == 5){
        recursive = 1;
        file = argv[3];
        codebook = argv[4];
        if(strcmp(argv[1], "-R") == 0) {
            flag = argv[2];
        } else if(strcmp(argv[2], "-R") == 0) {
            flag = argv[1];
        }
    }

    node* root = (node*)malloc(sizeof(node));
    root = NULL;

    if(recursive == 1){
        root = recursiveRead(file, root);
    } else {
        int fd = open(file, O_RDONLY);
        root = readIntoAVL(fd, root);
        close(fd);
    }

    if(strcmp(flag, "-b") == 0){
        int numNodes = countAVLNodes(root, 0);
        node** arr = (node**)(malloc(sizeof(node*) * numNodes));
        int i;
        for (i = 0; i < numNodes; i++){
            node* insertNode = newNode("");
            arr[i] = insertNode;
        }
        AVLToArr(root, arr, 0); //frees AVL
        
        node* hufTree = buildHufTree(numNodes, arr);
       
        int codebookFD = open("HuffmanCodebook", O_WRONLY | O_CREAT, 00666);
        write(codebookFD, "\\\n", 2);
        char code[100] = "";
        buildCodebook(hufTree, code, codebookFD);
        close(codebookFD);

        for (i = 0; i < numNodes; i++){
            if(strcmp(arr[i]->key, "$") == 0){
                freeTree(arr[i]);
            }
        }
        free(arr);
       
        return 1;
    }
    
    char buffer = '\0';
    node* codebookAVL = NULL;
    int cbTreeFD = open(codebook, O_RDONLY);
    if(cbTreeFD < 0){
        printf("Fatal Error: Invalid Codebook\n");
        return -1;
    } else if(cbTreeFD == 0) {
        printf("Warning: Codebook is empty\n");
    } else {
        //reads the initial "\" and "\n" 
        int i;
        for(i = 0; i < 2; i++){
            read(cbTreeFD, &buffer, 1);
        }
        codebookAVL = buildCodebookAVL(cbTreeFD);
    }
    close(cbTreeFD);

    if(recursive == 0 && strcmp(flag, "-c") == 0) {
        int compressFDIn = open(file, O_RDONLY);
        char* newFileName = (char*)malloc(strlen(file) + 4);
        strcpy(newFileName, file);
        strcat(newFileName, ".hcz");
        int compressFDOut = open(newFileName, O_WRONLY | O_CREAT, 00666);
        compress(compressFDIn, compressFDOut, codebookAVL);
        close(compressFDIn);
        close(compressFDOut);
        free(newFileName);
        return 1;
    }
    
    if(recursive == 0 && strcmp(flag, "-d") == 0) {
        char* extension = strstr(file, ".hcz");
        if(extension == NULL) { return 1; }
        int decompressFDIN = open(file, O_RDONLY);
        memset(extension, 0, strlen(extension));
        char* newFileName = (char*)malloc(strlen(file));
        strcpy(newFileName, file);
        
        int decompressFDOut = open(newFileName, O_WRONLY | O_CREAT, 00666);
        decompress(decompressFDIN, decompressFDOut, codebookAVL);
        close(decompressFDIN);
        close(decompressFDOut);
        free(newFileName);
        return 1;
    }
    
    if(recursive == 1){
        recursiveCD(file, flag, codebookAVL);
        return 1;
    }

}

void freeTree(node* root){
    if(root == NULL) { return; }
    freeTree(root->left);
    freeTree(root->right);
    free(root->key);
    free(root);
}

node* readIntoAVL(int fd, node* root) {
    char str[1023] = "";
    char buffer; 
    int openFlag = read(fd, &buffer, 1);
    if(openFlag < 0){
        printf("Fatal Error: Invalid file\n");
        return NULL;
    } else if(openFlag == 0) {
        printf("Warning: File is empty\n");
    } else {
        do {
            if(buffer == ' ' || buffer == '\t' || buffer == '\n') {
                if(strlen(str) > 0) {
                    node* searchResult = searchKey(root, str);
                    if(searchResult == NULL){
                        root = insertNode(root, str);
                    } else {
                        searchResult->freq++;
                    }
                    memset(str, 0, sizeof(str));
                }

                char buffstr[1023] = "";
                strncat(buffstr, &buffer, 1);
                node* bufSearchRes = searchKey(root, buffstr);
                if(bufSearchRes == NULL){
                    root = insertNode(root, buffstr);
                } else {
                    bufSearchRes->freq++;
                }
                memset(buffstr, 0, sizeof(str));
                
            } else {
                strncat(str, &buffer, 1);
            }
        } while(openFlag = read(fd, &buffer, 1) > 0);
    }
    close(fd);
    return root;
}

node* recursiveRead(char* dirName, node* root) {
    struct dirent* dp;
    DIR* dir = opendir(dirName);

    if(dir == NULL){
        printf("Fatal Error: Invalid Directory Path\n");
        return NULL;
    }

    while((dp = readdir(dir)) != NULL) {
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        char* currentElementPath = (char*)malloc(strlen(dp->d_name) + strlen(dirName + 2));
        strcpy(currentElementPath, dirName);
        strcat(currentElementPath, "/");
        strcat(currentElementPath, dp->d_name);        
        if(dp->d_type == 8){
            int fd = open(currentElementPath, O_RDONLY);
            if(fd < 0){
                printf("Fatal Error: Invalid File\n");
                return NULL;
            }

            root = readIntoAVL(fd, root);
        }
        else if(dp->d_type == 4){
            root = recursiveRead(currentElementPath, root);
        }

    }

    closedir(dir);
    return root;
}

void recursiveCD(char* dirName, char* flag, node* codebookAVL ) {
    char path[1000];
    struct dirent* dp;
    DIR* dir = opendir(dirName);

    if(dir == NULL){
        printf("Fatal Error: Invalid Directory Path\n");
        return;
    }

    while((dp = readdir(dir)) != NULL) {
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) { continue; }

        char* currentElementPath = (char*)malloc(strlen(dp->d_name) + strlen(dirName + 2));
        strcpy(currentElementPath, dirName);
        strcat(currentElementPath, "/");
        strcat(currentElementPath, dp->d_name);

        if(dp->d_type == 8){
            if(strcmp(flag, "-c") == 0){
                //char* extension = strstr(currentElementPath, ".txt");
                //if(extension == NULL) { continue; }
                
                int FDIn = open(currentElementPath, O_RDONLY);
                
                //memset(extension, 0, strlen(extension));
                char* newFileName = (char*)malloc(strlen(currentElementPath) + 4);
                strcpy(newFileName, currentElementPath);
                strcat(newFileName, ".hcz");

                int FDOut = open(newFileName, O_WRONLY | O_CREAT, 00666);
                compress(FDIn, FDOut, codebookAVL);

                close(FDIn);
                close(FDOut);
            } 
            else if(strcmp(flag, "-d") == 0){
                char* extension = strstr(currentElementPath, ".hcz");
                if(extension == NULL) { continue; }
                
                int FDIn = open(currentElementPath, O_RDONLY);
                
                memset(extension, 0, strlen(extension));
                char* newFileName = (char*)malloc(strlen(currentElementPath));
                strcpy(newFileName, currentElementPath);
                //strcat(newFileName, ".txt");

                int FDOut = open(newFileName, O_WRONLY | O_CREAT, 00666);
                decompress(FDIn, FDOut, codebookAVL);

                close(FDIn);
                close(FDOut);
            } 
        }
        else if(dp->d_type == 4){
            recursiveCD(currentElementPath, flag, codebookAVL);
        }
    }
    closedir(dir);
}

/**** AVL FUNCTIONS ****/ 
node* newNode(char* str) {
    node* retNode = (node*)malloc(sizeof(node));
    char* data = (char*)malloc(100);
    strcpy(data, str);
    retNode->key = data;
    retNode->left = NULL;
    retNode->right = NULL;
    retNode->freq = 1;
    retNode->height = 1;

    return retNode;
}

node* insertNode(node* root, char* str){
    /* Normal BST insert */
    if(root == NULL) { return newNode(str); }
    if(strcmp(str, root->key) < 0){
        root->left = insertNode(root->left, str);
    } else if(strcmp(str, root->key) > 0) {
        root->right = insertNode(root->right, str);
    } else {
        return root;
    }

    /* Update height */
    root->height = 1 + getMax(getHeight(root->left), getHeight(root->right));

    /* Check for balance */
    int balance = getBalance(root);

    /* Left Left Rotation */
    if(balance > 1 && strcmp(str, root->left->key) < 0) {
        return rightRotate(root);
    }

    /* Right Right Rotation */
    if(balance < -1 && strcmp(str, root->right->key) > 0) {
        return leftRotate(root);
    }

    /* Left Right Rotate */
    if(balance > 1 && strcmp(str, root->left->key) > 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    /* Right Left Rotate */
    if(balance < -1 && strcmp(str, root->right->key) < 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

node* rightRotate(node* toRotate) {
    node* subtree_1 = toRotate->left;
    node* subtree_2 = subtree_1->right;

    subtree_1->right = toRotate;
    toRotate->left = subtree_2;

    toRotate->height = getMax(getHeight(toRotate->left), getHeight(toRotate->right)) + 1;
    subtree_1->height = getMax(getHeight(subtree_1->left), getHeight(subtree_1->right)) + 1;

    return subtree_1;
}

node* leftRotate(node* toRotate) {
    node* subtree_1 = toRotate->right;
    node* subtree_2 = subtree_1->left;

    subtree_1->left = toRotate;
    toRotate->right = subtree_2;

    toRotate->height = getMax(getHeight(toRotate->left), getHeight(toRotate->right)) + 1;
    subtree_1->height = getMax(getHeight(subtree_1->left), getHeight(subtree_1->right)) + 1;

    return subtree_1;

}

node* searchKey(node* root, char* toSearch) {
    if(root == NULL || strcmp(root->key, toSearch) == 0) { return root; }
    if(strcmp(root->key, toSearch) < 0) { return searchKey(root->right, toSearch); }
    return searchKey(root->left, toSearch);
}

node* searchEncoding(node* root, char* toSearch) {
    if(root == NULL || strcmp(root->encoding, toSearch) == 0) { return root; }
    node* retVal = searchEncoding(root->right, toSearch);
    if(retVal == NULL){
        return searchEncoding(root->left, toSearch);
    } else {
        return retVal;
    }
}

int getBalance(node* toCheck) {
    if(toCheck == NULL) { return 0; }
    return (getHeight(toCheck->left) - getHeight(toCheck->right));
}

int getHeight(node* toCheck) {
    if(toCheck == NULL) { return 0; }
    return toCheck->height;
}

int getMax(int a, int b) {
    if(a >= b) { return a; } 
    else { return b; }
}

void inorder(node* root){
    if(root == NULL) { return; }

    inorder(root->left);
    if(strcmp(root->key, " ") == 0){
        printf("\' \'\t%d\n", root->freq);
    } else if(strcmp(root->key, "\t") == 0){
        printf("\\t\t%d\n", root->freq);
    } else if(strcmp(root->key, "\n") == 0){ 
        printf("\\n\t%d\n", root->freq);
    } else {
        printf("%s\t%d\n", root->key, root->freq);
    }
    inorder(root->right);
}

void preorder(node* root){
    if(root == NULL) { return; }
    /*
    if(strcmp(root->key, " ") == 0){
        printf("\' \'\t%d\n", root->freq);
    } else if(strcmp(root->key, "\t") == 0){
        printf("\\t\t%d\n", root->freq);
    } else if(strcmp(root->key, "\n") == 0){ 
        printf("\\n\t%d\n", root->freq);
    } else {
        printf("%s\t%d\n", root->key, root->freq);
    }
    */
    if(strcmp(root->encoding, " ") == 0){
        printf("\' \'\t%s\n", root->key);
    } else if(strcmp(root->encoding, "\t") == 0){
        printf("\\t\t%s\n", root->key);
    } else if(strcmp(root->encoding, "\n") == 0){ 
        printf("\\n\t%s\n", root->key);
    } else {
        printf("%s\t%s\n", root->encoding, root->key);
    }
    preorder(root->left);
    preorder(root->right);
}

void levelorder(node* root) {
    int h = root->height;
    int i;
    for(i = 1; i <= h; i++){
        printf("Level: %d ", i);printLevel(root, i);
    }
}

void printLevel(node* root, int level){
    if(root == NULL) { return; }
    if(level == 1) { printf("%s\t%d\n", root->key, root->freq); }
    else if (level > 1) {
        printLevel(root->left, level - 1);
        printLevel(root->right, level - 1);
    }
}

void AVLToArr(node* root, node** arr, int i){
    if(root == NULL) { return; }
    AVLToArr(root->left, arr, i);

    while(strcmp("", arr[i]->key) != 0) { i++; }
    strcpy(arr[i]->key, root->key);
    arr[i]->freq = root->freq;

    node* right = root->right;
    free(root->key);
    free(root);
    
    AVLToArr(right, arr, i);
}

int countAVLNodes(node* root, int start) {
    int retNum = start;
    if(root == NULL) { return retNum; }

    retNum = countAVLNodes(root->left, start);
    retNum++;
    retNum = countAVLNodes(root->right, retNum);

    return retNum;
}

void printArr(node** arr, int size) {
    int i;
    for (i = 0; i < size; i++){
        if(strcmp(arr[i]->key, " ") == 0){
            printf("\' \'\t%d\n", arr[i]->freq);
        } else if(strcmp(arr[i]->key, "\t") == 0){
            printf("\\t\t%d\n", arr[i]->freq);
        } else if(strcmp(arr[i]->key, "\n") == 0){ 
            printf("\\n\t%d\n", arr[i]->freq);
        } else {
            printf("%s\t%d\n", arr[i]->key, arr[i]->freq);
        }
    }
}

/**** MINHEAP FUNCTIONS ****/
minheap* initMinheap(int capacity, node** arr){
    minheap* heap = (minheap*)malloc(sizeof(minheap));
    heap->capacity = capacity;
    heap->array = arr;
    heap->size = capacity;

    int n = heap->size - 1;
    int i;
    for(i = (n-1)/2; i >= 0; i--){
        heapify(heap, i);
    }

    return heap;
}

void heapify(minheap* heap, int i) {
    int smallest = i;
    int left = (2*i) + 1;
    int right = (2*i) + 2;

    if(left < heap->size && heap->array[left]->freq < heap->array[smallest]->freq){
        smallest = left;
    }
    if(right < heap->size && heap->array[right]->freq < heap->array[smallest]->freq){
        smallest = right;
    }
    if(smallest != i){
        node* temp = heap->array[smallest];
        heap->array[smallest] = heap->array[i];
        heap->array[i] = temp;

        heapify(heap, smallest);
    }
}

node* getMin(minheap* heap){
    node* temp = heap->array[0];
    heap->array[0] = heap->array[heap->size - 1];

    heap->size--;
    heapify(heap, 0);

    return temp;
}

void insertMinHeap(minheap* heap, node* toInsert){
    int i = heap->size;
    heap->size++;

    while (i && toInsert->freq < heap->array[(i-1)/2]->freq) {
        heap->array[i] = heap->array[(i-1)/2];
        i = (i-1)/2;
    }
    
    heap->array[i] = toInsert;
}

/**** HUFFMAN TREE FUNCTIONS ****/
node* buildHufTree(int size, node** arr){
    minheap* heap = initMinheap(size, arr);
    
    while(heap->size != 1){
        node* left;
        node* right;
        node* top = newNode("$");
    
        left = getMin(heap);
        right = getMin(heap);

        top->freq = left->freq + right->freq;
        top->left = left;
        top->right = right;

        insertMinHeap(heap, top);
    }
 
    return getMin(heap); ;
}

void printCodes(node* root, char* encoding){
    char* current = encoding;
    if(root == NULL) { return; }

    if(root->left == NULL && root->right == NULL){
        if(strcmp(root->key, " ") == 0){
            printf("%s/t\' \'\n", encoding);
        } else if(strcmp(root->key, "\t") == 0){
            printf("%s\t\\t\n", encoding);
        } else if(strcmp(root->key, "\n") == 0){ 
            printf("%s\t\\n\n", encoding);
        } else {
            printf("%s\t%s\n", encoding, root->key);
        } 
        return;
    }

    printCodes(root->left, strcat(encoding, "0"));
    if(strcmp(encoding, "") != 0){
        encoding[strlen(encoding)-1] = '\0';
    }
    printCodes(root->right, strcat(encoding, "1"));
    if(strcmp(encoding, "") != 0){
        encoding[strlen(encoding)-1] = '\0';
    }
}

void buildCodebook(node* root, char* encoding, int fd){
    if(root == NULL) { return; }

    char* buffer;
    int buffLen;
    if(root->left == NULL && root->right == NULL){
        if(strcmp(root->key, " ") == 0){
            buffLen = strlen(encoding) + strlen("\t\' \'\n") + 1;
            buffer = (char*)malloc(sizeof(char)*buffLen);
            strcpy(buffer, encoding);
            strcat(buffer, "\t\' \'\n");
            write(fd, buffer, buffLen-1);
        } else if(strcmp(root->key, "\t") == 0){
            buffLen = strlen(encoding) + strlen("\t\\t\n") + 1;
            buffer = (char*)malloc(sizeof(char)*buffLen);
            strcpy(buffer, encoding);
            strcat(buffer, "\t\\t\n");
            write(fd, buffer, buffLen-1);
        } else if(strcmp(root->key, "\n") == 0){
            buffLen = strlen(encoding) + strlen("\t\\n\n") + 1;
            buffer = (char*)malloc(sizeof(char)*buffLen);
            strcpy(buffer, encoding);
            strcat(buffer, "\t\\n\n");
            write(fd, buffer, buffLen-1);
        } else {
            buffLen = strlen(encoding) + strlen("\t") +strlen(root->key) + strlen("\n") + 1;
            buffer = (char*)malloc(sizeof(char)*buffLen);
            strcpy(buffer, encoding);
            strcat(buffer, "\t");
            strcat(buffer, root->key);
            strcat(buffer, "\n");
            write(fd, buffer, buffLen-1);
        } 
        free(buffer);
        return;
    }

    buildCodebook(root->left, strcat(encoding, "0"), fd);
    if(strcmp(encoding, "") != 0){
        encoding[strlen(encoding)-1] = '\0';
    }
    buildCodebook(root->right, strcat(encoding, "1"), fd);
    if(strcmp(encoding, "") != 0){
        encoding[strlen(encoding)-1] = '\0';
    }
}

node* buildCodebookAVL(int codebookFD){
    node* root = (node*)malloc(sizeof(node));
    root = NULL;
    node* curr;
    char str[1023] = "";
    char buffer;

    while(read(codebookFD, &buffer, 1) > 0) {
        if(buffer == '\n') {
            if(strlen(str) > 0) {
                char* data = malloc((strlen(str)+1));
                strcpy(data, str);
                curr->encoding = data;
                //strcpy(curr->encoding, str);
                curr = NULL;
                memset(str, 0, sizeof(str));
            }
            
        } else if(buffer == '\t'){
            if(strlen(str) > 0) {
                root = insertNode(root, str);
                curr = searchKey(root, str);
                memset(str, 0, sizeof(str));
            }
        } else {
            strncat(str, &buffer, 1);
        }
    }

    return root;
}

void decompress(int inFD, int outFD, node* codebookAVL){
    char str[1023] = "";
    char buffer;
    if(inFD < 0) {
        char* errorMsg = "Fatal Error: Invalid file";
        printf("%s\n", errorMsg);
        return;
    }
    int openFlag = read(inFD, &buffer, 1) > 0;
    if(openFlag < 0){
        char* errorMsg = "Fatal Error: Invalid file";
        printf("%s\n", errorMsg);
        return;
    } else if(openFlag == 0) {
        char* errorMsg = "Warning: File is empty";
        printf("%s\n", errorMsg);
    } else {
        do {
            node* search = searchKey(codebookAVL, str);
            if(search != NULL) {
                if(strcmp(search->encoding, "\\t") == 0){
                    write(outFD, "\t", 1);
                } else if(strcmp(search->encoding, "\\n") == 0){ 
                    write(outFD, "\n", 1);
                } else if(strcmp(search->encoding, "' '") == 0){
                    write(outFD, " ", 1);
                } else {
                    write(outFD, search->encoding, strlen(search->encoding));
                }
                memset(str, 0, sizeof(str));
            }
            strncat(str, &buffer, 1);
        } while(openFlag = read(inFD, &buffer, 1) > 0);
    }
    close(inFD);
    close(outFD);
}

void compress(int inFD, int outFD, node* codebookAVL){
    char str[1023] = "";
    char buffer;
    if(inFD < 0) {
        char* errorMsg = "Fatal Error: Invalid file";
        printf("%s\n", errorMsg);
        return;
    }
    int openFlag = read(inFD, &buffer, 1) > 0;
    if(openFlag < 0){
        char* errorMsg = "Fatal Error: Invalid file";
        printf("%s\n", errorMsg);
        return;
    } else if(openFlag == 0) {
        char* errorMsg = "Warning: File is empty";
        printf("%s\n", errorMsg);
    } else {
        do {
            if(buffer == ' ' || buffer == '\t' || buffer == '\n') {
                if(strlen(str) > 0) {
                    node* searchResult = searchEncoding(codebookAVL, str);
                    if(searchResult == NULL){
                        continue;
                    } else {
                        write(outFD, searchResult->key, strlen(searchResult->key));
                    }
                    memset(str, 0, sizeof(str));
                }

                char* buffstr;
                if(buffer == ' ') { buffstr = "' '"; } else 
                if(buffer == '\t') { buffstr = "\\t"; } else 
                if(buffer == '\n') { buffstr = "\\n"; }

                node* bufSearchRes = searchEncoding(codebookAVL, buffstr);
                if(bufSearchRes == NULL){
                    continue;
                } else {
                    write(outFD, bufSearchRes->key, strlen(bufSearchRes->key));
                }                  
            } else {
                strncat(str, &buffer, 1);
            }
        } while(openFlag = read(inFD, &buffer, 1) > 0);
    }
    close(inFD);
    close(outFD);
}
