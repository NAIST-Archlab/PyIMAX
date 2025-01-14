#include "../include/pyimax.h"

PYBIND11_MODULE(pyimax, m) {
    m.doc() = "pyimax module";

    init_imax(m);
}
