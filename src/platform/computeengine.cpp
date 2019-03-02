#include "computeengine.h"

#include "knob.h"
#include "node.h"

#include <tbb/task.h>

#include <stack>

namespace
{

using namespace platform;

class ComputeTask : public tbb::task, private EnableDispatch
{
  public:
    ComputeTask(NodeContext n_ctx, const EnableDispatch& enable_dispatch)
        : EnableDispatch(enable_dispatch)
        , m_node_ctx(std::move(n_ctx))
    {}

    virtual tbb::task* execute() override
    {
        auto output_knobs = m_node_ctx.node
                                ->compute(m_node_ctx.state.input_knobs,
                                          m_node_ctx.state.input_knob_refs);

        UpdateNodeOutput action {m_node_ctx.id, output_knobs};
        dispatch(std::move(action));
        return nullptr;
    }

    const NodeContext& node_context() const
    {
        return m_node_ctx;
    }

  private:
    NodeContext m_node_ctx;
};

} // namespace

namespace platform
{

ComputeEngine::ComputeEngine(const EnableDispatch& enable_dispatch)
    : EnableDispatch(enable_dispatch)
{}

void ComputeEngine::on_update(NodeCollection node_storage_state) const
{
    
}

void ComputeEngine::compute(NodeContext    ctx,
                            NodeCollection node_storage_state) const
{
    std::stack<ComputeTask*> task_stack;

    auto root_task = new (tbb::task::allocate_root())
        ComputeTask(std::move(ctx), *this);
    task_stack.push(root_task);

    tbb::task_list task_list;

    while (true)
    {
        if (task_stack.empty())
        {
            break;
        }

        ComputeTask* t = task_stack.top();
        task_stack.pop();

        for (auto ref : t->node_context().state.input_knob_refs)
        {
            auto node_id   = ref.second.node_id;
            auto knob_name = ref.second.knob_name;

            auto n_ctx      = node_storage_state.get(node_id);
            auto child_task = new (t->allocate_child())
                ComputeTask(n_ctx, *this);
            task_stack.push(child_task);
        }

        task_list.push_back(*t);
    }

    tbb::task::spawn(task_list);
}

} // namespace platform
