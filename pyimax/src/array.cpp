#include "../include/pyimax.h"
#include "../include/imax_mv.h"

namespace py = pybind11;

/// インスタンスメソッド: 自身のデータを NumPy 配列に転送
py::array IMAXArray::imax_to_numpy() const {
    return IMAXArray::to_numpy(*this);
}

/// インスタンスメソッド: NumPy 配列から自身のデータを更新
void IMAXArray::numpy_to_imax(py::array array) {
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

std::string IMAXArray::repr() const {
    std::stringstream ss;
    ss << "<IMAXArray device_ptr=0x" << std::hex << device_ptr << ", shape=[";
    std::string repr_str = ss.str();
    for (size_t i = 0; i < shape.size(); ++i) {
        repr_str += std::to_string(shape[i]);
        if (i < shape.size()-1)
            repr_str += ", ";
    }
    repr_str += "], dtype=" + dtype + ">";
    return repr_str;
}

IMAXArray IMAXArray::mv(const IMAXArray &vector) {
    IMAXArray result({shape[0]}, dtype);
    std::cout << "IMAXArray::mv: shape[0]=" << shape[0] << ", shape[1]=" << shape[1] << std::endl;
    std::cout << "IMAXArray::mv imax_shape[0]=" << imax_shape[0] << ", imax_shape[1]=" << imax_shape[1] << std::endl;
    if (this->is_row_major) {
        this->change_major();
    }
    imax_mv((unsigned char*)result.device_ptr, (unsigned char*)device_ptr, (unsigned char*)vector.device_ptr, imax_shape[1], imax_shape[0], 0);
    return result;
}

void IMAXArray::change_major() {
    if (imax_shape.size() == 1) {
        return;
    }
    std::cout << "imax_size=" << imax_size << ", imax_nbytes=" << imax_nbytes << std::endl;
    float* tmp = new float[200000];
    std::cout << "imax_size=" << imax_size << ", imax_nbytes=" << imax_nbytes << std::endl;
    std::memcpy(tmp, (void *)device_ptr, imax_nbytes);
    std::cout << "imax_size=" << imax_size << ", imax_nbytes=" << imax_nbytes << std::endl;
    float *p = (float *)device_ptr;
    for (ssize_t i = 0; i < imax_shape[0]; ++i) {
        for (ssize_t j = 0; j < imax_shape[1]; ++j) {
            p[(j * imax_shape[0]) + i] = tmp[(i * imax_shape[1]) + j];
        }
    }
    delete tmp;
    is_row_major = !is_row_major;
}

// 初期グローバルメモリアドレスの設定
#ifdef ARMZYNQ
std::uintptr_t IMAXArray::global_memory_addr = (std::uintptr_t) 0x0000050000000000LL;
#else
std::uintptr_t IMAXArray::global_memory_addr = (std::uintptr_t)(new char[200000]);
#endif

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
        .def("__repr__", &IMAXArray::repr)
        .def("mv", &IMAXArray::mv);
}