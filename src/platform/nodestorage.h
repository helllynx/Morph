#pragma once

#include "nodestorageactions.h"

#include "base/immutable/map.h"

#include <functional>


namespace platform { class Node; }
namespace platform { class NodeContext; }
namespace platform { class NodeFactoryRegistry; }

namespace platform
{

using NodeCollection = immutable::Map<NodeId, NodeContext>;

class NodeStorage
{
  public:
    using OnNextFn = std::function<void(NodeCollection)>;

    NodeStorage(NodeFactoryRegistry* registry);
    ~NodeStorage();

    void           dispatch(NodeStorageAction action);
    NodeCollection state() const;
    void           subscribe(OnNextFn on_next);

  private:
    struct Impl;
    Impl* impl;
};

} // namespace platform
