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

source /opt/rh/rh-python36/enable

project_config_file=./project_config
monitor_py_name=$(cat ${project_config_file} | grep "monitor_py_name" | awk -F"=" '{print $2}')
APPNAME=./${monitor_py_name}

getpid() {
    pid=$(ps -x | grep $APPNAME | grep -v grep | awk '{print $1}' | tail -1)
}

start_server() {
    echo "Starting $APPNAME..."
    getpid
    if [ "X$pid" = "X" ]; then
        nohup python $APPNAME >/dev/null 2>&1 &
        echo "$APPNAME Start finished"
    else
        echo "$APPNAME is already running."
        exit 1
    fi
}

stop_server() {
    echo "Stopping $APPNAME..."
    getpid
    if [ "X$pid" = "X" ]; then
        echo "$APPNAME was not running."
    else
        # Running so try to stop it.
        kill $pid
        if [ $? -ne 0 ]; then
            echo "Unable to stop $APPNAME."
            exit 1
        fi

        #  Loop until it does.
        savepid=$pid
        CNT=0
        TOTCNT=0
        while [ "X$pid" != "X" ]; do
            # Loop for up to 5 minutes
            if [ "$TOTCNT" -lt "30" ]; then
                if [ "$CNT" -lt "5" ]; then
                    CNT=$(expr $CNT + 1)
                else
                    echo "Waiting for $APPNAME. to exit..."
                    CNT=0
                fi
                TOTCNT=$(expr $TOTCNT + 1)

                sleep 1

                getpid
            else
                pid=
            fi
        done

        pid=$savepid
        getpid
        if [ "X$pid" != "X" ]; then
            echo "Timed out waiting for $APPNAME to exit."
            echo "  Attempting a forced exit..."
            kill -9 $pid
        fi

        pid=$savepid
        getpid
        if [ "X$pid" != "X" ]; then
            echo "Failed to stop $APPNAME."
            exit 1
        else
            echo "Stopped $APPNAME."
        fi
    fi
}

status_server() {
    getpid
    if [ "X$pid" = "X" ]; then
        echo "the ${APPNAME} server is not running."
        exit 1
    else
        echo "the ${APPNAME} server is running ($pid)."
        exit 0
    fi
}

case "$1" in
'stop')
    stop_server $1
    ;;
'start')
    start_server $1
    ;;
'restart')
    stop_server $1
    start_server $1
    ;;
'status')
    status_server $1
    ;;
*)
    printf "action : start | stop | restart | status \n"
    exit 1
    ;;
esac
