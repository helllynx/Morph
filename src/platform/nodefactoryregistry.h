#pragma once

#include <map>
#include <string>

namespace platform { class NodeFactory; }

namespace platform
{

class NodeFactoryRegistry
{
  public:
    void               register_node_factory(NodeFactory* factory);
    const NodeFactory* get_node_factory(const std::string& model) const;

  private:
    std::map<std::string, NodeFactory*> m_factory_collection;
};

} // namespace platform
