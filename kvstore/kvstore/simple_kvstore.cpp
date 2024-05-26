#include "simple_kvstore.hpp"

bool SimpleKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  this->mutex_1.lock();
  if (this->KeyValueMap.count(req->key) == 0) {
    // printf("不存在该键");
    this->mutex_1.unlock();
    return false;
  }
  res->value = this->KeyValueMap[req->key];
    this->mutex_1.unlock();
  return true;
}

bool SimpleKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
    this->mutex_1.lock();
  if (this->KeyValueMap.count(req->key) == 0) {
    // printf("不存在该键");
    KeyValueMap.insert(
        std::pair<std::string, std::string>(req->key, req->value));
  } else {
    this->KeyValueMap[req->key] = req->value;
  }

  this->mutex_1.unlock();

  return true;
}

bool SimpleKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  this->mutex_1.lock();
  //不存在该键值对
  if (this->KeyValueMap.count(req->key) == 0) {
    this->KeyValueMap.insert(std::pair<std::string, std::string>(req->key, req->value));
  }
  //存在该键值对,append附加到值后面
  else {
    std::string finalValue = this->KeyValueMap[req->key] + req->value;
    this->KeyValueMap[req->key] = finalValue;
  }
  this->mutex_1.unlock();

  return true;
}

bool SimpleKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  this->mutex_1.lock();
  if (this->KeyValueMap.count(req->key) == 0) {
    // printf("不存在该键");
     this->mutex_1.unlock();
    return false;
  } else {
    res->value = this->KeyValueMap[req->key];
    this->KeyValueMap.erase(req->key);
     this->mutex_1.unlock();
    return true;
  }
}

bool SimpleKvStore::MultiGet(const MultiGetRequest* req,
                             MultiGetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  this->mutex_1.lock();
  bool ifGet = true;
  for (int i = 0; i < req->keys.size(); i++) {
    if (this->KeyValueMap.count(req->keys[i]) != 0) {
      res->values.push_back(this->KeyValueMap[req->keys[i]]);
    } else {
      ifGet = false;
    }
  }
 
  if (ifGet == false) {
     this->mutex_1.unlock();
    return false;
  } else {
    this->mutex_1.unlock();
    return true;
  }
}

bool SimpleKvStore::MultiPut(const MultiPutRequest* req, MultiPutResponse*) {
  // put的key数量和value数量对不上,直接return
  this->mutex_1.lock();
  if (req->keys.size() != req->values.size()) {
    this->mutex_1.unlock();
    return false;
  }
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::vector<std::string> ReqKeyVector = req->keys;
  std::vector<std::string> ReqValuesVector = req->values;
  
  for (int i = 0; i < ReqKeyVector.size(); i++) {
    if (this->KeyValueMap.count(ReqKeyVector[i]) != 0) {
      this->KeyValueMap[ReqKeyVector[i]] = ReqValuesVector[i];
    } else {
      KeyValueMap.insert(
          std::pair<std::string, std::string>(req->keys[i], req->values[i]));
    }
  }
  this->mutex_1.unlock();
  return true;
}

std::vector<std::string> SimpleKvStore::AllKeys() {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::vector<std::string> KeyVector;
  this->mutex_1.lock();
  std::map<std::string, std::string>::iterator it = this->KeyValueMap.begin();
  for (; it != this->KeyValueMap.end(); it++) {
    //等价于
    // KeyVector.push_back((*it).first);
    KeyVector.push_back(it->first);
  }
   this->mutex_1.unlock();
  return KeyVector;
}
