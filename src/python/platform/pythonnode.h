#pragma once

#include "platform/node.h"
#include "platform/nodefactory.h"

#include <boost/python.hpp>

namespace python
{

class PythonNode : public platform::Node
{
  public:
    PythonNode(const boost::python::object& py_object);

    virtual platform::NodeState initial() const override;

    virtual platform::KnobCollection compute(
        platform::KnobCollection    input_knobs,
        platform::KnobRefCollection input_knob_refs) const override;

  private:
    boost::python::object m_py_object;
};

} // namespace python
