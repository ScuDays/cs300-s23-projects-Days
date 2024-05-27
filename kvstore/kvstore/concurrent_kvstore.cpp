#include "concurrent_kvstore.hpp"

#include <optional>

bool ConcurrentKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
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
  this->store.insertItem(this->store.bucket(req->key), req->key, req->value);
  return true;
}

bool ConcurrentKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
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
  std::optional<DbItem> ifGet =
      this->store.getIfExists(this->store.bucket(req->key), req->key);
  if (this->store.removeItem(this->store.bucket(req->key), req->key) == false) {
    return false;
  } else {
    res->value = ifGet->value;
    return true;
  }
}

bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
                                 MultiGetResponse* res) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  for (const auto& item : req->keys) {
    std::optional<DbItem> ifGet =
        this->store.getIfExists(this->store.bucket(item), item);
    if (!ifGet) {
      return false;
    } else {
      res->values.push_back(ifGet->value);
    }
  }
  return true;
}

bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
                                 MultiPutResponse*) {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  if (req->keys.size() != req->values.size()) {
    return false;
  }
  for (int i = 0; i < req->keys.size(); i++) {
    this->store.insertItem(this->store.bucket(req->keys[i]), req->keys[i],
                           req->values[i]);
  }
  return true;
}

std::vector<std::string> ConcurrentKvStore::AllKeys() {
  // TODO (Part A, Steps 4 and 5): IMPLEMENT
  std::vector<std::string> returnVector;
  for (int i = 0; i < this->store.BUCKET_COUNT; i++) {
    for (const auto& item : this->store.buckets[i]) {
      returnVector.push_back(item.key);
    }
  }
  return returnVector;
}
