f0 = 0
f1 = 1
sum = 0
output = 0

for i in range(42):
    print(str(output) + ", ", end='')
    sum = f0 + f1
    output = output + sum
    f0 = f1
    f1 = sum
    #print(sum);

print()