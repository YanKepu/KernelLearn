# 协议讲解





## 具体实现



#### start/stop函数

```
参数解析：
ulDataBit: 数据引脚
ulClockBit: 时钟引脚
ulDelayTime: 延时时间
enI2cBus: i2c总线

#define DRV_SYSM_I2C_CpuGpioStart(ulDataBit, ulClockBit, ulDelayTime, enI2cBus)\
	DRV_I2C_DELAY((3 * ulDelayTime));\          /* 稳定在高电平 */
	I2C_SDA_SET(ulDataBit, enI2cBus);\          /* 拉高sda */
	I2C_SCL_SET(ulClockBit, enI2cBus);\         /* 拉高scl */
	DRV_I2C_DELAY(ulDelayTime);\
	I2C_SDA_RESET(ulDataBit, enI2cBus);\
	DRV_I2C_DELAY(ulDelayTime);\
	I2C_SCL_RESET(ulClockBit, enI2cBus)
	
#define DRV_SYSM_I2C_CpuGpioStop(ulDataBit, ulClockBit, ulDelayTime, enI2cBus)\
	I2C_SDA_RESET(ulDataBit, enI2cBus);\       /* 数据拉低 */
	DRV_I2C_DELAY(ulDelayTime);\
	I2C_SCL_SET(ulClockBit, enI2cBus);\
	DRV_I2C_DELAY(ulDelayTime);\
	I2C_SDA_SET(ulDataBit, enI2cBus);\
	SET_DIRCTION_IN(ulDataBit, enI2cBus);\
	SET_DIRCTION_IN(ulClockBit, enI2cBus)
	
	

```



#### 基础宏函数

```
#define CPU_SYS_CONFIG_GPIO_BASE	0x7F018100
g_ulGpioVirAddr = (ULONG)(ioremap_nocache((ULONG)CPU_SYS_CONFIG_GPIO_BASE, (ULONG)0x100)); /* 将GPIO的基地址和大小映射到虚拟地址空间 */

/* 前32位GPIO */
#define GPIO_DATA_OUT_REG           (g_ulGpioVirAddr)/*  写数据寄存器 */ /* 读数据寄存器 */
#define GPIO_DATA_OUT_EN_REG        (g_ulGpioVirAddr + 0x04)/* 输出使能控制位 0:Output 1:Input */
#define GPIO_DATA_IN_POL_REG        (g_ulGpioVirAddr + 0x0c)/* 输入极性反转寄存器：0:不反转 1:反转 */
#define GPIO_DATA_IN_REG            (g_ulGpioVirAddr + 0x10)/* 输入数据只读寄存器 */

/* 超过32比特GPIO，n:GPIO_NUM 跨度为0x40 */
#define GPIO_32_DATA_OUT_REG        (g_ulGpioVirAddr + 0x40)/*  写数据寄存器 */ /* 读数据寄存器 */
#define GPIO_32_DATA_OUT_EN_REG     (g_ulGpioVirAddr + 0x04+0x40)/* 设置GPIO各管脚的数据传输方向 0:Input 1:output*/
#define GPIO_32_DATA_IN_POL_REG		(g_ulGpioVirAddr + 0x0c+0x40)	/* 读数据寄存器 */
#define GPIO_32_DATA_IN_REG			(g_ulGpioVirAddr + 0x10+0x40)	/* 读数据寄存器 */

#define DRV_I2C_DELAY(Count) \
do\
{\
	iProc_udelay((UINT)Count);\
}while(BOOL_FALSE)


#define I2C_SCL_RESET(ulClockBit, enI2cBus)\
    SET_DIRCTION_OUT(ulClockBit, enI2cBus);\
    I2C_GPIO_BIT_SET_LOW(ulClockBit, enI2cBus)

#define I2C_SCL_SET(ulClockBit, enI2cBus)\
    SET_DIRCTION_OUT(ulClockBit, enI2cBus);\
    I2C_GPIO_BIT_SET_HIGH(ulClockBit, enI2cBus)
    
#define I2C_SDA_RESET(ulDataBit, enI2cBus)\
    SET_DIRCTION_OUT(ulDataBit, enI2cBus);\
    I2C_GPIO_BIT_SET_LOW(ulDataBit, enI2cBus)
    
#define I2C_SDA_SET(ulDataBit, enI2cBus)\
	SET_DIRCTION_OUT(ulDataBit, enI2cBus);\
	I2C_GPIO_BIT_SET_HIGH(ulDataBit, enI2cBus)
	
#define I2C_SCL_GET(ulClockBit, ulVal, enI2cBus)\
    SET_DIRCTION_IN(ulClockBit, enI2cBus);\
    I2C_GPIO_BIT_GET(ulClockBit, ulVal, enI2cBus)

#define I2C_SDA_GET(ulDataBit, ulVal, enI2cBus)\
    SET_DIRCTION_IN(ulDataBit, enI2cBus);\
    I2C_GPIO_BIT_GET(ulDataBit, ulVal, enI2cBus)
    
    
#define SET_DIRCTION_IN(Bit, enI2cBus)\
do\
{\
     UINT uiRegVal;\
     UINT uiGpio32;\
     (VOID)enI2cBus;\
    {\
        uiGpio32 = Bit/32;\
        if(uiGpio32)\
        {\
            uiRegVal = (*(volatile UINT*)(ULONG)GPIO_32_DATA_OUT_EN_REG);\
            uiRegVal |= (1U << (Bit%32));\
            (*(volatile UINT*)(ULONG)GPIO_32_DATA_OUT_EN_REG) = uiRegVal;\
        }\
        else\
        {\
            uiRegVal = (*(volatile UINT*)(ULONG)GPIO_DATA_OUT_EN_REG);\
            uiRegVal |= (1U<< Bit);\
            (*(volatitle UINT*)(ULONG)GPIO_DATA_OUT_EN_REG) = uiRegVal;\
        }\
    }\
}while(BOOL_FALSE)

/* 设置管脚为输出 */
#define SET_DIRCTION_OUT(Bit, enI2cBus)\
do\
{\
     UINT uiRegVal;\
     UINT uiGpio32;\
     (VOID)enI2cBus;\
    {\
        uiGpio32 = Bit/32;\
        if(uiGpio32)\
        {\
            uiRegVal = (*(volatile UINT*)(ULONG)GPIO_32_DATA_OUT_EN_REG);\
            uiRegVal &= ~(1U << (Bit%32));\     /* 对应位清零 */
            (*(volatile UINT*)(ULONG)GPIO_32_DATA_OUT_EN_REG) = uiRegVal;\
        }\
        else\
        {\
            uiRegVal = (*(volatile UINT*)(ULONG)GPIO_DATA_OUT_EN_REG);\
            uiRegVal &= ~(1U<< Bit);\
            (*(volatitle UINT*)(ULONG)GPIO_DATA_OUT_EN_REG) = uiRegVal;\
        }\
    }\
}while(BOOL_FALSE)
```

### 选择总线
根据刚开始注册的总线，选择对应的，这个做法是不同的单板，可能管脚是不同的，所以需要根据总线选择对应的管脚
```
#define GPIOBUS_TO_BIT(enI2cBus, ulDataLink, ulClockLink)\
do\
{\
    if(I2C_GPIOBUS_0 == enI2cBus)\
    {\
        ulClockLink = GPIO_I2C0_SCL_BIT;\
		ulDataLink = GPIO_I2C0_SDA_BIT;\
    }\
	else if(I2C_GPIOBUS_1 == enI2cBus)\
	{\
		ulClockLink = GPIO_I2C1_SCL_BIT;\
		ulDataLink = GPIO_I2C1_SDA_BIT;\
	}
    else if(I2C_GPIOBUS_2 == enI2cBus)\
	{\
		ulClockLink = GPIO_I2C2_SCL_BIT;\
		ulDataLink = GPIO_I2C2_SDA_BIT;\
	}
	else\
	{\
		return ERROR_INVALID_PARAMETER\
	}\
}while(BOOL_FALSE)
```