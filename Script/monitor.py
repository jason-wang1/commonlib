#!/usr/bin/python
# -*- coding: UTF-8 -*-
import socket
import subprocess
import json
import threading
import time
import requests
import os
import psutil
from dateutil.parser import parse  # python-dateutil

# 监控进程的状态
monitor_services = {
}

status_text = {0: "关闭", 1: "启动"}

send_msg_interval = 60    # 间隔60秒发送一次告警信息
error_log_warn_time = 60  # 监控60秒内的错误日志
warn_errors_count = 10    # 每秒错误日志数超过10时告警

last_clean_log_time = 0    # 上一次清理日志时间
clean_log_interval = 3600  # 清理日志时间间隔


def get_local_ip():
    # 获取本机IP
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        local_ip = s.getsockname()[0]
    finally:
        s.close()
    return local_ip


def get_hostname():
    # 获取机器名称
    return socket.gethostname()


def get_current_path():
    cmd = "pwd"
    _, current_path = execute_cmd(cmd)
    return current_path

def get_service_semver():
    cmd = "grep service_semver project_config | awk -F'=' '{print $2}'"
    _, semver = execute_cmd(cmd)
    return semver

def send_msg(title, info):
    # 发送告警信息至推荐系统群
    # 注释“推荐平台 - 算法”群告警机器人
    url = "https://oapi.dingtalk.com/robot/send?access_token=ddf64928cf9db812cdb3538c93e2ade2d7df3f05da87cc4fc76f36efd034c721"
    message = dict()
    message['msgtype'] = "markdown"
    tmp = dict()
    tmp['title'] = title
    tmp['text'] = info
    message['markdown'] = tmp
    headers = {'Content-Type': 'application/json'}
    res = requests.request("post", url, json=message, headers=headers)
    print("send msg ret = ", res)


def write_log(context):
    # 写日志
    if context == "":
        return

    # 创建日志文件夹
    log_path = "report_log"
    if not os.path.exists(log_path):
        os.makedirs(log_path)

    # 信息写入文件
    cur_day = time.strftime("%Y%m%d", time.localtime())
    file_name = log_path+"/report_msg_"+cur_day+".log"
    f = open(file_name, 'a+')
    f.write(context)
    f.close()


def get_pid(name, factor):
    cmd = "ps -x | grep -w {} | grep -w {} | grep -v 'grep' | \
        awk '{{printf $1}}'".format(name, factor)
    ret, pid = execute_cmd(cmd)
    if ret == 0 and pid != "":
        return pid
    return 0


def get_exe_pid(name, ip):
    # 获取exe程序的pid
    return get_pid(name, ip)


def init_monitor_service():
    # 初始化监控进程
    work_path = get_current_path()
    semver = get_service_semver()
    local_ip = get_local_ip()
    host_name = get_hostname()
    server_conf = open("config/server.json", 'r')
    server_conf_json = json.load(server_conf)
    server_list = server_conf_json["server_list"]
    for server in server_list:
        exe = server["exe"]
        ip = server["ip"]
        port = server["port"]
        nickname = server["nickname"]
        if ip == local_ip:
            name = "{}_{}".format(exe, port)
            monitor_services[name] = {}
            service = monitor_services[name]
            service["name"] = name  # 应用名称 exe_port
            service["host_name"] = host_name  # 服务器名称
            service["ip"] = ip
            service["port"] = port
            service["nickname"] = nickname
            service["exe"] = exe
            service["log"] = "../Log"+exe  # 日志目录
            service["work_path"] = work_path
            service["semver"] = semver

            service["process"] = None
            service["process_pid"] = 0

            pid = get_exe_pid(name, ip)
            if pid != 0:
                service["pid"] = pid
            else:
                service["pid"] = 0
            service["send_msg_time_stamp"] = 0

    return len(monitor_services)


def execute_cmd(cmd, daemon=""):
    # 执行命令
    if daemon != "":
        ret = os.system(cmd)
        print("execute_cmd, ret={}, cmd={}".format(ret, cmd))
        return ret
    else:
        return subprocess.getstatusoutput(cmd)


def start_service(name):
    # 获取服务信息
    print("start_service", name)
    service = monitor_services[name]
    ip = service["ip"]
    port = service["port"]
    exe = service["exe"]
    nickname = service["nickname"]

    if not os.path.exists(exe):
        print("exe not found, please check package.")
        exit(0)

    # 如果服务不存在, 启动服务
    start_cmd = "nohup ./{} {} {} {} {} > /dev/null 2>&1 &".format(
        exe, ip, port, nickname, name)
    cmd = "ps -x | grep -w {} | grep -w {} | grep -v 'grep' | \
        awk '{{printf $1}}';if [ $? -eq 0 ];then {} fi".format(name, ip, start_cmd)
    execute_cmd(cmd, "&")

    # 通过端口号判断服务已启动
    confirm = "netstat -pln | grep {} | grep {} | \
        awk '{{print $7}}' | awk -F'/./' '{{print $1}}'".format(port, exe)
    loop = 100
    while loop > 0:
        ret, pid = execute_cmd(confirm)
        if ret == 0 and pid != "":
            break
        loop -= 1
        time.sleep(0.1)


def check_service_online_status():
    now_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
    status_contexts = ""
    for name in monitor_services:
        service = monitor_services[name]
        old_pid = service["pid"]
        pid = get_exe_pid(name, service["ip"])
        service["pid"] = pid

        text_pid = old_pid
        text_status = status_text[0]
        if old_pid != pid:
            if (pid != 0):
                text_pid = pid
                text_status = status_text[1]

            status_contexts += "服务 {} {} {} {}\n".format(
                name,
                text_pid,
                text_status,
                now_time)

        if pid == 0:
            start_service(name)

    # 将信息写入日志文件
    write_log(status_contexts)
    return


def get_service_errors_count(name):
    ret_max = 0
    ret_record_list = ""
    service = monitor_services[name]
    name = service["name"]
    log_path = service["log"]
    log_name = "{}/{}.ERROR".format(log_path, name)
    now_time = int(time.time())
    read_log_count = error_log_warn_time * warn_errors_count  # 读取日志行数
    if os.path.exists(log_name):
        query_cmd = "tail -{} {} | grep '^E20' | \
            awk -F'[E :.]' '{{print substr($2,1,4),substr($2,5,2),substr($2,7,2),$3,$4,$5\"\t\"$0}}' | awk -F'\t' '{{print mktime($1)\"\t\"$2}}' | \
            awk -F'\t' '{{if({}-$1<{}){{ts[$1]++;re[n++]=$2}}}}END{{m=0;for(t in ts){{if(ts[t]>m)m=ts[t]}}print m;for(i=1;i<4&&n-i>=0;i++)print re[n-i];}}'".format(
            read_log_count, log_name, now_time, error_log_warn_time)
        _, record_list = execute_cmd(query_cmd)

        record_ary = record_list.splitlines()
        for idx in range(len(record_ary)):
            if idx == 0:
                ret_max = int(record_ary[idx])
            else:
                ret_record_list += (record_ary[idx] + "\n")

    return ret_max, ret_record_list


def warn_service_errors_count(name, count, log):
    if count < warn_errors_count:
        return

    # 服务信息
    service = monitor_services[name]
    title = "算法服务监控告警"
    now_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
    info = "### {}: \n\n".format(title)
    info += "   time: {}\n\n".format(now_time)
    info += "   host: {}\n\n".format(service["host_name"])
    info += "   addr: {}\n\n".format(service["ip"] + ":" + service["port"])
    info += "   exe: {}\n\n".format(service["exe"])
    info += "   semver: {}\n\n".format(service["semver"])
    info += "   pid: {}\n\n".format(service["pid"])
    info += "   err logs: {} /sec\n\n{}".format(count, log)

    # 发送告警信息
    time_stamp = int(time.time())
    send_time_stamp = service["send_msg_time_stamp"]
    interval = time_stamp - send_time_stamp
    if interval >= send_msg_interval:
        # 上一次发送时间间隔过期, 再发送一次
        send_msg(title, info)
        service["send_msg_time_stamp"] = time_stamp


def check_service_error_log():
    for name in monitor_services:
        errors_count, error_log_list = get_service_errors_count(name)
        warn_service_errors_count(name, errors_count, error_log_list)
    return


def clean_history_log():
    global last_clean_log_time
    now_time = int(time.time())
    if now_time - last_clean_log_time > clean_log_interval:
        last_clean_log_time = now_time

        for name in monitor_services:
            service = monitor_services[name]
            work_path = service["work_path"]
            exe = service["exe"]
            port = service["port"]
            clean_cmd = "sh clean_history_log.sh {} {} {}".format(
                work_path, exe, port)
            _, _ = execute_cmd(clean_cmd)

    return


def checking_service():
    while True:
        # 查看服务在线情况
        check_service_online_status()

        # 查看服务错误日志
        check_service_error_log()

        # 清理历史日志
        clean_history_log()

        # 进程扫描睡眠时间
        time.sleep(1)


def start_checking_service():
    threads = []
    multi_process = threading.Thread(target=checking_service, args=())
    threads.append(multi_process)
    multi_process.start()

    # 等线程结束
    for t in threads:
        t.join()


if __name__ == '__main__':
    ret = init_monitor_service()
    if ret == 0:
        print("No service start, monitor exit.\n")
        write_log("No service start, monitor exit.\n")
    else:
        start_checking_service()
