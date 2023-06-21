#pragma once
#include "gtest/gtest.h"
#include "ProtoFactory/ProtoFactory.h"
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

class Test_ProtoFactory : public testing::Test
{
public:
    // 整体test执行前执行
    static void SetUpTestSuite()
    {
    }

    // 整体test执行完毕后执行
    static void TearDownTestSuite()
    {
    }

    // 单个Test执行前执行
    virtual void SetUp()
    {
    }

    // 单个Test执行后执行
    virtual void TearDown()
    {
    }

public:
    static const std::string folder;
    static const std::string file;
    static const std::string test_proto_name;

    static const std::string none_folder;     // 不存在的文件夹
    static const std::string none_file;       // 不存在的文件
    static const std::string no_proto_file;   // 不是proto的文件
    static const std::string none_proto_name; // 不存在的proto名称
};

const std::string Test_ProtoFactory::folder = "/data_package/algo_proto/redis/";
const std::string Test_ProtoFactory::file = "/data_package/algo_proto/redis/user_feature.proto";
const std::string Test_ProtoFactory::test_proto_name = "RSP_UserFeature.UserFeatureBasic";

const std::string Test_ProtoFactory::none_folder = "/data_package/algo_protos";
const std::string Test_ProtoFactory::none_file = "/data_package/algo_protos/abc.proto";
const std::string Test_ProtoFactory::no_proto_file = "./config/server.json";
const std::string Test_ProtoFactory::none_proto_name = "RSP_UserFeature.UserFeatureNone";

// 测试条件: 文件路径初始化, 返回初始化成功
TEST_F(Test_ProtoFactory, InitFile)
{
    ProtoFactory factory;
    bool ret = factory.Init(file);
    ASSERT_EQ(ret, true);
}

// 测试条件: 文件夹路径初始化, 返回初始化成功
TEST_F(Test_ProtoFactory, InitFolder)
{
    ProtoFactory factory;
    bool ret = factory.Init(folder);
    ASSERT_EQ(ret, true);
}

// 测试条件: 初始化不存在的文件夹路径, 返回初始化失败
TEST_F(Test_ProtoFactory, NotExistFolder)
{
    ProtoFactory factory;
    bool ret = factory.Init(none_folder);
    ASSERT_EQ(ret, false);
}

// 测试条件: 初始化不存在的文件, 返回初始化失败
TEST_F(Test_ProtoFactory, NotExistFile)
{
    ProtoFactory factory;
    bool ret = factory.Init(none_file);
    ASSERT_EQ(ret, false);
}

// 测试条件: 初始化不是proto的文件, 返回初始化失败
TEST_F(Test_ProtoFactory, NoProtoFile)
{
    ProtoFactory factory;
    bool ret = factory.Init(no_proto_file);
    ASSERT_EQ(ret, false);
}

// 测试条件: 文件夹初始化
// 获取Proto成功
// 获取不存在的Proto失败
TEST_F(Test_ProtoFactory, FolderGetProto)
{
    ProtoFactory factory;
    bool ret = factory.Init(folder);
    ASSERT_EQ(ret, true);

    // 获取proto成功
    auto proto = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto, nullptr);

    // 获取不存在的proto失败
    auto _none_proto = factory.GetProtoMessage(none_proto_name);
    ASSERT_EQ(_none_proto, nullptr);
}

// 测试条件: 文件初始化
// 获取Proto成功
// 获取不存在的Proto失败
TEST_F(Test_ProtoFactory, FileGetProto)
{
    ProtoFactory factory;
    bool ret = factory.Init(file);
    ASSERT_EQ(ret, true);

    // 获取proto成功
    auto proto = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto, nullptr);

    // 获取不存在的proto失败
    auto _none_proto = factory.GetProtoMessage(none_proto_name);
    ASSERT_EQ(_none_proto, nullptr);
}

// 测试条件: 重复初始化ProtoFactory
TEST_F(Test_ProtoFactory, DoubleInitFoler)
{
    ProtoFactory factory;
    bool ret1 = factory.Init(folder);
    ASSERT_EQ(ret1, true);

    auto proto1 = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto1, nullptr);

    bool ret2 = factory.Init(folder);
    ASSERT_EQ(ret2, true);

    auto proto2 = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto2, nullptr);

    ASSERT_NE(proto1, proto2);
}

#include "google/protobuf/util/json_util.h"

// 测试条件: 重复初始化ProtoFactory
TEST_F(Test_ProtoFactory, DoubleInitFile)
{
    ProtoFactory factory;
    bool ret1 = factory.Init(file);
    ASSERT_EQ(ret1, true);

    auto proto1 = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto1, nullptr);

    bool ret2 = factory.Init(file);
    ASSERT_EQ(ret2, true);

    auto proto2 = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto2, nullptr);

    std::string str_proto1;
    auto status = google::protobuf::util::MessageToJsonString(*proto1.get(), &str_proto1);
    if (!status.ok())
    {
        std::cout << "proto1 to json failed." << std::endl;
    }

    ASSERT_NE(proto1, proto2);
}

// 测试条件: 重复初始化ProtoFactory
TEST_F(Test_ProtoFactory, MixInitFolerAndFile)
{
    ProtoFactory factory;
    bool ret1 = factory.Init(folder);
    ASSERT_EQ(ret1, true);

    auto proto1 = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto1, nullptr);

    bool ret2 = factory.Init(file);
    ASSERT_EQ(ret2, true);

    auto proto2 = factory.GetProtoMessage(test_proto_name);
    ASSERT_NE(proto2, nullptr);

    ASSERT_NE(proto1, proto2);
}
