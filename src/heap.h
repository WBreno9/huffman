#ifndef HEAP_H
#define HEAP_H

template<typename T, typename Comp>
class MinHeap {
   public:
    struct HeapNode {
        uint32_t id;
        T v;
    };

    MinHeap() : size_(0) {}
    MinHeap(const std::vector<HeapNode>& nodes) : nodes_(nodes) {
        size_ = nodes_.size();

        uint32_t k = 0;
        for (auto& p : pos_)
            p = k++;

        uint32_t j = (size_ / 2.0f) - 1;
        for (uint32_t i = j + 1; i != 0; --i) heapify(i - 1);
    }

    T extract() {
        HeapNode extracted = nodes_[0];

        pos_[nodes_[size_ - 1].id] = 0;
        pos_[extracted.id] = size_ - 1;

        std::swap(nodes_[0] ,nodes_[size_ - 1]);
        --size_;

        heapify(0);
        return extracted.v;
    }

    void heapify(uint32_t index) {
        if (size_ == 1) return;

        uint32_t t = index;

        while (true) {
            uint32_t l = left_child(t);
            uint32_t r = right_child(t);

            if ((r < size_) && Comp()(nodes_[t].v, nodes_[r].v)) t = r;
            if ((l < size_) && Comp()(nodes_[t].v, nodes_[l].v)) t = l;

            if (t != index) {
                pos_[nodes_[t].id] = index;
                pos_[nodes_[index].id] = t;
                std::swap(nodes_[t], nodes_[index]);
                index = t;
            } else {
                break;
            }
        }
    }

    void insert(T v) {
        HeapNode n;
        n.v = v;
        n.id = size_;

        size_++;
        if (size_ < nodes_.size()) {
            nodes_[size_ - 1] = n;
            pos_[size_ - 1] = size_ - 1;
        } else {
            nodes_.push_back(n);
            pos_.push_back(size_ - 1);
        }

        uint32_t i = size_ - 1;

        while (i != 0 && Comp()(nodes_[parent(i)].v, nodes_[i].v)) {
            pos_[nodes_[i].id] = parent(i);
            pos_[nodes_[parent(i)].id] = i;

            uint32_t p_id = nodes_[parent(i)].id;

            std::swap(nodes_[i], nodes_[parent(i)]);

            i = pos_[p_id];
        }
    }

    std::vector<HeapNode> get_nodes() const { return std::vector<HeapNode>(nodes_.begin(), (nodes_.begin() + size_)); }
    bool is_empty() const { return size_ == 0; }
    uint32_t size() const { return size_; }

   private:
    std::vector<uint32_t> pos_;
    std::vector<HeapNode> nodes_;

    uint32_t size_;

    uint32_t parent(uint32_t index) const { return (index - 1) / 2.0f; }
    uint32_t left_child(uint32_t index) const { return 2 * index + 1; }
    uint32_t right_child(uint32_t index) const { return 2 * index + 2; }
};

#endif // !HEAP_H