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
with open(file) as inputfile:
    for row in csv.reader(inputfile):
        print ( row )
        if (len(row) > 2 and is_number(row[0])):
            data1.append(float(row[0]))
            data2.append(float(row[1]))
            data3.append(float(row[2]))

print("File length:" + str(len(data1)))

data1 = np.asarray(data1)
data2 = np.asarray(data2)
data3 = np.asarray(data3)

fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.plot(data1,'r', label='entities')
ax1.plot(data2,'g', label='fps')
ax1.plot(data3,'b', label='elapsedtime')
plt.legend(loc='upper left')

plt.savefig('output.png')

plt.show()



