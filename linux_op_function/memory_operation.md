# 内存操作



## dd

用于在linux中读取、转换并写入数据，常用于数据备份、格式化和刻录等操作。



```
dd if=/dev/zero of=/dev/null bs=1M count=1000 conv=fsync

dd if=/dev/zero of=1gb_file.bin bs=1M count=1024	// 

dd if=/dev/zero of=1gb_file.bin bs=1G count=1 	// 一次性分配1GB用户态缓冲区 + 1GB page cache
```





## 巨型页

```
/proc/sys/vm/nr_hugepages

/proc/meminfo    ## 看是否有Huge相关字样
```

