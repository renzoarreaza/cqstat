# CQStat
The purpose of this code is to measure the outgoing bandwidth usage on a per class/qdisc basis. 


## ??
The basis of this code was taken from https://github.com/sbhTWR/qdisc_stats/
The iproute2 utility source code was also of great help in using rtnetlink(7).

This is my first C project, the quality of the code is therefore not great. Feel free to contact me if you find a bug, or suggest improvements.

## To Do: </br>


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
