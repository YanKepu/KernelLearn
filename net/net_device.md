# TCP/IP模型 & OSI模型

应用层： HTTP、FTP、DNS、HTTPS、POP3、DHCP等
传输层：TCP、UDP
网络层：ICMP、IGMP、IP、
网络接口层（链路层、物理层）

## 1、linux网络核心架构
分为三层：用户空间的应用层、内核空间的网络协议栈层，硬件物理层。其中最重要的核心是内核空间的协议栈层。在整个栈按照严格分层设计思想可分为五层：系统调用接口层 --> 协议无关的接口层 --> 网络协议实现层(核心) --> 驱动接口层 --> 驱动程序层。

## 2、内核接收网络包（设备层）
    网卡作为一个硬件接收到网络包。NAPI，

```c
static struct pci_driver ixgb_driver = {
	.name     = ixgb_driver_name,
	.id_table = ixgb_pci_tbl,
	.probe    = ixgb_probe,
	.remove   = ixgb_remove,
	.err_handler = &ixgb_err_handler
};

MODULE_AUTHOR("Intel Corporation, <linux.nics@intel.com>");
MODULE_DESCRIPTION("Intel(R) PRO/10GbE Network Driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRV_VERSION);

#define DEFAULT_MSG_ENABLE (NETIF_MSG_DRV|NETIF_MSG_PROBE|NETIF_MSG_LINK)
static int debug = -1;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

/**
 * ixgb_init_module - Driver Registration Routine
 *
 * ixgb_init_module is the first routine called when the driver is
 * loaded. All it does is register with the PCI subsystem.
 **/

static int __init
ixgb_init_module(void)
{
	pr_info("%s - version %s\n", ixgb_driver_string, ixgb_driver_version);
	pr_info("%s\n", ixgb_copyright);

	return pci_register_driver(&ixgb_driver);
}

```
网卡驱动程序初始化的时候，会调用ixgb_init_module(注册一个驱动ixbg_driver)

## 3 linux内核网络栈涉及3层，
主要是L2  L3  L4。内核栈的任务就是将接收到的数据包从L2传递给L3，接下来，如果网络数据包的目的地为当前设备，linux内核网络栈就将其传递给L4。如果数据包需要进行转发，就将其交给L2进行传输。有可能产生：数据包可能被丢失、可能需要重组数据包、需要计算数据包的校验和等。

## 4 在内核中sk_buff表示一个网络数据包，它由双向链表构成，sk_buff结构表示一个包含报文的入栈或者出栈的数据包。

```c
// include\linux\skbuff.h 表示网络数据包，是一个双向链表

struct sk_buff {
	union {
		struct {
			/* These two members must be first. */
			struct sk_buff		*next;   /* sk_buff是双向链表，指向前进指针 */
			struct sk_buff		*prev;      // 指向前一个sk_buff结构体指针

			union {
                // 收到此报文的网络设备（指定处理分组的网络设备）
				struct net_device	*dev;
				/* Some protocols might use this space to store information,
				 * while device pointer would be NULL.
				 * UDP receive path is one user.
				 */
				unsigned long		dev_scratch;
			};
		};
		struct rb_node		rbnode; /* used in netem, ip4 defrag, and tcp stack */
		struct list_head	list;
	};

	union {
		struct sock		*sk;        // 本网络报文所属的sock结构，此值仅在本地发出的报文中有效，从网络收到的报文这个为空
		int			ip_defrag_offset;
	};

	union {
		ktime_t		tstamp;     // 报文收到的时间戳
		u64		skb_mstamp_ns; /* earliest departure time */
	};
	/*
	 * This is the control buffer. It is free to use for every
	 * layer. Please put your private variables there. If you
	 * want to keep them across layers you have to do a skb_clone()
	 * first. This is owned by whoever has the skb queued ATM.
	 */
	char			cb[48] __aligned(8);    // 用于控制缓冲区

	union {
		struct {
			unsigned long	_skb_refdst;
			void		(*destructor)(struct sk_buff *skb);
		};
		struct list_head	tcp_tsorted_anchor;
	};

#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	unsigned long		 _nfct;
#endif
	unsigned int		len,        // 有效数据长度
				data_len;           // 数据长度
	__u16			mac_len,        // MAC报文长度
				hdr_len;            // skb的可写头部长度

	/* Following fields are _not_ copied in __copy_skb_header()
	 * Note that queue_mapping is here mostly to fill a hole.
	 */
	__u16			queue_mapping;

/* if you move cloned around you also must adapt those constants */
#ifdef __BIG_ENDIAN_BITFIELD
#define CLONED_MASK	(1 << 7)
#else
#define CLONED_MASK	1
#endif
#define CLONED_OFFSET()		offsetof(struct sk_buff, __cloned_offset)

	/* private: */
	__u8			__cloned_offset[0];
	/* public: */
	__u8			cloned:1,
				nohdr:1,
				fclone:2,
				peeked:1,
				head_frag:1,
				pfmemalloc:1;
#ifdef CONFIG_SKB_EXTENSIONS
	__u8			active_extensions;
#endif
	/* fields enclosed in headers_start/headers_end are copied
	 * using a single memcpy() in __copy_skb_header()
	 */
	/* private: */
	__u32			headers_start[0];
	/* public: */

/* if you move pkt_type around you also must adapt those constants */
#ifdef __BIG_ENDIAN_BITFIELD
#define PKT_TYPE_MAX	(7 << 5)
#else
#define PKT_TYPE_MAX	7
#endif
#define PKT_TYPE_OFFSET()	offsetof(struct sk_buff, __pkt_type_offset)

	/* private: */
	__u8			__pkt_type_offset[0];
	/* public: */
	__u8			pkt_type:3;
	__u8			ignore_df:1;
	__u8			nf_trace:1;
	__u8			ip_summed:2;
	__u8			ooo_okay:1;

	__u8			l4_hash:1;
	__u8			sw_hash:1;
	__u8			wifi_acked_valid:1;
	__u8			wifi_acked:1;
	__u8			no_fcs:1;
	/* Indicates the inner headers are valid in the skbuff. */
	__u8			encapsulation:1;
	__u8			encap_hdr_csum:1;
	__u8			csum_valid:1;

#ifdef __BIG_ENDIAN_BITFIELD
#define PKT_VLAN_PRESENT_BIT	7
#else
#define PKT_VLAN_PRESENT_BIT	0
#endif
#define PKT_VLAN_PRESENT_OFFSET()	offsetof(struct sk_buff, __pkt_vlan_present_offset)
	/* private: */
	__u8			__pkt_vlan_present_offset[0];
	/* public: */
	__u8			vlan_present:1;
	__u8			csum_complete_sw:1;
	__u8			csum_level:2;
	__u8			csum_not_inet:1;
	__u8			dst_pending_confirm:1;
#ifdef CONFIG_IPV6_NDISC_NODETYPE
	__u8			ndisc_nodetype:2;
#endif

	__u8			ipvs_property:1;
	__u8			inner_protocol_type:1;
	__u8			remcsum_offload:1;
#ifdef CONFIG_NET_SWITCHDEV
	__u8			offload_fwd_mark:1;
	__u8			offload_l3_fwd_mark:1;
#endif
#ifdef CONFIG_NET_CLS_ACT
	__u8			tc_skip_classify:1;
	__u8			tc_at_ingress:1;
#endif
#ifdef CONFIG_NET_REDIRECT
	__u8			redirected:1;
	__u8			from_ingress:1;
#endif
#ifdef CONFIG_TLS_DEVICE
	__u8			decrypted:1;
#endif

#ifdef CONFIG_NET_SCHED
	__u16			tc_index;	/* traffic control index */
#endif

	union {
		__wsum		csum;
		struct {
			__u16	csum_start;
			__u16	csum_offset;
		};
	};
	__u32			priority;
	int			skb_iif;
	__u32			hash;
	__be16			vlan_proto;
	__u16			vlan_tci;
#if defined(CONFIG_NET_RX_BUSY_POLL) || defined(CONFIG_XPS)
	union {
		unsigned int	napi_id;
		unsigned int	sender_cpu;
	};
#endif
#ifdef CONFIG_NETWORK_SECMARK
	__u32		secmark;
#endif

	union {
		__u32		mark;
		__u32		reserved_tailroom;
	};

	union {
		__be16		inner_protocol;     // 协议
		__u8		inner_ipproto;
	};

	__u16			inner_transport_header;
	__u16			inner_network_header;   // IP头
	__u16			inner_mac_header;

	__be16			protocol;
	__u16			transport_header;       // 传输层头部，L4帧头
	__u16			network_header;         // IP帧头,指向IP结构体指针
	__u16			mac_header;             // 指向MAC头的指针

	/* private: */
	__u32			headers_end[0];
	/* public: */

	/* These elements must be at the end, see alloc_skb() for details.  */
	sk_buff_data_t		tail;           // 数据尾指针
	sk_buff_data_t		end;            // 报文缓冲区的尾部
	unsigned char		*head,          // 报文缓冲区的头
				*data;                  // 数据的头指针
	unsigned int		truesize;       // 报文缓冲区的大小
	refcount_t		users;              // skb被克隆引用的次数，在内存申请

#ifdef CONFIG_SKB_EXTENSIONS
	/* only useable after checking ->active_extensions != 0 */
	struct skb_ext		*extensions;
#endif
};
```

## 5 net_device
net_device结构体存储着网络设备的所有信息，每个设备都有这种结构。所有设备的net_device结构放在一个全局变量dev_base 的所有全局列表中。和sk_buff一样，整体结构相当庞大。结构体中有一个next指针，用来连接系统中所有网络设备。内核把这些连接起来的设备组成一个链表，并由全局变量dev_base指向链表中的第一个元素。net_device结构体源码分析如下。

```c
/**
 *	struct net_device - The DEVICE structure.
 *
 *	Actually, this whole structure is a big mistake.  It mixes I/O
 *	data with strictly "high-level" data, and it has to know about
 *	almost every data structure used in the INET module.
 *
 *	@name:	This is the first field of the "visible" part of this structure
 *		(i.e. as seen by users in the "Space.c" file).  It is the name
 *		of the interface.
 *
 *	@name_node:	Name hashlist node
 *	@ifalias:	SNMP alias
 *	@mem_end:	Shared memory end
 *	@mem_start:	Shared memory start
 *	@base_addr:	Device I/O address
 *	@irq:		Device IRQ number
 *
 *	@state:		Generic network queuing layer state, see netdev_state_t
 *	@dev_list:	The global list of network devices
 *	@napi_list:	List entry used for polling NAPI devices
 *	@unreg_list:	List entry  when we are unregistering the
 *			device; see the function unregister_netdev
 *	@close_list:	List entry used when we are closing the device
 *	@ptype_all:     Device-specific packet handlers for all protocols
 *	@ptype_specific: Device-specific, protocol-specific packet handlers
 *
 *	@adj_list:	Directly linked devices, like slaves for bonding
 *	@features:	Currently active device features
 *	@hw_features:	User-changeable features
 *
 *	@wanted_features:	User-requested features
 *	@vlan_features:		Mask of features inheritable by VLAN devices
 *
 *	@hw_enc_features:	Mask of features inherited by encapsulating devices
 *				This field indicates what encapsulation
 *				offloads the hardware is capable of doing,
 *				and drivers will need to set them appropriately.
 *
 *	@mpls_features:	Mask of features inheritable by MPLS
 *	@gso_partial_features: value(s) from NETIF_F_GSO\*
 *
 *	@ifindex:	interface index
 *	@group:		The group the device belongs to
 *
 *	@stats:		Statistics struct, which was left as a legacy, use
 *			rtnl_link_stats64 instead
 *
 *	@rx_dropped:	Dropped packets by core network,
 *			do not use this in drivers
 *	@tx_dropped:	Dropped packets by core network,
 *			do not use this in drivers
 *	@rx_nohandler:	nohandler dropped packets by core network on
 *			inactive devices, do not use this in drivers
 *	@carrier_up_count:	Number of times the carrier has been up
 *	@carrier_down_count:	Number of times the carrier has been down
 *
 *	@wireless_handlers:	List of functions to handle Wireless Extensions,
 *				instead of ioctl,
 *				see <net/iw_handler.h> for details.
 *	@wireless_data:	Instance data managed by the core of wireless extensions
 *
 *	@netdev_ops:	Includes several pointers to callbacks,
 *			if one wants to override the ndo_*() functions
 *	@ethtool_ops:	Management operations
 *	@l3mdev_ops:	Layer 3 master device operations
 *	@ndisc_ops:	Includes callbacks for different IPv6 neighbour
 *			discovery handling. Necessary for e.g. 6LoWPAN.
 *	@xfrmdev_ops:	Transformation offload operations
 *	@tlsdev_ops:	Transport Layer Security offload operations
 *	@header_ops:	Includes callbacks for creating,parsing,caching,etc
 *			of Layer 2 headers.
 *
 *	@flags:		Interface flags (a la BSD)
 *	@priv_flags:	Like 'flags' but invisible to userspace,
 *			see if.h for the definitions
 *	@gflags:	Global flags ( kept as legacy )
 *	@padded:	How much padding added by alloc_netdev()
 *	@operstate:	RFC2863 operstate
 *	@link_mode:	Mapping policy to operstate
 *	@if_port:	Selectable AUI, TP, ...
 *	@dma:		DMA channel
 *	@mtu:		Interface MTU value
 *	@min_mtu:	Interface Minimum MTU value
 *	@max_mtu:	Interface Maximum MTU value
 *	@type:		Interface hardware type
 *	@hard_header_len: Maximum hardware header length.
 *	@min_header_len:  Minimum hardware header length
 *
 *	@needed_headroom: Extra headroom the hardware may need, but not in all
 *			  cases can this be guaranteed
 *	@needed_tailroom: Extra tailroom the hardware may need, but not in all
 *			  cases can this be guaranteed. Some cases also use
 *			  LL_MAX_HEADER instead to allocate the skb
 *
 *	interface address info:
 *
 * 	@perm_addr:		Permanent hw address
 * 	@addr_assign_type:	Hw address assignment type
 * 	@addr_len:		Hardware address length
 *	@upper_level:		Maximum depth level of upper devices.
 *	@lower_level:		Maximum depth level of lower devices.
 *	@neigh_priv_len:	Used in neigh_alloc()
 * 	@dev_id:		Used to differentiate devices that share
 * 				the same link layer address
 * 	@dev_port:		Used to differentiate devices that share
 * 				the same function
 *	@addr_list_lock:	XXX: need comments on this one
 *	@name_assign_type:	network interface name assignment type
 *	@uc_promisc:		Counter that indicates promiscuous mode
 *				has been enabled due to the need to listen to
 *				additional unicast addresses in a device that
 *				does not implement ndo_set_rx_mode()
 *	@uc:			unicast mac addresses
 *	@mc:			multicast mac addresses
 *	@dev_addrs:		list of device hw addresses
 *	@queues_kset:		Group of all Kobjects in the Tx and RX queues
 *	@promiscuity:		Number of times the NIC is told to work in
 *				promiscuous mode; if it becomes 0 the NIC will
 *				exit promiscuous mode
 *	@allmulti:		Counter, enables or disables allmulticast mode
 *
 *	@vlan_info:	VLAN info
 *	@dsa_ptr:	dsa specific data
 *	@tipc_ptr:	TIPC specific data
 *	@atalk_ptr:	AppleTalk link
 *	@ip_ptr:	IPv4 specific data
 *	@dn_ptr:	DECnet specific data
 *	@ip6_ptr:	IPv6 specific data
 *	@ax25_ptr:	AX.25 specific data
 *	@ieee80211_ptr:	IEEE 802.11 specific data, assign before registering
 *	@ieee802154_ptr: IEEE 802.15.4 low-rate Wireless Personal Area Network
 *			 device struct
 *	@mpls_ptr:	mpls_dev struct pointer
 *
 *	@dev_addr:	Hw address (before bcast,
 *			because most packets are unicast)
 *
 *	@_rx:			Array of RX queues
 *	@num_rx_queues:		Number of RX queues
 *				allocated at register_netdev() time
 *	@real_num_rx_queues: 	Number of RX queues currently active in device
 *	@xdp_prog:		XDP sockets filter program pointer
 *	@gro_flush_timeout:	timeout for GRO layer in NAPI
 *
 *	@rx_handler:		handler for received packets
 *	@rx_handler_data: 	XXX: need comments on this one
 *	@miniq_ingress:		ingress/clsact qdisc specific data for
 *				ingress processing
 *	@ingress_queue:		XXX: need comments on this one
 *	@nf_hooks_ingress:	netfilter hooks executed for ingress packets
 *	@broadcast:		hw bcast address
 *
 *	@rx_cpu_rmap:	CPU reverse-mapping for RX completion interrupts,
 *			indexed by RX queue number. Assigned by driver.
 *			This must only be set if the ndo_rx_flow_steer
 *			operation is defined
 *	@index_hlist:		Device index hash chain
 *
 *	@_tx:			Array of TX queues
 *	@num_tx_queues:		Number of TX queues allocated at alloc_netdev_mq() time
 *	@real_num_tx_queues: 	Number of TX queues currently active in device
 *	@qdisc:			Root qdisc from userspace point of view
 *	@tx_queue_len:		Max frames per queue allowed
 *	@tx_global_lock: 	XXX: need comments on this one
 *	@xdp_bulkq:		XDP device bulk queue
 *	@xps_cpus_map:		all CPUs map for XPS device
 *	@xps_rxqs_map:		all RXQs map for XPS device
 *
 *	@xps_maps:	XXX: need comments on this one
 *	@miniq_egress:		clsact qdisc specific data for
 *				egress processing
 *	@qdisc_hash:		qdisc hash table
 *	@watchdog_timeo:	Represents the timeout that is used by
 *				the watchdog (see dev_watchdog())
 *	@watchdog_timer:	List of timers
 *
 *	@pcpu_refcnt:		Number of references to this device
 *	@todo_list:		Delayed register/unregister
 *	@link_watch_list:	XXX: need comments on this one
 *
 *	@reg_state:		Register/unregister state machine
 *	@dismantle:		Device is going to be freed
 *	@rtnl_link_state:	This enum represents the phases of creating
 *				a new link
 *
 *	@needs_free_netdev:	Should unregister perform free_netdev?
 *	@priv_destructor:	Called from unregister
 *	@npinfo:		XXX: need comments on this one
 * 	@nd_net:		Network namespace this network device is inside
 *
 * 	@ml_priv:	Mid-layer private
 * 	@lstats:	Loopback statistics
 * 	@tstats:	Tunnel statistics
 * 	@dstats:	Dummy statistics
 * 	@vstats:	Virtual ethernet statistics
 *
 *	@garp_port:	GARP
 *	@mrp_port:	MRP
 *
 *	@dev:		Class/net/name entry
 *	@sysfs_groups:	Space for optional device, statistics and wireless
 *			sysfs groups
 *
 *	@sysfs_rx_queue_group:	Space for optional per-rx queue attributes
 *	@rtnl_link_ops:	Rtnl_link_ops
 *
 *	@gso_max_size:	Maximum size of generic segmentation offload
 *	@gso_max_segs:	Maximum number of segments that can be passed to the
 *			NIC for GSO
 *
 *	@dcbnl_ops:	Data Center Bridging netlink ops
 *	@num_tc:	Number of traffic classes in the net device
 *	@tc_to_txq:	XXX: need comments on this one
 *	@prio_tc_map:	XXX: need comments on this one
 *
 *	@fcoe_ddp_xid:	Max exchange id for FCoE LRO by ddp
 *
 *	@priomap:	XXX: need comments on this one
 *	@phydev:	Physical device may attach itself
 *			for hardware timestamping
 *	@sfp_bus:	attached &struct sfp_bus structure.
 *	@qdisc_tx_busylock_key: lockdep class annotating Qdisc->busylock
 *				spinlock
 *	@qdisc_running_key:	lockdep class annotating Qdisc->running seqcount
 *	@qdisc_xmit_lock_key:	lockdep class annotating
 *				netdev_queue->_xmit_lock spinlock
 *	@addr_list_lock_key:	lockdep class annotating
 *				net_device->addr_list_lock spinlock
 *
 *	@proto_down:	protocol port state information can be sent to the
 *			switch driver and used to set the phys state of the
 *			switch port.
 *
 *	@wol_enabled:	Wake-on-LAN is enabled
 *
 *	@net_notifier_list:	List of per-net netdev notifier block
 *				that follow this device when it is moved
 *				to another network namespace.
 *
 *	FIXME: cleanup struct net_device such that network protocol info
 *	moves out.
 */

struct net_device {
	char			name[IFNAMSIZ];
	struct netdev_name_node	*name_node;
	struct dev_ifalias	__rcu *ifalias;
	/*
	 *	I/O specific fields
	 *	FIXME: Merge these and struct ifmap into one
	 */
	unsigned long		mem_end;
	unsigned long		mem_start;
	unsigned long		base_addr;
	int			irq;

	/*
	 *	Some hardware also needs these fields (state,dev_list,
	 *	napi_list,unreg_list,close_list) but they are not
	 *	part of the usual set specified in Space.c.
	 */

	unsigned long		state;

	struct list_head	dev_list;
	struct list_head	napi_list;
	struct list_head	unreg_list;
	struct list_head	close_list;
	struct list_head	ptype_all;
	struct list_head	ptype_specific;

	struct {
		struct list_head upper;
		struct list_head lower;
	} adj_list;

	netdev_features_t	features;
	netdev_features_t	hw_features;
	netdev_features_t	wanted_features;
	netdev_features_t	vlan_features;
	netdev_features_t	hw_enc_features;
	netdev_features_t	mpls_features;
	netdev_features_t	gso_partial_features;

	int			ifindex;
	int			group;

	struct net_device_stats	stats;

	atomic_long_t		rx_dropped;
	atomic_long_t		tx_dropped;
	atomic_long_t		rx_nohandler;

	/* Stats to monitor link on/off, flapping */
	atomic_t		carrier_up_count;
	atomic_t		carrier_down_count;

#ifdef CONFIG_WIRELESS_EXT
	const struct iw_handler_def *wireless_handlers;
	struct iw_public_data	*wireless_data;
#endif
	const struct net_device_ops *netdev_ops;
	const struct ethtool_ops *ethtool_ops;
#ifdef CONFIG_NET_L3_MASTER_DEV
	const struct l3mdev_ops	*l3mdev_ops;
#endif
#if IS_ENABLED(CONFIG_IPV6)
	const struct ndisc_ops *ndisc_ops;
#endif

#ifdef CONFIG_XFRM_OFFLOAD
	const struct xfrmdev_ops *xfrmdev_ops;
#endif

#if IS_ENABLED(CONFIG_TLS_DEVICE)
	const struct tlsdev_ops *tlsdev_ops;
#endif

	const struct header_ops *header_ops;

	unsigned int		flags;
	unsigned int		priv_flags;

	unsigned short		gflags;
	unsigned short		padded;

	unsigned char		operstate;
	unsigned char		link_mode;

	unsigned char		if_port;
	unsigned char		dma;

	/* Note : dev->mtu is often read without holding a lock.
	 * Writers usually hold RTNL.
	 * It is recommended to use READ_ONCE() to annotate the reads,
	 * and to use WRITE_ONCE() to annotate the writes.
	 */
	unsigned int		mtu;
	unsigned int		min_mtu;
	unsigned int		max_mtu;
	unsigned short		type;
	unsigned short		hard_header_len;
	unsigned char		min_header_len;

	unsigned short		needed_headroom;
	unsigned short		needed_tailroom;

	/* Interface address info. */
	unsigned char		perm_addr[MAX_ADDR_LEN];
	unsigned char		addr_assign_type;
	unsigned char		addr_len;
	unsigned char		upper_level;
	unsigned char		lower_level;
	unsigned short		neigh_priv_len;
	unsigned short          dev_id;
	unsigned short          dev_port;
	spinlock_t		addr_list_lock;
	unsigned char		name_assign_type;
	bool			uc_promisc;
	struct netdev_hw_addr_list	uc;
	struct netdev_hw_addr_list	mc;
	struct netdev_hw_addr_list	dev_addrs;

#ifdef CONFIG_SYSFS
	struct kset		*queues_kset;
#endif
	unsigned int		promiscuity;
	unsigned int		allmulti;


	/* Protocol-specific pointers */

#if IS_ENABLED(CONFIG_VLAN_8021Q)
	struct vlan_info __rcu	*vlan_info;
#endif
#if IS_ENABLED(CONFIG_NET_DSA)
	struct dsa_port		*dsa_ptr;
#endif
#if IS_ENABLED(CONFIG_TIPC)
	struct tipc_bearer __rcu *tipc_ptr;
#endif
#if IS_ENABLED(CONFIG_IRDA) || IS_ENABLED(CONFIG_ATALK)
	void 			*atalk_ptr;
#endif
	struct in_device __rcu	*ip_ptr;
#if IS_ENABLED(CONFIG_DECNET)
	struct dn_dev __rcu     *dn_ptr;
#endif
	struct inet6_dev __rcu	*ip6_ptr;
#if IS_ENABLED(CONFIG_AX25)
	void			*ax25_ptr;
#endif
	struct wireless_dev	*ieee80211_ptr;
	struct wpan_dev		*ieee802154_ptr;
#if IS_ENABLED(CONFIG_MPLS_ROUTING)
	struct mpls_dev __rcu	*mpls_ptr;
#endif

/*
 * Cache lines mostly used on receive path (including eth_type_trans())
 */
	/* Interface address info used in eth_type_trans() */
	unsigned char		*dev_addr;

	struct netdev_rx_queue	*_rx;
	unsigned int		num_rx_queues;
	unsigned int		real_num_rx_queues;

	struct bpf_prog __rcu	*xdp_prog;
	unsigned long		gro_flush_timeout;
	rx_handler_func_t __rcu	*rx_handler;
	void __rcu		*rx_handler_data;

#ifdef CONFIG_NET_CLS_ACT
	struct mini_Qdisc __rcu	*miniq_ingress;
#endif
	struct netdev_queue __rcu *ingress_queue;
#ifdef CONFIG_NETFILTER_INGRESS
	struct nf_hook_entries __rcu *nf_hooks_ingress;
#endif

	unsigned char		broadcast[MAX_ADDR_LEN];
#ifdef CONFIG_RFS_ACCEL
	struct cpu_rmap		*rx_cpu_rmap;
#endif
	struct hlist_node	index_hlist;

/*
 * Cache lines mostly used on transmit path
 */
	struct netdev_queue	*_tx ____cacheline_aligned_in_smp;
	unsigned int		num_tx_queues;
	unsigned int		real_num_tx_queues;
	struct Qdisc		*qdisc;
	unsigned int		tx_queue_len;
	spinlock_t		tx_global_lock;

	struct xdp_dev_bulk_queue __percpu *xdp_bulkq;

#ifdef CONFIG_XPS
	struct xps_dev_maps __rcu *xps_cpus_map;
	struct xps_dev_maps __rcu *xps_rxqs_map;
#endif
#ifdef CONFIG_NET_CLS_ACT
	struct mini_Qdisc __rcu	*miniq_egress;
#endif

#ifdef CONFIG_NET_SCHED
	DECLARE_HASHTABLE	(qdisc_hash, 4);
#endif
	/* These may be needed for future network-power-down code. */
	struct timer_list	watchdog_timer;
	int			watchdog_timeo;

	struct list_head	todo_list;
	int __percpu		*pcpu_refcnt;

	struct list_head	link_watch_list;

	enum { NETREG_UNINITIALIZED=0,
	       NETREG_REGISTERED,	/* completed register_netdevice */
	       NETREG_UNREGISTERING,	/* called unregister_netdevice */
	       NETREG_UNREGISTERED,	/* completed unregister todo */
	       NETREG_RELEASED,		/* called free_netdev */
	       NETREG_DUMMY,		/* dummy device for NAPI poll */
	} reg_state:8;

	bool dismantle;

	enum {
		RTNL_LINK_INITIALIZED,
		RTNL_LINK_INITIALIZING,
	} rtnl_link_state:16;

	bool needs_free_netdev;
	void (*priv_destructor)(struct net_device *dev);

#ifdef CONFIG_NETPOLL
	struct netpoll_info __rcu	*npinfo;
#endif

	possible_net_t			nd_net;

	/* mid-layer private */
	union {
		void					*ml_priv;
		struct pcpu_lstats __percpu		*lstats;
		struct pcpu_sw_netstats __percpu	*tstats;
		struct pcpu_dstats __percpu		*dstats;
	};

#if IS_ENABLED(CONFIG_GARP)
	struct garp_port __rcu	*garp_port;
#endif
#if IS_ENABLED(CONFIG_MRP)
	struct mrp_port __rcu	*mrp_port;
#endif

	struct device		dev;
	const struct attribute_group *sysfs_groups[4];
	const struct attribute_group *sysfs_rx_queue_group;

	const struct rtnl_link_ops *rtnl_link_ops;

	/* for setting kernel sock attribute on TCP connection setup */
#define GSO_MAX_SIZE		65536
	unsigned int		gso_max_size;
#define GSO_MAX_SEGS		65535
	u16			gso_max_segs;

#ifdef CONFIG_DCB
	const struct dcbnl_rtnl_ops *dcbnl_ops;
#endif
	s16			num_tc;
	struct netdev_tc_txq	tc_to_txq[TC_MAX_QUEUE];
	u8			prio_tc_map[TC_BITMASK + 1];

#if IS_ENABLED(CONFIG_FCOE)
	unsigned int		fcoe_ddp_xid;
#endif
#if IS_ENABLED(CONFIG_CGROUP_NET_PRIO)
	struct netprio_map __rcu *priomap;
#endif
	struct phy_device	*phydev;
	struct sfp_bus		*sfp_bus;
	struct lock_class_key	qdisc_tx_busylock_key;
	struct lock_class_key	qdisc_running_key;
	struct lock_class_key	qdisc_xmit_lock_key;
	struct lock_class_key	addr_list_lock_key;
	bool			proto_down;
	unsigned		wol_enabled:1;

	struct list_head	net_notifier_list;
};
```

## 9 网络设备（注册和注销）

```c

// \linux-5.6.14\net\core\dev.c
/**
 *	register_netdevice	- register a network device
 *	@dev: device to register
 *
 *	Take a completed network device structure and add it to the kernel
 *	interfaces. A %NETDEV_REGISTER message is sent to the netdev notifier
 *	chain. 0 is returned on success. A negative errno code is returned
 *	on a failure to set up the device, or if the name is a duplicate.
 *
 *	Callers must hold the rtnl semaphore. You may want
 *	register_netdev() instead of this.
 *
 *	BUGS:
 *	The locking appears insufficient to guarantee two parallel registers
 *	will not get the same name.
 */

int register_netdevice(struct net_device *dev)
{
	int ret;
	struct net *net = dev_net(dev);

	BUILD_BUG_ON(sizeof(netdev_features_t) * BITS_PER_BYTE <
		     NETDEV_FEATURE_COUNT);
	BUG_ON(dev_boot_phase);
	ASSERT_RTNL();

	might_sleep();

	/* When net_device's are persistent, this will be fatal. */
	BUG_ON(dev->reg_state != NETREG_UNINITIALIZED);
	BUG_ON(!net);

	spin_lock_init(&dev->addr_list_lock);
	lockdep_set_class(&dev->addr_list_lock, &dev->addr_list_lock_key);

	ret = dev_get_valid_name(net, dev, dev->name);
	if (ret < 0)
		goto out;

	ret = -ENOMEM;
	dev->name_node = netdev_name_node_head_alloc(dev);
	if (!dev->name_node)
		goto out;

	/* Init, if this function is available */
	if (dev->netdev_ops->ndo_init) {
		ret = dev->netdev_ops->ndo_init(dev);
		if (ret) {
			if (ret > 0)
				ret = -EIO;
			goto err_free_name;
		}
	}

	if (((dev->hw_features | dev->features) &
	     NETIF_F_HW_VLAN_CTAG_FILTER) &&
	    (!dev->netdev_ops->ndo_vlan_rx_add_vid ||
	     !dev->netdev_ops->ndo_vlan_rx_kill_vid)) {
		netdev_WARN(dev, "Buggy VLAN acceleration in driver!\n");
		ret = -EINVAL;
		goto err_uninit;
	}

	ret = -EBUSY;
	if (!dev->ifindex)
		dev->ifindex = dev_new_index(net);
	else if (__dev_get_by_index(net, dev->ifindex))
		goto err_uninit;

	/* Transfer changeable features to wanted_features and enable
	 * software offloads (GSO and GRO).
	 */
	dev->hw_features |= (NETIF_F_SOFT_FEATURES | NETIF_F_SOFT_FEATURES_OFF);
	dev->features |= NETIF_F_SOFT_FEATURES;

	if (dev->netdev_ops->ndo_udp_tunnel_add) {
		dev->features |= NETIF_F_RX_UDP_TUNNEL_PORT;
		dev->hw_features |= NETIF_F_RX_UDP_TUNNEL_PORT;
	}

	dev->wanted_features = dev->features & dev->hw_features;

	if (!(dev->flags & IFF_LOOPBACK))
		dev->hw_features |= NETIF_F_NOCACHE_COPY;

	/* If IPv4 TCP segmentation offload is supported we should also
	 * allow the device to enable segmenting the frame with the option
	 * of ignoring a static IP ID value.  This doesn't enable the
	 * feature itself but allows the user to enable it later.
	 */
	if (dev->hw_features & NETIF_F_TSO)
		dev->hw_features |= NETIF_F_TSO_MANGLEID;
	if (dev->vlan_features & NETIF_F_TSO)
		dev->vlan_features |= NETIF_F_TSO_MANGLEID;
	if (dev->mpls_features & NETIF_F_TSO)
		dev->mpls_features |= NETIF_F_TSO_MANGLEID;
	if (dev->hw_enc_features & NETIF_F_TSO)
		dev->hw_enc_features |= NETIF_F_TSO_MANGLEID;

	/* Make NETIF_F_HIGHDMA inheritable to VLAN devices.
	 */
	dev->vlan_features |= NETIF_F_HIGHDMA;

	/* Make NETIF_F_SG inheritable to tunnel devices.
	 */
	dev->hw_enc_features |= NETIF_F_SG | NETIF_F_GSO_PARTIAL;

	/* Make NETIF_F_SG inheritable to MPLS.
	 */
	dev->mpls_features |= NETIF_F_SG;

	ret = call_netdevice_notifiers(NETDEV_POST_INIT, dev);
	ret = notifier_to_errno(ret);
	if (ret)
		goto err_uninit;

	ret = netdev_register_kobject(dev);
	if (ret) {
		dev->reg_state = NETREG_UNREGISTERED;
		goto err_uninit;
	}
	dev->reg_state = NETREG_REGISTERED;

	__netdev_update_features(dev);

	/*
	 *	Default initial state at registry is that the
	 *	device is present.
	 */

	set_bit(__LINK_STATE_PRESENT, &dev->state);

	linkwatch_init_dev(dev);

	dev_init_scheduler(dev);
	dev_hold(dev);
	list_netdevice(dev);
	add_device_randomness(dev->dev_addr, dev->addr_len);

	/* If the device has permanent device address, driver should
	 * set dev_addr and also addr_assign_type should be set to
	 * NET_ADDR_PERM (default value).
	 */
	if (dev->addr_assign_type == NET_ADDR_PERM)
		memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);

	/* Notify protocols, that a new device appeared. */
	ret = call_netdevice_notifiers(NETDEV_REGISTER, dev);
	ret = notifier_to_errno(ret);
	if (ret) {
		rollback_registered(dev);
		rcu_barrier();

		dev->reg_state = NETREG_UNREGISTERED;
	}
	/*
	 *	Prevent userspace races by waiting until the network
	 *	device is fully setup before sending notifications.
	 */
	if (!dev->rtnl_link_ops ||
	    dev->rtnl_link_state == RTNL_LINK_INITIALIZED)
		rtmsg_ifinfo(RTM_NEWLINK, dev, ~0U, GFP_KERNEL);

out:
	return ret;

err_uninit:
	if (dev->netdev_ops->ndo_uninit)
		dev->netdev_ops->ndo_uninit(dev);
	if (dev->priv_destructor)
		dev->priv_destructor(dev);
err_free_name:
	netdev_name_node_free(dev->name_node);
	goto out;
}
EXPORT_SYMBOL(register_netdevice);


// 注销网络设备函数
static inline void unregister_netdevice(struct net_device *dev)
{
	unregister_netdevice_queue(dev, NULL);
}
```

## 10 网络设备中NAPI
老式网络设备驱动是在中断驱动模式下工作。意味着没接收一个数据包，就需要中断一次，事实证明此工作方式在负载很高情况下效率降低，为了解决这个问题，开发了一种新的软件技术 --> NAPI （New API）。采用NAPIer技术时，如果负载很高，网络设备驱动程序将在轮询模式，而不是中断驱动模式下运行。

数据包的收发：网络设备驱动程序主要任务：接收目的地为当前主机的数据包，并将其传递给网络层，之后再将其传递给传输层。传输当前主机生成的外出数据包或者转换当前主机收到的数据包。对于每个数据包，无论它是接收到还是发送出去，都需要在路由子系统中执行一次查找操作。

当数据包位于网络设备驱动程序接收路径的L2时，sk_buff->data指向的是L2帧头；调用方法 eth_type_trans() 后，数据包即将进入第三层，因此sk_buff->data 应指向L3包头，而这个包头紧跟在以太网帧头后面。
IPv4数据包为例：
| eth 14bytes | IPv4  20~60 bytes | UDP 8 bytes | data |

每个sk_buff都有一个dev成员（一个net_device结构实例），对于到来的数据包，这个成员表示接受它的网络设备；而对于外出的数据包，这个成员表示发送它的网络设备。