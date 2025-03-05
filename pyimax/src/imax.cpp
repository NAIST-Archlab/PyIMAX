#include "../include/pyimax.h"
#include "../include/imax_utils.h"

namespace py = pybind11;

void cpp_sysinit(size_t nlane) {
    py::print("sysinit: nlane = ", nlane);
    // std::uintptr_t IMAXArray::global_memory_addr = (std::uintptr_t)(new char[100000]);
    IMAXArray::global_memory_addr = (std::uintptr_t) sysinit(20000000, 32, nlane);
}

void cpp_execute(const std::string& kernel_name, py::kwargs kwargs) {
    py::print("execute: kernel_name = ", kernel_name);
    for (auto item : kwargs) {
        py::print(item.first, " = ", item.second);
    }
}

void init_imax(pybind11::module &m) {
    m.def("sysinit", &cpp_sysinit, "Initialize the system with the specified number of lanes", py::arg("nlane"));
    m.def("execute", &cpp_execute, "Execute the specified kernel", py::arg("kernel_name"));
}