#include "nodefactoryregistry.h"

#include "nodefactory.h"

namespace platform
{

void NodeFactoryRegistry::register_node_factory(NodeFactory* factory)
{
    auto& model = factory->model();

    auto itr = m_factory_collection.find(model);
    if (itr != m_factory_collection.end())
    {
        // TODO: use better exception type.
        throw 0;
    }

    m_factory_collection.emplace(model, factory);
}

const NodeFactory* NodeFactoryRegistry::get_node_factory(
    const std::string& model) const
{
    auto itr = m_factory_collection.find(model);
    if (itr == m_factory_collection.end())
    {
        // TODO: use better exception type.
        throw 0;
    }

    return itr->second;
}

} // namespace platform
