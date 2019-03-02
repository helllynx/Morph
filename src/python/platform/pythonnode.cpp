#include "pythonnode.h"

using namespace platform;
using namespace boost::python;

namespace python
{

PythonNode::PythonNode(const object& py_object)
    : m_py_object(py_object)
{}

NodeState PythonNode::initial() const
{
    return m_py_object.attr("initial")();
}

KnobCollection compute(KnobCollection    input_knobs,
                       KnobRefCollection input_knob_refs) const
{
    return m_py_object.attr("compute")(input_knobs, input_knob_refs);
}

} // namespace python
