#include "taskqueue.h"

#include <tbb/concurrent_queue.h>
#include <tbb/spin_mutex.h>
#include <tbb/task.h>

#include <functional>

namespace platform
{

struct TaskQueue::Impl
{
    Impl()
        : m_is_busy(false)
    {}

    TaskQueue::Task on_task_end();

    tbb::concurrent_queue<Task> m_queue;
    tbb::task_group_context     m_ctx;
    tbb::spin_mutex             m_mutex;
    bool                        m_is_busy;
};

TaskQueue::Task TaskQueue::Impl::on_task_end()
{
    tbb::spin_mutex::scoped_lock lock(m_mutex);

    if (m_ctx.is_group_execution_cancelled())
    {
        return {};
    }

    TaskQueue::Task t;
    if (m_queue.try_pop(t))
    {
        return t;
    }
    else
    {
        m_is_busy = false;
    }

    return {};
}

namespace
{

class TaskWrapper : public tbb::task
{
  public:
    using OnTaskEnd = std::function<TaskQueue::Task(void)>;

    TaskWrapper(TaskQueue::Task&& t, OnTaskEnd&& cb)
        : m_t(std::move(t))
        , m_cb(std::move(cb))
    {}

    virtual tbb::task* execute() override;

  private:
    TaskQueue::Task m_t;
    OnTaskEnd       m_cb;
};

tbb::task* TaskWrapper::execute()
{
    if (m_t)
    {
        m_t();
    }

    if (!is_cancelled() && m_cb)
    {
        m_t = m_cb();

        if (m_t)
        {
            increment_ref_count();
            recycle_as_safe_continuation();
        }
    }

    return nullptr;
}

} // namespace

TaskQueue::TaskQueue()
    : impl(new Impl())
{}

TaskQueue::~TaskQueue()
{
    impl->m_ctx.cancel_group_execution();
    delete impl;
}

void TaskQueue::post(Task task)
{
    tbb::spin_mutex::scoped_lock lock(impl->m_mutex);

    if (!impl->m_is_busy)
    {
        TaskWrapper* w = new (tbb::task::allocate_root(impl->m_ctx))
            TaskWrapper(std::move(task), [=]() { return impl->on_task_end(); });

        tbb::task::spawn(*w);
        impl->m_is_busy = true;
        
        return;
    }

    impl->m_queue.push(std::move(task));
}

} // namespace platform
