# Lab1 Writeup
written by Zihan Chen

## Topic 1
### ifconfig
```bash
ifconfig displays the status of the currently active
interfaces.  If a single interface argument is given, it displays  the  status  of
the given interface only; if a single -a argument is given, it displays the status
of all interfaces, even those that are down.  Otherwise, it configures  an  in
```
### ip addr
```bash
ip - show / manipulate routing, network devices, interfaces and tunnels
```

### sudo tcpdump
```bash
12:48:47.393873 IP _gateway.domain > seed-virtual-machine.59222: 14453 NXDomain*- 0/0/0 (90)
12:48:49.520871 IP 192.168.92.1.57621 > 192.168.92.255.57621: UDP, length 44
12:49:06.756768 IP 192.168.92.132 > seed-virtual-machine: ICMP echo request, id 2, seq 1, length 64
12:49:06.756915 IP seed-virtual-machine > 192.168.92.132: ICMP echo reply, id 2, seq 1, length 64
12:49:07.757938 IP 192.168.92.132 > seed-virtual-machine: ICMP echo request, id 2, seq 2, length 64
12:49:07.757989 IP seed-virtual-machine > 192.168.92.132: ICMP echo reply, id 2, seq 2, length 64
12:49:08.761636 IP 192.168.92.132 > seed-virtual-machine: ICMP echo request, id 2, seq 3, length 64
12:49:08.761714 IP seed-virtual-machine > 192.168.92.132: ICMP echo reply, id 2, seq 3, length 64
```