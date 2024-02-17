# Create 3rd grade calibration formula based on provided data in calibrations.csv
import csv
from matplotlib import pyplot
from scipy.optimize import curve_fit
from numpy import arange

def objective(x, a, b, c):
	return a * x + b * x**2 + c

# read datapoints
x,y = [], []
with open('calibrations.csv','r') as f:
    reader = csv.reader(f)
    for row in reader:
        x.append(float(row[0]))
        y.append(float(row[1]))
pyplot.scatter(x, y)

# calculate the curve factors
popt, _ = curve_fit(objective, x, y)
a, b, c = popt
print('SG=%.8f*tilt+%.8f*tilt^2+%.8f' % (a, b, c))

# use the factors to calculate the gravity (verify)
xx,yy = [], []
for r in x:
    xx.append(r)
    yy.append( a*r + b*r*r + c )
pyplot.scatter(xx, yy, color='orange')

# plot the curve
x_line = arange(min(x), max(x), 1)
y_line = objective(x_line, a, b, c)
pyplot.plot(x_line, y_line, '--', color='red')
pyplot.show()
