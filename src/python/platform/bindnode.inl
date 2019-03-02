#include "platform/node.h"
#include "platform/nodefactory.h"

#include <boost/python.hpp>

using namespace platform;
using namespace boost::python;

namespace
{

struct NodeWrapper : Node, wrapper<Node>
{
    virtual NodeState initial() const override
    {
        return this->get_override("initial")();
    }

    virtual KnobCollection compute(
        KnobCollection    input_knobs,
        KnobRefCollection input_knob_refs) const override
    {
        return this->get_override("compute")(std::move(input_knobs),
                                             std::move(input_knob_refs));
    }
};

} // namespace

static void bind_node()
{
    class_<NodeWrapper, boost::noncopyable>("Node")
        .def("initial", pure_virtual(&Node::initial))
        .def("compute", pure_virtual(&Node::compute));
}
