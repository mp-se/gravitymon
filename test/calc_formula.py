# plot "Population" vs "Employed"
#from pandas import read_csv
import csv
from matplotlib import pyplot
from scipy.optimize import curve_fit
from numpy import arange

def objective(x, a, b):
    return a * x + b # linear curvefitting

# read datapoints
x,y = [], []
with open('calibrations.csv','r') as f:
    reader = csv.reader(f)
    for row in reader:
        x.append(float(row[0]))
        y.append(float(row[1]))
#print( x ) # Angle
#print( y ) # Gravity
pyplot.scatter(x, y)

# calculate the linear factors
popt, _ = curve_fit(objective, x, y)
a, b = popt
print('SG=%.8f*tilt+%.8f' % (a, b))

# use the factors to calculate the gravity (verify)
xx,yy = [], []
for r in x:
    xx.append(r)
    yy.append( a * r + b )
pyplot.scatter(xx, yy, color='orange')

# plot the curve
x_line = arange(min(x), max(x), 1)
y_line = objective(x_line, a, b)
pyplot.plot(x_line, y_line, '--', color='red')
pyplot.show()
