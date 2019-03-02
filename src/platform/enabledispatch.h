#pragma once

#include "nodestorage.h"
#include "nodestorageactions.h"

#include <functional>
#include <utility>

namespace platform
{

class EnableDispatch
{
  public:
    using DispatchFn = std::function<void(NodeStorageAction)>;
    using StateFn    = std::function<NodeCollection()>;

    EnableDispatch(const DispatchFn& dispatch, const StateFn& state)
        : m_dispatch(dispatch)
        , m_state(state)
    {}

    EnableDispatch(const EnableDispatch& other)
        : m_dispatch(other.m_dispatch)
        , m_state(other.m_state)
    {}

  protected:
    void dispatch(NodeStorageAction action) const
    {
        if (m_dispatch)
        {
            m_dispatch(std::move(action));
        }
    }

    NodeCollection state() const
    {
        if (m_state)
        {
            return m_state();
        }

        return NodeCollection();
    }

  private:
    DispatchFn m_dispatch;
    StateFn    m_state;
};

} // namespace platform
