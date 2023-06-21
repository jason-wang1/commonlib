#!/usr/bin/bash
export LANG="zh_CN.UTF-8"

# 保证执行路径从项目目录开始
# '/*' 绝对路径
# '*'  表示任意字符串
PWD=$(pwd)
case $0 in
/*)
    SCRIPT="$0"
    ;;
*)
    SCRIPT="$PWD/$0"
    ;;
esac
REALPATH=$(dirname $SCRIPT)
cd $REALPATH

source /opt/rh/rh-python36/enable

local_ip=$(
    python <<-EOF
import socket
import subprocess
s= socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(('8.8.8.8', 80))
use_ip = s.getsockname()[0]
s.close()
print(use_ip)
EOF
)

username=$(whoami)
jq -r '.server_list[]|"\(.ip) \(.exe) \(.port)"' config/server.json | while read ip exe port; do
    if [ ${ip} = ${local_ip} ]; then
        ServiceName_Port=${exe}_${port}
        pid=$(ps -u ${username} -f | grep -w ${ServiceName_Port} | grep -w ${ip} | grep -v 'grep' | awk '{printf("%d ",$2);}')
        if [[ -n "$pid" ]]; then
            kill -35 ${pid}
            echo "-kill 35 ${pid}, update ${ServiceName_Port} config ..."
        else
            echo "process not running"
        fi
    fi
done
