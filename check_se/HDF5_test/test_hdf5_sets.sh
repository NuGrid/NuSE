

path1='/astro/critter/critter1/CADC/NuGrid/data/set1/set1.1/ppd_wind'
path1='/astro/critter/critter1/CADC/NuGrid/data/set1/set1.2/ppd_wind'

#report name, add date
report='hdf5_report_set1.2.txt'

dirs1=`find $path1 -name 'H5_*' -type d`

#echo $dirs1

#exit 0

#echo $dirs1

count=0
#echo $dirs1
for d in $dirs1; do
	if [ "$count" == "2" ];then
		echo 'zero########'
		break
		#./test_hdf5.py "$d" $report
	fi
	echo "$d"
	echo "$d" >> $report
	./test_hdf5.py "$d" $report
	count=($count+1)
done

echo '##################################' >> $report
echo '######## Test finished ###########'>> $report
echo '##################################' >> $report





