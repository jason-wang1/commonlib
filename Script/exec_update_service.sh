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

if [ "$#" -le 2 ]; then
    echo "please input process name ip port!"
    exit 1
fi

exe=$1
ip=$2
port=$3

username=$(whoami)
ServiceName_Port=${exe}_${port}

pid=$(ps -u ${username} -f | grep -w ${ServiceName_Port} | grep -w ${ip} | grep -v 'grep' | awk '{printf("%d ",$2);}')
if [[ ${pid} ]]; then
    kill -35 ${pid}
    echo "-kill 35 ${pid}, update ${ServiceName_Port} config ..."
else
    echo "process not running..."
fi
