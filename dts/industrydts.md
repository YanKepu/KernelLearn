/*
 * Marvell AC5 (Armada 3700) 开发板设备树文件
 *
 * 该文件描述了 Marvell AC5 DB 评估板的硬件拓扑结构，
 * 包括 CPU、内存、总线、外设（UART/I2C/SPI/SDIO/ETH/USB/PCIe/NAND）、
 * 时钟、中断控制器、引脚复用等硬件信息。
 *
 * DTS (Device Tree Source) 是 Linux 内核用于描述不可发现硬件的
 * 数据结构，内核在启动时解析 DTB (Device Tree Blob) 来完成
 * 平台设备的注册和驱动匹配。
 */

/dts-v1/;

/{
    model = "Marvell AC5 DB board"';
    compatible = "marvell,armada3700"; /* 兼容 Marvell Armada 3700 系列 */
    interrupt-parent = <0x1>; /* 全局默认中断父节点，phandle=0x1 指向 GIC */
    #address-cells = <0x2>; /* 地址用2个cell表示 */
    #size-cells = <0x2>; /* 大小用2个cell表示 */

    /*
     * 保留内存区域
     * 这些内存区域不会被 Linux 内存管理器分配给普通应用，
     * 通常用于 DMA 共享内存、固件保留、日志缓冲等特殊用途
     */
    reserved-memory {
        #address-cells = <0x2>;
        #size-cells = <0x2>;
        ranges; /* 地址空间 1:1 映射 */

        /*
         * Prestera 交换芯片保留的 DMA 缓冲区
         * 起始地址: 0x40000000, 大小: 1GB
         * 预留给 Marvell Prestera 以太网交换芯片使用
         */
        prestera_rsvd: buffer@0x40000000 {
            compatible = "shared-dma-pool"; /* 共享 DMA 内存池 */
            no-map; /* 不做内核线性映射，防止被误访问 */
            reg = <0x0 0x40000000 0x0 0x40000000>; /* 地址0x40000000, 大小1GB */
            phandle = <0x2>;
        };

        /*
         * 辅助日志缓冲区保留内存
         * 起始地址: 0x66c00000, 大小: 1MB
         * 用于多核/多系统场景下共享日志
         */
        secondarylogbuf: buffer@0x66c00000 {
            compatible = "shared-dma-pool";
            no-map;
            reg = <0x0 0x66c00000 0x0 0x100000>; /* 地址0x66c00000, 大小1MB */
        };
    };

    /*
     * Marvell DMA 引擎节点
     * 用于管理 SoC 内部的 DMA 传输
     * 注意：phandle=<0x2>与prestera_rsvd冲突，可能是原始文件错误
     */
    mvDma {
        compatible = "marvell,mv_dma";
        phandle = <0x2>;
        status = "okay";
    };

    /*
     * 别名节点
     * 为设备路径定义短名称，方便内核和用户空间引用
     * 例如 /dev/ttyS0 对应 serial0 别名
     */
    aliases {
        serial0 = "/soc/internal-regs@7f000000/serial@12000"; /* 串口0别名 */
        spiflash0 = "/soc/spi@805a0000/spi_flash@0"; /* SPI Flash别名 */
        gpio0 = "/soc/internal-reg@7f000000/gpio@18100"; /* GPIO0别名 */
    };

    /*
     * PSCI (Power State Coordination Interface) 节点
     * ARM 标准电源管理接口，用于 CPU 空闲、挂起、
     * CPU 热插拔及二次启动等操作，遵循 PSCI 0.2 规范
     */
    psci { 
        compatible = "arm,psci-0.2";
        status = "okay";
    };

    /*
     * ARMv8 架构定时器节点
     * CPU 内核自带通用定时器，提供高精度计时
     * 4个中断: EL1物理(1,9) EL1虚拟(1,8) EL2物理(1,10) EL2虚拟(1,7)
     * 格式: <中断类型 中断号 触发方式>, 0x1=PPI, 0x4=上升沿触发
     */
    timer {
        compatible = "arm,armv8-timer";
        interrupts = <0x1 0x9 0x4 0x1 0x8 0x4 0x1 0xa 0x4 0x1 0x7 0x4>;
        status = "okay";
    };

    /*
     * PMU (Performance Monitoring Unit) 节点
     * ARMv8 性能监视单元，用于硬件性能计数（周期数、指令数、缓存命中率等）
     * armv8-pmuv3 表示 ARMv8 PMU 架构版本3
     */
    pmu {
        compatible = "arm,armv8-pmuv3";
        status = "okay";
    };

    /*
     * SoC (System on Chip) 总线节点
     * 描述 SoC 内部总线拓扑和所有外设
     * simple-bus 表示简单总线，内核会遍历子节点并注册设备
     */
    soc {
        compatible = "simple-bus";
        #address-cells = <0x2>;
        #size-cells = <0x2>;
        ranges; /* 子节点地址与父节点 1:1 映射 */
        dma-ranges; /* DMA 地址空间与 CPU 地址空间 1:1 映射 */
        
        /*
         * 内部寄存器区域
         * 映射物理地址 0x7f000000 开始的 16MB 空间
         * 包含 UART、MDIO、I2C、GPIO 等低速外设
         */
        internal-regs@7f000000 {
            #address-cells = <0x2>;
            #size-cells = <0x2>;
            compatible = "simple-bus";
            ranges = <0x0 0x0 0x7f000000 0x1000000>; /* 子地址0映射到父地址0x7f000000, 大小16MB */
            dma-coherent; /* DMA操作与CPU缓存一致性由硬件保证 */

            /*
             * UART0 串口节点
             * Synopsys DesignWare APB UART，用于系统控制台输出
             */
            uart0: serial@12000 {
                compatible = "snps,dw-apd-uart"; /* Synopsys DesignWare APB Debug UART */
                reg = <0x12000 0x100>; /* 基地址0x12000, 大小256字节 */
                reg-shift = <0x2>; /* 寄存器间隔 2^2=4 字节 */
                interrupts = <0x0 0x53 0x4>; /* SPI中断, 中断号83, 上升沿触发 */
                reg-io-width = <0x1>; /* 寄存器访问宽度1字节 */
                clock-frequency = <0x138ce200>; /* 时钟频率≈327MHz */
                status = "okay";
            };

            /*
             * MDIO 总线控制器节点
             * MDIO 是用于配置 PHY 芯片的标准总线，基地址0x22004
             */
            mdio: mdio@20000 {
                #address-cells = <0x1>;
                #size-cells = <0x1>;
                compatible = "marvell, orion-mdio";
                reg = <0x22004 0x4>; /* 基地址0x22004, 大小4字节 */
                clocks = <0x3>; /* 引用 core_clock (phandle=0x3) */

                /*
                 * 以太网 PHY 设备，PHY 地址为 0
                 */
                phy0: ethernet-phy@0 {
                    reg = <0x0 0x0>; /* MDIO总线上的PHY地址0 */
                };
            };

            /*
             * I2C0 控制器节点
             * Marvell mv78230 兼容 I2C 控制器，基地址0x11000
             */
            i2c0: i2c@11000 {
                compatible = "marvell,mv78230-i2c";
                reg = <0x11000 0x20>; /* 基地址0x11000, 大小32字节 */
                clocks = <0x3>;
                clock-names = "core";
                interrupts = <0x0 0x57 0x4>; /* 中断号87 */
                clock-frequency = <0x186a0>; /* 总线频率100kHz (标准模式) */
                status = "okay";
                pinctrl-names = "i2c-mpp-state" /* 引脚复用状态名称 */
            };

            /*
             * I2C1 控制器节点（第二路 I2C）
             * 注意: reg中地址为0x1100，可能原始文件有误，应为0x11100
             */
            i2c1: i2c@11100 {
                compatible = "marvell,mv78230-i2c";
                reg = <0x1100 0x20>; /* 基地址0x1100, 大小32字节 */
                clocks = <0x3>;
                clock-names = "core";
                interrupts = <0x0 0x58 0x4>; /* 中断号88 */
                clock-frequency = <0x186a0>; /* 总线频率100kHz */
                status = "okay";
                pinctrl-names = "i2c-mpp-state"
            };

            /*
             * GPIO0 控制器节点
             * Marvell AC5 GPIO 控制器，46个GPIO引脚
             */
            gpio0: gpio@18100 {
                compatible = "marvell,ac5-gpio";
                #gpio-cells = <0x1>;
                gpio-controller; /* 声明为GPIO控制器 */
                reg = <0x18100 0x200>; /* 基地址0x18100, 大小512字节 */
                ngpios = <0x2e>; /* 46个GPIO引脚 */
                phandle = <0x6>;
            };
        };

        /*
         * MMC DMA 外设区域
         * 包含 eMMC/SD 控制器，支持 DMA 一致性操作
         */
        mmc_dma: mmc-dma-peripherals@80500000 {
            compatible = "simple-bus";
            #address-cells = <0x2>;
            #size-cells = <0x2>;
            ranges;
            dma-coherent;

            /*
             * SDHCI eMMC 控制器节点
             * 兼容 Marvell AC5 SDHCI 和 Armada AP806 SDHCI 驱动
             */
            sdhci0: sdhci@805c0000 {
                compatible = "marvell,ac5-sdhci", "marvell,armada-ap806-sdhci"; /* 按顺序匹配驱动 */
                reg = <0x0 0x805c0000 0x0 0x300>; /* 控制寄存器基地址0x805c0000, 大小768字节 */
                reg-names = "ctrl", "decoder";
                interrupts = <0x0 0x5c 0x4>; /* 中断号92 */
                clocks = <0x3>;
                clock-names = "core";
                status = "okay";
                bus-width = <0x8>; /* 8位数据总线 (eMMC) */
                non-removable; /* 不可移除（焊接在板上的eMMC） */
                mmc-ddr-1_8v; /* 支持1.8V DDR模式 */
                mmc-hs200-1_8v; /* 支持HS200模式 (200MHz, 1.8V) */
                mmc-hs400-1_8v; /* 支持HS400模式 (200MHz DDR, 1.8V)，最高性能 */
            };
        };

        /*
         * 32位 DMA 外设区域
         * 包含以太网、USB 等需要 DMA 访问的外设
         * 使用64位地址格式 (#address-cells=2, #size-cells=2)
         */
        32bit-dma-peripherals@7f000000 {
            compatible = "simple-bus";
            #address-cells = <0x2>; /* 64位地址格式 */
            #size-cells = <0x2>;
            ranges = <0x0 0x0 0x0 0x7f000000 0x0 0x1000000>; /* 子地址0映射到父地址0x7f000000, 大小16MB */
            /*
             * DMA 地址映射: CPU地址0x0映射到DMA地址0x2_00000000, 大小4GB
             * 表示DMA地址空间有2GB偏移
             */
            dma-ranges = <0x0 0x0 0x2 0x0 0x1 0x0>;
            dma-coherent;

            /*
             * 以太网口0 (eth0)
             * Marvell Armada AC5 NETA 以太网控制器
             * 使用 SGMII 接口连接 PHY，速率1Gbps
             */
            eth0: ethernet@20000 {
                compatible = "marvell,armada-ac5-neta";
                reg = <0x0 0x20000 0x0 0x4000>; /* 基地址0x20000, 大小16KB */
                interrupts = <0x0 0x2d 0x4>; /* 中断号45 */
                clocks = <0x3>;
                status = "okay";
                index = <0x00>; /* 网口索引号0 */
                phy-mode = "sgmii"; /* 使用SGMII接口连接PHY */

                /*
                 * 固定链路配置
                 * 不使用自动协商，直接配置速率和双工模式
                 */
                fixed-link {
                    speed = <0x3e8>; /* 1Gbps */
                    full-duplex; /* 全双工 */
                };
            };

            /*
             * 以太网口1 (eth1)
             * 第二个以太网控制器，SGMII接口，速率100Mbps
             */
            eth1: ethernet@24000 {
                compatible = "marvell,armada-ac5-neta";
                reg = <0x0 0x24000 0x0 0x4000>; /* 基地址0x24000, 大小16KB */
                interrupts = <0x0 0x37 0x4>; /* 中断号55 */
                clocks = <0x3>;
                status = "okay";
                index = <0x01>; /* 网口索引号1 */
                phy-mode = "sgmii";

                fixed-link {
                    speed = <0x64>; /* 100Mbps */
                    full-duplex;
                };
            };

            /*
             * USB PHY 节点
             * usb-nop-xceiv 是虚拟/空 USB 收发器驱动
             * 当板载 PHY 不需要额外配置时使用
             */
            usb1phy: usbphy {
                compatible = "usb-nop-xceiv";
                #phy-cells = <0x0>; /* 引用此PHY不需要额外参数 */
                phandle = <0x7>;
            };

            /*
             * USB0 节点，使用虚拟 USB 收发器
             */
            usb0: usb@80000 {
                compatible = "usb_nop_xceiv";
                phandle = <0x7>;
            };

            /*
             * USB1 EHCI 控制器节点
             * Marvell Orion/AC5 EHCI (USB 2.0 增强主机控制器)
             */
            usb1: usb@A0000 {
                compatible = "marvell,orion-ehci", "marvell,ac5-ehci";
                reg = <0x0 0xa0000 0x0 0x500>; /* 基地址0xa0000, 大小1280字节 */
                interrupts = <0x0 0x54 0x4>; /* 中断号84 */
                status = "okay";
            }
        };

        /*
         * 引脚复用控制器 (Pin Controller) 节点
         * 控制 SoC 引脚的功能复用（GPIO/I2C/SPI等功能切换）
         * syscon: 可通过系统控制器寄存器访问
         * simple-mfd: 简单多功能设备，子节点为独立功能块
         */
        pinctrl0: pinctrl@80020100 {
            compatible = "marvell,ac5-pinctrl", "syscon","simple-mfd";
            reg = <0x0 0x80020100 0x0 0x20>; /* 基地址0x80020100, 大小32字节 */

            /*
             * I2C 引脚复用配置 - I2C 功能
             * 将 mpp26、mpp27 配置为 I2C0 可选功能
             */
            i2c_mpps: i2c-mpps {
                marvell,pins = "mpp26", "mpp27"; /* 使用MPP26和MPP27引脚 */
                marvell,function = "i2c0-opt"; /* 复用为I2C0可选功能 */
                phandle = <0x4>;
            };
            
            /*
             * I2C 引脚复用配置 - GPIO 功能
             * 将 mpp26、mpp27 配置为 GPIO 模式
             */
            i2c_gpio: i2c-gpio {
                marvell,pins = "mpp26", "mpp27";
                marvell,function = "gpio"; /* 复用为GPIO功能 */
                phandle = <0x5>;
            };
        };


        /*
         * PCIe 控制器节点
         * Marvell AC5 PCIe 控制器，兼容 Synopsys DesignWare PCIe IP
         * 用于连接外部 PCIe 设备（网卡、SSD等）
         */
        pcie0: pcie@800a0000 {
            compatible = "marvell,ac5-pcie", "snps,dw-pcie";
            reg = <0x0 0x800a0000 0x0 0x20000 0x0 0x3fff0000 0x0 0x10000>; /* ctrl:128KB + config:64KB */
            reg-names = "ctrl", "config";
            #address-cells = <0x3>; /* PCIe地址含总线号/设备号/功能号，3个cell */
            #size-cells = <0x2>;
            #interrupt-cells = <0x1>; /* 中断用1个cell */
            device_type = "pci";
            dma-coherent;
            bus-range = <0x0 0xff>; /* PCI总线号0~255 */
            ranges = <0x82000000 0x0 0x30000000 0x0 0x30000000 0x0 0xffff0000>; /* 非预取内存空间映射 */
            interrupt-map-mask = <0x0 0x0 0x0 0x1>; /* 中断映射掩码，只匹配中断引脚位 */
            /*
             * 中断映射: PCIe INTA映射到GIC的SPI 99 (0x63)
             * 格式: <pcie_addr_slot pin parent_phandle parent_addr parent_irq trigger>
             */
            interrupt-map = <0x0 0x0 0x0 0x1 0x1 0x0 0x63 0x4>;
            interrupts = <0x0 0x63 0x4>; /* PCIe控制器自身中断: SPI 99 */
            num-lanes = <0x1>; /* 单lane (x1) */
            status = "okay";
            clocks = <0x3>;
        };

        /*
         * 核心时钟节点
         * 固定频率400MHz，作为SoC核心时钟源
         * 被多个外设引用 (phandle=0x3)
         */
        core_clock: core_clock@0 {
            compatible = "fixed-clock";
            #clock-cells = <0x0>; /* 引用此时钟不需要额外参数 */
            clock-frequency = <0x17d78400>; /* 400MHz */
            phandle = <0x3>;
        };

        /*
         * AXI 总线时钟节点
         * 固定频率325MHz，用于AXI总线互联
         */
        axi_clock: axi_clock@0 {
            compatible = "fixed-clock";
            #clock-cells = <0x0>;
            clock-frequency = <0x135f1b40>; /* 325MHz */
        };

        /*
         * SPI 时钟节点
         * 固定频率200MHz，作为SPI控制器时钟源
         */
        spi_clock: spi_clock@0 {
            compatible = "fixed-clock";
            #clock-cells = <0x0>;
            clock-frequency = <0xbebc200>; /* 200MHz */
            phandle = <0x8>;
        };

        /*
         * SPI0 控制器节点
         * Marvell Armada 3700 兼容 SPI 控制器，基地址0x805a0000
         */
        spi@805a0000 {
            compatible = "marvell,armada-3700-spi";
            reg = <0x0 0x805a0000 0x0 0x50>; /* 基地址0x805a0000, 大小80字节 */
            #address-cells = <0x1>; /* 子设备地址1个cell */
            #size-cells = <0x0>; /* 子设备不需要大小 */
            clocks = <0x8>; /* 引用spi_clock (phandle=0x8) */
            interrupts = <0x0 0x5a 0x4>; /* 中断号90 */
            num-cs = <0x1>; /* 1个片选信号 */
            status = "okay";

            /*
             * SPI NOR Flash 设备
             * 连接在SPI总线上的NOR Flash芯片
             */
            spiflash0: spi-flash@0 {
                compatible = "spi-nor"; /* Linux SPI NOR Flash 通用驱动 */
                spi-max-frequency = <0x2faf080>; /* 最大频率50MHz */
                spi-tx-bus-width = <0x1>; /* 发送1位 (单线模式) */
                spi-rx-bus-width = <0x1>; /* 接收1位 (单线模式) */
                reg = <0x0>; /* 片选编号0 */
                #address-cells = <0x1>;
                #size-cells = <0x1>;

                /*
                 * SPI Flash 分区表
                 * 将16MB SPI Flash划分为3个分区
                 */
                partition@0 {
                    label = "spi_flash_part0";
                    reg = <0x0 0x800000>; /* 偏移0, 大小8MB */
                };

                parition@1 {
                    label = "spi_flash_part1";
                    reg = <0x800000 0x700000>; /* 偏移8MB, 大小7MB */
                };

                parition@2 {
                    label = "spi_flash_part2";
                    reg = <0xf00000 0x100000>; /* 偏移15MB, 大小1MB */
                };
            };
        };

        /*
         * SPI1 控制器节点（第二路 SPI）
         * 基地址0x805a8000，默认禁用
         */
        spi@805a8000 {
            compatible = "marvell,armada-3700-spi";
            reg = <0x0 0x805a8000 0x0 0x50>; /* 基地址0x805a8000, 大小80字节 */
            #address-cells = <0x1>;
            #size-cells = <0x0>;
            clocks = <0x8>;
            interrupts = <0x0 0x5b 0x4>; /* 中断号91 */
            num-cs = <0x1>; /* 1个片选信号 */
            status = "disabled"; /* 默认禁用 */
        };

        /*
         * NAND Flash 控制器节点
         * Marvell AC5 NAND 控制器，支持硬件 ECC
         * 多组寄存器: 控制寄存器(0x805b0000) + 配置寄存器(0x840f8204, 0x80013010)
         */
        nand@805b0000 {
            compatible = "marvell,ac5-nand-controller";
            reg = <0x0 0x805b0000 0x0 0x54 0x0 0x840f8204 0x0 0x4 0x0 0x80013010 0x0 0x20>;
            #address-cells = <0x1>;
            #size-cells = <0x0>;
            marvell,nand-enable-arbiter; /* 启用NAND仲裁器 */
            num-cs = <0x1>; /* 1个片选信号 */
            interrupts = <0x0 0x59 0x4>; /* 中断号89 */
            clocks = <0x3>;
            status = "okay";

            /*
             * NAND Flash 芯片子节点
             * 描述连接在NAND控制器上的Flash芯片参数
             */
            nand@0 {
                reg = <0x0>; /* 片选编号0 */
                label = "main-storage";
                nand-rb = <0x0>; /* 使用Ready/Busy引脚0 */
                nand-ecc-mode = "hw"; /* 硬件ECC */
                nand-ecc-strength = <0xc>; /* 每ECC步长可纠正12位错误 */
                nand-ecc-step-size = <0x200>; /* ECC步长512字节 */
            };
        };

        /*
         * Prestera 以太网交换芯片节点
         * Marvell Armada AC5 交换芯片驱动，中断号35
         */
        prestera {
            compatible = "marvell,armada-ac5-switch";
            interrupts = <0x0 0x23 0x4>; /* 中断号35 */
            status = "okay";
        };

        /*
         * GPIO 不可屏蔽中断 (NMI) 节点 0
         * H3C 自定义的 GPIO 中断设备，中断号78
         */
        gpio-nmi{
            compatible = "H3C,interrupt-gpio";
            interrupt-parent = <&gic>; /* 显式指定中断父节点为GIC */
            interrupts = <0x0 0x4e 0x4>; /* 中断号78 */
        };

        /*
         * GPIO 不可屏蔽中断 (NMI) 节点 1
         * 第二个 H3C GPIO 中断设备，中断号77
         */
        gpio-nmi1{
            compatible = "H3C,interrupt-gpio";
            interrupt-parent = <&gic>;
            interrupts = <0x0 0x4d 0x4>; /* 中断号77 */
        };
    };

    /*
     * GIC (Generic Interrupt Controller) v3 节点
     * ARM GICv3 中断控制器，管理所有外设中断的分配和路由
     * 被全局 interrupt-parent=<0x1> 引用
     */
    gic: interrupt-controller@80600000 {
        compatible = "arm,gic-v3";
        #interrupt-cells = <0x3>; /* 中断描述用3个cell */
        interrupt-controller; /* 声明为中断控制器 */
        redistributor-stride = <0x0 0x20000>; /* Redistributor步长128KB */
        reg = <0x0 0x80600000 0x0 0x10000 0x0 0x80660000 0x0 0x40000>; /* GICD:64KB + GICR:256KB */
        interrupts = <0x1 0x6 0x4>; /* GIC维护中断: PPI 6 */
        phandle = <0x1>;
    };

    /*
     * CPU 拓扑节点
     * 描述系统中所有 CPU 核心的配置
     */
    cpus {
        #address-cells = <0x2>; /* ARMv8 MPIDR寄存器格式，2个cell */
        #size-cells = <0x0>; /* CPU不需要大小 */

        /*
         * CPU 拓扑映射
         * 描述 CPU 的缓存和调度拓扑结构
         */
        cpu-map {
            cluster0 { /* 集群0 */
                core0 {
                    cpu = <0x9>; /* CPU0 (phandle=0x9) */
                };
                core1 {
                    cpu = <0xa>; /* CPU1 (phandle=0xa) */
                };
            };
        };

        /*
         * CPU0 核心配置
         * ARMv8 架构，通过 PSCI 启动
         */
        CPU0: cpu@0 {
            device_type = "cpu";
            compatible = "arm,armv8";
            reg = <0x0 0x0>; /* MPIDR值，Affinity Level 0=0 */
            enable-method = "psci"; /* 通过PSCI启动次级CPU */
            next-level-cache = <0xb>; /* L2缓存 (phandle=0xb) */
            phandle = <0x9>;
        };

        /*
         * CPU1 核心配置
         * 双核 SoC 的第二个核心，共享 L2 缓存
         */
        CPU1: cpu@1 {
            device_type = "cpu";
            compatible = "arm,armv8";
            reg = <0x0 0x100>; /* MPIDR值，Affinity Level 1=1 */
            enable-method = "psci";
            next-level-cache = <0xb>; /* 共享L2缓存 */
            phandle = <0xa>;
        };

        /*
         * L2 缓存节点
         * 双核共享的二级缓存
         */
        L2_0: l2-cache0 {
            compatible = "cache";
            phandle = <0xb>;
        };
    };

    /*
     * 物理内存节点
     * 起始地址0x40000000 (1GB偏移处), 大小约1008MB
     * 前1GB (0x0~0x40000000) 被预留给了Prestera交换芯片
     */
    memory@00000000 {
        device_type = "memory";
        reg = <0x0 0x40000000 0x0 0x3E800000>;
    };

    /*
     * chosen 节点
     * 由 bootloader 在启动时传递给内核的配置信息
     * 不对应真实硬件，仅用于内核启动参数传递
     */
    chosen {
        stdout-path = "serial0:9600n8"; /* 标准输出: serial0, 波特率9600, 无校验, 8位数据 */
        /*
         * 内核启动参数 (bootargs):
         *   rdinit=/sbin/scmd          - 指定init进程
         *   cconsole-ttyS0,9600        - 控制台ttyS0, 波特率9600
         *   rootfstype=tmpfs           - 根文件系统类型tmpfs (内存文件系统)
         *   rasdiskstart=1             - RAM disk起始编号
         *   pci=pcie_bus_safe          - PCIe安全模式
         *   cpuidle.off=1              - 禁用CPU idle
         *   loglevel=0                 - 日志级别0 (仅紧急消息)
         *   kdb=off                    - 禁用内核调试器
         *   kgdboc=ttyS0,9600          - KGDB over console
         *   tmpfs_quota=4/5            - tmpfs配额
         *   secondary_log_buf=0x100000@0x66c00000 - 辅助日志缓冲区
         *   squashfs_storage_medium-mmc0 - squashfs存储介质为mmc0
         *   default_hugepagesz=32M     - 默认大页32MB
         *   hugepagesz=32M             - 大页32MB
         *   hugepages=4                - 预分配4个大页
         *   phylink_mii_ioctl_with_page=1 - PHY MII IOCTL分页支持
         *   earlyprintk                - 早期打印
         *   earlycon=uart8250,mmio8,0x7f012000 - 早期控制台
         *   usb_serial_need_match=1    - USB串口匹配
         *   yaffs2multi.yaffs_check_enable=1  - YAFFS2检查使能
         *   yaffs2multi.yaffs_refresh_enable=0 - YAFFS2刷新禁用
         */
        bootargs = "rdinit=/sbin/scmd cconsole-ttyS0,9600 rootfstype=tmpfs rasdiskstart=1 pci=pcie_bus_safe cpuidle.off=1 loglevel=0 kdb=off kgdboc=ttyS0,9600 tmpfs_quota=4/5 secondary_log_buf=0x100000@0x66c00000 squashfs_storage_medium-mmc0 default_hugepagesz=32M hugepagesz=32M hugepages=4 phylink_mii_ioctl_with_page=1 earlyprintk earlycon=uart8250,mmio8,0x7f012000 usb_serial_need_match=1 yaffs2multi.yaffs_check_enable=1 yaffs2multi.yaffs_refresh_enable=0 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
        linux,initrd-start = <0x0000000044200000>; /* initrd起始地址 */
        linux,initrd-end = <0x0000000044801D1B>; /* initrd结束地址 */
    };
};