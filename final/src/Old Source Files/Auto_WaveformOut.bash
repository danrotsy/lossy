#!/bin/sh
for i in 0 1 2 3 4 5; do
	for j in "Vin" "Vout" "Vn001" "Vn014" "Vn026"; do
		./WaveformOutCSV_2.exe "AC_Analysis_Cdrp_${i}p_${j}"
		./WaveformOutCSV_2.exe "Pulse_Analysis_Cdrp_${i}p_${j}"
	done
done
