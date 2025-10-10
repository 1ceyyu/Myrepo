#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_PLATE 20
int PARK_CAPACITY ; // 由用户输入停车位
float per;
// 时间结构体
typedef struct {
    int year, month, day, hour, min;
} Time;

// 车辆信息结构体
typedef struct Car {
    char plate[MAX_PLATE];
    Time in_time;
    struct Car *next;
} Car;

// 栈结构体（停车场、临时区）
typedef struct {
    Car *top;
    int size;
} Stack;

// 队列结构体（便道）
typedef struct QueueNode {
    Car car;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front, *rear;
    int size;
} Queue;

// 计算时间差，返回小时数，向下取整
int time_diff_hour(Time *in, Time *out) {
    struct tm tin = {0}, tout = {0};
    tin.tm_year = in->year - 1900;
    tin.tm_mon = in->month - 1;
    tin.tm_mday = in->day;
    tin.tm_hour = in->hour;
    tin.tm_min = in->min;
    tout.tm_year = out->year - 1900;
    tout.tm_mon = out->month - 1;
    tout.tm_mday = out->day;
    tout.tm_hour = out->hour;
    tout.tm_min = out->min;
    time_t t1 = mktime(&tin);//mktime 函数把一个 struct tm 结构体（即年月日时分秒）转换为时间戳
    time_t t2 = mktime(&tout);
    int diff = (int)((t2 - t1) / 60); // 分钟数
    if (diff < 0) return 0;
    if (diff < 60) return 0;
    return diff / 60;
} 

// 栈操作
void stack_init(Stack *s) { s->top = NULL; s->size = 0; }
int stack_empty(Stack *s) { return s->size == 0; }
int stack_size(Stack *s) { return s->size; }
void stack_push(Stack *s, Car *car) {
    Car *node = (Car*)malloc(sizeof(Car));
    *node = *car;
    node->next = s->top;
    s->top = node;
    s->size++;
}
Car stack_pop(Stack *s) {
    Car ret = *s->top;
    Car *tmp = s->top;
    s->top = s->top->next;
    free(tmp);
    s->size--;
    return ret;
}
Car* stack_find(Stack *s, const char *plate, int *pos) {
    Car *p = s->top;
    int i = 0;
    while (p) {
        if (strcmp(p->plate, plate) == 0) {
            if (pos) *pos = i;
            return p;
        }
        p = p->next;
        i++;
    }
    return NULL;
}

// 队列操作
void queue_init(Queue *q) { 
    q->front = q->rear = NULL; 
    q->size = 0;
 }
int queue_empty(Queue *q) { 
    return q->size == 0; 
}
int queue_size(Queue *q) { 
    return q->size; 
}
void queue_push(Queue *q, Car *car) {
    QueueNode *node = (QueueNode*)malloc(sizeof(QueueNode));
    node->car = *car;
    node->next = NULL;
    if (q->rear) q->rear->next = node;
    else q->front = node;
    q->rear = node;
    q->size++;
}
Car queue_pop(Queue *q) {
    QueueNode *node = q->front;
    Car ret = node->car;
    q->front = node->next;
    if (!q->front) q->rear = NULL;
    free(node);
    q->size--;
    return ret;
}

// 显示状态
void show_status(Stack *park, Queue *road) {
    printf("\n停车场: ");
    Car *p = park->top;
    Stack tmp; stack_init(&tmp);
    while (p) { 
         stack_push(&tmp, p);
         p = p->next; 
        }
    while (!stack_empty(&tmp)) {
        Car c = stack_pop(&tmp);
        printf("[%s] ", c.plate);
    }
    printf("\n便道: ");
    QueueNode *q = road->front;
    while (q) { 
        printf("[%s] ", q->car.plate);
        q = q->next; 
        }
    printf("\n");
}

// 车辆到达
void input_time(Time *t) {
    printf("请输入时间(年 月 日 时 分): ");
    scanf("%d%d%d%d%d", &t->year, &t->month, &t->day, &t->hour, &t->min);
}

Time last_time = {0,0,0,0,0}; // 全局变量，记录最近一次进出车辆的时间

// 比较时间大小，返回1表示t1>=t2，0表示t1<t2
int time_geq(Time *t1, Time *t2) {
    struct tm tm1 = {0}, tm2 = {0};
    tm1.tm_year = t1->year - 1900;
    tm1.tm_mon = t1->month - 1;
    tm1.tm_mday = t1->day;
    tm1.tm_hour = t1->hour;
    tm1.tm_min = t1->min;
    tm2.tm_year = t2->year - 1900;
    tm2.tm_mon = t2->month - 1;
    tm2.tm_mday = t2->day;
    tm2.tm_hour = t2->hour;
    tm2.tm_min = t2->min;
    time_t tt1 = mktime(&tm1);
    time_t tt2 = mktime(&tm2);
    return tt1 >= tt2;
}

void car_arrive(Stack *park, Queue *road) {
    Car car;
    printf("请输入车牌号: ");
    scanf("%s", car.plate);
    if (stack_size(park) < PARK_CAPACITY) {
        // 输入时间必须大于等于last_time
        while (1) {
            printf("进入停车场");
            input_time(&car.in_time);
            if (time_geq(&car.in_time, &last_time)) {
                break;
            } else {
                printf("错误：本次进场时间不能早于上一次车辆进出时间，请重新输入！\n");
            }
        }
        car.next = NULL;
        printf("--车辆到达，准备入栈--\n");
        stack_push(park, &car);
        printf("车辆[%s]入栈，进入停车场\n", car.plate);
        last_time = car.in_time;
    } else {
        printf("--停车场已满，车辆进入便道--\n");
        memset(&car.in_time, 0, sizeof(Time));
        car.next = NULL;
        queue_push(road, &car);
        printf("车辆[%s]入队，进入便道\n", car.plate);
    }
    show_status(park, road);
}

// 车辆离开
void car_leave(Stack *park, Queue *road) {
    char plate[MAX_PLATE];
    printf("请输入离开车牌号: ");
    scanf("%s", plate);
    int pos = -1;
    Car *target = stack_find(park, plate, &pos);
    if (!target) {
        // 检查便道
        QueueNode *q = road->front;
        while (q) {
            if (strcmp(q->car.plate, plate) == 0) {
                printf("车辆[%s]在便道离开, 不收费\n", plate);
                // 便道离开
                Queue tmp; queue_init(&tmp);
                while (!queue_empty(road)) {
                    Car c = queue_pop(road);
                    if (strcmp(c.plate, plate) != 0) {
                        queue_push(&tmp, &c);
                        printf("车辆[%s]出队，暂存到临时队列\n", c.plate);
                    } else {
                        printf("车辆[%s]出队，离开便道\n", c.plate);
                    }
                }
                while (!queue_empty(&tmp)) {
                    Car c = queue_pop(&tmp);
                    queue_push(road, &c);
                    printf("车辆[%s]从临时队列回到便道\n", c.plate);
                }
                show_status(park, road);
                return;
            }
            q = q->next;
        }
        printf("未找到该车辆\n");
        return;
    }
    // 临时区
    Stack temp; stack_init(&temp);
    int i;
    printf("--为让[%s]离开，后进车辆依次出栈到临时区--\n", plate);
    for (i = 0; i < pos; ++i) {
        Car tmpCar = stack_pop(park);
        stack_push(&temp, &tmpCar);
        printf("车辆[%s]出栈，进入临时区\n", tmpCar.plate);
    }
    Car out_car = stack_pop(park);
    printf("车辆[%s]出栈，离开停车场\n", out_car.plate);
    Time out_time;
    // 检查离开时间合法性
    while (1) {
        printf("请输入出场");
        input_time(&out_time);
        struct tm tin = {0}, tout = {0};
        tin.tm_year = out_car.in_time.year - 1900;
        tin.tm_mon = out_car.in_time.month - 1;
        tin.tm_mday = out_car.in_time.day;
        tin.tm_hour = out_car.in_time.hour;
        tin.tm_min = out_car.in_time.min;
        tout.tm_year = out_time.year - 1900;
        tout.tm_mon = out_time.month - 1;
        tout.tm_mday = out_time.day;
        tout.tm_hour = out_time.hour;
        tout.tm_min = out_time.min;
        time_t t1 = mktime(&tin);
        time_t t2 = mktime(&tout);
        if (t2 < t1) {
            printf("错误：离开时间不能早于入场时间，请重新输入！\n");
        } else if (!time_geq(&out_time, &last_time)) {
            printf("错误：本次离开时间不能早于上一次车辆进出时间，请重新输入！\n");
        } else {
            break;
        }
    }
    int hours = time_diff_hour(&out_car.in_time, &out_time);
    float fee = hours * per; // 每小时收费
    if (hours == 0) {
        printf("车辆[%s]离开, 停车不足1小时, 不收费\n", out_car.plate);
    } else {
        printf("车辆[%s]离开, 停车%d小时, 收费%.1f元\n", out_car.plate, hours, fee); 
    }
    printf("--临时区车辆依次回到停车场--\n");
    while (!stack_empty(&temp)) {
        Car tmpCar = stack_pop(&temp);
        stack_push(park, &tmpCar);
        printf("车辆[%s]出临时区，回到停车场入栈\n", tmpCar.plate);
    }
    // 便道第一辆进场
    if (!queue_empty(road)) {
        printf("--便道第一辆准备入栈--\n");
        Car c = queue_pop(road);
        printf("车辆[%s]出队，准备进入停车场\n", c.plate);
        // 进入停车场时重新输入入场时间，且不能早于刚离开的车辆的离开时间和last_time
        while (1) {
            printf("请输入车辆[%s]进入停车场的时间: ", c.plate);
            input_time(&c.in_time);
            struct tm tprev = {0}, tcurr = {0};
            tprev.tm_year = out_time.year - 1900;
            tprev.tm_mon = out_time.month - 1;
            tprev.tm_mday = out_time.day;
            tprev.tm_hour = out_time.hour;
            tprev.tm_min = out_time.min;
            tcurr.tm_year = c.in_time.year - 1900;
            tcurr.tm_mon = c.in_time.month - 1;
            tcurr.tm_mday = c.in_time.day;
            tcurr.tm_hour = c.in_time.hour;
            tcurr.tm_min = c.in_time.min;
            time_t t1 = mktime(&tprev);
            time_t t2 = mktime(&tcurr);
            if (t2 < t1) {
                printf("错误：进入停车场时间不能早于上一辆离开车辆的离开时间，请重新输入！\n");
            } else if (!time_geq(&c.in_time, &last_time)) {
                printf("错误：本次进场时间不能早于上一次车辆进出时间，请重新输入！\n");
            } else {
                break;
            }
        }
        stack_push(park, &c);
        printf("车辆[%s]入栈，进入停车场\n", c.plate);
        last_time = c.in_time;
    }
    show_status(park, road);
}

int main() {
    Stack park; 
    stack_init(&park);
    Queue road; 
    queue_init(&road);
    int choice;
    printf("========欢迎使用停车场管理系统========\n");
    printf("请输入停车场车位数: ");
    scanf("%d", &PARK_CAPACITY);
    printf ("请输入每小时收费标准: ");
    scanf("%f", &per);
    while (1) {
        printf("\n1. 显示停车场状态\n2. 车辆到达\n3. 车辆离开\n4. 修改每小时收费标准\n5. 退出\n请选择: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: show_status(&park, &road); break;
            case 2: car_arrive(&park, &road); break;
            case 3: car_leave(&park, &road); break;
            case 4:
                printf("请输入新的每小时收费标准: ");
                scanf("%f", &per);
                printf("已修改为每小时%.2f元\n", per);
                break;
            case 5: exit(0);
            default: printf("无效选择\n");
        }
    }
    return 0;
}
