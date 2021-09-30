dataFile = open("data32.txt", "rb")
data = dataFile.read()
dataFile.close()
print(data)
for each in data:
    if each > 100:
        print('%x' % each)
    else:
        print(each)
# print(data[5])