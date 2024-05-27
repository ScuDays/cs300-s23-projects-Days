#include "concurrent_kvstore.hpp"

#include <optional>

bool ConcurrentKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT

  std::shared_lock guard(this->store.mtx[this->store.bucket(req->key)]);
  std::optional<DbItem> ifGet =
      this->store.getIfExists(this->store.bucket(req->key), req->key);
  if (ifGet != std::nullopt) {
    res->value = ifGet->value;
    return true;
  } else {
    return false;
  }
}

bool ConcurrentKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  std::unique_lock guard(this->store.mtx[this->store.bucket(req->key)]);
  this->store.insertItem(this->store.bucket(req->key), req->key, req->value);
  return true;
}

bool ConcurrentKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  std::unique_lock guard(this->store.mtx[this->store.bucket(req->key)]);
  std::string finalValue;
  std::optional<DbItem> ifGet =
      this->store.getIfExists(this->store.bucket(req->key), req->key);
  if (!ifGet) {
    finalValue = req->value;
  } else {
    finalValue = ifGet->value + req->value;
  }
  this->store.insertItem(this->store.bucket(req->key), req->key, finalValue);
  return true;
}

bool ConcurrentKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  std::unique_lock guard(this->store.mtx[this->store.bucket(req->key)]);
  std::optional<DbItem> ifGet =
      this->store.getIfExists(this->store.bucket(req->key), req->key);
  if (this->store.removeItem(this->store.bucket(req->key), req->key) == false) {
    return false;
  } else {
    res->value = ifGet->value;
    return true;
  }
}

/* 错误: 没考虑到整个操作的原子性,虽然MultiGet中的每一个操作是原子的,但整个的操作不是原子的**/
// bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
//                                  MultiGetResponse* res) {
//   // TODO (Part A, Steps 4 and 5): IMPLEMENT
//   for (const auto& item : req->keys) {
//     // std::shared_lock guard(this->store.mtx[this->store.bucket(item)]);
//     this->store.mtx[this->store.bucket(item)].lock_shared();
//     std::optional<DbItem> ifGet =
//         this->store.getIfExists(this->store.bucket(item), item);
//     if (!ifGet) {
//       this->store.mtx[this->store.bucket(item)].unlock_shared();
//       return false;
//     } else {
//       res->values.push_back(ifGet->value);
//       this->store.mtx[this->store.bucket(item)].unlock_shared();
//     }
//   }
//   return true;
// }

bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
                                 MultiGetResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT

  std::vector<size_t> bucket_nums;

  for(int i = 0; i < (int) req->keys.size(); i++){
    size_t b_num = store.bucket(req->keys[i]);
    bucket_nums.push_back(b_num);
  }
  //去除重复的,避免上锁的时候重复,导致死锁.
  std::sort(bucket_nums.begin(), bucket_nums.end());
  auto last = std::unique(bucket_nums.begin(), bucket_nums.end());
  bucket_nums.erase(last, bucket_nums.end());

  //一次性上锁,保证整个MultiGet的原子性
  for(int i = 0; i < bucket_nums.size(); i++){
    store.mtx[bucket_nums[i]].lock_shared();
  }

  for(int i = 0; i < (int) req->keys.size(); i++) {
    size_t b_num = store.bucket(req->keys[i]);
    std::optional<DbItem> opt = store.getIfExists(b_num, req->keys[i]);
    if(opt == std::nullopt) {
      return false;
    } 
    (res->values).push_back(opt.value().value);
  }

  for(int i = 0; i < bucket_nums.size(); i++){
    store.mtx[bucket_nums[i]].unlock_shared();
  }
  
  return true;
}
/* 错误: 没考虑到整个操作的原子性,虽然MultiPut中的每一个操作是原子的,但整个的操作不是原子的**/
// bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
//                                  MultiPutResponse*) {
//   // TODO (Part A, Steps 4 and 5): IMPLEMENT
//   if (req->keys.size() != req->values.size()) {
//     return false;
//   }

//   for (int i = 0; i < req->keys.size(); i++) {
//     //std::unique_lock guard(this->store.mtx[this->store.bucket(req->keys[i])]);
//     this->store.mtx[this->store.bucket(req->keys[i])].lock();
//     this->store.insertItem(this->store.bucket(req->keys[i]), req->keys[i],
//                            req->values[i]);
//     this->store.mtx[this->store.bucket(req->keys[i])].unlock();
//   }
//   return true;
// }
bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
                                 MultiPutResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  if (req->keys.size() != req->values.size()) {
    return false;
  }

  std::vector<size_t> bucket_nums;

  for(int i = 0; i < (int) req->keys.size(); i++){
    size_t b_num = store.bucket(req->keys[i]);
    bucket_nums.push_back(b_num);
  }

  std::sort(bucket_nums.begin(), bucket_nums.end());
  auto last = std::unique(bucket_nums.begin(), bucket_nums.end());
  bucket_nums.erase(last, bucket_nums.end());
  

  for(int i = 0; i < bucket_nums.size(); i++){
    store.mtx[bucket_nums[i]].lock();
  }


  for(int i = 0; i < (int) req->keys.size(); i++) {
    size_t b_num = store.bucket(req->keys[i]);
    std::optional<DbItem> opt = store.getIfExists(b_num, req->keys[i]);
    store.insertItem(b_num, req->keys[i], req->values[i]);
  }

  for(int i = 0; i < bucket_nums.size(); i++){
    store.mtx[bucket_nums[i]].unlock();
  }

  return true;
}

std::vector<std::string> ConcurrentKvStore::AllKeys() {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  std::vector<std::string> returnVector;
  for (int i = 0; i < this->store.BUCKET_COUNT; i++) {
      std::shared_lock guard(this->store.mtx[i]);
    for (const auto& item : this->store.buckets[i]) {
      returnVector.push_back(item.key);
    }
  }
  return returnVector;
}
