
## Makefile函数


### patsubst
patsubst 是 GNU Make 中的一个函数，用于模式替换。它可以用来在一组文件名中替换某个模式，并生成新的文件名列表。其基本语法如下：

> ` $(patsubst pattern,replacement,text) `
​
* pattern 是要匹配的模式，可以包含 % 作为通配符。
* replacement 是替换后的字符串，也可以包含 %，表示与 pattern 中的 % 相匹配的部分。
* text 是要进行替换操作的字符串列表，通常是文件名列表。
* patsubst 的作用是：对 text 中的每一个单词，检查它是否符合 pattern，如果符合，则用 replacement 替代它。





# 示例
## 常用的变量
```mk
OBJ_DIR（对象文件目录）
LIB_DIR（库文件目录）
BIN_DIR（可执行文件目录）
```

```makefile
# 通过 patsubst 函数，将源文件路径（$(SRC_TOP_DIR)/%.c）转换为目标文件路径（$(OBJ_DIR)/%.o）。
OBJS = $(patsubst $(SRC_TOP_DIR)/%.c , $(OBJ_DIR)/%.o, $(SRCS)) $(EXTRA_OBJS)

# 判断是否是GCOV，如果是的话增加编译选项-fprofile-arcs -ftest-coverage
ifeq ($(IS_GCOV),yes)
CPPFLAGS += -fprofile-arcs -ftest-coverage
endif

# 定义LIBS变量，包含有LIB_DIR下的lib + LIB + .a的文件
LIBS = $(LIB_DIR)/lib$(LIB).a

# 目标是LIBS中的文件，依赖是OBJS里的文件，规则在下面
$(LIBS): $(OBJS)
    @echo "LIB => $@" && \
    if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi && \
    rm -rf $@ $^ && \
    $(AR) $@ $^ && \
    $(RANLIB) $@

.PHONY: clean
clean:
    rm -rf $(LIB_DIR)/lib$(LIB).a $(OBJS) $(EXTRA_CLEANS)

include $(MK_DIR)/obj.mk
```