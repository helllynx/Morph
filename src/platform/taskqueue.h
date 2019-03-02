#pragma once

#include <functional>

namespace platform
{

class TaskQueue
{
  public:
    using Task = std::function<void()>;

    TaskQueue();
    ~TaskQueue();

    void post(Task task);

  private:
    struct Impl;
    Impl* impl;
};

} // namespace platform
