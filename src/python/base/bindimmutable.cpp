#include "base/immutable/map.h"

#include <boost/python.hpp>

namespace
{

struct BindHash
{
    std::size_t operator()(const boost::python::object& value)
    {
        return PyObject_Hash(value.ptr());
    }
};

}

BOOST_PYTHON_MODULE(_morph)
{

using Map = immutable::Map<
    boost::python::object,
    boost::python::object,
    BindHash>;

boost::python::class_<Map>("Map")
    .def("__setitem__", &Map::set)
    .def("__getitem__", &Map::get,
         boost::python::return_value_policy<copy_const_reference>())
    .def("__delitem__", &Map::erase);

}
