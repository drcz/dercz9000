#!/usr/bin/python

import sys
## apt-get install python-imaging
from PIL import Image

file = sys.argv[1]

img = Image.open(file)
pix = img.load()

scale = 4 # maybe img.size[1]/8 ?

h = img.size[1] / scale
w = img.size[0] / scale

#print "/// %s" % file
print "{ %d,%d, " % (w,h)
for y in range(0, h):
    row = ''
    for x in range(0, w):
        if pix[x*scale,y*scale] == 0 :
            row = row + '0'
        else :
            row = row + '1'
    if y < (h - 1) :
        maybe_comma = ','
    else:
        maybe_comma = ''
    print " 0b%s%s" % (row , maybe_comma)
print "}"
