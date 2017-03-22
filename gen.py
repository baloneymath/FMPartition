import sys
import random as rd

ofile = sys.argv[1]
bf = sys.argv[2]
nsize = int(sys.argv[3])
csize = int(sys.argv[4])

crange = [1, csize]
nrange = [1, nsize]

f = open(ofile, 'w')

f.write(bf + '\n')
for i in range(0):
    f.write('NET n{} '.format(i + 1))
    cc = rd.sample(range(crange[0], crange[1]), 30000)
    for j in range(len(cc)):
        f.write('c{} '.format(cc[j]))
    f.write(';\n')
for i in range(nsize):
    Cs = rd.randint(1, int(4))
    f.write('NET n{} '.format(i + 1))
    cc = rd.sample(range(crange[0], crange[1]), Cs)
    for j in range(len(cc)):
        f.write('c{} '.format(cc[j]))
    f.write(';\n')

f.close()
