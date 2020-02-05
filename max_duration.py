k = open("durations.txt",'r')
lines = k.readlines()
max = 0
for line in lines:
   conv = float(line)
   if(conv > max):
       max = conv
print(max)
