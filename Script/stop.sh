#!/usr/bin/bash
export LANG="zh_CN.UTF-8"

# 保证执行路径从项目目录开始
# '/*' 绝对路径
# '*'  表示任意字符串
case $0 in
/*)
    SCRIPT="$0"
    ;;
*)
    PWD=$(pwd)
    SCRIPT="$PWD/$0"
    ;;
esac
REALPATH=$(dirname $SCRIPT)
cd $REALPATH

# 停止守护进程
./monitor.sh stop

# 杀死工作进程
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

jq -r '.server_list[]|"\(.ip) \(.exe) \(.port)"' config/server.json | while read ip exe port; do
    if [ ${ip} = ${local_ip} ]; then
        source ./exec_stop_service.sh $exe $ip $port
    fi
done
