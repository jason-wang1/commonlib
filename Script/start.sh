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

# 启动守护进程, 由守护进程启动服务进程
./monitor.sh start
