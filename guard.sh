count=0
while [ true  ];do
    sleep 10
    processExist=`ps |grep main|grep -v "grep" `
    if [ -z "$processExist" ];then
        exec ./main
        let count++
	if [ $count -ge 3 ];then
		echo "reboot"
		exec reboot
	fi
    #else
    #   echo "running"
    fi
done

