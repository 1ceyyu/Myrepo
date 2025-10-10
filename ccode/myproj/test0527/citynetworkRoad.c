#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_CITIES 20
#define MAX_NAME_LENGTH 50
#define INF INT_MAX

// 城市结构
typedef struct City {
    char name[MAX_NAME_LENGTH];
    int index;
} City;

// 邻接表节点
typedef struct AdjListNode {
    int dest;
    int weight;
    struct AdjListNode* next;
} AdjListNode;

// 邻接表
typedef struct AdjList {
    AdjListNode* head;
} AdjList;

// 图结构
typedef struct Graph {
    int numCities;
    City cities[MAX_CITIES];
    int matrix[MAX_CITIES][MAX_CITIES];
    AdjList* array; // 邻接表
} Graph;

// 赫夫曼树节点
typedef struct HuffmanNode {
    char cityCode;
    int frequency;
    struct HuffmanNode *left, *right;
} HuffmanNode;

// 队列节点（用于层次遍历）
typedef struct QueueNode {
    void *node;
    char position; // 'L'左孩子, 'R'右孩子, ' '根节点
    int isAVL; // 1表示AVL节点，0表示Huffman节点
    struct QueueNode *next;
} QueueNode;

// 队列（用于层次遍历）
typedef struct Queue {
    QueueNode *front, *rear;
} Queue;

// AVL树节点
typedef struct AVLNode{
    int cityNumber;
    struct AVLNode *left, *right;
    int height;
}AVLNode; 

// 全局图变量
Graph graph;
int graphCreated = 0;//判断图是否创建完成 

// 函数声明
void createGraph();
void addCity(char* name);
int getCityIndex(char* name);
void queryShortestPath();
void dijkstra(int start, int dist[], int prev[]);
void printPath(int prev[], int end);
void depthFirstSearch();
void DFS(int start, int visited[]);
void DFS_collect(int start, int visited[], int path[], int* pathLen);
void huffmanTree();
HuffmanNode* createHuffmanNode(char code, int freq);
HuffmanNode* buildHuffmanTree(char codes[], int freqs[], int n);
void levelOrderTraversal_Hu(HuffmanNode* root);
Queue* createQueue();
void enqueueHuffman(Queue* q, HuffmanNode* node, char pos);
QueueNode* dequeue(Queue* q);
int isQueueEmpty(Queue* q);
void AVLsort();
AVLNode* createAVLNode(int cityNumber);
int getHeight(AVLNode* node);
int getBalanceFactor(AVLNode* node);
AVLNode* rightright(AVLNode* y);
AVLNode* leftleft(AVLNode* x);
AVLNode* AVLinsert(AVLNode* node, int cityNumber);
void levelOrderTraversal_AVL(AVLNode* root);
void enqueueAVL(Queue* q, AVLNode* node, char pos);
void freeAVLTree(AVLNode *root);
void visualizeMap(); // 地图可视化函数
AdjListNode* createAdjListNode(int dest, int weight); // 创建邻接表节点
void addEdgeToAdjList(int src, int dest, int weight); // 添加边到邻接表
void printAdjList(); // 打印邻接表
void showCityList(); // 显示当前已创建的城市列表

// 创建邻接表节点
AdjListNode* createAdjListNode(int dest, int weight) {
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    newNode->dest = dest;
    newNode->weight = weight;
    newNode->next = NULL;
    return newNode;
}

// 添加边到邻接表
void addEdgeToAdjList(int src, int dest, int weight) {
    // 添加到src的邻接表
    AdjListNode* newNode = createAdjListNode(dest, weight);
    newNode->next = graph.array[src].head;
    graph.array[src].head = newNode;
    
    // 无向图，也添加到dest的邻接表
    newNode = createAdjListNode(src, weight);
    newNode->next = graph.array[dest].head;
    graph.array[dest].head = newNode;
}

// 打印邻接表形式的地图
void printAdjList() {
    printf("\n=== 城市交通网络地图（邻接表形式） ===\n");
    for (int i = 0; i < graph.numCities; i++) {
        printf("%s -> ", graph.cities[i].name);
        
        AdjListNode* temp = graph.array[i].head;
        while (temp != NULL) {
            printf("%s(%dkm) -> ", graph.cities[temp->dest].name, temp->weight);
            temp = temp->next;
        }
        printf("NULL\n");
    }
    printf("======================================\n");
}

// 可视化地图
void visualizeMap() {
    if (!graphCreated) {
        printf("请先创建地图！\n");
        return;
    }
    
    printf("\n=== 城市交通网络可视化 ===\n");
    
    // 打印邻接表
    printAdjList();
    
    // 打印矩阵形式（可选）
    printf("\n邻接矩阵形式：\n");
    printf("     ");
    for (int i = 0; i < graph.numCities; i++) {
        printf("%-6.3s ", graph.cities[i].name);
    }
    printf("\n");
    
    for (int i = 0; i < graph.numCities; i++) {
        printf("%-5.3s ", graph.cities[i].name);
        for (int j = 0; j < graph.numCities; j++) {
            if (graph.matrix[i][j] == INF) {
                printf("INF   ");
            } else {
                printf("%-6d", graph.matrix[i][j]);
            }
        }
        printf("\n");
    }
    printf("===========================\n");
}

// 创建地图
void createGraph() {
    int i, j, n, distance;
    char name[MAX_NAME_LENGTH];

    printf("\n==============================\n");
    printf("★ 城市交通网络创建 ★\n");
    printf("==============================\n");
    printf("请输入城市数量（最多%d个）: ", MAX_CITIES);
    scanf("%d", &n);
    getchar();

    if (n > MAX_CITIES) {
        printf("城市数量超过上限，自动设置为%d\n", MAX_CITIES);
        n = MAX_CITIES;
    }

    graph.numCities = n;
    
    // 初始化邻接表
    graph.array = (AdjList*)malloc(n * sizeof(AdjList));
    for (i = 0; i < n; i++) {
        graph.array[i].head = NULL;
    }

    // 初始化城市数组
    for (i = 0; i < n; i++) {
        printf("请输入第%d个城市的拼音: ", i+1);
        fgets(name, MAX_NAME_LENGTH, stdin);
        name[strcspn(name, "\n")] = 0; // 移除换行符
        strcpy(graph.cities[i].name, name);
        graph.cities[i].index = i;
        showCityList();
    }

    // 初始化邻接矩阵
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i == j)
                graph.matrix[i][j] = 0;
            else
                graph.matrix[i][j] = INF;
        }
    }

    printf("\n请输入城市之间的道路连接距离\n(格式: 城市1 城市2 距离，输入0 0 0结束)：\n");
    showCityList();
    while (1) {
        char name1[MAX_NAME_LENGTH], name2[MAX_NAME_LENGTH];
        printf(">> ");
        scanf("%s %s %d", name1, name2, &distance);
        getchar();
        if (strcmp(name1, "0") == 0 && strcmp(name2, "0") == 0 && distance == 0)
            break;
        int idx1 = getCityIndex(name1);
        int idx2 = getCityIndex(name2);
        if (idx1 == -1 || idx2 == -1) {
            printf("[错误] 城市不存在，请重新输入。\n");
            showCityList();
            continue;
        }
        graph.matrix[idx1][idx2] = distance;
        graph.matrix[idx2][idx1] = distance; // 无向图
        addEdgeToAdjList(idx1, idx2, distance);
    }

    graphCreated = 1;
    printf("\n[提示] 地图创建成功！\n");
    visualizeMap();
}

// 获取城市索引
int getCityIndex(char* name) {
    for (int i = 0; i < graph.numCities; i++) {
        if (strcmp(graph.cities[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 查询最短路径
void queryShortestPath() {
    char startName[MAX_NAME_LENGTH], endName[MAX_NAME_LENGTH];
    int start, end;

    printf("\n========== 路线查询 =========\n");
    showCityList();
    printf("请输入起始城市: ");
    fgets(startName, MAX_NAME_LENGTH, stdin);
    startName[strcspn(startName, "\n")] = 0;
    printf("请输入目标城市: ");
    fgets(endName, MAX_NAME_LENGTH, stdin);
    endName[strcspn(endName, "\n")] = 0;

    start = getCityIndex(startName);
    end = getCityIndex(endName);

    if (start == -1 || end == -1) {
        printf("[错误] 城市不存在！\n");
        showCityList();
        return;
    }

    int dist[MAX_CITIES], prev[MAX_CITIES];
    dijkstra(start, dist, prev);

    if (dist[end] == INF) {
        printf("城市 %s 和 %s 之间没有路径相连。\n", startName, endName);
    } else {
        printf("最短距离为: %d 公里\n", dist[end]);
        printf("路径: ");
        printPath(prev, end);
        printf("\n");
    }
}

void dijkstra(int start, int dist[], int prev[]) {
    int visited[MAX_CITIES] = {0};
    int i, j, min, u;

    // 初始化距离和前驱数组
    for (i = 0; i < graph.numCities; i++) {
        dist[i] = graph.matrix[start][i];
        if (dist[i] < INF && i != start) {
            prev[i] = start;
        } else {
            prev[i] = -1;
        }
    }

    // 设置起始点
    dist[start] = 0;
    visited[start] = 1;
    prev[start] = -1;  // 起始点的前驱为-1

    for (i = 1; i < graph.numCities; i++) {
        min = INF;
        u = -1;
        
        // 找到未访问节点中距离最小的
        for (j = 0; j < graph.numCities; j++) {
            if (!visited[j] && dist[j] < min) {
                min = dist[j];
                u = j;
            }
        }

        if (u == -1) break;  // 所有可达节点都已访问
        visited[u] = 1;

        // 更新相邻节点的距离
        for (j = 0; j < graph.numCities; j++) {
            if (!visited[j] && graph.matrix[u][j] < INF) {
                // 防止整数溢出
                if (dist[u] < INF && dist[u] + graph.matrix[u][j] < dist[j]) {
                    dist[j] = dist[u] + graph.matrix[u][j];
                    prev[j] = u;
                }
            }
        }
    }
}

// 打印路径
void printPath(int prev[], int end) {
    int path[MAX_CITIES];
    int count = 0;
    int current = end;
    
    // 反向追踪路径
    while (current != -1) {
        path[count++] = current;
        current = prev[current];
    }
    
    // 正向打印路径
    for (int i = count - 1; i >= 0; i--) {
        printf("%s", graph.cities[path[i]].name);
        if (i > 0) {
            printf(" -> ");
        }
    }
}

// 深度优先搜索
void depthFirstSearch() {
    char startName[MAX_NAME_LENGTH];
    int start;
    int visited[MAX_CITIES] = {0};
    int path[MAX_CITIES];
    int pathLen = 0;
    printf("\n---------- 深度优先搜索 ----------\n");
    showCityList();
    printf("请输入起始城市: ");
    fgets(startName, MAX_NAME_LENGTH, stdin);
    startName[strcspn(startName, "\n")] = 0;

    start = getCityIndex(startName);
    if (start == -1) {
        printf("[错误] 城市不存在！\n");
        showCityList();
        return;
    }

    DFS_collect(start, visited, path, &pathLen);
    printf("DFS遍历路线: ");
    for (int i = 0; i < pathLen; i++) {
        printf("%s", graph.cities[path[i]].name);
        if (i < pathLen - 1) printf(" -> ");
    }
    printf("\n城市清单: ");
    for (int i = 0; i < pathLen; i++) {
        printf("%s", graph.cities[path[i]].name);
        if (i < pathLen - 1) printf(", ");
    }
    printf("\n");
}

// DFS递归函数
void DFS(int start, int visited[]) {
    printf("%s ", graph.cities[start].name);
    visited[start] = 1;

    for (int i = 0; i < graph.numCities; i++) {
        if (graph.matrix[start][i] != INF && !visited[i]) {
            DFS(i, visited);
        }
    }
}

void DFS_collect(int start, int visited[], int path[], int* pathLen) {
    visited[start] = 1;
    path[(*pathLen)++] = start;
    for (int i = 0; i < graph.numCities; i++) {
        if (graph.matrix[start][i] != INF && !visited[i]) {
            DFS_collect(i, visited, path, pathLen);
        }
    }
}

// 创建赫夫曼节点
HuffmanNode* createHuffmanNode(char code, int freq) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->cityCode = code;
    node->frequency = freq;
    node->left = node->right = NULL;
    return node;
}

// 构建赫夫曼树
HuffmanNode* buildHuffmanTree(char codes[], int freqs[], int n) {
    HuffmanNode *nodes[MAX_CITIES];
    int i, j;
    for (i = 0; i < n; i++) {
        nodes[i] = createHuffmanNode(codes[i], freqs[i]);
    }
    for (i = 0; i < n-1; i++) {
        // 找到两个频率最小的节点
        int min1 = -1, min2 = -1;
        for (j = 0; j < n; j++) {
            if (nodes[j] != NULL) {
                if (min1 == -1 || nodes[j]->frequency < nodes[min1]->frequency) {
                    min2 = min1;
                    min1 = j;
                } else if (min2 == -1 || nodes[j]->frequency < nodes[min2]->frequency) {
                    min2 = j;
                }
            }
        }
        // 创建新节点
        HuffmanNode* newNode = createHuffmanNode('\0', nodes[min1]->frequency + nodes[min2]->frequency);
        newNode->left = nodes[min1];
        newNode->right = nodes[min2];
        nodes[min1] = newNode;
        nodes[min2] = NULL;
    }
    for (i = 0; i < n; i++) {
        if (nodes[i] != NULL) {
            return nodes[i];
        }
    }
    return NULL;
}

// 构建赫夫曼树
void huffmanTree() {
    int n, i;
    char codes[MAX_CITIES];
    int freqs[MAX_CITIES];
    printf("请输入城市数量（最多20个）: ");
    scanf("%d", &n);
    getchar();
    if (n > MAX_CITIES) {
        printf("数量超过上限，自动设置为%d\n", MAX_CITIES);
        n = MAX_CITIES;
    }
    for (i = 0; i < n; i++) {
        printf("请输入第%d个城市的编码（单个字母）: ", i+1);
        scanf("%c", &codes[i]);
        getchar();
        printf("请输入查询次数: ");
        scanf("%d", &freqs[i]);
        getchar();
    }
    HuffmanNode* root = buildHuffmanTree(codes, freqs, n);
    root->cityCode = '#'; 
    printf("赫夫曼树层次遍历:\n");
    levelOrderTraversal_Hu(root);
}

// 层次遍历赫夫曼树
void levelOrderTraversal_Hu(HuffmanNode* root) {
    if (root == NULL) {
        printf("树为空！\n");
        return;
    }
    Queue* q = createQueue();
    enqueueHuffman(q, root, ' '); // 根节点没有位置标识
    int currentLevelCount = 1;
    int nextLevelCount = 0;
    while (!isQueueEmpty(q)) {
        QueueNode* current = dequeue(q);
        currentLevelCount--;
        HuffmanNode* huffNode = (HuffmanNode*)current->node;
        // 输出当前节点信息
        if (huffNode->cityCode != '\0') {
            printf("%c%c ", huffNode->cityCode, current->position);
        } else {
            printf("*%c ", current->position); // 内部节点用*表示
        }
        if (huffNode->left != NULL) {
            enqueueHuffman(q, huffNode->left, 'L');
            nextLevelCount++;
        }
        if (huffNode->right != NULL) {
            enqueueHuffman(q, huffNode->right, 'R');
            nextLevelCount++;
        }
        free(current); 
        // 当前层结束，换行
        if (currentLevelCount == 0) {
            printf("\n");
            currentLevelCount = nextLevelCount;
            nextLevelCount = 0;
        }
    }
    free(q);
}

AVLNode* createAVLNode(int cityNumber){
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->cityNumber=cityNumber;
    node->height=1;
    node->left=NULL;
    node->right=NULL;
    return node;
}

int getHeight(AVLNode* node){
    if(node==NULL)return 0;
    return node->height;
}

int getBalanceFactor(AVLNode* node){
    if(node==NULL)return 0;
    return getHeight(node->left)-getHeight(node->right);
}

//左左情况 
AVLNode* leftleft(AVLNode* node){
    AVLNode* temp = node->left;
    AVLNode* cemp = temp->right;
    temp->right = node;
    node->left = cemp; 
    int leftHeight = getHeight(node->left);
    int rightHeight = getHeight(node->right);
    if(leftHeight>rightHeight){
        node->height=leftHeight+1;
    }else {
        node->height=rightHeight+1;
    }
    
    leftHeight = getHeight(temp->left);
    rightHeight = getHeight(temp->right);
    if(leftHeight>rightHeight){
        temp->height=leftHeight+1;
    }else {
        temp->height=rightHeight+1;
    }
    return temp; 
}

//右右情况
AVLNode* rightright(AVLNode* node){
    AVLNode* temp = node->right;
    AVLNode* cemp = temp->left;
    temp->left=node;
    node->right=cemp;
    int leftHeight = getHeight(node->left);
    int rightHeight = getHeight(node->right);
    if(leftHeight>rightHeight){
        node->height=leftHeight+1;
    }else {
        node->height=rightHeight+1;
    }
    leftHeight = getHeight(temp->left);
    rightHeight = getHeight(temp->right);
    if(leftHeight>rightHeight){
        temp->height=leftHeight+1;
    }else {
        temp->height=rightHeight+1;
    }
    return temp;
} 

//AVL平衡树的插入 
AVLNode* AVLinsert(AVLNode* node,int cityNumber){
     if (node == NULL) {
        return createAVLNode(cityNumber);
    }

    if (cityNumber < node->cityNumber) {
        node->left = AVLinsert(node->left, cityNumber);
    } else if (cityNumber > node->cityNumber) {
        node->right = AVLinsert(node->right, cityNumber);
    } else {
        return node; // 重复值不插入
    }
      // 更新高度
    node->height = 1 + (getHeight(node->left) > getHeight(node->right) ? getHeight(node->left) : getHeight(node->right));
    // 获取平衡因子
    int balance = getBalanceFactor(node);
    // 左左情况
    if (balance > 1 && cityNumber < node->left->cityNumber) {
        return leftleft(node);
    }
    // 右右情况
    if (balance < -1 && cityNumber > node->right->cityNumber) {
        return rightright(node);
    }
    // 左右情况
    if (balance > 1 && cityNumber > node->left->cityNumber) {
        node->left = rightright(node->left);
        return leftleft(node);
    }
    // 右左情况
    if (balance < -1 && cityNumber < node->right->cityNumber) {
        node->right = leftleft(node->right);
        return rightright(node);
    }
    return node;
} 

void levelOrderTraversal_AVL(AVLNode* root){
     if (root == NULL) {
        printf("树为空！\n");
        return;
    }

    Queue* q = createQueue();
    enqueueAVL(q, root, ' '); // 根节点没有位置标识
    
    int currentLevelCount = 1;
    int nextLevelCount = 0;

    while (!isQueueEmpty(q)) {
        QueueNode* current = dequeue(q);
        currentLevelCount--;
        
        // 输出当前节点信息
        AVLNode* avlNode = (AVLNode*)current->node;
        printf("%d%c ", avlNode->cityNumber, current->position);

        // 将子节点加入队列并记录位置
        if (avlNode->left != NULL) {
            enqueueAVL(q, avlNode->left, 'L');
            nextLevelCount++;
        }
        if (avlNode->right != NULL) {
            enqueueAVL(q, avlNode->right, 'R');
            nextLevelCount++;
        }

        free(current);

        // 当前层结束，换行
        if (currentLevelCount == 0) {
            printf("\n");
            currentLevelCount = nextLevelCount;
            nextLevelCount = 0;
        }
    }
    
    free(q);
}

//AVL树排序 
void AVLsort(){
    int n;
    AVLNode* root = NULL;
    printf("\n-------城市名排序AVL树-------\n");
    printf("输入城市数：");
    scanf("%d",&n);
    int citynum[n+1];
    getchar();
    if(n<=0||n>30){
        printf("请保证城市数量在1~30之间!");
        return ;
    }
    printf("输入各个城市编号：\n");
    for(int i=0;i<n;i++){
        scanf("%d",&citynum[i]);
    }
    for(int j=0;j<n;j++){
        root = AVLinsert(root,citynum[j]); 
    }
    printf("\n城市名AVL树层次遍历结果：\n");
    levelOrderTraversal_AVL(root) ;
    freeAVLTree(root);
} 

void freeAVLTree(AVLNode* root) {
    if (root == NULL) return;
    freeAVLTree(root->left);
    freeAVLTree(root->right);
    free(root);
}

// 创建队列
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

//哈夫曼树入队（修改为包含位置信息）
void enqueueHuffman(Queue* q, HuffmanNode* node, char pos) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->node = node;
    newNode->position = pos;
    newNode->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

//AVL树入队
void enqueueAVL(Queue* q,AVLNode* node,char pos){
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    newNode->node = node;
    newNode->position = pos;
    newNode->isAVL = 1;
    newNode->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
} 

// 出队（返回队列节点）
QueueNode* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    
    QueueNode* temp = q->front;
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    return temp;
}

// 判断队列是否为空
int isQueueEmpty(Queue* q) {
    return q->front == NULL;
}

// 显示当前已创建的城市列表
void showCityList() {
    if (graph.numCities == 0) {
        printf("[无城市信息]\n");
        return;
    }
    printf("[城市列表]: ");
    for (int i = 0; i < graph.numCities; i++) {
        printf("%s", graph.cities[i].name);
        if (i < graph.numCities - 1) printf(", ");
    }
    printf("\n");
}

// 主函数
int main() {
    int choice;
    while (1) {
        printf("\n====================================\n");
        printf("      城市交通网络系统 主菜单      \n");
        printf("====================================\n");
        printf("1. 创建地图\n");
        printf("2. 查询路线\n");
        printf("3. 显示搜索路线（DFS）\n");
        printf("4. 城市名查询赫夫曼树\n");
        printf("5. 城市名排序（AVL树）\n");
        printf("6. 显示地图可视化\n");
        printf("7. 退出\n");
        printf("------------------------------------\n");
        printf("请输入功能编号(1-7): ");
        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice) {
            case 1:
                createGraph();
                break;
            case 2:
                if (graphCreated)
                    queryShortestPath();
                else
                    printf("[提示] 请先创建地图！\n");
                break;
            case 3:
                if (graphCreated)
                    depthFirstSearch();
                else
                    printf("[提示] 请先创建地图！\n");
                break;
            case 4:
                huffmanTree();
                break;
            case 5:
                AVLsort();
                break;
            case 6:
                visualizeMap();
                break;
            case 7:
                printf("程序退出！\n");
                exit(0);
            default:
                printf("[错误] 无效选择，请重新输入。\n");
        }
    }
    return 0;
}