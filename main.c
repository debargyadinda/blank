#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int key;
    char name[50];
    struct Node *left;
    struct Node *right;
} Node;

typedef void (*Traversal)(Node *);

Node* createNode(int key, const char *name) {
    Node *node = (Node*)malloc(sizeof(Node));

    if (node == NULL) {
        printf("Memory Allocation Failed!\n");
        exit(EXIT_FAILURE);
    }

    node->key = key;
    strcpy(node->name, name);
    node->left = NULL;
    node->right = NULL;

    return node;
}

Node* insert(Node *root, int key, const char *name) {

    if (root == NULL)
        return createNode(key, name);

    if (key < root->key)
        root->left = insert(root->left, key, name);
    else if (key > root->key)
        root->right = insert(root->right, key, name);
    else
        printf("Duplicate key ignored.\n");

    return root;
}

Node* search(Node *root, int key) {

    if (root == NULL || root->key == key)
        return root;

    if (key < root->key)
        return search(root->left, key);

    return search(root->right, key);
}

Node* minValue(Node *node) {

    Node *current = node;

    while (current && current->left != NULL)
        current = current->left;

    return current;
}

Node* deleteNode(Node *root, int key) {

    if (root == NULL)
        return root;

    if (key < root->key)
        root->left = deleteNode(root->left, key);

    else if (key > root->key)
        root->right = deleteNode(root->right, key);

    else {

        if (root->left == NULL) {
            Node *temp = root->right;
            free(root);
            return temp;
        }

        if (root->right == NULL) {
            Node *temp = root->left;
            free(root);
            return temp;
        }

        Node *temp = minValue(root->right);

        root->key = temp->key;
        strcpy(root->name, temp->name);

        root->right = deleteNode(root->right, temp->key);
    }

    return root;
}

void inorder(Node *root) {

    if (root == NULL)
        return;

    inorder(root->left);

    printf("%4d  %-20s\n", root->key, root->name);

    inorder(root->right);
}

void preorder(Node *root) {

    if (root == NULL)
        return;

    printf("%4d  %-20s\n", root->key, root->name);

    preorder(root->left);
    preorder(root->right);
}

void postorder(Node *root) {

    if (root == NULL)
        return;

    postorder(root->left);
    postorder(root->right);

    printf("%4d  %-20s\n", root->key, root->name);
}

void executeTraversal(Traversal t, Node *root) {
    printf("\nID    Name\n");
    printf("---------------------------\n");
    t(root);
}

void freeTree(Node *root) {

    if (root == NULL)
        return;

    freeTree(root->left);
    freeTree(root->right);

    free(root);
}

int main() {

    Node *root = NULL;

    int choice;
    int key;
    char name[50];

    while (1) {

        printf("\n========== Binary Search Tree ==========\n");
        printf("1. Insert Record\n");
        printf("2. Delete Record\n");
        printf("3. Search Record\n");
        printf("4. Inorder Traversal\n");
        printf("5. Preorder Traversal\n");
        printf("6. Postorder Traversal\n");
        printf("7. Exit\n");

        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

        case 1:

            printf("Enter ID: ");
            scanf("%d", &key);

            printf("Enter Name: ");
            scanf("%49s", name);

            root = insert(root, key, name);

            break;

        case 2:

            printf("Enter ID to delete: ");
            scanf("%d", &key);

            root = deleteNode(root, key);

            break;

        case 3: {

            printf("Enter ID to search: ");
            scanf("%d", &key);

            Node *found = search(root, key);

            if (found)
                printf("Found -> ID: %d Name: %s\n", found->key, found->name);
            else
                printf("Record not found.\n");

            break;
        }

        case 4:
            executeTraversal(inorder, root);
            break;

        case 5:
            executeTraversal(preorder, root);
            break;

        case 6:
            executeTraversal(postorder, root);
            break;

        case 7:

            freeTree(root);
            printf("Memory Freed. Exiting...\n");
            return 0;

        default:
            printf("Invalid Choice.\n");
        }
    }

    return 0;
}
