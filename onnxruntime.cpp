// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/**
 * This sample application demonstrates how to use components of the experimental C++ API
 * to query for model inputs/outputs and how to run inferrence on a model.
 *
 * This example is best run with one of the ResNet models (i.e. ResNet18) from the onnx model zoo at
 *   https://github.com/onnx/models
 *
 * Assumptions made in this example:
 *  1) The onnx model has 1 input node and 1 output node
 *  2) The onnx model should have float input
 *
 *
 * In this example, we do the following:
 *  1) read in an onnx model
 *  2) print out some metadata information about inputs and outputs that the model expects
 *  3) generate random data for an input tensor
 *  4) pass tensor through the model and check the resulting tensor
 *
 */

#include <algorithm>  // std::generate
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <onnxruntime_float16.h>
#include <onnxruntime_cxx_api.h>

typedef unsigned short ushort;
typedef unsigned int uint;

uint as_uint(const float x) {
    return *(uint*)&x;
}
float as_float(const uint x) {
    return *(float*)&x;
}

float half_to_float(const ushort x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
    const uint e = (x&0x7C00)>>10; // exponent
    const uint m = (x&0x03FF)<<13; // mantissa
    const uint v = as_uint((float)m)>>23; // evil log2 bit hack to count leading zeros in denormalized format
    return as_float((x&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000))); // sign : normalized : denormalized
}
ushort float_to_half(const float x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
    const uint b = as_uint(x)+0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const uint e = (b&0x7F800000)>>23; // exponent
    const uint m = b&0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    return (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
}

 // pretty prints a shape dimension vector
std::string print_shape(const std::vector<std::int64_t>& v) {
    std::stringstream ss("");
    for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
    ss << v[v.size() - 1];
    return ss.str();
}

int calculate_product(const std::vector<std::int64_t>& v) {
    int total = 1;
    for (auto& i : v) total *= i;
    return total;
}

template <typename T>
Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
    Ort::MemoryInfo mem_info =
        Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
    return tensor;
}

#ifdef _WIN32
int wmain(int argc, ORTCHAR_T* argv[]) {
#else
int main(int argc, ORTCHAR_T * argv[]) {
#endif
   /* if (argc != 2) {
        std::cout << "Usage: ./onnx-api-example <onnx_model.onnx>" << std::endl;
        return -1;
    }*/

    std::basic_string<ORTCHAR_T> model_file = L"c:/Users/admin/Documents/GitHub/ClashRoyaleBuildABot/clashroyalebuildabot/models/units_M_480x352.onnx";

    // onnxruntime setup
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
    Ort::SessionOptions session_options;
    try {
        Ort::Session session = Ort::Session(env, model_file.c_str(), session_options);

        // print name/shape of inputs
        Ort::AllocatorWithDefaultOptions allocator;
        std::vector<std::string> input_names;
        std::vector<std::int64_t> input_shapes;
        std::cout << "Input Node Name/Shape (" << input_names.size() << "):" << std::endl;
        for (std::size_t i = 0; i < session.GetInputCount(); i++) {
            input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
            input_shapes = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
            std::cout << "\t" << input_names.at(i) << " : " << print_shape(input_shapes) << std::endl;
        }
        // some models might have negative shape values to indicate dynamic shape, e.g., for variable batch size.
        for (auto& s : input_shapes) {
            if (s < 0) {
                s = 1;
            }
        }

        // print name/shape of outputs
        std::vector<std::string> output_names;
        std::cout << "Output Node Name/Shape (" << output_names.size() << "):" << std::endl;
        for (std::size_t i = 0; i < session.GetOutputCount(); i++) {
            output_names.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
            auto output_shapes = session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
            std::cout << "\t" << output_names.at(i) << " : " << print_shape(output_shapes) << std::endl;
        }

        // Assume model has 1 input node and 1 output node.
        assert(input_names.size() == 1 && output_names.size() == 1);

        // Create a single Ort tensor of random numbers
        auto input_shape = input_shapes;
        auto total_number_elements = calculate_product(input_shape);

        // generate random numbers in the range [0, 255]
        onnxruntime_float16::Float16Impl::t(1);
        std::vector<> input_tensor_values(total_number_elements);
        std::generate(input_tensor_values.begin(), input_tensor_values.end(), [&] { return rand() % 255; });
        std::vector<Ort::Value> input_tensors;
        input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values, input_shape));

        // double-check the dimensions of the input tensor
        assert(input_tensors[0].IsTensor() && input_tensors[0].GetTensorTypeAndShapeInfo().GetShape() == input_shape);
        std::cout << "\ninput_tensor shape: " << print_shape(input_tensors[0].GetTensorTypeAndShapeInfo().GetShape()) << std::endl;

        // pass data through model
        std::vector<const char*> input_names_char(input_names.size(), nullptr);
        std::transform(std::begin(input_names), std::end(input_names), std::begin(input_names_char),
            [&](const std::string& str) { return str.c_str(); });

        std::vector<const char*> output_names_char(output_names.size(), nullptr);
        std::transform(std::begin(output_names), std::end(output_names), std::begin(output_names_char),
            [&](const std::string& str) { return str.c_str(); });

        std::cout << "Running model..." << std::endl;
        try {
            auto output_tensors = session.Run(Ort::RunOptions{ nullptr }, input_names_char.data(), input_tensors.data(),
                input_names_char.size(), output_names_char.data(), output_names_char.size());
            std::cout << "Done!" << std::endl;

            // double-check the dimensions of the output tensors
            // NOTE: the number of output tensors is equal to the number of output nodes specifed in the Run() call
            assert(output_tensors.size() == output_names.size() && output_tensors[0].IsTensor());
        }
        catch (const Ort::Exception& exception) {
            std::cout << "ERROR running model inference: " << exception.what() << std::endl;
            exit(-1);
        }
    }
    catch (std::exception e) {
        std::cout << e.what();
    }
}