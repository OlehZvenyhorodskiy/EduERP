#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

namespace eduerp::core {

/**
 * @brief Thread pool for async I/O, network calls, and background sim computations.
 *        Defaults to hardware_concurrency threads minus one (leave main thread free).
 */
class ThreadPool {
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    bool m_stop = false;

public:
    explicit ThreadPool(size_t numThreads = 0) {
        if (numThreads == 0) {
            numThreads = std::max(1u, std::thread::hardware_concurrency() - 1);
        }
        for (size_t i = 0; i < numThreads; ++i) {
            m_workers.emplace_back([this] { workerLoop(); });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_stop = true;
        }
        m_condition.notify_all();
        for (auto& worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // Non-copyable, non-movable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief Enqueue a callable and get a future for the result.
     */
    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            if (m_stop) {
                throw std::runtime_error("Cannot enqueue on a stopped ThreadPool");
            }
            m_tasks.emplace([task]() { (*task)(); });
        }
        m_condition.notify_one();
        return result;
    }

    size_t threadCount() const { return m_workers.size(); }

private:
    void workerLoop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_condition.wait(lock, [this] { return m_stop || !m_tasks.empty(); });
                if (m_stop && m_tasks.empty()) return;
                task = std::move(m_tasks.front());
                m_tasks.pop();
            }
            task();
        }
    }
};

} // namespace eduerp::core
