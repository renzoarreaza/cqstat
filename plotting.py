#!/usr/bin/python3
import csv, os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.font_manager import FontProperties

# added backlog as second to last
## example data
#time, dev, qdisc, handle, parent, bytes, packets, qlen, drops
#1570217724.221082, wlp3s0, mq, none, root, 107843, 935, 0, 0
#1570217724.221082, wlp3s0, pfifo_fast, none, :4, 0, 0, 0, 0
#1570217724.221082, wlp3s0, pfifo_fast, none, :3, 107148, 928, 0, 0
#1570217724.221082, wlp3s0, pfifo_fast, none, :2, 0, 0, 0, 0
#1570217724.221082, wlp3s0, pfifo_fast, none, :1, 695, 7, 0, 0
#1570217724.231677, wlp3s0, mq, none, root, 107843, 935, 0, 0

# Getting latest csv file
datafile = ""
mtime_max = 0
for filename in os.listdir(os.getcwd()):
	if filename.endswith(".csv"):
		mtime = os.stat(filename).st_mtime
		if mtime > mtime_max:
			mtime_max = mtime
			datafile = filename
print("processing: " + datafile)
devs = set()
data = {}
time = []
with open(datafile, 'rU') as f:
	reader = csv.reader(f, skipinitialspace=True)
	next(reader) #skipping the header for now
	for row in reader:
		if len(row) != 10:
			continue
		devs.add(row[1])
		if len(time) == 0 or float(row[0]) != time[-1]:
			time.append(float(row[0]))
		id = (row[1], row[4], row[3])
		try:
			data[id][0].append(int(row[5]))
			data[id][1].append(int(row[6]))
			data[id][2].append(int(row[7]))
			data[id][3].append(int(row[8]))
			data[id][4].append(int(row[9]))
		except (KeyError, IndexError):
			data[id] = [[],[],[],[],[]]
			data[id][0].append(int(row[5]))
			data[id][1].append(int(row[6]))
			data[id][2].append(int(row[7]))
			data[id][3].append(int(row[8]))
			data[id][4].append(int(row[9]))

lens = []
for key in data:
	for row in data[key]:
		lens.append(len(row))

mini = min(lens)
time = [x - time[0] for x in time]
time = time[:mini]

def incr2inst(times, values):	#expects times to be in epoch unix style. converts data to data/sec
	values = [values[i]-values[i-1] for i in range(1, len(values))]  #instantanious values
	times = [times[i]-times[i-1] for i in range(1, len(times))]  #instantanious times
	values = [0] + [value/time for value, time in zip(values, times)]
	return values

def ma(data, n):
	assert(n < len(data))
	assert(n > 1)
	return data[:n-1] + [sum(data[i-(n-1):i+1])/n for i in range(n-1, len(data))]


if not os.path.exists(str(os.getcwd()) + "/" + datafile[:-4] + "/"):
	os.mkdir(str(os.getcwd()) + "/" + datafile[:-4] + "/")
for dev in devs:
	for i in range(5):
		fig, ax = plt.subplots()
		for id in data:
			if dev in id:
				if id[1] == "root":  # or id[1] == "100:1":
					continue
				if i == 0:
					data_1 = incr2inst(time,data[id][i][:mini])
					data_2 = [x*8 for x in data_1]  # in bits/s
					data_3 = ma(data_2, 10)
					ax.plot(time,data_3,label=str(id), alpha=0.7)  
				if i == 1:
					ax.plot(time,incr2inst(time,data[id][i][:mini]),label=str(id))
				if i in [2,3,4]:
					ax.plot(time,data[id][i][:mini],label=str(id))
					if i == 3:
						q_byte = data[id][i][:mini]
						print("max queue byte: " + str(max(q_byte)))
						print("avg queue byte: " + str(sum(q_byte)/len(q_byte)))

#time, dev, qdisc, handle, parent, bytes, packets, qlen, drops
		if i == 0:
			ax.set(xlabel='time', ylabel='Bandwidth (bits/s)')
			plt.title("Bandwidth", y=1.02)
		if i == 1:
			ax.set(xlabel='time', ylabel='Rate (packet/s)')
			plt.title("Packet rate", y=1.02)
		if i == 2:
			ax.set(xlabel='time', ylabel='Queue length (packets)')
			plt.title("Queue Length", y=1.02)
		if i == 3:
			ax.set(xlabel='time', ylabel='Queue length (bytes)')
			plt.title("Queue Length", y=1.02)
		if i == 4:
			ax.set(xlabel='time', ylabel='Drops (packets?)')
			plt.title("Packet drops", y=1.02)

		ax.ticklabel_format(axis='y', style='sci', scilimits = (0,0))
		ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.03), ncol=3, fancybox=True, shadow=True, fontsize = 'small')
		# End testing improvements
		ax.grid()
		ax.set_axisbelow(True)
#		if len(devs) <= 1:
#			fig.savefig(str(dev) + "_" + str(i) + "_" + datafile[:-4] + ".png")
#			fig.savefig(str(dev) + "_" + str(i) + "_" + datafile[:-4] + ".pdf")
#		else:
		fig.savefig("./" + datafile[:-4] + "/" + str(dev) + "_" + str(i) + "_" + datafile[:-4] + ".png")
		fig.savefig("./" + datafile[:-4] + "/" + str(dev) + "_" + str(i) + "_" + datafile[:-4] + ".pdf")


