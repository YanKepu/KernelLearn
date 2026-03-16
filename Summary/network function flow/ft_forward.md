# FT forward功能测试

用户态接口：

内核态接口：



```c
DRV_FT_PortForwardCheck		// 对同一组速率的端口做转发检测
    // 获取mac&phy速率
   
    // for 对端口做轮询，起始端口是startport ，最终端口是 startport + portmax - 1
    	mac端口速率获取
    	// if带phy的前兆口，获取phy的速率和软表中获取的速率对比，不通则
    
    	DRV_FT_FwdVlanSet	// 端口转发测试的VLAN和端口配置
    		/* vlan策略：cpu端口和端口0设置为VLAN 100， 端口1和端口2设置为VLAN 101
            			端口3和端口4设置为VLAN 102， 端口5和端口6为VLAN 103.
            			最后一个端口和cpu端口设置为VLAN 100 + loopbackportnum/2 */
    	
    
    	port_BMP:是一种位图，具体的定义如下：
        typedef struct{
            GT_U32 ports[CPSS_MAX_PORTS_BMP_NUM_CNS];   // 大小为32
        } CPSS_PORT_BMP_STC; 

		#define DRV_PBMP_S	CPSS_PORTS_BMP_STC

		设置位图 for 循环，增加每个轮询的端口到位图中

		DRV_FT_TxByRawMode	// 利用发包，这个收包数量
            DRV_FT_Tx
            
        DRV_FT_LoopBackPktInit	// 初始化要发送的环回报文
    
    	port1设置VLANID
    	port2设置VLANID
            
DRV_FT_Tx		// NI内部函数用于FT检测，使用raw模式发送报文
	devnum 是单板ID号
    PdtIntfInfo		参数： 0 0x18 0xb2ac4338 0x64 5 OUT 1
            
    drv_rxtx_tx_rawtransmitforward   // 入参：报文、 报文长度、tx信息、优先级
        DRV_RXTX_IsDbgSndPermit		// 发包debugging notice信息过滤开关，按照报文特性、长度和优先级过滤，入参：tx信息、报文、报文长度，优先级
            DRV_RXTX_DebugToIC
        DRV_RXTX_DebugSendPacket_Ex
        cpssDxChNetIfSdmaTxPacketSend
```











```mermaid
---
title: forward
---
flowchart forward
	DRV_FT_PortForwardCheck[] --> 
```

