#pragma once

#include <cstring>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <thread>

#include "KICachePolicy.h"

namespace KamaCache
{
    template<typename Node> class LruList;

    // LRU节点
    template<typename Key, typename Value>
    class LruNode 
    {
    public:
        LruNode () 
        : key_()
        , value_()
        , accessCount_(0)
        {}

        LruNode (Key key, Value value) 
        : key_(key)
        , value_(value)
        , accessCount_(0)
        {}

        // 提供必要的访问器
        Key getKey() const { return key_; }
        Value getValue() const { return value_; }
        void setValue(const Value& value) { value_ = value; }
        size_t getAccessCount() const { return accessCount_; }
        void incrementAccessCount() { ++accessCount_; }

        friend class LruList<LruNode>;

    private:
        Key key_;
        Value value_;
        size_t accessCount_;
        std::weak_ptr<LruNode<Key, Value>> prev_;
        std::shared_ptr<LruNode<Key, Value>> next_;
    };

    // 双向链表
    template<typename Node>
    class LruList {
        using NodePtr = std::shared_ptr<Node>;
    public:
        LruList() : size_(0) {
            dummy_ = std::make_shared<Node>(Node{});
            dummy_->next_ = dummy_;
            dummy_->prev_ = dummy_;
        }
        
        // 插入节点(尾插)
        void insertNode(NodePtr node) {
            node->next_ = dummy_;
            node->prev_ = dummy_->prev_;
            node->prev_.lock()->next_ = node;
            dummy_->prev_ = node;
            size_++;
        }

        // 移除特定节点
        void removeNode(NodePtr node) {
            auto prev = node->prev_.lock();
            prev->next_ = node->next_;   // 使用shared_ptr保证 next_不被释放
            node->next_->prev_ = prev;
            node->next_ = nullptr;
            size_--;
        }

        // 获取最久未访问的节点
        NodePtr getOldestNode() {
            return dummy_->next_;
        }

        // 获取节点数量
        size_t size() const { return size_; }

    private:
        size_t      size_;
        NodePtr     dummy_;   // 哨兵节点
    };

    // LRU缓存
    template<typename Key, typename Value>
    class KLruCache : public KICachePolicy<Key, Value>
    {
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeList = LruList<LruNodeType>;
        using NodeMap = std::unordered_map<Key, NodePtr>;
    public:
        // 构造函数
        KLruCache(int capacity) : capacity_(capacity) {}

        // 析构函数(默认)
        ~KLruCache() override = default;

        // 获取 value
        bool get(Key key, Value& value) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                nodeList_.removeNode(it->second);
                nodeList_.insertNode(it->second);
                value = it->second->getValue();
                return true;
            }   
            return false;
        }

        // 获取 value
        Value get(Key key) {
            Value value{};
            get(key, value);    // 获取失败，返回空
            return value;
        }

        // 插入 key-value
        void put(Key key, Value value) {
            std::lock_guard<std::mutex> lock(mutex_);
            // 已有，更新
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                it->second->setValue(value);
                nodeList_.removeNode(it->second);
                nodeList_.insertNode(it->second);
                return ;
            }

            // 没有，添加新节点
            if (nodeMap_.size() == capacity_) {
                auto node = nodeList_.getOldestNode();
                nodeList_.removeNode(node);
                nodeMap_.erase(node->getKey());
            }
            auto newNode = std::make_shared<LruNodeType>(key, value);
            nodeList_.insertNode(newNode);
            nodeMap_[key] = newNode;
        }

        // 删除 key-value
        void remove(Key key) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                nodeList_.removeNode(it->second);
                nodeMap_.erase(it);
            }
        }

    private:
        int           capacity_;    // 缓存容量
        std::mutex    mutex_;       // 互斥锁, 保护Map
        NodeMap       nodeMap_;
        NodeList      nodeList_;    // 双向链表, 头节点为最久未访问的节点
    };  


    /*原本实现直接继承Lru，存在问题：historyValueMap_只在将数据从历史缓存移动到主缓存时才删除，可能导致historyValueMap_中存在大量无效数据*/

    // LRU优化：Lru-k版本
    // 在普通LRU中访问次数达到k次时，才将数据从历史缓存移动到主缓存
    template<typename Key, typename Value>
    class KLruKCache : public KICachePolicy<Key, Value>
    {
        // 主缓存
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeList = LruList<LruNodeType>;
        using NodeMap = std::unordered_map<Key, NodePtr>;

        // 历史缓存多存储
        using HistoryNode = std::pair<int, NodePtr>;
        using HistoryList = LruList<LruNodeType>;
        using HistoryMap = std::unordered_map<Key, HistoryNode>;
        
    public:
        // 构造函数
        // @param capacity: 主缓存容量
        // @param historyCapacity: 历史缓存容量
        // @param k: 访问次数阈值
        KLruKCache(int capacity, int historyCapacity, int k)
            : k_(k)
            , capacity_(capacity)
            , historyCapacity_(historyCapacity) 
        {}

        // 析构函数(默认)
        ~KLruKCache() override = default;

        // 获取 value
        bool get(Key key, Value& value) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            // 主缓存查找
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                nodeList_.removeNode(it->second);
                nodeList_.insertNode(it->second);
                value = it->second->getValue();
                return true;
            }

            // 历史缓存查找/更新
            auto historyIt = historyMap_.find(key);
            if (historyIt != historyMap_.end()) {
                value = historyIt->second.second->getValue();
                visitHistoryNode(historyIt->second);       // 访问历史缓存节点
            }

            // 未在主缓存
            return false;
        }

        // 获取 value
        Value get(Key key) {
            Value value{};
            get(key, value);    // 获取失败，返回空
            return value;
        }

        // 插入 key-value
        void put(Key key, Value value) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            // 主缓存查找
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) {
                it->second->setValue(value);
                nodeList_.removeNode(it->second); // 移除旧节点
                nodeList_.insertNode(it->second); // 插入新节点
                return ;
            }

            // 历史缓存查找/更新
            auto historyIt = historyMap_.find(key);
            if (historyIt != historyMap_.end()) {
                historyIt->second.second->setValue(value); // 更新值
                visitHistoryNode(historyIt->second);       // 访问历史缓存节点
                return ;
            }

            // 缓存中都不存在, 历史缓存插入新节点
            if (historyMap_.size() == historyCapacity_) {
                auto oldNode = historyList_.getOldestNode();
                historyList_.removeNode(oldNode);
                historyMap_.erase(oldNode->getKey());
            }
            historyMap_[key] = std::pair<int, NodePtr>(0, std::make_shared<LruNodeType>(key, value));
            historyList_.insertNode(historyMap_[key].second);
        }

    private:
        // 访问历史缓存节点 (访问次数+1、移动到头部、满足k次访问则移动到主缓存)
        void visitHistoryNode(HistoryNode& historyNode) {  
            historyNode.first++;
            auto key = historyNode.second->getKey();

            // 记录不在主缓存，满足k次访问，移动到主缓存
            if (historyNode.first >= k_) {
                // 主缓存容量不足，删除最久未访问的节点
                if (nodeMap_.size() == capacity_) {
                    auto oldNode = nodeList_.getOldestNode();
                    nodeList_.removeNode(oldNode);
                    nodeMap_.erase(oldNode->getKey());
                }
                
                historyList_.removeNode(historyNode.second);  // nodeptr 断开， 计数=1
                
                nodeList_.insertNode(historyNode.second);     // nodeptr 连接，计数=2
                nodeMap_[key] = historyNode.second;           // 获取 shared_ptr，计数=3

                historyMap_.erase(key);                       // 清除pair<int, shared_ptr>， 计数=2
            }
            else {  // 未满足k次访问，移动到历史缓存头部
                historyList_.removeNode(historyNode.second);
                historyList_.insertNode(historyNode.second);    
            }
        }

        // Lru 缓存数据
        int             capacity_;          // 缓存容量
        NodeMap         nodeMap_;           // 存储缓存数据
        NodeList        nodeList_;          // 双向链表, 头节点为最久未访问的节点

        // 历史数据
        int              historyCapacity_;  // 历史缓存容量
        HistoryMap       historyMap_;      // 存储未到达k次访问的 <int, LruNodePtr> 
        HistoryList      historyList_;       // 存储未达到k次访问的 LruNode
        
        int              k_;                // 进入缓存队列的评判标准
        std::mutex       mutex_;            // 互斥锁, 保护Map
    };
}