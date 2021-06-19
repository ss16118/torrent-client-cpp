//
// Created by siyuan on 15/06/2021.
//

#ifndef BITTORRENTCLIENT_SHAREDQUEUE_H
#define BITTORRENTCLIENT_SHAREDQUEUE_H

#include <queue>
#include <ostream>
#include <mutex>
#include <condition_variable>

/**
 * Implementation of a thread-safe Queue. Code from
 * https://stackoverflow.com/questions/36762248/why-is-stdqueue-not-thread-safe
 */
template <typename T>
class SharedQueue
{
public:
    SharedQueue();
    ~SharedQueue();

    T& front();
    T& pop_front();

    void push_back(const T& item);
    void push_back(T&& item);
    void clear();

    int size();
    bool empty();

private:
    std::deque<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

template <typename T>
SharedQueue<T>::SharedQueue() = default;

template <typename T>
SharedQueue<T>::~SharedQueue() = default;

template <typename T>
T& SharedQueue<T>::front()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
        cond_.wait(mlock);
    }
    return queue_.front();
}

template <typename T>
T& SharedQueue<T>::pop_front()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
        cond_.wait(mlock);
    }
    T& front = queue_.front();
    queue_.pop_front();
    return front;
}

template <typename T>
void SharedQueue<T>::push_back(const T& item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(item);
    mlock.unlock();     // unlock before notification to minimize mutex con
    cond_.notify_one(); // notify one waiting thread

}

template <typename T>
void SharedQueue<T>::push_back(T&& item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(std::move(item));
    mlock.unlock();     // unlock before notification to minimize mutex con
    cond_.notify_one(); // notify one waiting thread

}

template <typename T>
int SharedQueue<T>::size()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    int size = queue_.size();
    mlock.unlock();
    return size;
}

template<typename T>
bool SharedQueue<T>::empty()
{
    return size() == 0;
}

/**
 * Empties the queue
 */
template<typename T>
void SharedQueue<T>::clear()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    std::deque<T>().swap(queue_);
    mlock.unlock();
    cond_.notify_one();
}

#endif //BITTORRENTCLIENT_SHAREDQUEUE_H
