#include <benchmark/benchmark.h>

#include <mln/actor/actor.hpp>
#include <mln/actor/scheduler.hpp>
#include <mln/util/thread.hpp>

#include <cstddef>
#include <memory>
#include <thread>
#include <vector>

namespace {

constexpr std::size_t messagesPerSender = 10000;

class Counter {
public:
    Counter(mln::ActorRef<Counter>) {}

    void increment() { ++count; }
    std::size_t get() const { return count; }

private:
    std::size_t count = 0;
};

// Sends `messagesPerSender` messages from each of `senders` threads, the calling thread being one of them, sender `i`
// to `counters[i % counters.size()]`. Then waits until all of them are received.
void sendMessages(const std::vector<mln::ActorRef<Counter>>& counters, std::size_t senders) {
    const auto send = [&counters](std::size_t sender) {
        const auto& counter = counters[sender % counters.size()];
        for (std::size_t i = 0; i < messagesPerSender; ++i) {
            counter.invoke(&Counter::increment);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(senders - 1);
    for (std::size_t sender = 1; sender < senders; ++sender) {
        threads.emplace_back(send, sender);
    }
    send(0);
    for (auto& thread : threads) {
        thread.join();
    }
    for (const auto& counter : counters) {
        benchmark::DoNotOptimize(counter.ask(&Counter::get).get());
    }
}

// One actor on the background thread pool, sent to from `range(0)` threads.
void Mailbox_ThreadPool(benchmark::State& state) {
    const auto senders = static_cast<std::size_t>(state.range(0));
    mln::Actor<Counter> counter(mln::Scheduler::GetBackground());
    const std::vector<mln::ActorRef<Counter>> counters{counter.self()};

    for (auto _ : state) {
        sendMessages(counters, senders);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations() * senders * messagesPerSender));
}

// `range(0)` actors on the background thread pool, each sent to from its own thread.
void Mailbox_ThreadPoolActors(benchmark::State& state) {
    const auto actors = static_cast<std::size_t>(state.range(0));
    std::vector<std::unique_ptr<mln::Actor<Counter>>> owners;
    std::vector<mln::ActorRef<Counter>> counters;
    for (std::size_t i = 0; i < actors; ++i) {
        owners.push_back(std::make_unique<mln::Actor<Counter>>(mln::Scheduler::GetBackground()));
        counters.push_back(owners.back()->self());
    }

    for (auto _ : state) {
        sendMessages(counters, actors);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations() * actors * messagesPerSender));
}

// One actor on a util::Thread run loop, sent to from `range(0)` threads.
void Mailbox_RunLoop(benchmark::State& state) {
    const auto senders = static_cast<std::size_t>(state.range(0));
    mln::util::Thread<Counter> counter("Counter");
    const std::vector<mln::ActorRef<Counter>> counters{counter.actor()};

    for (auto _ : state) {
        sendMessages(counters, senders);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations() * senders * messagesPerSender));
}

} // namespace

BENCHMARK(Mailbox_ThreadPool)->Arg(1)->Arg(4)->UseRealTime();
BENCHMARK(Mailbox_ThreadPoolActors)->Arg(4)->UseRealTime();
BENCHMARK(Mailbox_RunLoop)->Arg(1)->Arg(4)->UseRealTime();
