#pragma once

#include <mln/util/async_request.hpp>

#include <memory>

namespace mln {

class WorkTask;

class WorkRequest : public AsyncRequest {
public:
    using Task = std::shared_ptr<WorkTask>;
    WorkRequest(Task);
    ~WorkRequest() override;

private:
    std::shared_ptr<WorkTask> task;
};

} // namespace mln
