#!/bin/bash

NAME=`echo "$1" | cut -d "." -f1`
EXT=`echo "$1" | cut -d "." -f2`

cp $NAME.$EXT $NAME-right.$EXT
#convert -flip $NAME.$EXT $NAME-right2.$EXT
convert -flop $NAME.$EXT $NAME-left.$EXT
#convert -flip -flop $NAME.$EXT $NAME-left2.$EXT
convert -flop -rotate -90 $NAME.$EXT $NAME-down2.$EXT
convert -flop -rotate 90 $NAME.$EXT $NAME-up2.$EXT
convert -rotate 90 $NAME.$EXT $NAME-down.$EXT
convert -rotate -90 $NAME.$EXT $NAME-up.$EXT
rm $NAME.$EXT

