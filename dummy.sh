#!/bin/bash

function cleanup {
	echo "cleaning up..." 
	sudo tc qdisc del dev veth1 root
	sudo ip link del veth0 type veth peer name veth1
} 
trap cleanup EXIT 


sudo ip link add veth0 type veth peer name veth1

# qdisc for delay simulation
sudo tc qdisc replace dev veth1 root pfifo_fast 2>&1 | awk '{ print "2-pfifo: "$0 }'
sudo tc qdisc replace dev veth1 root handle 100: htb default 1 | awk '{ print "2-htbq: "$0 }'
sudo tc class add dev veth1 parent 100: classid 100:1 htb rate 18mbit | awk '{ print "2-htbc: "$0 }'

sudo tc qdisc add dev veth1 parent 100:1 handle 1: prio 2>&1 | awk '{ print "2-prio: "$0 }'
sudo tc qdisc add dev veth1 parent 1:1 handle 11: pfifo limit 1000 2>&1 | awk '{ print "2-fifo-c: "$0 }'
sudo tc qdisc add dev veth1 parent 1:2 handle 12: fq_codel quantum 300 limit 800 target 2ms interval 50ms noecn 2>&1 | awk '{ print "2-fq-codel: "$0 }'

echo "done!" 
echo "	 
		+------------+
		|    Root    |
		+------------+
			  |
		+------------+
		|HTB  (qdisc)| handle 100: root
		+------------+
			  |
		+------------+
		|HTB  (class)| handle 100:1 root leaf 1: 
		+------------+
			  |
		+------------+
		|Prio (qdisc)| handle 1: parent 100: 
		+------------+

handle 1:1      handle 1:2	    handle 1:3      
+------------+  +------------+  +------------+
|Prio (class)|  |Prio (class)|  |Prio (class)| 
+------------+  +------------+  +------------+
parent 1:       parent 1:	    parent 1:

handle 11:       handle 12:	    
+-------------+  +----------------+  
|pfifo (qdisc)|  |fq_codel (qdisc)|  
+-------------+  +----------------+  
parent 1:1       parent 1:2	    
"

tail -f /dev/null
