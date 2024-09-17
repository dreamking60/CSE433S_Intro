#!/usr/bin/python3

from scapy.all import *

a = IP()
a.src = '192.168.92.134'
a.dst = '192.168.92.132'
b = ICMP()
p = a/b
send(p)