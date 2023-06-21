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

# 项目配置文件移动到Release目录下
project_config_file=./Release/project_config

common_script_dir=$(cat ${project_config_file} | grep "common_script_dir" | awk -F"=" '{print $2}')

package_name=$(cat ${project_config_file} | grep "package_name" | awk -F"=" '{print $2}')

build_folder_path=$(cat ${project_config_file} | grep "build_folder_path" | awk -F"=" '{print $2}')
build_exec_name=$(cat ${project_config_file} | grep "build_exec_name" | awk -F"=" '{print $2}')
build_test_name=$(cat ${project_config_file} | grep "build_test_name" | awk -F"=" '{print $2}')

monitor_py_name=$(cat ${project_config_file} | grep "monitor_py_name" | awk -F"=" '{print $2}')

version_file_path=$(cat ${project_config_file} | grep "version_file_path" | awk -F"=" '{print $2}')
version_key=$(cat ${project_config_file} | grep "version_key" | awk -F"=" '{print $2}')
version=$(cat ${version_file_path} | grep ${version_key} | awk -F '"' '{print $2}')

proto_generate_path=$(cat ${project_config_file} | grep "proto_generate_path" | awk -F"=" '{print $2}')
proto_out_path=$(cat ${project_config_file} | grep "proto_out_path" | awk -F"=" '{print $2}')

proxy_generate_path=$(cat ${project_config_file} | grep "proxy_generate_path" | awk -F"=" '{print $2}')
proxy_out_path=$(cat ${project_config_file} | grep "proxy_out_path" | awk -F"=" '{print $2}')

log_file_path=$(cat ${project_config_file} | grep "log_file_path" | awk -F"=" '{print $2}')
feature_field_path=$(cat ${project_config_file} | grep "feature_field_path" | awk -F"=" '{print $2}')

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

func_get_pid() {
    echo $(ps -x | grep ${1}_${3} | grep ${2} | grep -v 'grep')
}

func_start_server() {
    jq -r '.server_list[]|"\(.ip) \(.exe) \(.port) \(.nickname)"' config/server.json | while read ip exe port nickname; do
        if [ $ip = $local_ip ]; then
            if [ "$#" -ge 1 ] && [ ${exe} == "$1" ]; then
                ./exec_start_service.sh $exe $ip $port $nickname
            fi
            if [ "$#" -eq 0 ]; then
                ./exec_start_service.sh $exe $ip $port $nickname
            fi
        fi
    done
}

func_stop_server() {
    jq -r '.server_list[]|"\(.ip) \(.exe) \(.port)"' config/server.json | while read ip exe port; do
        if [ $ip = $local_ip ]; then
            if [ "$#" -ge 1 ] && [ ${exe} == "$1" ]; then
                ./exec_stop_service.sh $exe $ip $port
            fi
            if [ "$#" -eq 0 ]; then
                ./exec_stop_service.sh $exe $ip $port
            fi
        fi
    done
}

func_conf_update() {
    jq -r '.server_list[]|"\(.ip) \(.exe) \(.port)"' config/server.json | while read ip exe port; do
        if [ $ip = $local_ip ]; then
            if [ "$#" -ge 1 ] && [ ${exe} == "$1" ]; then
                ./exec_update_service.sh $exe $ip $port
            fi
            if [ "$#" -eq 0 ]; then
                ./exec_update_service.sh $exe $ip $port
            fi
        fi
    done
}

func_status_server() {
    jq -r '.server_list[]|"\(.ip) \(.exe) \(.port)"' config/server.json | while read ip exe port; do
        if [ $ip = $local_ip ]; then
            func_get_pid $exe $ip $port
        fi
    done
}

func_build() {
    # 通过参数决定编译项目/编译模式
    param_failed=true
    debug_build=true
    if [ "$#" -eq 1 ]; then
        if [ "$1" = "debug" ]; then
            debug_build=true
            param_failed=false
        elif [ "$1" = "release" ]; then
            debug_build=false
            param_failed=false
        fi
    fi

    if ${param_failed}; then
        echo "please input build_mode!!!"
        echo "  server.sh build debug"
        echo "  server.sh build release"
        exit 1
    fi

    echo "build "${build_exec_name}" project"

    # goto Folder
    cd ./${build_folder_path}

    # gcc env
    source /opt/rh/devtoolset-10/enable

    # clean old exe file
    rm -rf Bin/${build_exec_name}
    if [ -f Bin/${build_test_name} ]; then
        rm -rf Bin/${build_test_name}
    fi

    # make
    mkdir -p ./cmake/build
    processors_num=$(cat /proc/cpuinfo| grep processor | wc -l)
    if ${debug_build}; then
        cd cmake/build/
        cmake3 -DDEBUG=ON -DCPP20=OFF ../../
        make -j${processors_num}
        cd ../../
    else
        cd cmake/build/
        cmake3 -DDEBUG=OFF -DCPP20=OFF ../../
        make -j${processors_num}
        cd ../../
    fi

    # clean Bin and Release 目录 build_exec_name
    rm -rf ../Bin/${build_exec_name}
    rm -rf ../Bin/${build_test_name}
    rm -rf ../Release/${build_exec_name}

    # 复制到Bin或Release目录内
    if [ -f Bin/${build_exec_name} ]; then

        if ${debug_build}; then
            cp Bin/${build_exec_name} ../Bin/${build_exec_name}
        else
            cp Bin/${build_exec_name} ../Bin/${build_exec_name}
            cp Bin/${build_exec_name} ../Release/${build_exec_name}
        fi
    fi

    # 单元测试文件
    if [ -f Bin/${build_test_name} ]; then
        cp Bin/${build_test_name} ../Bin/${build_test_name}
    fi

    # 移除 Bin 目录
    rm -rf ./Bin

    # return dir
    cd ../

    # Bin文件夹下文件存在, 说明编译成功
    if [ -f Bin/${build_exec_name} ]; then
        if ${debug_build}; then
            echo "debug build succ, version = "${version}
        else
            echo "release build succ, version = "${version}
        fi
    else
        echo "build failed, version = "${version}
        exit 1
    fi
}

func_clean() {
    rm -rf ./Bin/*.core
    rm -rf ./Bin/*.out
    rm -rf ./Bin/monitor/__pycache__
    rm -rf ./Bin/monitor/__init__.py
    rm -rf ./Bin/monitor/*.pyc
    rm -rf ./Bin/report_log
    rm -rf ./Bin/${build_exec_name}
    rm -rf ./Bin/Log${build_exec_name}/

    # 删除单元测试文件
    if [ -f ./Bin/${build_test_name} ]; then
        rm -rf ./Bin/${build_test_name}
    fi

    rm -rf ./Release/*.core
    rm -rf ./Release/*.out
    rm -rf ./Release/monitor/__pycache__
    rm -rf ./Release/monitor/__init__.py
    rm -rf ./Release/monitor/*.pyc
    rm -rf ./Release/report_log
    rm -rf ./Release/${build_exec_name}
    rm -rf ./Release/Log${build_exec_name}/

    rm -rf ./${build_folder_path}/cmake
    rm -rf ./${build_folder_path}/Bin

    rm -rf ${package_name}_*

    rm -rf Log${build_exec_name}
    rm -rf cpptools-*.core
}

func_package() {
    # 编译
    func_clean
    func_build release

    # 创建临时目录, 将文件夹copy过去
    rm -rf ${package_name}_*
    folderName="${package_name}_${version}"
    cp -r ./Release ./${folderName}
    echo -e "\n# 版本信息\nservice_semver=${version}" >>./${folderName}/project_config

    # 拷贝公共库脚本
    cp ${common_script_dir}/init_env.sh ./${folderName}/.
    cp ${common_script_dir}/monitor.sh ./${folderName}/.
    cp ${common_script_dir}/start.sh ./${folderName}/.
    cp ${common_script_dir}/stop.sh ./${folderName}/.
    cp ${common_script_dir}/exec_stop_service.sh ./${folderName}/.
    cp ${common_script_dir}/update.sh ./${folderName}/.
    cp ${common_script_dir}/clean_history_log.sh ./${folderName}/.
    cp ${common_script_dir}/monitor.py ./${folderName}/${monitor_py_name}
}

func_proto() {
    if [ -f ./${proto_generate_path} ]; then
        ./${proto_generate_path} ${proto_out_path}
    fi

    if [ -f ./${proxy_generate_path} ]; then
        ./${proxy_generate_path} ${proxy_out_path}
    fi
}

func_check_feature() {
    DEF='\e[0m'
    OK='\e[1;32m'
    FAILED='\e[1;31m'

    log_feature=$(grep 'RankLog' ${log_file_path} | sed "s/^.*, FMFeature = \(.*\)$/\1/g" | awk '{for(i=1;i<=NF;i++){split($i,fvw,":");fset[rshift(fvw[1],32)]++}}END{for(f in fset){print f"="fset[f]}}')
    declare -A log_feature_pool
    for feature in ${log_feature}; do
        feature_count=(${feature//=/ })
        log_feature_pool[${feature_count[0]}]=${feature_count[1]}
    done

    field_list=$(grep ',$' ${feature_field_path} | sed 's/^.*    \(.*\),$/\1/g' | awk -F' = ' '{print $1"="$2}')
    for field in ${field_list}; do
        field_name=(${field//=/ })
        if [ "${log_feature_pool[${field_name[1]}]}" != "" ]; then
            echo -e "[${field_name[0]} = ${field_name[1]}]${OK} is ok.${DEF}"
        else
            echo -e "[${field_name[0]} = ${field_name[1]}]${FAILED} check failed!!!${DEF}"
        fi
    done
}

func_unit_test() {
    # 执行单元测试
    if [ -f ./${build_test_name} ]; then
        ./${build_test_name} 2>&1 | tee unit_test.out
    else
        echo "test file not exist."
    fi
}

case "$1" in
'build')
    func_build $2
    ;;
'clean')
    func_clean
    ;;
'package')
    func_package $2
    ;;
'start')
    cd Bin
    func_start_server
    ;;
'stop')
    cd Bin
    func_stop_server
    ;;
'update')
    cd Bin
    func_conf_update
    ;;
'restart')
    cd Bin
    func_stop_server
    func_start_server
    ;;
'status')
    cd Bin
    func_status_server
    ;;
'proto')
    func_proto
    ;;
'check_feature')
    func_check_feature
    ;;
'unit_test')
    cd Bin
    func_unit_test
    ;;
*)
    printf "action list: \n"
    printf "  help              -- 帮助菜单 \n"
    printf "  build             -- 编译项目 \n"
    printf "  clean             -- 清理项目 \n"
    printf "  status            -- 本机服务状态 \n"
    printf "  package           -- 打包项目 \n"
    printf "  proto             -- 生成Proto \n"
    printf "  check_feature     -- 检查特征 \n"
    printf "  unit_test         -- 执行单元测试 \n"
    exit 1
    ;;
esac
