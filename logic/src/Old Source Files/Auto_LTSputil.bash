#!/bin/bash
for j in "1.0" "2.0" "3.0" "4.0" "5.0"; do
	for k in "1.0" "2.0" "3.0" "4.0" "5.0" "6.0" "7.0" "8.0" "9.0" "10."; do
		for l in "
		for l m n
		for i in "V(inpulse)" "V(out)" "V(n001)" "V(n014)" "V(n026)"; do
			./ltsputil -xo2 tran_cdrp=${j}p_len${k}.raw tran_cdrp=${j}p_len=${k}_${i}.csv "%14.6e" "," "#DATA" "time" ${i}
			./ltsputil -xo2dp ac_cdrp=${j}p_len${k}.raw ac_cdrp=${j}p_len${k}_${i}.csv "%14.6e" "," "#DATA" "frequency" ${i}		
		done
	done
done 