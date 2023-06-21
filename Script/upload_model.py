# -*- coding:utf-8 -*-
import requests
import json
import socket


def get_local_ip():
    # 获取本机IP
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(('8.8.8.8', 80))
        local_ip = s.getsockname()[0]
    finally:
        s.close()
    return local_ip


def upload_model(uuid, service_name, model_name, upload_path):
    url = "http://algo.mini1.cn/api/v1/model/upload"
    request = dict()
    request['uuid'] = uuid
    request['addr'] = get_local_ip() + ":6020"
    request['model_name'] = model_name
    request['service_name'] = service_name
    request['upload_path'] = upload_path
    headers = {'Content-Type': 'application/json;charset=UTF-8'}
    res = requests.post(url, json=request, headers=headers)
    response = dict()
    response = json.loads(res.content)
    print(response)
    return bool(response['code'] == 0 and response['msg'] == "ok"), response['msg']


def update_model(uuid, operator_id, service_name, model_name, upload_path, comment):
    url = "http://algo.mini1.cn/api/v1/model/upload-and-update"
    request = dict()
    request['uuid'] = uuid
    request['operator_id'] = operator_id
    request['addr'] = get_local_ip() + ":6020"
    request['comment'] = comment
    request['model_name'] = model_name
    request['service_name'] = service_name
    request['upload_path'] = upload_path
    headers = {'Content-Type': 'application/json;charset=UTF-8'}
    res = requests.post(url, json=request, headers=headers)
    response = dict()
    response = json.loads(res.content)
    print(response)
    return bool(response['code'] == 0 and response['msg'] == "ok"), response['msg']
