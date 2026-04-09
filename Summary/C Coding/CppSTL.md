# STL使用

C++ STL 容器主要分为**三大类**：顺序容器、关联容器、容器适配器。下面按分类清晰说明，每个容器的用途、特点和适用场景。



## 顺序容器Sequence Containers

主要说明元素是按照顺序存放的，可以随机或者顺序访问

### vector动态数组

#### create

```c 

vector<int> v;			// 空vector
vector<int> v(10);		// 10个int，默认为0
vector<int> v(10,5);	// 10个5
```



### list双向链表



### forward_list单向链表



### deque双端队列



### array固定大小数组



## 关联容器

按照**键**组织，自动哈希/排序，用于快速查找



### set



### multiset



### map



## 无序关联容器



### unordered_map



### unordered_set



### unordered_multiset



### unordered_multimap





