#!/usr/bin/python3

#renzo@Renzo-Ubuntu:~/qdisc_bandwidth$ ./qdisc_stats -c
#major, minor of parent, handle and leaf
# qdisc htb handle 100:1 dev veth1 root  leaf 1: 
# qdisc prio handle 1:1 dev veth1 parent 1:  leaf 11: 
# qdisc prio handle 1:2 dev veth1 parent 1:  leaf 12: 
# qdisc prio handle 1:3 dev veth1 parent 1: 

class Item:
	def __init__(self, interface, cq, type, maj_p, min_p, maj_h, min_h, maj_c, min_c):
		self.interface = interface
		self.cq = cq
		self.type = type
		self.maj_p = maj_p
		self.min_p = min_p
		self.maj_h = maj_h
		self.min_h = min_h
		self.maj_c = maj_c
		self.min_c = min_c


items = [Item("veth1", "class", "htb", None, None, 100, 1, 1, None), 
		Item("veth1", "class", "prio",1, None, 1, 1, 11, None), 
		Item("veth1", "class", "prio",1, None, 1, 2, 12, None), 
		Item("veth1", "class", "prio",1, None, 1, 3, 13, None),
		Item("veth1", "qdisc", "htb", None, None, 100, None, 0, None),
		Item("veth1", "qdisc", "fq_codel", 1, 2, 12, None, 0, None),
		Item("veth1", "qdisc", "prio", 100, 1, 1, None, 0, None),
		Item("veth1", "qdisc", "pfifo", 1, 1, 11, None, 0, None)]

#print(items[0].__dict__)	

# determine number of interfaces
interfaces = []
for i in items:
	interfaces.append(i.interface)
interfaces = list(set(interfaces))
print(interfaces)


#for each interface
for inter in interfaces:
	#find root qdisc	
	for item in items:
		if item.cq == "qdisc" and item.maj_p is None and item.min_p is None:
			edge = "+" +"-"*(4+len(item.type))+"+"  
			print(edge)	
			print("|  "+str(item.type)+"  |")
			print(edge+"\n")	
			break

	#find root class(es)
	line1 = ""
	line2 = ""
	line3 = ""
	for item in items:
		#if item.cq == "class" and item.maj_p == 1 and item.min_p is None: #test
		if item.cq == "class" and item.maj_p is None and item.min_p is None:
			edge = "+" +"-"*(4+len(item.type))+"+"  
			line1 += edge + "\t" 
			line2 += "|  "+str(item.type)+"  |"+"\t"
			line3 += edge + "\t" 
	print(line1)
	print(line2)
	print(line3+"\n")

exit(0)

#renzo@Renzo-Ubuntu:~/qdisc_bandwidth$ ./qdisc_stats -q
# qdisc htb handle 100: dev veth1 root  leaf 0: 
# qdisc fq_codel handle 12: dev veth1 parent 1:2  leaf 0: 
# qdisc prio handle 1: dev veth1 parent 100:1  leaf 0: 
# qdisc pfifo handle 11: dev veth1 parent 1:1  leaf 0: 

# for each qdisc, 
	

def block(text):
	pass
# find qdisc with root as parent and go from there. # this won't work either 

def parent(items, a, b):	#find class/qdisc with parent a:b
	pass
	


for qdisc in qdiscs:
	if qdisc[0] is None and qdisc[1] is None: # parent is root 
		print(qdisc[-1])
		break # only one qdisc can be attached to root

for clas in classes:
	if clas[0] is None and clas[1] is None: # parent is root 
		print(clas[-1])



# notes for C code

# use one array of arrays*
# for each "message", create an entry with:
#  - interfacename
#  - class/qdisc
#  - type
#  - parent major
#  - parent minor
#  - handle major
#  - handle minor
#  - child major
#  - child minor
