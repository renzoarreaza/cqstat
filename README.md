# CQStat
Tool to measure the outgoing bandwidth usage on a per class/qdisc basis. </br>


This is my first C project, the quality of the code is therefore not great. Feel free to contact me if you find a bug, or suggest improvements. It's still a work in progress and I already have improvements planned.


## Prerequisites
Linux kernel headers are required to run this program.
```bash
sudo apt update
sudo apt install linux-headers-$(uname -r)
```

## Compile
Execute the following command in the terminal in the path containing nlcomm.c:

```gcc nlcomm.c cqstat.c -o cqstat```

This should generate a binary file named qdisc_stats in the current working directory.

## Execute
Execute the following command in the terminal in the path containing nlcomm.c:

```./cqstat```

The data will be stored in a csv file, either with the provided name or using the default name `cqstat.csv`.
This data can be plotted using `plotting.py`. This will generate one figure per network interface. 

The `dummy.sh` script can be used to create a virtual interface pair and configure veth1 with a hierarchical queueing setup. This was made purely for testing purposes.

## To Do / Known issues </br>
- Improve plotting </br>
- upload example plot </br>
- Class based measurement not working... 
    - Current workaround: -c/-q flag is being ignored. Internally always being set to -q </br>
- Implement ingress bw measurement </br>


## Final Note
The basis of this code was taken from https://github.com/sbhTWR/qdisc_stats/
This program fetches queue statistics once and displays them on the terminal

