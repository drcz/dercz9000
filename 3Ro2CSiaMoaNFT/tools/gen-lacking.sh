#!/bin/sh

./gen-spr.sh hero-square.png
./gen-spr.sh hero-square2.png
./gen-spr.sh hero-triangle2.png
./gen-spr.sh hero-triangle.png
./gen-spr.sh hero-disc.png
./gen-spr.sh hero-disc2.png
./gen-spr.sh gun.png
#rm gun-right2.png
#rm gun-left2.png
rm gun-up2.png
rm gun-down2.png
convert particle-h.png -rotate 90 particle-v.png
convert pipe-h.png -rotate 90 pipe-v.png
convert turncock-h.png -rotate 90 turncock-v.png
convert pipe-dl.png -rotate 90 pipe-ul.png
convert pipe-dl.png -rotate 180 pipe-ur.png
convert pipe-dl.png -rotate -90 pipe-dr.png

