#!/usr/bin/bash
export LANG="zh_CN.UTF-8"

if [ "$#" -le 1 ]; then
    echo "please input process exe ip port!"
    exit 1
fi  

exe=$1
ip=$2
port=$3

username=$(whoami)
ServiceName_Port=${exe}_${port}

pid=$(ps -u ${username} -f | grep -w ${ServiceName_Port} | grep -w ${ip} | grep -v 'grep' | awk '{printf("%d ",$2);}')
if [[ $pid -ne 0 ]]; then
    echo "kill ${pid}, stop ${ServiceName_Port}"
    kill ${pid}

    loop=100
    stop_succ=false
    while [[ "${loop}" -gt "0" ]]; do
        usleep 100000
        ((--loop))

        pid=$(ps -u ${username} -f | grep -w ${ServiceName_Port} | grep -w ${ip} | grep -v 'grep' | awk '{printf("%d ",$2);}')

        if [[ $pid -eq 0 ]]; then
            stop_succ=true
            break
        fi
    done

    if ${stop_succ}; then
        echo "stop ${ServiceName_Port} succ."
    else
        echo "stop ${ServiceName_Port} failed, Wait 10s ..."
    fi
else
    echo "process not running"
fi
