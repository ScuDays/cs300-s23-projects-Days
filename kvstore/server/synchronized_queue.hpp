#ifndef SYNCHRONIZED_QUEUE_HPP
#define SYNCHRONIZED_QUEUE_HPP

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "net/network_conn.hpp"

/**
 * Implementation of a thread-safe, generic queue.
 *
 * This class is a wrapper for a std::queue that handles all synchronization
 * automatically. Any number of threads can be using this queue concurrently,
 * without having to explicitly lock and unlock mutexes.
 *
 * The class is parametrized by <T>, which can be any object or struct. This
 * allows the queue to be used in a very similar way to the bare std::queue.
 *
 * Before termination of a program the queue needs to be stopped, to allow all
 * waiting threads to continue and to allow the std::condition_variable to be
 * cleaned up.
 */
template <typename T>
class synchronized_queue {
 public:
  /**
   * Get current size of queue.
   *
   * NOTE: By the time this call returns, another thread may have changed the
   * size of the queue. For this reason it is not advised to use this call for
   * anything other than logging.
   *
   * @return current size of the queue
   */
  size_t size();

  /**
   * Pop and set the elt pointer element from the front of the queue. Returns
   * true if the queue has been stopped, or false otherwise.
   * 从队列前面弹出并设置elt指针元素。如果队列已停止，则返回true；否则返回false。
   * 
   * Unless the queue has been stopped, calling pop() will always set elt and
   * return false. If the queue is empty, pop will wait until an element is
   * pushed onto it.
   * 除非队列已停止，否则调用pop()将始终设置elt并返回false。如果队列为空，pop将等待直到有元素被推送进来。
   * (会堵塞直到被唤醒?)
   * 
   * However, if the queue has been stopped, even if the queue is not empty, the
   * function will return true with no element set.
   * 然而，如果队列已经停止，即使队列不为空，该函数也会返回 true 而没有设置任何元素。
   * 
   * @param elt pointer to be set to popped element
   * @return true if the queue has been stopped, or false otherwise
   */
  bool pop(T* elt);

  /**
   * Push an element onto the back of the queue.
   * 将一个元素推送到队列的末尾。
   * 
   * If the queue is empty, this call will signal any one thread that is waiting
   * to pop from the queue.
   * 如果队列为空，此调用将通知任何一个正在等待从队列中弹出元素的线程。
   * @param elt element to be pushed on the queue
   */
  void push(T elt);

  /**
   * Flush will return a vector of all elements currently in the queue, even if
   * the queue has been stopped.
   * Flush会返回队列中当前所有元素的向量，即使队列已经停止。
   * 
   * After this function is called the queue will be empty. This call may
   * return an empty vector without blocking.
   * 调用此函数后，队列将为空。该调用可能会立即返回一个没有阻塞的vector。
   * 
   * @return vector of all elements in the queue
   */
  std::vector<T> flush();

  /**
   * Stop the queue.
   * 停止队列
   * 
   * This function serves as a way to release any threads waiting for the
   * queue to become non-empty. Doing this also allows the condition variable
   * to be cleaned up when the program exists.
   * 该函数用于释放任何等待队列变为非空的线程。这样做还可以在程序退出时清理条件变量。
   * 
   */
  void stop();

 private:
  // Underlying queue data structure.
  std::queue<T> q;

  // Synchronization fields.
  std::mutex mtx;
  std::condition_variable cv;
  // Atomic, thread-safe boolean that indicates whether the queue has been
  // stopped; false until stop() is called, which sets it to true.
  std::atomic_bool is_stopped{false};
};

#endif /* end of include guard */
