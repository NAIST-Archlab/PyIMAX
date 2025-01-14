#include "../include/pyimax.h"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

unsigned char* allocate(size_t size) {
    return new unsigned char[size];
}

void deallocate(unsigned char* ptr) {
    delete[] ptr;
}

template <typename T>
unsigned char* to_imax(py::array_t<T> x) {
    const auto &buff_info = x.request();
    size_t size = buff_info.size * sizeof(T);
    unsigned char* ptr = allocate(size);

    std::memcpy(ptr, buff_info.ptr, size);
    return ptr;
}

void init_imax(pybind11::module &m) {
    pybind11::module imax = m.def_submodule("imax");

    imax.def("allocate", &allocate, "allocate memory");
    imax.def("deallocate", &deallocate, "deallocate memory");
    imax.def("to_imax", &to_imax<int>, "convert numpy array to imax");
}