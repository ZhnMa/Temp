'''
Python file for Individual Project - Server Data processing

Read data from data file generated earlier

Developed by: Zhaonan Ma el20z2m@leeds.ac.uk

28 Sep 2021
'''

import matplotlib.pyplot as pl
import numpy as np

dataFile = open("data32.txt", "rb")
data = dataFile.read()
dataFile.close()
data = list(data)
print(data)
if data[0] < 240 or data[0] > 243:  # the first one needs to be a configuration byte
    raise ValueError('Incomplete Data Flow: data[0] = {}, needs to be a configuration byte'.format(data[0]))

'''
Configuration:
0xf3 243: 32 bits, default, both temperature and humidity with decimal points
0xf1 241: 16 bits, both temperature and humidity without decimal points
oxf0 240: 16 bits, only one of both with decimal points then another

Pin information:
0xex 224 + x: x: 0bxxxx four pins
------
New form:
Data = [f1, p1, [t1, t2, ...], [h1, h2, ...], f2, p2, [t1, t2, ...], ...]
f: configuration 
p: pin info
t: temperature
h: humidity
'''

Data = []
Temp = []
Humi = []

for each in data:
    if each >= 244:  # incorrect signal, raise an error
        raise ValueError('Incorrect Signal: data: {}'.format(each))
    elif 240 <= each < 244:
        if not Data:  # empty
            config = each
            Data.append(config)
        else:
            # configuration changed, start new process
            Data.append(Temp)
            Data.append(Humi)
            Temp = []
            Humi = []
            config = each
            Data.append(config)
            Data.append(Data[-4])  # pin
        oneData = 0  # next value is data
    elif 224 <= each < 240:  # pin info byte
        if len(Data) == 1:
            Data.append(each)
        else:
            # pin changed, start new process
            # ToDo: pin change requires data shift
            Data.append(Temp)
            Data.append(Humi)
            Temp = []
            Humi = []
            Data.append(config)
            Data.append(each)
        oneData = 0  # next value is data
    else:  # data, separate two data and keep them continual
        # 32 bits || 16 bits with decimals, !!might face mal-position issue
        if config == 243 or config == 240:
            if oneData == 0 or oneData == 2:  # decimal points byte
                deci = each / 10
                oneData += 1
            elif oneData == 1:  # temperature integer byte
                temp = each
                temp += deci
                Temp.append(temp)
                oneData += 1
            elif oneData == 3:  # humidity integer byte
                humi = each
                humi += deci
                Humi.append(humi)
                oneData = 0
            else:  # wrong value
                raise IndexError('Incorrect Index Value: oneData = {}'.format(oneData))
        elif config == 241:  # 16 bits, both data with no decimal points
            if oneData == 0:  # first the temperature
                Temp.append(each)
                oneData += 1
            elif oneData == 1:  # then the humidity
                Humi.append(each)
                oneData = 0
            else:  # wrong value, raise an error
                raise IndexError('Incorrect Index Value: oneData = {}'.format(oneData))
        else:  # no such a configuration type, raise an error
            raise ValueError('Incorrect Configuration Number: config = {}'.format(config))
# include the last data
if data[-1] < 224:
    Data.append(Temp)
    Data.append(Humi)
print(Data)
'''
print(data[1:5])
x = np.linspace(0, 10)
pl.plot(x[0:4], data[1:5])
pl.plot(x[4:8], data[6:10])
pl.show()
'''