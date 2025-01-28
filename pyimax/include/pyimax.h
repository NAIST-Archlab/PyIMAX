#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "imax_utils.h"

#define PYBIND11_DETAILED_ERROR_MESSAGE
namespace py = pybind11;

class IMAXArray {
public:
    std::uintptr_t device_ptr;       // シミュレーション上のデバイスメモリアドレス
    std::vector<ssize_t> shape;      // 配列の形状
    size_t size;                     // 要素数
    size_t nbytes;                   // 総バイトサイズ
    std::string dtype;               // データ型 ("float32" など)
    py::array host_data;             // シミュレーション用にホスト側（NumPy 配列）のデータを保持
    bool is_row_major;               // 行優先かどうか

    // クラス変数としてグローバルな次回割り当てアドレスを管理
    static std::uintptr_t global_memory_addr;

    /// コンストラクタ
    IMAXArray(const std::vector<ssize_t>& shape, const std::string & dtype)
      : shape(shape), dtype(dtype)
    {
        // shape から要素数を計算
        size = 1;
        for (auto s: shape) {
            size *= s;
        }

        // dtype に応じたバイトサイズの算出 (ここでは float32, int32 のみ対応)
        if (this->dtype == "float32") {
            nbytes = size * sizeof(float);
        }
        else if (this->dtype == "int32") {
            nbytes = size * sizeof(int);
        }
        else {
            throw std::runtime_error("Unsupported dtype: " + this->dtype);
        }

        // グローバルアドレスから割り当て (シミュレーション)
        memory_align(56*2);
        device_ptr = global_memory_addr;
        xmax_bzero((Uint *)device_ptr, imax_nbytes);
        global_memory_addr += imax_nbytes;
        is_row_major = true;

        // host_data は初期状態では空 (py::array() により初期化)
        host_data = py::array();
    }


    /// 静的メソッド: NumPy 配列から IMAXArray インスタンスを作成（シミュレーション上のデバイス転送）
    static IMAXArray from_numpy(py::array array) {
        py::buffer_info info = array.request();

        // dtype の判定 (ここでは float32, int32 のみ対応)
        std::string dtype_str;
        if(info.format == py::format_descriptor<float>::format())
            dtype_str = "float32";
        else if(info.format == py::format_descriptor<int>::format())
            dtype_str = "int32";
        else {
            throw std::runtime_error("Unsupported array data type");
        }

        // shape の vector 生成
        std::vector<ssize_t> shape_vec(info.shape.begin(), info.shape.end());

        // 新規 IMAXArray インスタンス生成
        IMAXArray imax(shape_vec, dtype_str);

        // シミュレーション：ホスト側のデータコピーを保持
        imax.host_data = array.attr("copy")();

        py::buffer_info info_imax = imax.host_data.request();

        // imaxサイズとnumpyサイズが違うので（メモリアラインメントのため）imax_sizeに合わせて値をコピー
        for (size_t i = 0; i < imax.shape[0]; i++) {
            if (shape_vec.size() == 1) {
                float *p = (float *)imax.device_ptr;
                p[i] = ((float*)info_imax.ptr)[i];
            } else {
                for (size_t j = 0; j < shape_vec[1]; j++) {
                    float *p = (float *)imax.device_ptr;
                    p[(i * imax.imax_shape[1]) + j] = ((float* )info_imax.ptr)[(i * imax.shape[1]) + j];
                }
            }
        }

        return imax;
    }

    /// スタティックメソッド: IMAXArray から NumPy 配列に転送 (ホスト側のデータコピーを返す)
    static py::array to_numpy(const IMAXArray &imax) {
        py::dtype dtype(imax.dtype);
        py::array new_data(dtype, imax.shape);
        
        py::buffer_info info = new_data.request();
        std::memcpy(info.ptr, (void *) imax.device_ptr, imax.nbytes);

        return new_data;
    }

    /// インスタンスメソッド: 自身のデータを NumPy 配列に転送
    py::array imax_to_numpy() const;

    /// インスタンスメソッド: NumPy 配列から自身のデータを更新
    void numpy_to_imax(py::array array);

    std::string repr() const;

    IMAXArray mv(const IMAXArray &vector);

    void change_major() const;
private:
    std::vector<ssize_t> imax_shape;      // 配列の形状
    size_t imax_size;
    size_t imax_nbytes;

    void memory_align(int alignment) {
        imax_shape = shape;

        for (auto &s: imax_shape) {
            ssize_t remainder = s % alignment;
            if (remainder != 0) {
                ssize_t padding = alignment - remainder;
                s += padding;
            }
        }

        imax_size = 1;
        for (auto s: imax_shape) {
            imax_size *= s;
        }

        imax_nbytes = imax_size * sizeof(float);
    }
};

void init_imax(pybind11::module &m);
void init_imax_array(pybind11::module &m);
