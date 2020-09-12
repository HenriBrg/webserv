## SHELL

*   curl            : à approfondir à fond

*   netcat          :
*   ping            :
*   traceroute      : tool for displaying the route (path) and measuring transit delays of packets across an Internet Protocol (IP) network, shows the number of hops taken during the entire transmission route. What traceroute does is it sends a packet towards that remote host, only a single hop onto the network. And then, causes the network to send a message back.
-   traceroute google.com

*   nmap            : Outil d'exploration réseau et scanneur de ports
-   nmap 127.0.0.1

*   ifconfig        : configure network interface parameters (sur macos, on aura l'IP devant le inet en en0)
-   ifconfig en0

*   netstat         : Affiche les connexions entrantes / sortantes ainsi que les tables de routages, les ports, les protocoles
-   netstat -an

*   nslookup        : Interroge le DNS pour obtenir un nom de domaine / une adresse iP
-   nslookup google.com

*   tcpdump         : On a switched network, each packet moves between a computer and a port on a switch, or between two switches. It's the job of the switch to transmit a packet only when the line is clear, and only to the necessary ports. This way, it's like each computer is on it's own private network. We never have packet collisions and we leave as much of our network clear as possible. This means we get the most throughput possible out of our network.
On an unswitched network, where Ethernet packets pass through every device on the network, expecting each system device to only look at the packets sent to its destination address. But it is quite trivial to set a device to promiscuous mode, which causes it to look at all packets, regardless of the destination address. Most packet-capturing codes, such as tcpdump, drop the device they are listening to into promiscuous mode by default. This promiscuous more can be set using ifconfig:

-   Tout est expliqué ici : https://danielmiessler.com/study/tcpdump/
-   Exemple : sudo tcpdump -i en0
-   Exemple : sudo tcpdump icmp --> écoute le trafic ICMP --> essayer d'aller sur une mauvaise URL comme : http://localhost:30fdgsf/

-   BIG Exemple :   1) sudo tcpdump port 80 -XX -s 1024
-                   2) wget http://ad.doubleclick.net/ads/813/;order=smsn0009;sz=1x1;o=1922687960

## SOFTWARE

* Telnet            : Telnet est un protocole informatique qui a été conçu pour interagir avec des ordinateurs distants. Outil puissant à approfondir, voici un usag très basique :
-   python -m SimpleHTTPServer 8080
-   telnet 127.0.0.1 8080
-   HEAD / HTTP/1.0
-   ^]

* Wireshark         :