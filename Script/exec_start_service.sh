#!/usr/bin/bash
export LANG="zh_CN.UTF-8"

if [ "$#" -le 2 ]; then
    echo "please input process name and port!"
    exit 1
fi

exe=$1
ip=$2
port=$3
params=""

i=0
for param in "$@"; do
    if [ "$i" -ne 0 -a "$i" -ne "$#" ]; then
        params=$params" "$param
    fi
    let i+=1
done

username=$(whoami)
ServiceName_Port=${exe}_${port}

pid=$(ps -u ${username} -f | grep -w ${ServiceName_Port} | grep -w ${ip} | grep -v 'grep' | awk '{printf("%d ",$2);}')
if [[ $pid -eq 0 ]]; then
    echo "nohup start $ServiceName_Port"
    nohup ./$exe $params $ServiceName_Port >/dev/null 2>&1 &

    loop=100
    start_succ=false
    while [[ $loop != "0" ]]; do
        usleep 100000
        ((--loop))

        pid=$(netstat -pln | grep ":::${port}" | awk '{print $7}' | awk -F'/./' '{printf("%d ",$1);}')

        if [[ $pid -ne 0 ]]; then
            start_succ=true
            break
        fi
    done

    if ${start_succ}; then
        echo "start ${ServiceName_Port} succ, pid $pid"
    else
        echo "start ${ServiceName_Port} failed, Wait 10s ..."
    fi
else
    echo "process start repeat"
fi
