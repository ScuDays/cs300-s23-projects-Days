#include "synchronized_queue.hpp"

// 获取当前队列大小。
// 在此调用返回时，另一个线程可能已经改变了队列的大小。因此，不建议将此调用用于除日志记录之外的任何其他目的。
template <typename T>
size_t synchronized_queue<T>::size() {
  // TODO (Part A, Step 3): IMPLEMENT
  std::unique_lock guard(this->mtx);
  return this->q.size();
}

// 从队列前面弹出并设置elt指针元素。如果队列已停止，则返回true；否则返回false。
// 除非队列已停止，否则调用pop()将始终设置elt并返回false。如果队列为空，pop将等待直到有元素被推送进来。
// (会堵塞直到被唤醒?)
// 然而，如果队列已经停止，即使队列不为空，该函数也会返回 true
// 而没有设置任何元素。
template <typename T>
bool synchronized_queue<T>::pop(T* elt) {
  // TODO (Part A, Step 3): IMPLEMENT
  std::unique_lock<std::mutex> lck(mtx);
  if (this->is_stopped == true) {
    return true;
  }
  while (this->q.empty()) {
    if (this->is_stopped == true) {
      return true;
    }
    this->cv.wait(lck);
  }

  *elt = this->q.front();
  this->q.pop();
  return false;
}

// 将一个元素推送到队列的末尾。
// 如果队列为空，此调用将通知任何一个正在等待从队列中弹出元素的线程。
template <typename T>
void synchronized_queue<T>::push(T elt) {
  // TODO (Part A, Step 3): IMPLEMENT
  this->mtx.lock();
  if (this->q.size() == 0) {
    this->q.push(elt);
    this->mtx.unlock();
    this->cv.notify_all();
  } else {
    this->q.push(elt);
    this->mtx.unlock();
  }
}

// Flush会返回队列中当前所有元素的vector，即使队列已经停止。
// 调用此函数后，队列将为空。该调用可能会立即返回一个没有阻塞的vector。
template <typename T>
std::vector<T> synchronized_queue<T>::flush() {
  // TODO (Part A, Step 3): IMPLEMENT
  this->mtx.lock();
  std::vector<T> returnVector;
  while (!this->q.empty()) {
    returnVector.push_back(q.front());
    q.pop();
  }
  this->mtx.unlock();
  return {returnVector};
}

// 停止队列
// 该函数用于释放任何等待队列变为非空的线程。这样做还可以在程序退出时清理条件变量。
template <typename T>
void synchronized_queue<T>::stop() {
  // TODO (Part A, Step 3): IMPLEMENT
  this->is_stopped = true;
  this->cv.notify_all();
}

// NOTE: DO NOT TOUCH! Why is this necessary? Because C++ is a beautiful
// language:
// https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
template class synchronized_queue<int>;
template class synchronized_queue<std::shared_ptr<ClientConn>>;
