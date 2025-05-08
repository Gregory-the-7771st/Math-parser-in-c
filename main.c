#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct Token {
    int type; // 0 means operation, 1 means number, 2 means bracket
    char value[10];
};

struct Node {
    int type;
    char value[10];
    struct Node* left;
    struct Node* right;
};

struct Token tokens[20];
struct Node node;
char input[100];
int tokensLen = 0;
char result[10];
bool debug = true;
// so that the pointers in the node dont cease to be valid as soon as the parse function is exited
struct Node lefts[20];
struct Node rights[20];
int leftIndex = 0;
int rightIndex = 0;

bool hasHigherPrecende(value) {
    return value == '*' || value == '/' || value == '(' || value == ')';
}

bool isNum(char c) {
    char numChars[14] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '+', '.', ','};
    for (int i = 0; i < 14; i++) {
        if (c == numChars[i]) {
            return true;
        }
    }
    return false;
}

bool isOperation(char c) {
    char operations[6] = {'+', '-', '/', '*', '(', ')'};
    for (int i = 0; i < 6; i++) {
        if (c == operations[i]) {
            return true;
        }
    }
    return false;
}

void tokenize() {
    char currTok[10] = "";
    int currType = -1;
    bool setToken = false;
    for (int i = 0; i < strlen(input) - 1; i++) { // - 1 for the \n at the end
        char character = input[i];
        setToken = false;
        if (isNum(character) && isOperation(character)) { // + or -
            if (i == 0 || tokens[tokensLen - 1].type == 0) {
                currType = 1;
                char string[2] = {character, '\0'};
                strcat(currTok, string);
            } else {
                currType = 0;
                char string[2] = {character, '\0'};
                strcpy(currTok, string);
                setToken = true;
            }
        } else if (isNum(character)) {
            if (isOperation(input[i + 1])) {
                setToken = true;
            }
            currType = 1;
            char string[2] = {character, '\0'};
            strcat(currTok, string);
        } else if (isOperation(character)) {
            if (character == '(' || character == ')') {
                currType = 2;
            } else {
                currType = 0;
            }
            char string[2] = {character, '\0'};
            strcat(currTok, string);
            setToken = true;
        } else if (character == ' ') {
            if (input[i - 1] != ' ' && strlen(currTok) > 0) {
                setToken = true;
            }
        } else {
            printf("Error while tokenizing input: Unrecognized chacter");
            exit(1);
        }

        if (setToken || i == strlen(input) - 2) { // the last thing is for checking if it is the last character of the input

            if (strlen(currTok) > 10) {
                printf("Error while tokenizing input: Token too large (above 10 characters)");
                exit(1);
            }

            if (tokensLen == 20) {
                printf("Error while tokenizing input: Too many tokens (above 20 tokens)");
                exit(1);
            }

            struct Token token;
            token.type = currType;
            strcpy(token.value, currTok);
            tokens[tokensLen] = token;
            tokensLen++;
            strcpy(currTok, "");
        }
    }
}

int findHead(struct Token tokens[], size) {
    if (tokens[0].type == 0) {
        printf("Error while parsing operation: Nothing to the left of operator");
        exit(1);
    }

    if (tokens[size - 1].type == 0) {
        printf("Error while parsing operation: Nothing to the right of operator");
        exit(1);
    }

    int head = -1;
    bool headHasHigherPrecedence = false;
    bool isInsideParantheses = false;
    int paranthesesDepth = 0;
    for (int i = 0; i < size; i++) { // find the head of the node
        struct Token token = tokens[i];
        if (token.type == 0 && !isInsideParantheses) {
            if (!hasHigherPrecende(token.value[0]) && headHasHigherPrecedence) {
                head = i;
                headHasHigherPrecedence = false;
            } else if (head == -1) {
                head = i;
                headHasHigherPrecedence = hasHigherPrecende(token.value[0]);
            }
        } else if (token.type == 2) {
            if (token.value[0] == '(') {
                paranthesesDepth++;
                isInsideParantheses = true;
            } else if (!isInsideParantheses && token.value[0] == ')') {
                printf("Error while parsing operation: Missing opening paranthesis");
                exit(1);
            } else {
                paranthesesDepth--;
                if (paranthesesDepth == 0) {
                    isInsideParantheses = false;
                }
            }
        }
    }

    if (head == -1) {
        if (size == 1) {
            head = 0;
        } else {
            if (tokens[0].type == 2 && tokens[size - 1].type == 2) {
                struct Token bracket[size - 2];
                for (int j = 1; j < size - 1; j++) {
                    bracket[j - 1] = tokens[j];
                }
                return findHead(bracket, size - 2);
            } else {
                printf("Error while parsing operation: Unable to find head node");
                exit(1);
            }
        }
    }

    return head;
}

struct Node parse(struct Token tokens[], int size) {
    if (size == 2 && (tokens[0].type == 1 || tokens[1].type == 1)) {
        printf("Error while parsing operation: Missing operand");
        exit(1);
    }

    int headIndex = findHead(tokens, size);

    if (tokens[0].type == 2 && tokens[size - 1].type == 2) {
        for (int i = 1; i < size - 1; i++) {
            tokens[i - 1] = tokens[i];
        }
        size -= 2;
        return parse(tokens, size);
    }

    struct Node head;
    head.type = tokens[headIndex].type;
    strcpy(head.value, tokens[headIndex].value);
    head.left = NULL;
    head.right = NULL;

    if (size != 1) {
        struct Token leftTokens[headIndex];
        for (int i = 0; i < headIndex; i++) {
            leftTokens[i] = tokens[i];
        }
        struct Node left = parse(leftTokens, headIndex);
        lefts[leftIndex] = left;
        head.left = &lefts[leftIndex];
        leftIndex++;
    }

    if (size != 1) {
        struct Token rightTokens[size - headIndex];
        for (int i = headIndex + 1; i < size; i++) {
            rightTokens[i - headIndex - 1] = tokens[i];
        }
        struct Node right = parse(rightTokens, size - headIndex - 1);
        rights[rightIndex] = right;
        head.right = &rights[rightIndex];
        rightIndex++;
    }

    return head;
}

char evaluate(struct Node node) {
    if (node.left -> type == 0 && node.right -> type == 0) {
        evaluate(*node.left);
        char left[10];
        strcpy(left, result);
        evaluate(*node.right);
        char right[10];
        strcpy(right, result);
        if (node.type == 0) {
            if (node.value[0] == '+') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) + strtof(right, NULL));
            } else if (node.value[0] == '-') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) - strtof(right, NULL));
            } else if (node.value[0] == '*') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) * strtof(right, NULL));
            } else if (node.value[0] == '/') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) / strtof(right, NULL));
            }
        } else {
            printf("Error while evaluating expression: Nodes that are numbers cannot have left or right nodes");
            exit(1);
        }
    } else if (node.left -> type != 0 && node.right -> type == 0) {
        evaluate(*node.right);
        char right[10];
        strcpy(right, result);
        if (node.type == 0) {
            if (node.value[0] == '+') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) + strtof(right, NULL));
            } else if (node.value[0] == '-') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) - strtof(right, NULL));
            } else if (node.value[0] == '*') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) * strtof(right, NULL));
            } else if (node.value[0] == '/') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) / strtof(right, NULL));
            }
        } else {
            printf("Error while evaluating expression: Nodes that are numbers cannot have left or right nodes");
            exit(1);
        }
    } else if (node.left -> type == 0 && node.right -> type != 0) {
        evaluate(*node.left);
        char left[10];
        strcpy(left, result);
        if (node.type == 0) {
            if (node.value[0] == '+') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) + strtof(&(node.right -> value), NULL));
            } else if (node.value[0] == '-') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) - strtof(&(node.right -> value), NULL));
            } else if (node.value[0] == '*') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) * strtof(&(node.right -> value), NULL));
            } else if (node.value[0] == '/') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(left, NULL) / strtof(&(node.right -> value), NULL));
            }
        } else {
            printf("Error while evaluating expression: Nodes that are numbers cannot have left or right nodes");
            exit(1);
        }
    } else {
        if (node.type == 0) {
            if (node.value[0] == '+') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) + strtof(&(node.right -> value), NULL));
            } else if (node.value[0] == '-') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) - strtof(&(node.right -> value), NULL));
            } else if (node.value[0] == '*') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) * strtof(&(node.right -> value), NULL));
            } else if (node.value[0] == '/') {
                snprintf(&result, 10 * sizeof(char), "%.2f", strtof(&(node.left -> value), NULL) / strtof(&(node.right -> value), NULL));
            }
        } else {
            printf("Error while evaluating expression: Nodes that are numbers cannot have left or right nodes");
            exit(1);
        }
    }
    return result;
}

int main() {
    printf("Operation: ");
    fgets(input, sizeof(input), stdin);
    tokenize();
    node = parse(tokens, tokensLen);
    evaluate(node);
    printf("Result: %s", result);
    return 0;
}
