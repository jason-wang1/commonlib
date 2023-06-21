#!/bin/sh
# 正式环境一键发布, 配置环境
export LANG=zh_CN.utf-8

username=$(whoami)
if [ ! ${username} == "root" ]; then
    echo "please su root, cur_user: " ${username}
    echo Failed
    exit 1
fi

# 传入参数
if [ "$#" -le 0 ]; then
    echo "please input cmd service and semver !!!"
    echo "example: TDBuild.sh start ws_map_rank 0.0.1"
    echo Failed
    exit 1
fi

action=${1}
serviceName=${2}
semver=${3}
folderName=${serviceName}_${semver}
tarFileName=${serviceName}_${semver}".tar.gz"

serverPath=/data/${serviceName}_server/
releasePath=/data_package/release/${serviceName}/

copy_file() {
    mkdir -p ${serverPath}
    cd ${serverPath}

    rm -rf ${tarFileName} ${folderName}/
    cp ${releasePath}/${tarFileName} .

    # 如果tar文件存在才进行解压
    if [ -e ${serverPath}/${tarFileName} ]; then
        tar -zxf ${tarFileName} && echo "tar file ${tarFileName} succ."
        chown -R root:root ${folderName}/
        cd ./${folderName}/
        chmod +x init_env.sh stop.sh start.sh monitor.sh update.sh exec_stop_service.sh
    else
        echo "file ${serverPath}/${tarFileName} not exist."
    fi
}

start_func() {
    if [ ! -d ${serverPath}/${folderName} ]; then
        copy_file
    fi

    if [ ! -d ${serverPath}/${folderName} ]; then
        echo Failed
        exit 1
    fi

    cd ${serverPath}/${folderName}
    ./stop.sh
    ./start.sh
    sleep 3 # 等3秒启动完毕.
    echo Succeed
}

stop_func() {
    if [ -d ${serverPath}/${folderName} ]; then
        cd ${serverPath}/${folderName}
        ./stop.sh
        echo Succeed
    else
        echo Failed
        exit 1
    fi
}

env_func() {
    if [ ! -d ${serverPath}/${folderName} ]; then
        copy_file
    fi

    if [ -d ${serverPath}/${folderName} ]; then
        cd ${serverPath}/${folderName}
        ./init_env.sh
        echo Succeed
    else
        echo Failed
        exit 1
    fi
}

update_func() {
    if [ -d ${serverPath}/${folderName} ]; then
        cd ${serverPath}/${folderName}
        ./update.sh
        echo Succeed
    else
        echo Failed
        exit 1
    fi
}

tfs_update_func() {
    if [ -x /data_package/tfs_model/update_tfs.sh ]; then
        /data_package/tfs_model/update_tfs.sh ${serviceName} ${semver}
    else
        echo "/data_package/tfs_model/update_tfs.sh not exist."
        echo Failed
        exit 1
    fi
}

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

case "$1" in
'start')
    start_func
    ;;
'stop')
    stop_func
    ;;
'env')
    env_func
    ;;
'update')
    update_func
    ;;
'tfs_update')
    tfs_update_func
    ;;
*)
    printf "action : start | stop | env | update \n"
    printf "action : start      -- 启动服务 \n"
    printf "action : stop       -- 停止服务 \n"
    printf "action : env        -- 安装环境 \n"
    printf "action : update     -- 更新配置 \n"
    printf "action : tfs_update -- 更新TFServing\n"
    exit 1
    ;;
esac
