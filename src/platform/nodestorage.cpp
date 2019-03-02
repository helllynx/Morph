#include "nodestorage.h"

#include "node.h"
#include "nodefactory.h"
#include "nodefactoryregistry.h"
#include "taskqueue.h"

#include <vector>

namespace
{

using namespace platform;

class Reducer
{
  public:
    Reducer(NodeFactoryRegistry* registry)
        : m_node_factory_registry(registry)
    {}

    NodeCollection reduce(NodeCollection state, NodeStorageAction&& action)
    {
        // TODO: handle exceptions.
        return std::
            visit([&](auto&&
                          a) { return reduce(std::move(state), std::move(a)); },
                  std::move(action));
    }

    NodeCollection reduce(NodeCollection&& state, CreateNode&& action)
    {
        auto  factory = m_node_factory_registry->get_node_factory(action.model);
        Node* n       = factory->create();
        auto  node_state = n->initial();

        auto next_state = state.set(action.id,
                                    NodeContext {action.id,
                                                 std::move(node_state),
                                                 n});

        return next_state;
    }

    NodeCollection reduce(NodeCollection&& state, RemoveNode&& action)
    {
        return state;
    }

  private:
    NodeFactoryRegistry* m_node_factory_registry;
};

} // namespace

namespace platform
{

struct NodeStorage::Impl
{
    Impl(NodeFactoryRegistry* registry)
        : m_reducer(registry)
    {}

    TaskQueue             m_action_queue;
    NodeCollection        m_state;
    Reducer               m_reducer;
    std::vector<OnNextFn> m_subscribers;
};

NodeStorage::NodeStorage(NodeFactoryRegistry* registry)
    : impl(new Impl(registry))
{}

NodeStorage::~NodeStorage()
{
    delete impl;
}

void NodeStorage::dispatch(NodeStorageAction action)
{
    impl->m_action_queue.post([=]() mutable 
    {
        impl->m_state = impl->m_reducer.reduce(std::move(impl->m_state),
                                               std::move(action));
        for (auto& cb : impl->m_subscribers)
        {
            cb(impl->m_state);
        }
    });
}

NodeCollection NodeStorage::state() const
{
    // TODO: synchronization needed! (atomic root ptr in the map?)
    return impl->m_state;
}

void NodeStorage::subscribe(OnNextFn on_next)
{
    impl->m_subscribers.push_back(std::move(on_next));
}

} // namespace platform
