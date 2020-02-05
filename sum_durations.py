k = open("durations.txt",'r')
lines = k.readlines()
counter = 0
for line in lines:
   conv_float = float(line)
   counter = counter + conv_float
print(counter)
