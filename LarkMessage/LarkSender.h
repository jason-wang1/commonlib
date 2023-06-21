#pragma once
#include "LarkMessage.h"
#include "Common/Function.h"

namespace Common
{
    /**
     * @brief 发送服务上线下线消息
     * @param webhook 机器人地址
     * @param title 消息标题
     * @param addr 服务地址
     * @param service 服务名称
     * @param semver 服务版本号
     * @param other 其他信息, 不需要可传空
     * @note 同步网络请求, 可能会阻塞.
     **/
    inline void NotifyLark_Change(
        const std::string &webhook,
        const std::string &title,
        const std::string &addr,
        const std::string &service,
        const std::string &semver,
        const std::string &other)
    {
        if (webhook.empty())
        {
            return;
        }

        std::string current_time = Common::timestamp2date(
            Common::get_timestamp(), "%Y-%m-%d %H:%M:%S");

        // 拼接信息
        LarkMessage msg;
        msg.SetOther(other);
        msg.SetTitle(title);
        msg.AddText("time: ", current_time);
        msg.AddText("addr: ", addr);
        msg.AddText("service: ", service);
        msg.AddText("semver: ", semver);

        // 发送数据
        std::string retMsg;
        if (!msg.Send(webhook, retMsg))
        {
            LOG(ERROR) << "Application::NotifyLark_Change() Send Failed, retMsg = " << retMsg;
        }
    }

    /**
     * @brief 发送服务上线下线消息到飞书
     * @param webhook 机器人地址
     * @param title 消息标题
     * @param service_name 服务名称
     * @param group 服务分组
     * @param service_type 服务类型
     * @param other 其他信息, 不需要可传空
     * @param vecAtUser 通知用户 @
     * @note 同步网络请求, 可能会阻塞.
     **/
    inline void NotifyLark_RelyWarning(
        const std::string &webhook,
        const std::string &title,
        const std::string &group_tab,
        const std::string &service_type,
        const std::string &service_name,
        const std::string &other)
    {
        if (webhook.empty())
        {
            return;
        }

        std::string current_time = Common::timestamp2date(
            Common::get_timestamp(), "%Y-%m-%d %H:%M:%S");

        // 拼接信息
        LarkMessage msg;
        msg.SetOther(other);
        msg.SetTitle(title);
        msg.AddText("time: ", current_time);
        msg.AddText("group_tab: ", group_tab);
        msg.AddText("service_type: ", service_type);
        msg.AddText("service_name: ", service_name);

        // 发送数据
        std::string retMsg;
        if (!msg.Send(webhook, retMsg))
        {
            LOG(ERROR) << "Application::NotifyLark_RelyWarning() Send Failed, retMsg = " << retMsg;
        }
    }
}
