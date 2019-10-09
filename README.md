# CQStat
Tool to measure the outgoing bandwidth usage on a per class/qdisc basis. 


## Notes
This is still a work in progress. 

The basis of this code was taken from https://github.com/sbhTWR/qdisc_stats/
The iproute2 utility source code was also of great help in using rtnetlink(7).

This is my first C project, the quality of the code is therefore not great. Feel free to contact me if you find a bug, or suggest improvements.




## Prerequisites
Linux kernel headers are required to run this program.
```bash
sudo apt update
sudo apt install linux-headers-$(uname -r)
```

## Compile
Execute the following command in the terminal in the path containing nlcomm.c:

```gcc nlcomm.c qstats.c -o qdisc_stats```

This should generate a binary file named qdisc_stats in the current working directory.

## Execute
Execute the following command in the terminal in the path containing nlcomm.c:

```./qdisc_stats```

### CLI Options


## To Do / Known issues </br>
- Doesn't work on virtual machine (Ubuntu 16 and 18 running on OpenStack) </br>
- Implement ingress bw measurement

## Final Note
The basis of this code was taken from https://github.com/sbhTWR/qdisc_stats/
This program fetches queue statistics once and displays them on the terminal

