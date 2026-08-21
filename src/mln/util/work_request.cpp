#include <mln/util/work_request.hpp>
#include <mln/util/work_task.hpp>

#include <cassert>
#include <utility>

namespace mln {

WorkRequest::WorkRequest(Task task_)
    : task(std::move(task_)) {
    assert(task);
}

WorkRequest::~WorkRequest() {
    task->cancel();
}

} // namespace mln
