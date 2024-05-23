#include "simple_kvstore.hpp"

bool SimpleKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  if (this->KeyValueMap.count(req->key) == 0) {
    // printf("不存在该键");
    return false;
  }
  res->value = this->KeyValueMap[req->key];
  return true;
}

bool SimpleKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  if (this->KeyValueMap.count(req->key) == 0) {
    // printf("不存在该键");
    KeyValueMap.insert(
        std::pair<std::string, std::string>(req->key, req->value));
  } else {
    this->KeyValueMap[req->key] = req->value;
  }
  return true;
}

bool SimpleKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  //不存在该键值对
  if (this->KeyValueMap.count(req->key) == 0) {
    std::pair<std::string, std::string> insertPair;
    insertPair.first = req->key;
    insertPair.second = req->value;
    this->KeyValueMap.insert(insertPair);
    return true;
  }
  //存在该键值对,append附加到值后面
  else {
    std::string finalValue = this->KeyValueMap[req->key] + req->value;
    // printf("%s\n", finalValue.c_str());
    this->KeyValueMap[req->key] = finalValue;
    return true;
  }
}

bool SimpleKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  if (this->KeyValueMap.count(req->key) == 0) {
    // printf("不存在该键");
    return false;
  } else {
    res->value = this->KeyValueMap[req->key];
    this->KeyValueMap.erase(req->key);
    return true;
  }
}

bool SimpleKvStore::MultiGet(const MultiGetRequest* req,
                             MultiGetResponse* res) {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  bool ifGet = true;
  for (int i = 0; i < req->keys.size(); i++) {
    if (this->KeyValueMap.count(req->keys[i]) != 0) {
      res->values.push_back(this->KeyValueMap[req->keys[i]]);
    } else {
      ifGet = false;
    }
  }
  if (ifGet == false) {
    return false;
  } else {
    return true;
  }
}

bool SimpleKvStore::MultiPut(const MultiPutRequest* req, MultiPutResponse*) {
  // put的key数量和value数量对不上,直接return
  if (req->keys.size() != req->values.size()) {
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
  return true;
}

std::vector<std::string> SimpleKvStore::AllKeys() {
  // TODO (Part A, Steps 1 and 2): IMPLEMENT
  std::vector<std::string> KeyVector;
  std::map<std::string, std::string>::iterator it = this->KeyValueMap.begin();
  for (; it != this->KeyValueMap.end(); it++) {
    //等价于
    // KeyVector.push_back((*it).first);
    KeyVector.push_back(it->first);
  }
  return KeyVector;
}
