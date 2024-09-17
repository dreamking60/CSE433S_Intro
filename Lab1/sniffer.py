#!/usr/bin/python3

from scapy.all import *

# Sample Code 2
def print_pkt(pkt):
    pkt.show()
    
pkt = sniff(filter='icmp', prn=print_pkt)
#pkt = sniff(filter='tcp and (port 23)', prn=print_pkt)