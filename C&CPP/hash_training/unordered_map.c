/* 一个用于管理phy寄存器的哈希结构 
 * 功能：
 * 创建：create
 * 访问：
 * 查找键：find
 * 统计键：count
 * 遍历键：
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// 哈希表节点（链表结构解决哈希冲突）
typedef struct HashNode {
    __int8_t device;              // 键（假设为字符串）
    __int8_t regaddr;              // 值（假设为整数）
    struct HashNode *next;  // 指向下一个节点
} HashNode;

// 哈希表主体
typedef struct {
    HashNode **buckets;     // 桶数组
    int capacity;           // 桶数量
    int size;               // 当前元素总数
} HashMap;

/************************************************
 * Function name: hash_function
 * Description  : 
 * Parameter    :
 * @key        
 * @capacity  
 * return       : 0 - successful  1- fail
*************************************************/
unsigned long hash_function(const char *key, int capacity) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % capacity;
}

/************************************************
 * Function name: hash_function
 * Description  : 
 * Parameter    :
 * @key        
 * @capacity  
 * return       : 0 - successful  1- fail
*************************************************/
HashMap *hashmap_create(int capacity) {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    map->capacity = capacity;
    map->size = 0;
    map->buckets = (HashNode **)calloc(capacity, sizeof(HashNode *));
    return map;
}

void hashmap_put(HashMap *map, const char *key, int value) {
    unsigned long index = hash_function(key, map->capacity);
    HashNode *current = map->buckets[index];
    
    // 检查键是否已存在
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value = value; // 更新已有键的值
            return;
        }
        current = current->next;
    }
    
    // 创建新节点并插入链表头部
    HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));
    new_node->key = strdup(key); // 复制键字符串
    new_node->value = value;
    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
    map->size++;
}

int hashmap_get(HashMap *map, const char *key, int *found) {
    unsigned long index = hash_function(key, map->capacity);
    HashNode *current = map->buckets[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (found) *found = 1;
            return current->value;
        }
        current = current->next;
    }
    
    if (found) *found = 0;
    return 0; // 默认返回值
}

void hashmap_remove(HashMap *map, const char *key) {
    unsigned long index = hash_function(key, map->capacity);
    HashNode *current = map->buckets[index];
    HashNode *prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->key);
            free(current);
            map->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}
void hashmap_destroy(HashMap *map) {
    for (int i = 0; i < map->capacity; i++) {
        HashNode *current = map->buckets[i];
        while (current != NULL) {
            HashNode *temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

void hashmap_resize(HashMap *map, int new_capacity) {
    HashNode **new_buckets = (HashNode **)calloc(new_capacity, sizeof(HashNode *));
    
    // 重新哈希所有元素
    for (int i = 0; i < map->capacity; i++) {
        HashNode *current = map->buckets[i];
        while (current != NULL) {
            HashNode *next = current->next;
            unsigned long new_index = hash_function(current->key, new_capacity);
            current->next = new_buckets[new_index];
            new_buckets[new_index] = current;
            current = next;
        }
    }
    
    free(map->buckets);
    map->buckets = new_buckets;
    map->capacity = new_capacity;
}

int main() {
    HashMap *map = hashmap_create(10);
    
    // 插入数据
    hashmap_put(map, "apple", 10);
    hashmap_put(map, "banana", 20);
    
    // 查找数据
    int found;
    int value = hashmap_get(map, "apple", &found);
    if (found) {
        printf("apple: %d\n", value); // 输出 apple: 10
    }
    
    // 删除数据
    hashmap_remove(map, "banana");
    
    // 销毁哈希表
    hashmap_destroy(map);
    return 0;
}

HashMap *hashmap_create(int capacity) {
    HashMap *map = kmalloc(sizeof(HashMap), GFP_KERNEL);
    if (!map) return NULL; // 失败检查
    map->buckets = kzalloc(capacity * sizeof(HashNode *), GFP_KERNEL);
    if (!map->buckets) {
        kfree(map);
        return NULL;
    }
    // 其他初始化...
}