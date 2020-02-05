i=0
j=0
with open('GUIDs.txt') as f:
    seen = set()
    for line in f:
        if line in seen:
            j = j+1
            # print('##' + line)
        else:
            seen.add(line)
            i = i + 1

print('duplicates = ', j)
print('nr of lines without duplicates = ', i)
