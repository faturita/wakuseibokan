import matplotlib.pyplot as plt
import csv
import numpy as np


import sys


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        pass
 
    try:
        import unicodedata
        unicodedata.numeric(s)
        return True
    except (TypeError, ValueError):
        pass
 
    return False

if (len(sys.argv)<2):
	print ("Log File parameter should be provided.")
	quit()

file = sys.argv[1]

data1 = []
data2 = []
data3 = []
data4 = []
with open(file) as inputfile:
    for row in csv.reader(inputfile):
        print ( row )
        if (len(row) > 2 and is_number(row[0])):
            data1.append(float(row[0]))
            data2.append(float(row[1]))
            data3.append(float(row[2]))
            data4.append(float(row[3]))

print("File length:" + str(len(data1)))

data1 = np.asarray(data1)
data2 = np.asarray(data2)
data3 = np.asarray(data3)
data4 = np.asarray(data4)

fig = plt.figure(figsize=(7,7))
ax1 = fig.add_subplot(411)

ax1.plot(data1,'r', label='entities')
ax1.set_ylim([0, 650])
ax1.legend()

ax1 = fig.add_subplot(412)
ax1.plot(data2,'g', label='fps')
ax1.set_ylim([0, 65])
ax1.legend()

ax1 = fig.add_subplot(413)
ax1.plot(data3,'b', label='elapsedmodeltime')
ax1.set_ylim([0, 20000])
ax1.legend()

ax1 = fig.add_subplot(414)
ax1.plot(data4,'y', label='elapseddrawtime')
ax1.set_ylim([0, 20000])
ax1.legend()

plt.savefig('output.png')

plt.show()



