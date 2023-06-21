#include "TFModelGrpc.h"
#include "TFTrans.h"
#include "glog/logging.h"

using namespace TDPredict;

bool TFModelGrpc::CallBackScoreFunc(
    const grpc::Status status,
    ResponseProto &response,
    std::any data)
{
    if (data.has_value() && data.type() != typeid(TFModelGrpc::ScoreDataPtr))
    {
        LOG(ERROR) << "CallBackScoreFunc() data.type() != typeid(TFModelGrpc::ScoreDataPtr) check failed.";
        return false;
    }

    const auto &spData = std::any_cast<TFModelGrpc::ScoreDataPtr>(data);
    if (spData == nullptr || spData->obj == nullptr)
    {
        LOG(ERROR) << "CallBackScoreFunc() check spData == nullptr failed.";
        return false;
    }

    if (!status.ok())
    {
        LOG(ERROR) << "CallBackScoreFunc() Request Failed"
                   << ", ModelName = " << spData->obj->m_ModelName
                   << ", Version = " << spData->obj->m_Version
                   << ", status.error_code() = " << status.error_code()
                   << ", status.error_message() = " << status.error_message();
        return false;
    }

    const std::string output_name = spData->obj->m_OutputName;
    const OutputMap &outputs = response.outputs();
    const auto iter = outputs.find(output_name);
    if (iter == outputs.end())
    {
        LOG(ERROR) << "CallBackScoreFunc() outputs.find() Failed"
                   << ", ModelName = " << spData->obj->m_ModelName
                   << ", output_name = " << output_name;
        for (auto &pr : outputs)
        {
            LOG(ERROR) << "CallBackScoreFunc() Outputs"
                       << ", ModelName = " << spData->obj->m_ModelName
                       << ", pr.first = " << pr.first;
        }
        return false;
    }

    if (spData->vec_score == nullptr)
    {
        LOG(ERROR) << "CallBackScoreFunc() check spData->vec_score == nullptr failed.";
        return false;
    }
    auto &vec_score = *(spData->vec_score);
    const int vec_score_size = vec_score.size();
    if (vec_score_size < spData->end)
    {
        // 数组越界问题
        LOG(ERROR) << "CallBackScoreFunc() (vec_score_size < spData->end) check failed"
                   << ", vec_score_size = " << vec_score_size
                   << ", spData->end = " << spData->end;
        return false;
    }

    const auto &tensorProto = iter->second;
    const auto &tensorShape = tensorProto.tensor_shape();
    const int dim_size = tensorShape.dim_size();
    if (dim_size == 1)
    {
        const int request_count = spData->end - spData->begin;
        const int dim_batch_size = tensorShape.dim(0).size();
        const int float_val_size = tensorProto.float_val_size();
        if (float_val_size != dim_batch_size ||
            request_count != dim_batch_size)
        {
            LOG(ERROR) << "CallBackScoreFunc() request_count check failed"
                       << ", request_count = " << request_count
                       << ", float_val_size = " << float_val_size
                       << ", dim_batch_size = " << dim_batch_size;
            return false;
        }

        const int begin = spData->begin;
        for (int batch_idx = 0; batch_idx < dim_batch_size; batch_idx++)
        {
            vec_score[batch_idx + begin] = tensorProto.float_val(batch_idx);
        }
        return true;
    }
    else if (dim_size == 2)
    {
        const int request_count = spData->end - spData->begin;
        const int dim_batch_size = tensorShape.dim(0).size();
        const int dim_array_size = tensorShape.dim(1).size();
        const int float_val_size = tensorProto.float_val_size();
        if (float_val_size != dim_batch_size * dim_array_size ||
            request_count != dim_batch_size)
        {
            LOG(ERROR) << "CallBackEmbFunc() request_count check failed"
                       << ", request_count = " << request_count
                       << ", float_val_size = " << float_val_size
                       << ", dim_batch_size = " << dim_batch_size
                       << ", dim_array_size = " << dim_array_size;
            return false;
        }

        const int begin = spData->begin;
        for (int val_idx = 0, batch_idx = 0; batch_idx < dim_batch_size;
             batch_idx++, val_idx += dim_array_size)
        {
            // 取每个数组的第一个元素作为分数
            vec_score[batch_idx + begin] = tensorProto.float_val(val_idx);
        }
        return true;
    }
    else
    {
        LOG(ERROR) << "CallBackScoreFunc() dim_size check failed, dim_size = " << dim_size;
        return false;
    }
}

bool TFModelGrpc::CallBackEmbFunc(
    const grpc::Status status,
    ResponseProto &response,
    std::any data)
{
    if (data.has_value() && data.type() != typeid(TFModelGrpc::EmbAnyDataPtr))
    {
        LOG(ERROR) << "CallBackEmbFunc() data.type() != typeid(TFModelGrpc::EmbAnyDataPtr) check failed.";
        return false;
    }

    const auto &spData = std::any_cast<TFModelGrpc::EmbAnyDataPtr>(data);
    if (spData == nullptr || spData->obj == nullptr)
    {
        LOG(ERROR) << "CallBackEmbFunc() check spData == nullptr failed.";
        return false;
    }

    if (!status.ok())
    {
        LOG(ERROR) << "CallBackEmbFunc() Request Failed"
                   << ", ModelName = " << spData->obj->m_ModelName
                   << ", Version = " << spData->obj->m_Version
                   << ", status.error_code() = " << status.error_code()
                   << ", status.error_message() = " << status.error_message();
        return false;
    }

    const std::string output_name = spData->obj->m_OutputName;
    auto outputs = response.mutable_outputs();
    auto iter = outputs->find(output_name);
    if (iter == outputs->end())
    {
        LOG(ERROR) << "CallBackEmbFunc() outputs.find() Failed"
                   << ", ModelName = " << spData->obj->m_ModelName
                   << ", output_name = " << output_name;
        return false;
    }

    if (spData->vec_emb_any == nullptr)
    {
        LOG(ERROR) << "CallBackEmbFunc() check spData->vec_emb_any == nullptr failed.";
        return false;
    }
    auto &vec_emb_any = *(spData->vec_emb_any);
    const int begin = spData->begin;
    const int end = spData->end;
    const int vec_emb_any_size = vec_emb_any.size();
    if (vec_emb_any_size < end)
    {
        // 数组越界问题
        LOG(ERROR) << "CallBackEmbFunc() (vec_emb_any_size < end) check failed"
                   << ", vec_emb_any_size = " << vec_emb_any_size
                   << ", end = " << end;
        return false;
    }

    // 获取批量数及分组数
    auto &tensorProto = iter->second;
    const auto &tensorShape = tensorProto.tensor_shape();
    const int dim_size = tensorShape.dim_size();
    if (dim_size != 2)
    {
        LOG(ERROR) << "CallBackEmbFunc() dim_size == 2 check failed.";
        return false;
    }

    const int request_count = end - begin;
    const int dim_batch_size = tensorShape.dim(0).size();
    const int dim_array_size = tensorShape.dim(1).size();
    const int float_val_size = tensorProto.float_val_size();
    if (float_val_size != dim_batch_size * dim_array_size ||
        request_count != dim_batch_size)
    {
        LOG(ERROR) << "CallBackEmbFunc() val_size == batch_size * array_size check failed"
                   << ", request_count = " << request_count
                   << ", float_val_size = " << float_val_size
                   << ", dim_batch_size = " << dim_batch_size
                   << ", dim_array_size = " << dim_array_size;
        return false;
    }

    auto val_data_ptr = tensorProto.mutable_float_val()->mutable_data();
    for (int idx = begin; idx < end; idx++)
    {
        // Eigen::Map是将一段连续的内存空间映射成Eigen中Matrix的方法;
        // 可以直接使用相关的矩阵运算, 极大的节省了内存占用, 提升了计算处理效率。
        vec_emb_any[idx] = Eigen::Map<emb_any_type, Eigen::Unaligned>(val_data_ptr, dim_array_size);
        val_data_ptr += dim_array_size;
    }
    return true;
}

TFModelGrpc::TFModelGrpc(
    const std::vector<std::string> &vec_tfs_addrs,
    const std::string &signature_name,
    const std::string &model_name,
    const int timeout,
    const bool use_dropout_keep,
    const int version,
    const std::string &output_name,
    const bool use_user_type)
    : client(vec_tfs_addrs)
{
    // 更新数据
    m_Timeout = timeout;
    m_ModelName = model_name;
    m_SignatureName = signature_name;
    m_UseDropoutKeep = use_dropout_keep;
    m_Version = version;
    m_OutputName = output_name;
    m_UseUserType = use_user_type;
}

// 批量预测函数
bool TFModelGrpc::predict_score(
    const std::vector<TFSInputData> &vec_tfs_inputs,
    std::vector<TDPredict::score_type> &vec_tfs_results) const noexcept
{
    // 判断输入为空, 就不需要调用tfs计算了.
    if (vec_tfs_inputs.empty())
    {
        return true;
    }

    const int vec_tfs_inputs_size = vec_tfs_inputs.size();
    if (vec_tfs_inputs_size <= per_batch_size)
    {
        // 获取模型基础数据
        RequestProto request_proto;
        GenerateRequestProto(vec_tfs_inputs, 0, vec_tfs_inputs_size, request_proto);

        ScoreDataPtr spData = std::make_shared<ScoreData>();
        spData->obj = this;
        spData->vec_score = &vec_tfs_results;
        spData->begin = 0;
        spData->end = vec_tfs_inputs_size;
        return client.Send(request_proto, m_Timeout, CallBackScoreFunc, spData);
    }
    else
    {
        // 分批调用
        const int batch_count = (vec_tfs_inputs_size / per_batch_size) + ((vec_tfs_inputs_size % per_batch_size == 0) ? 0 : 1);
        std::vector<RequestProto> vec_request_proto(batch_count);
        std::vector<ScoreDataPtr> vec_response_data(batch_count);
        grpc::CompletionQueue cq;
        for (int batch_idx = 0; batch_idx < batch_count; batch_idx++)
        {
            const int begin = batch_idx * per_batch_size;
            const int end = std::min(vec_tfs_inputs_size, begin + per_batch_size);
            auto &request_proto = vec_request_proto[batch_idx];
            GenerateRequestProto(vec_tfs_inputs, begin, end, request_proto);

            vec_response_data[batch_idx] = std::make_shared<ScoreData>();
            auto &spData = vec_response_data[batch_idx];
            spData->obj = this;
            spData->vec_score = &vec_tfs_results;
            spData->begin = begin;
            spData->end = end;
            client.PipeSend(&cq, request_proto, m_Timeout, CallBackScoreFunc, spData);
        }

        // 接收处理返回结果
        for (int batch_idx = 0; batch_idx < batch_count; batch_idx++)
        {
            client.PipeRecv(&cq);
        }
    }
    return true;
}

// 批量预测函数
bool TFModelGrpc::predict_emb(
    const std::vector<TFSInputData> &vec_tfs_inputs,
    std::vector<TDPredict::emb_any_type> &vec_emb_any) const noexcept
{
    // 判断输入为空, 就不需要调用tfs计算了.
    if (vec_tfs_inputs.empty())
    {
        return true;
    }

    const int vec_tfs_inputs_size = vec_tfs_inputs.size();
    if (vec_tfs_inputs_size <= per_batch_size)
    {
        // 获取模型基础数据
        RequestProto request;
        GenerateRequestProto(vec_tfs_inputs, 0, vec_tfs_inputs_size, request);

        EmbAnyDataPtr spData = std::make_shared<EmbAnyData>();
        spData->obj = this;
        spData->vec_emb_any = &vec_emb_any;
        spData->begin = 0;
        spData->end = vec_tfs_inputs_size;
        return client.Send(request, m_Timeout, CallBackEmbFunc, spData);
    }
    else
    {
        // 分批调用
        const int batch_count = (vec_tfs_inputs_size / per_batch_size) + ((vec_tfs_inputs_size % per_batch_size == 0) ? 0 : 1);
        std::vector<RequestProto> vec_request_proto(batch_count);
        std::vector<EmbAnyDataPtr> vec_response_data(batch_count);
        grpc::CompletionQueue cq;
        for (int batch_idx = 0; batch_idx < batch_count; batch_idx++)
        {
            const int begin = batch_idx * per_batch_size;
            const int end = std::min(vec_tfs_inputs_size, begin + per_batch_size);
            auto &request_proto = vec_request_proto[batch_idx];
            GenerateRequestProto(vec_tfs_inputs, begin, end, request_proto);

            vec_response_data[batch_idx] = std::make_shared<EmbAnyData>();
            auto &spData = vec_response_data[batch_idx];
            spData->obj = this;
            spData->vec_emb_any = &vec_emb_any;
            spData->begin = begin;
            spData->end = end;
            client.PipeSend(&cq, request_proto, m_Timeout, CallBackEmbFunc, spData);
        }

        // 接收处理返回结果
        for (int batch_idx = 0; batch_idx < batch_count; batch_idx++)
        {
            client.PipeRecv(&cq);
        }
    }
    return true;
}

bool TFModelGrpc::GenerateRequestProto(
    const std::vector<TFSInputData> &vec_tfs_input,
    const int begin, const int end,
    RequestProto &request) const noexcept
{
    if (vec_tfs_input.empty())
    {
        return false;
    }

    request.mutable_model_spec()->set_name(m_ModelName);
    request.mutable_model_spec()->set_signature_name(m_SignatureName);
    if (m_Version > 0)
    {
        request.mutable_model_spec()->mutable_version()->set_value(m_Version);
    }

    const int batch_size = end - begin;
    const int input_size = vec_tfs_input[0].index.size();

    InputMap &inputs = *request.mutable_inputs();
    if (m_UseDropoutKeep)
    {
        tensorflow::TensorProto dropout_keep;
        dropout_keep.set_dtype(tensorflow::DataType::DT_FLOAT);
        for (int batch_idx = begin; batch_idx < end; batch_idx++)
        {
            dropout_keep.add_float_val(1);
        }
        dropout_keep.mutable_tensor_shape()->add_dim()->set_size(batch_size);
        dropout_keep.mutable_tensor_shape()->add_dim()->set_size(1);
        inputs["dropout_keep"] = dropout_keep;
    }

    if (m_UseUserType)
    {
        tensorflow::TensorProto user_type;
        user_type.set_dtype(tensorflow::DataType::DT_INT32);
        for (int batch_idx = begin; batch_idx < end; batch_idx++)
        {
            user_type.add_int_val(vec_tfs_input[batch_idx].user_type);
        }
        user_type.mutable_tensor_shape()->add_dim()->set_size(batch_size);
        user_type.mutable_tensor_shape()->add_dim()->set_size(1);
        inputs["user_type"] = user_type;
    }

    // input_index
    {
        tensorflow::TensorProto input_index;
        input_index.set_dtype(tensorflow::DataType::DT_INT32);
        for (int batch_idx = begin; batch_idx < end; batch_idx++)
        {
            const auto &index = vec_tfs_input[batch_idx].index;
            const int index_size = index.size();
            if (index_size != input_size)
            {
                return false;
            }
            for (int idx = 0; idx < index_size; idx++)
            {
                input_index.add_int_val(index[idx]);
            }
        }
        input_index.mutable_tensor_shape()->add_dim()->set_size(batch_size);
        input_index.mutable_tensor_shape()->add_dim()->set_size(input_size);
        inputs["input_index"] = input_index;
    }

    // input_value
    {
        tensorflow::TensorProto input_value;
        input_value.set_dtype(tensorflow::DataType::DT_FLOAT);
        for (int batch_idx = begin; batch_idx < end; batch_idx++)
        {
            const auto &value = vec_tfs_input[batch_idx].value;
            const int value_size = value.size();
            if (value_size != input_size)
            {
                return false;
            }
            for (int idx = 0; idx < value_size; idx++)
            {
                input_value.add_float_val(value[idx]);
            }
        }
        input_value.mutable_tensor_shape()->add_dim()->set_size(batch_size);
        input_value.mutable_tensor_shape()->add_dim()->set_size(input_size);
        inputs["input_value"] = input_value;
    }
    return true;
}
