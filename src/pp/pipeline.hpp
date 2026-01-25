#pragma once

#include "type.hpp"

namespace wibot {

template <u8 MaxNodes>
class PipelineChainBuilder;

// 输入端口：只读视图，指向外部存储（独立于节点）。
template <typename T>
struct In {
    const T*        ptr{nullptr};
    inline const T& get() const {
        return *ptr;
    }
    inline bool bound() const {
        return ptr != nullptr;
    }
};

// 输出端口：写视图，指向外部存储（独立于节点）。
template <typename T>
struct Out {
    T*        ptr{nullptr};
    inline T& ref() {
        return *ptr;
    }
    inline const T& get() const {
        return *ptr;
    }
    inline bool bound() const {
        return ptr != nullptr;
    }
};

// 简单节点接口：无递归，由调度器迭代调用。
struct INode {
    virtual bool ready()   = 0;  // 所有输入是否已绑定
    virtual void process() = 0;
    virtual void reset()   = 0;
    virtual ~INode()       = default;
};

// 运行时管道链（仅持有排序后节点，_edges 已释放）。
template <u8 MaxNodes>
class PipelineChain {
   public:
    PipelineChain() : _nodeCount(0) {
    }

    void reset() {
        for (u8 i = 0; i < _nodeCount; i++) {
            _nodes[i]->reset();
        }
    }

    void tick() {
        for (u8 i = 0; i < _nodeCount; i++) {
            INode* n = _nodes[i];
            n->process();
        }
    }

    u8 size() const {
        return _nodeCount;
    }

   private:
    friend class PipelineChainBuilder<MaxNodes>;
    INode* _nodes[MaxNodes];
    u8     _nodeCount;
};

// 构建器（持有临时 _edges，完成拓扑排序后释放）。
template <u8 MaxNodes>
class PipelineChainBuilder {
   public:
    static constexpr u8 MaxEdges = MaxNodes * 4;

    PipelineChainBuilder() : _nodeCount(0), _edgeCount(0) {
    }

    // 添加节点（支持链式调用）。
    PipelineChainBuilder& addNode(INode& node) {
        if (_nodeCount < MaxNodes) {
            _nodes[_nodeCount] = &node;
            _nodeCount++;
        }
        return *this;
    }

    // 将输出端口绑定到外部存储。
    template <typename T>
    static inline void bind(Out<T>& out, T& storage) {
        out.ptr = &storage;
    }

    // 批量绑定输出端口数组到存储数组（自动类型推导）。
    template <typename T, u8 N>
    static inline void bindArray(Out<T> (&outs)[N], T (&storage)[N]) {
        for (u8 i = 0; i < N; ++i) {
            bind(outs[i], storage[i]);
        }
    }

    // 连接上游与下游节点（直接传入节点引用）。
    template <typename T>
    bool connect(INode& from, Out<T>& out, INode& to, In<T>& in) {
        u8 fromId = findNodeId_(from);
        u8 toId   = findNodeId_(to);
        if (fromId == invalidId_() || toId == invalidId_()) {
            return false;
        }
        return connect_(fromId, out, toId, in);
    }

    // 计算拓扑顺序并构建 PipelineChain（_edges 随 Builder 析构释放）。
    bool build(PipelineChain<MaxNodes>& chain) {
        if (_nodeCount == 0) {
            chain._nodeCount = 0;
            return true;
        }

        // 在 build 阶段检查一次：所有节点是否 ready（所有输入已绑定）
        for (u8 i = 0; i < _nodeCount; i++) {
            if (!_nodes[i]->ready()) {
                return false;  // 节点未 ready，构建失败
            }
        }

        u8     indegree[MaxNodes] = {0};
        u8     queue[MaxNodes];
        INode* sorted[MaxNodes];

        for (u8 i = 0; i < _edgeCount; i++) {
            indegree[_edges[i].to]++;
        }

        u8 qHead = 0;
        u8 qTail = 0;
        for (u8 i = 0; i < _nodeCount; i++) {
            if (indegree[i] == 0) {
                queue[qTail++] = i;
            }
        }

        u8 sortedCount = 0;
        while (qHead < qTail) {
            u8 u                  = queue[qHead++];
            sorted[sortedCount++] = _nodes[u];
            for (u8 e = 0; e < _edgeCount; e++) {
                if (_edges[e].from == u) {
                    u8 v = _edges[e].to;
                    if (--indegree[v] == 0) {
                        queue[qTail++] = v;
                    }
                }
            }
        }

        if (sortedCount != _nodeCount) {
            return false;  // 检测到环
        }

        for (u8 i = 0; i < _nodeCount; i++) {
            chain._nodes[i] = sorted[i];
        }
        chain._nodeCount = _nodeCount;
        return true;
    }

    u8 size() const {
        return _nodeCount;
    }

   private:
    // 内部节点ID常量。
    static constexpr u8 invalidId_() {
        return static_cast<u8>(-1);
    }

    // 通过节点引用查找对应的节点ID（内部使用）。
    u8 findNodeId_(INode& node) {
        for (u8 i = 0; i < _nodeCount; i++) {
            if (_nodes[i] == &node) {
                return i;
            }
        }
        return invalidId_();
    }

    // 基于ID的连接（内部使用）。
    template <typename T>
    bool connect_(u8 fromId, Out<T>& out, u8 toId, In<T>& in) {
        // 约束: 每个输入只能绑定一个输出；每个输出可扇出至多个输入。
        if (fromId >= _nodeCount || toId >= _nodeCount) {
            return false;
        }
        if (_edgeCount >= MaxEdges) {
            return false;
        }
        if (in.bound()) {  // 输入已绑定，拒绝重复绑定
            return false;
        }
        if (!out.bound()) {  // 输出未绑定外部存储，无法连接
            return false;
        }

        in.ptr                  = out.ptr;
        _edges[_edgeCount].from = fromId;
        _edges[_edgeCount].to   = toId;
        _edgeCount++;
        return true;
    }

   private:
    struct Edge {
        u8 from;
        u8 to;
    };

    INode* _nodes[MaxNodes];
    Edge   _edges[MaxEdges];  // 仅构建期使用，build() 后可释放

    u8 _nodeCount;
    u8 _edgeCount;
};

}  // namespace wibot
