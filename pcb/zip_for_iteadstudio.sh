#!/bin/sh

rm *.zip
ORDERDETAILS="_5by5_Green_1mm_ENIG_10pcs.zip"

[ $# -eq 0 ] && { echo "Usage: $0 projectfile.brd"; exit 1; }

echo "creating zip file ... yeah!!!"
file=$1

filenoext=$(basename "$file" .brd)

zip $filenoext.zip $filenoext.GTL $filenoext.GBL $filenoext.GTS $filenoext.GBS $filenoext.GTO $filenoext.GBO $filenoext.TXT $filenoext.GML

echo "done. Rename the $filenoext.zip to [ORDERNUMBER][DIMENSION][COLOUR][THICKNESS][SURFACE FINISH HASL][AMOUNT].zip"
echo "for example: 422214_10by10_Green_1.6mm_ENIG_10pcs.zip"
echo "enter ordernumber: "

read ORDERNUMBER
newfile=$ORDERNUMBER$ORDERDETAILS
mv $filenoext.zip $newfile 
