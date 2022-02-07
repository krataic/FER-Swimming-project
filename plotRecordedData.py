import numpy as np
import matplotlib.pyplot as plt

X, Y = [], []
for line in open('2022_02_07_18_06_16.txt', 'r'):
	line.strip()
	values = [float(s) for s in line.split()]
	#X.append(values[0])
	Y.append(values)

print(Y)
plt.plot(Y)
plt.show()