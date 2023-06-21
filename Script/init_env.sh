#!/bin/bash
# 生产环境启动服务

cmd='sudo yum -y install centos-release-scl'
${cmd}

cmd='sudo yum -y install rh-python36'
${cmd}

cmd='sudo yum -y install python-pip'
${cmd}

cmd='sudo yum -y install gdb'
${cmd}

cmd='sudo yum -y install jq'
${cmd}

cmd='sudo yum -y install cpulimit'
${cmd}

cmd='sudo yum -y install lsof'
${cmd}

cmd='source /opt/rh/rh-python36/enable'
${cmd}

cmd='pip install --upgrade pip'
${cmd}

cmd='pip install requests'
${cmd}

cmd='pip install psutil'
${cmd}

cmd='pip install python-dateutil'
${cmd}

# 安装core相关的文件
echo $(modprobe br_netfilter)
echo $(ls /proc/sys/net/bridge)
echo $(sysctl -p)

# 替换有关键字的所有行
replace_line_by_word() {
    file="$1"
    old_word="$2"
    new_word="$3"

    result=$(grep -a "$old_word" $file)
    if [ ! -n "$result" ]; then
        echo "$new_word" >>$file
    else
        sed -i "/$old_word/d" $file
        echo "$new_word" >>$file
    fi
}

old_word="* soft core"
new_word="* soft core unlimited"
replace_line_by_word /etc/security/limits.conf "$old_word" "$new_word"

old_word="* hard core"
new_word="* hard core unlimited"
replace_line_by_word /etc/security/limits.conf "$old_word" "$new_word"

old_word="* soft nofile"
new_word="* soft nofile 65535"
replace_line_by_word /etc/security/limits.conf "$old_word" "$new_word"

old_word="* hard nofile"
new_word="* hard nofile 65535"
replace_line_by_word /etc/security/limits.conf "$old_word" "$new_word"

replace_line_by_word /etc/sysctl.conf "kernel.core_pattern" "kernel.core_pattern=%e.%p.%t.core"
replace_line_by_word /etc/sysctl.conf "fs.suid_dumpable" "fs.suid_dumpable=1"
replace_line_by_word /etc/sysconfig/init "DAEMON_COREFILE_LIMIT" "DAEMON_COREFILE_LIMIT='unlimited'"

sysctl -p /etc/sysctl.conf
echo "coredump配置成功"
