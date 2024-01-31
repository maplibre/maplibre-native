#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/util/util.hpp>

#include <QObject>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace QMapLibre {

class Scheduler : public QObject, public mbgl::Scheduler {
    Q_OBJECT

public:
    Scheduler();
    ~Scheduler() override;

    // mbgl::Scheduler implementation.
    void schedule(std::function<void()>&& function) final;

    std::size_t waitForEmpty(std::chrono::milliseconds timeout) override;

    mapbox::base::WeakPtr<mbgl::Scheduler> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

    void processEvents();

signals:
    void needsProcessing();

private:
    MBGL_STORE_THREAD(tid);

    std::mutex m_taskQueueMutex;
    std::condition_variable cvEmpty;
    std::atomic<std::size_t> pendingItems;
    std::queue<std::function<void()>> m_taskQueue;
    mapbox::base::WeakPtrFactory<Scheduler> weakFactory{this};
};

} // namespace QMapLibre
