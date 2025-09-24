
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <cstdint> // uint8_t


py::array_t<uint8_t> bitwise_and(py::array_t<uint8_t> voids_1, py::array_t<uint8_t> voids_2) {
    if (voids_1.dtype().itemsize() != voids_2.dtype().itemsize()) {
        throw std::runtime_error("Input arrays must have the same dtype itemsize.");
    }

    auto buf1 = voids_1.request();
    auto buf2 = voids_2.request();

    if (buf1.size != buf2.size) {
        throw std::runtime_error("Input arrays must have the same size.");
    }

    size_t n = buf1.size;
    size_t itemsize = buf1.itemsize;

    py::array_t<uint8_t> voids_3(voids_1.dtype(), voids_1.shape());
    auto buf3 = voids_3.request();

    uint8_t* ptr1 = static_cast<uint8_t*>(buf1.ptr);
    uint8_t* ptr2 = static_cast<uint8_t*>(buf2.ptr);
    uint8_t* ptr3 = static_cast<uint8_t*>(buf3.ptr);

    for (size_t i = 0; i < n * itemsize; ++i) {
        ptr3[i] = ptr1[i] & ptr2[i];
    }

    return voids_3;
}