#include "platform/nodefactory.h"
#include "platform/nodefactoryregistry.h"

#include <boost/python.hpp>

using namespace platform;
using namespace boost::python;

namespace
{

class PythonNodeFactory : public NodeFactory
{
  public:
    virtual Node* create() const override
    {
        return nullptr;
    }

    virtual const std::string& model() const override
    {
        static auto result = "abababa";
        return result;
    }
};

} // namespace

static void bind_nodefactoryregistry()
{
    class_<NodeWrapper, boost::noncopyable>("Node")
        .def("initial", pure_virtual(&Node::initial))
        .def("compute", pure_virtual(&Node::compute));
}
