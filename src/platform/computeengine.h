#pragma once

#include "enabledispatch.h"
#include "nodestorage.h"
#include "nodestorageactions.h"

namespace platform { struct NodeContext; }

namespace platform
{

//
//  ComputeEngine is responsible for updating Node
//  output attributes on demand.
//
class ComputeEngine : private EnableDispatch
{
  public:
    ComputeEngine(const EnableDispatch& enable_dispatch);

    void on_update(NodeCollection node_storage_state) const;

    //
    // Spawns compute task for this node context and for each
    // dependency in the graph.
    //
    void compute(NodeContext ctx, NodeCollection node_storage_state) const;
};

} // namespace platform