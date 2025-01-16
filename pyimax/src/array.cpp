#include "../include/pyimax.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace py = pybind11;

static const std::uintptr_t IMAX_GLOBAL_MEMORY_ADDR = 0x10000000;

class IMAXArray {
public:
    std::uintptr_t device_ptr;       // シミュレーション上のデバイスメモリアドレス
    std::vector<ssize_t> shape;      // 配列の形状
    size_t size;                     // 要素数
    size_t nbytes;                   // 総バイトサイズ
    std::string dtype;               // データ型 ("float32" など)
    py::array host_data;             // シミュレーション用にホスト側（NumPy 配列）のデータを保持

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
        device_ptr = global_memory_addr;
        global_memory_addr += nbytes;

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
        return imax;
    }

    /// スタティックメソッド: IMAXArray から NumPy 配列に転送 (ホスト側のデータコピーを返す)
    static py::array to_numpy(const IMAXArray &imax) {
        if (imax.host_data) {
            return imax.host_data.attr("copy")();
        } else {
            return py::array(py::dtype(imax.dtype), imax.shape);
        }
    }

    /// インスタンスメソッド: 自身のデータを NumPy 配列に転送
    py::array imax_to_numpy() const {
        return IMAXArray::to_numpy(*this);
    }

    /// インスタンスメソッド: NumPy 配列から自身のデータを更新
    void numpy_to_imax(py::array array) {
        py::buffer_info info = array.request();
        std::vector<ssize_t> new_shape(info.shape.begin(), info.shape.end());

        // dtype の判定
        std::string new_dtype;
        if(info.format == py::format_descriptor<float>::format())
            new_dtype = "float32";
        else if(info.format == py::format_descriptor<int>::format())
            new_dtype = "int32";
        else {
            throw std::runtime_error("Unsupported array data type");
        }

        // shape と dtype のチェック
        if(new_shape != shape || new_dtype != dtype) {
            throw std::runtime_error("Shape or dtype mismatch in numpy_to_imax");
        }

        // 更新：ホスト側データのコピー
        host_data = array.attr("copy")();
    }

    std::string repr() const {
        std::string repr_str = "<IMAXArray device_ptr=0x" + 
                               std::to_string(device_ptr) + ", shape=[";
        for (size_t i = 0; i < shape.size(); ++i) {
            repr_str += std::to_string(shape[i]);
            if (i < shape.size()-1)
                repr_str += ", ";
        }
        repr_str += "], dtype=" + dtype + ">";
        return repr_str;
    }
};

// 初期グローバルメモリアドレスの設定
// std::uintptr_t IMAXArray::global_memory_addr = IMAX_GLOBAL_BASE_ADDR;
std::uintptr_t IMAXArray::global_memory_addr = (std::uintptr_t)(new char[100000]);

void init_imax_array(pybind11::module &m) {
    py::class_<IMAXArray>(m, "IMAXArray")
        .def(py::init<const std::vector<ssize_t>&, const std::string &>(),
             py::arg("shape"), py::arg("dtype"))
        .def_readonly("device_ptr", &IMAXArray::device_ptr)
        .def_property_readonly("shape", [](const IMAXArray &a) { return py::cast(a.shape); })
        .def_readonly("size", &IMAXArray::size)
        .def_readonly("nbytes", &IMAXArray::nbytes)
        .def_readonly("dtype", &IMAXArray::dtype)
        // 静的メソッドとして from_numpy を登録
        .def_static("from_numpy", &IMAXArray::from_numpy,
                    "Convert a NumPy array to an IMAXArray (device simulation)",
                    py::arg("array"))
        // スタティックメソッドとして to_numpy を登録
        .def_static("to_numpy", &IMAXArray::to_numpy,
                    "Convert an IMAXArray to a NumPy array (host simulation)",
                    py::arg("imax"))
        // インスタンスメソッドによる転送と更新
        .def("imax_to_numpy", &IMAXArray::imax_to_numpy,
             "Convert this IMAXArray to a NumPy array")
        .def("numpy_to_imax", &IMAXArray::numpy_to_imax,
             "Update this IMAXArray from a NumPy array",
             py::arg("array"))
        .def("__repr__", &IMAXArray::repr);
}