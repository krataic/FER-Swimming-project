import serial
import csv
import time
import tkinter as tk
from threading import *
from numpy import *
from pyqtgraph.Qt import QtGui, QtCore
import pyqtgraph as pg
import matplotlib.pyplot as plt

root = tk.Tk()
frame = tk.Frame(root)
frame.pack()
arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)
inicijalizacija = 1
windowWidth = 500                       # width of the window displaying the curve
Xm = linspace(0,0,windowWidth)          # create array that will contain the relevant time series     
ptr = -windowWidth                      # set first x position
app = QtGui.QApplication([]) # initialization
win = pg.GraphicsWindow(title="Signal from serial port") # creates a window
p = win.addPlot(title="Realtime plot")  # creates empty space for the plot in the window
curve = p.plot()                        # create an empty "plot" (a curve to plot)

poljeVrijednosti = list()

def threadStartCSV():
	t1 = Thread(target=pokreniSnimanje)
	t1.start()

def threadPlot():
	t2 = Thread(target=plotTheRecord)
	t2.start()
	
def write_csv():
	global arduino
	ser_bytes = arduino.readline()
	decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
	with open("test_data2.csv","w") as f:
		writer = csv.writer(f,delimiter=",")
		writer.writerow([time.time(),decoded_bytes])

def pokreniSnimanje():
	global name, arduino, inicijalizacija
	name=time.strftime("%Y_%m_%d_%H_%M_%S", time.gmtime())
	#f=open(name+'.csv', 'wt');
	#writer = csv.writer(f,delimiter='\t')
	f = open(name+'.txt','w')
	if inicijalizacija:
		arduino.write(bytes('5', 'utf-8'))
		time.sleep(1)
	if inicijalizacija == 0:
		arduino = serial.Serial(port='COM13', baudrate=115200, timeout=.1)
		time.sleep(0.1)
		inicijalizacija = 1
		arduino.write(bytes('5', 'utf-8'))
		time.sleep(1)
	while True:
		#cijeliBroj = arduino.readline()
		#decoded_bytes = float(cijeliBroj[0:len(cijeliBroj)-2].decode("utf-8"))
		#print(cijeliBroj)
		#writer.writerow(cijeliBroj)
		
		#writer.writerow([time.time(),cijeliBroj])
		getData = str(arduino.readline())
		data = getData[2:][:-5]
		f.write(data+'\n')
		#writer.writerow(data)
		print(data)
		updatePlot()
	#arduino.close()
	pg.QtGui.QApplication.exec_() # you MUST put this at the end

def updatePlot():
	global curve, ptr, Xm, arduino    
	Xm[:-1] = Xm[1:]                      # shift data in the temporal mean 1 sample left
	value = arduino.readline()            # read line (single value) from the serial port
	Xm[-1] = float(value)                 # vector containing the instantaneous values      
	ptr += 1                              # update x position for displaying the curve
	curve.setData(Xm)                     # set the curve with this data
	curve.setPos(ptr,0)                   # set x position in the graph to 0
	QtGui.QApplication.processEvents()    # you MUST process the plot now
	
def zaustaviSnimanje():
	global arduino
	global t1, inicijalizacija
	if inicijalizacija:
		arduino.write(bytes('10', 'utf-8'))
		time.sleep(0.1)
		#t1.kill()
		arduino.close()
		inicijalizacija = 0
	
def plotTheRecord():
	global name
	X, Y = [], []
	for line in open(name+'.txt', 'r'):
		values = [float(s) for s in line.split()]
		#X.append(values[0])
		Y.append(values)
	plt.plot(Y)
	plt.show()
	
izlaz = tk.Button(frame, 
                   text="QUIT", 
				   width = 20,
				   height = 2,
                   fg="red",
                   command=quit)
izlaz.pack(side=tk.LEFT)

startSnimanje = tk.Button(frame,
					text="Pokreni Snimanje",
					width = 20,
					height = 2,
					fg = "green",
					command=threadStartCSV)
startSnimanje.pack(side=tk.LEFT)

stopSnimanje = tk.Button(frame,
                   text="Zaustavi Snimanje",
				   width = 20,
				   height = 2,
				   fg = "red",
                   command=zaustaviSnimanje)
stopSnimanje.pack(side=tk.LEFT)

plotSnimka = tk.Button(frame,
                   text="Nacrtaj snimku",
				   width = 20,
				   height = 2,
				   fg = "yellow",
				   bg = "blue",
                   command=plotTheRecord)
plotSnimka.pack(side=tk.LEFT)

#configSnimanje = tk.

root.mainloop()
	