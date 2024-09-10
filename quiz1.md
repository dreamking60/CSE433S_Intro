
# Week1 class2
## Three principles
Threat Model -> new cap
Security Goal
Security Mechanism -> keep up

Security Goal
- Confidential
- Integrity
- Availability

## Systems should always be password (or biometric) protected
False. We should also think of availability.

## system configure well, and not change?
No. Security is changing. Every day there is a new CVE. And what's more human is the weakness in a security system. Security is developing and involving every seconds.

## C.I.A stand for?

# Week1 class2 & Week3 class3

## ARP 
ARP Request
ARP Response

Lack of Secure Authentication
ARP Spoofing(ARP Poisoning)

### Middle Attack
Act in the middle

#### Prevention
- Static ARP table
- DHCP Certification(use access control to ensure that hosts only use the IP addresses assigned to them and that only authorized DHCP servers are accessible)

## IP
### Lack of Source IP Authentication
- Client is trusted to embed correct source IP

### Implication: Smurf Amplification Dos attack
Send ping request to broadcast addr (ICMP Echo Req)
Lots of responses

#### Prevention 
limit reply

### Lack of Confidentiality Protection
- Packet Sniffing

## TCP
Handshake, oepn and with lots of resources
### Denial of Service (Dos) vulnerabilities
e.g. TCP SYN Flood

### TCP Session hijacking

## DNS
### DNS Cache Poisoning
user browser -> local DNS resolver -> .com -> response
response is cached and restored in DNS
attacker use it.

## Protect
Protect in Application Level

Verus (no now)

Reflection on trusted trucks
