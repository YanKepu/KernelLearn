```mermaid
sequenceDiagram
	participant drv_port_link_change_event_proc
	participant drv_port_mrphy_speedmode
	participant phystatus
	phystatus->>drv_port_mrphy_speedmode: linkup
	drv_port_mrphy_speedmode ->> drv_port_link_change_event_proc: mode change
```

