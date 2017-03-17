import sys

infile = sys.argv[1]

nPin = 0
with open(infile, 'r') as f:
    for line in f:
        tokens = line.split();
        nPin += (len(tokens) - 3)
print(nPin)

