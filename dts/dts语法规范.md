# DTS (Device Tree Source) 语法规范

## 1. 基本结构

### 1.1 文件头

```dts
/dts-v1/;    /* 必须在文件首行，声明DTS规范版本，前面尽量不要有空行 */
```
属于 `.dtsi` 文件，必须在文件首行声明DTS规范版本。是固定写法，不允许修改。
这是一个指令声明，表示当前文件遵循DTS v1.0规范。
dts语法要求每个语句以分好结尾。

### 1.2 根节点

```dts
根节点就是  / 

/ {
    /* 所有硬件描述都在根节点内 */
};
```

### 1.3 文件包含

```dts

#include <dt-bindings/clock/imx6ull-clock.h>  /* 包含C头文件中的宏定义 */
/include/ "imx6ull.dtsi"                      /* 包含另一个dtsi文件 */
```

---

## 2. 节点 (Node)

### 2.1 节点定义

```dts

节点名@设备地址 {
    属性名 = 属性值;
    属性名;                    /* 布尔属性，存在即为真 */
};

node@address {
	compatible = "";
}
```

节点命名格式：`名称@地址`，地址部分用于区分同类设备的不同实例。

```dts
serial@12000 { };        /* UART设备，地址0x12000 */
i2c@11000 { };           /* I2C控制器，地址0x11000 */
ethernet@20000 { };       /* 以太网，地址0x20000 */
```

### 2.2 节点标签 (Label)

标签是为节点起的短别名，方便其他地方通过 `&标签名` 引用。

```dts
uart0: serial@12000 { };    /* uart0 是标签，serial@12000 是节点名 */
/*         ↑                  后续可用 &uart0 引用此节点       */
```

格式：`标签: 节点名@地址`

### 2.3 引用已有节点 (覆盖/追加)

```dts
&uart0 {                      /* 引用之前定义的uart0节点，追加或覆盖属性 */
    status = "okay";          /* 覆盖原有status */
    pinctrl-names = "default"; /* 追加新属性 */
};
```

这是 DTS 分层设计的核心机制：
- `.dtsi` 文件：芯片级基础定义（所有节点及默认属性）
- `.dts` 文件：板级定制（用 `&标签` 追加/覆盖属性）

---

## 3. 属性 (Property)

### 3.1 字符串

```dts
compatible = "marvell,armada3700";                    /* 单个字符串 */
compatible = "marvell,ac5-sdhci", "marvell,armada-ap806-sdhci";  /* 多个字符串，逗号分隔 */
```

### 3.2 整数 (32位)

```dts
clock-frequency = <0x138ce200>;    /* 十六进制 */
ngpios = <0x2e>;                    /* 十六进制，0x2e=46 */
bus-width = <0x8>;                  /* 十进制也可: <8> */
```

尖括号 `<>` 内的多个值用空格分隔，每个值占 32 位（1 个 cell）：

```dts
interrupts = <0x0 0x53 0x4>;       /* 3个cell: 中断类型、中断号、触发方式 */
```

### 3.3 64位整数（用2个cell）

```dts
reg = <0x0 0x40000000 0x0 0x40000000>;
/*    高位  低位      高位  低位       */
/*    ----地址----    ----大小----     */
```

cell 数量由 `#address-cells` 和 `#size-cells` 决定。

### 3.4 字节数组 (bytestring)

```dts
mac-address = [00 11 22 33 44 55];    /* 16进制，每个字节用空格分隔 */
local-mac-address = [000011223344];    /* 也可以连写 */
```

方括号 `[]` 表示原始字节数组。

### 3.5 字符串数组

```dts
clock-names = "core", "periph";       /* 逗号分隔多个字符串 */
```

### 3.6 布尔属性

```dts
no-map;              /* 存在即为真，不需要赋值 */
non-removable;       /* 存在=不可移除 */
dma-coherent;        /* 存在=DMA一致性 */
full-duplex;         /* 存在=全双工 */
```

### 3.7 属性引用 (phandle)

```dts
clocks = <0x3>;                    /* 用数字引用phandle=0x3的节点 */
interrupt-parent = <&gic>;         /* 用标签引用gic节点，更推荐 */
clocks = <&spi_clock>;             /* 用标签引用 */
```

### 3.8 混合类型

```dts
reg = <0x0 0x805c0000 0x0 0x300>;  /* 纯整数数组 */
ranges = <0x82000000 0x0 0x30000000 0x0 0x30000000 0x0 0xffff0000>;
```

---

## 4. 核心属性说明

### 4.1 compatible - 驱动匹配

```dts
compatible = "厂商,设备";
compatible = "marvell,ac5-sdhci", "marvell,armada-ap806-sdhci";
/* 多个compatible按顺序匹配，先匹配更具体的，再匹配通用的 */
```

内核驱动通过 `of_device_id` 表中的 `compatible` 字符串与设备匹配。

### 4.2 reg - 寄存器地址

```dts
reg = <地址 大小>;                          /* #address-cells=1, #size-cells=1 */
reg = <地址高位 地址低位 大小高位 大小低位>;    /* #address-cells=2, #size-cells=2 */
reg = <地址1 大小1 地址2 大小2>;              /* 多组寄存器 */
```

cell 数量由父节点的 `#address-cells` 和 `#size-cells` 决定。

### 4.3 #address-cells / #size-cells

```dts
#address-cells = <0x2>;    /* 子节点reg中地址占2个cell (64位) */
#size-cells = <0x2>;        /* 子节点reg中大小占2个cell (64位) */
#size-cells = <0x0>;        /* 子节点reg中不需要大小字段 (如CPU节点) */
```

### 4.4 ranges - 地址映射

```dts
ranges;                                              /* 空=1:1映射，子地址=父地址 */
ranges = <子地址 父地址 大小>;                          /* 一段映射 */
ranges = <子地址1 父地址1 大小1 子地址2 父地址2 大小2>; /* 多段映射 */
```

每个字段的 cell 数：
- 子地址：本节点的 `#address-cells`
- 父地址：父节点的 `#address-cells`
- 大小：本节点的 `#size-cells`

### 4.5 interrupts - 中断

```dts
interrupts = <中断类型 中断号 触发方式>;   /* #interrupt-cells=3 (GIC) */
```

ARM GICv3 的中断类型：
- `0x0` = SPI（共享外设中断），大多数外设用这个
- `0x1` = PPI（私有外设中断），定时器、PMU 等用这个

触发方式：
- `0x1` = 上升沿触发
- `0x2` = 下降沿触发
- `0x4` = 高电平触发
- `0x8` = 低电平触发

### 4.6 status - 设备状态

```dts
status = "okay";       /* 启用设备 */
status = "disabled";   /* 禁用设备 */
status = "fail";       /* 设备不可用（硬件故障） */
```

### 4.7 phandle - 引用句柄

```dts
phandle = <0x3>;      /* 数字ID，其他节点可通过此ID引用 */
```

推荐用标签+`&标签` 代替硬编码数字，避免冲突。

---

## 5. 特殊节点

### 5.1 chosen - 启动参数

```dts
chosen {
    stdout-path = "serial0:9600n8";   /* 控制台输出设备 */
    bootargs = "console=ttyS0 root=/dev/mmcblk0p2";  /* 内核启动参数 */
    linux,initrd-start = <0x44200000>; /* initrd起始地址 */
    linux,initrd-end = <0x44801D1B>;   /* initrd结束地址 */
};
```

### 5.2 aliases - 别名

```dts
aliases {
    serial0 = "/soc/serial@12000";  /* 为设备路径定义短名 */
    ethernet0 = &eth0;               /* 也可用标签引用 */
};
```

### 5.3 memory - 物理内存

```dts
memory@00000000 {
    device_type = "memory";
    reg = <0x0 0x40000000 0x0 0x40000000>;  /* 起始地址 + 大小 */
};
```

### 5.4 cpus - CPU拓扑

```dts
cpus {
    #address-cells = <0x2>;
    #size-cells = <0x0>;

    cpu-map {               /* CPU拓扑映射 */
        cluster0 {
            core0 { cpu = <&CPU0>; };
            core1 { cpu = <&CPU1>; };
        };
    };

    CPU0: cpu@0 {
        device_type = "cpu";
        compatible = "arm,armv8";
        reg = <0x0 0x0>;            /* MPIDR值 */
        enable-method = "psci";
        next-level-cache = <&L2_0>; /* 共享L2缓存 */
    };

    L2_0: l2-cache0 {
        compatible = "cache";
    };
};
```

### 5.5 reserved-memory - 保留内存

```dts
reserved-memory {
    #address-cells = <0x2>;
    #size-cells = <0x2>;
    ranges;

    buffer@0x40000000 {
        compatible = "shared-dma-pool";
        no-map;                                          /* 不做线性映射 */
        reg = <0x0 0x40000000 0x0 0x40000000>;          /* 保留区域 */
    };
};
```

---

## 6. 注释

```dts
/* 块注释，支持多行 */
/* 推荐：模块说明用多行长注释 */

属性 = <0x1>; /* 行尾短注释，推荐：单行属性用行尾注释 */

// C++风格行注释，也支持但不常用
```

---

## 7. 文件组织

| 文件后缀 | 用途 | 说明 |
|---|---|---|
| `.dtsi` | 通用硬件定义 | 类似头文件，定义芯片级基础节点 |
| `.dts` | 板级设备树 | 包含 `.dtsi`，用 `&标签` 做板级定制 |
| `.h` | C头文件 | 定义宏常量，供 `#include` 引入 |

典型分层：

```
imx6ull.dtsi              ← 芯片级：定义所有节点及默认属性
  └── imx6ull-mmc-npi.dts ← 板级：#include dtsi，用&覆盖/追加属性
```

---

## 8. 编译

```bash
# DTS → DTB (Device Tree Blob，二进制设备树)
dtc -I dts -O dtb -o output.dtb input.dts

# DTB → DTS (反编译)
dtc -I dtb -O dts -o output.dts input.dtb

# 带依赖包含的编译
dtc -I dts -O dtb -o output.dtb input.dts -i include/
```

---

## 9. 常见约定

| 约定 | 示例 | 说明 |
|---|---|---|
| compatible格式 | `"厂商,设备"` | 小写，逗号分隔厂商和设备名 |
| 节点名格式 | `名称@地址` | 地址用十六进制 |
| 标签大写 | `CPU0: cpu@0` | 标签推荐大写 |
| phandle从1开始 | `phandle = <0x1>` | 避免重复，推荐用标签替代 |
| disabled为默认 | `status = "disabled"` | dtsi中默认禁用，dts中按需启用 |



# 10.dtbo

新增设备节点的用法



```
/dts-v1/	/**/
/plugin/

/ {
	fragment@0 {
		target = <>;
		interrupts = <中断类型 中断号 触发类型>;
	}
}
```

