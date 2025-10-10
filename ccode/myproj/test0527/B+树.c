#include <stdio.h>
#include <stdlib.h>
#define M 3 // B+树的阶数

// B+树节点结构体
typedef struct BPTreeNode {
    int isLeaf;
    int numKeys;
    int keys[M];
    struct BPTreeNode* children[M+1];
    struct BPTreeNode* next; // 叶子节点链表
} BPTreeNode;

// 创建新节点
BPTreeNode* createNode(int isLeaf) {
    BPTreeNode* node = (BPTreeNode*)malloc(sizeof(BPTreeNode));
    node->isLeaf = isLeaf;
    node->numKeys = 0;
    for (int i = 0; i < M+1; i++) node->children[i] = NULL;
    node->next = NULL;
    return node;
}

// 查找插入位置
int findInsertPos(BPTreeNode* node, int key) {
    int idx = 0;
    while (idx < node->numKeys && key > node->keys[idx]) idx++;
    return idx;
}

// 分裂节点
void splitChild(BPTreeNode* parent, int idx) {
    BPTreeNode* child = parent->children[idx];
    BPTreeNode* newChild = createNode(child->isLeaf);
    int mid = M/2;
    newChild->numKeys = child->numKeys - mid;
    for (int i = 0; i < newChild->numKeys; i++)
        newChild->keys[i] = child->keys[mid + i];
    if (!child->isLeaf) {
        for (int i = 0; i <= newChild->numKeys; i++)
            newChild->children[i] = child->children[mid + i];
    }
    if (child->isLeaf) {
        newChild->next = child->next;
        child->next = newChild;
    }
    child->numKeys = mid;
    for (int i = parent->numKeys; i > idx; i--)
        parent->children[i+1] = parent->children[i];
    parent->children[idx+1] = newChild;
    for (int i = parent->numKeys; i > idx; i--)
        parent->keys[i] = parent->keys[i-1];
    parent->keys[idx] = newChild->keys[0];
    parent->numKeys++;
}

// 插入非满节点
void insertNonFull(BPTreeNode* node, int key) {
    int i = node->numKeys - 1;
    if (node->isLeaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i+1] = node->keys[i];
            i--;
        }
        node->keys[i+1] = key;
        node->numKeys++;
    } else {
        int idx = findInsertPos(node, key);
        if (node->children[idx]->numKeys == M) {
            splitChild(node, idx);
            if (key > node->keys[idx]) idx++;
        }
        insertNonFull(node->children[idx], key);
    }
}

// 插入主函数
BPTreeNode* insert(BPTreeNode* root, int key) {
    if (root == NULL) {
        root = createNode(1);
        root->keys[0] = key;
        root->numKeys = 1;
        return root;
    }
    if (root->numKeys == M) {
        BPTreeNode* newRoot = createNode(0);
        newRoot->children[0] = root;
        splitChild(newRoot, 0);
        insertNonFull(newRoot, key);
        return newRoot;
    } else {
        insertNonFull(root, key);
        return root;
    }
}

// 打印所有叶子节点
void printLeaves(BPTreeNode* root) {
    if (!root) return;
    while (!root->isLeaf) root = root->children[0];
    printf("B+树叶子节点: ");
    while (root) {
        for (int i = 0; i < root->numKeys; i++)
            printf("%d ", root->keys[i]);
        root = root->next;
    }
    printf("\n");
}

// 主函数示例
int main() {
    BPTreeNode* root = NULL;
    int n, key;
    printf("请输入要插入的关键字数量: ");
    scanf("%d", &n);
    printf("请输入%d个整数: ", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &key);
        root = insert(root, key);
    }
    printLeaves(root);
    return 0;
}
