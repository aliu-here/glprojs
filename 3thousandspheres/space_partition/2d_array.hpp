#include <vector>

template <typename T>
struct ll_node
{
    bool operator==(ll_node<T> other) {
        return (value == other.value) && (next == other.next);
    }

    bool operator!=(ll_node<T> other) {
        return (value != other.value) || (next != other.next);
    }

    T value;
    int next = -1;
};

template <typename T> class ll
{
    public:
    ll(int head, int end, std::vector<ll_node<T>>& nodes): head_idx{head}, end_idx{end}, nodes{nodes} 
    {}

    ll_node<T> next(ll_node<T> node)
    {
        if (node.next != -1)
            return nodes[node.next];
        return node;
    }

    void append(T val)
    {
        ll_node<T> node;
        node.value = val;

        nodes.push_back(node);
        nodes[end_idx].next = nodes.size() - 1;

        end_idx = nodes.size() - 1;
        size++;
    }

    ll_node<T> begin()
    {
        return nodes[head_idx];
    }

    ll_node<T> end()
    {
        return nodes[end_idx];
    }


    int size = 0;
    private:
    int head_idx, end_idx;
    std::vector<ll_node<T>>& nodes;
};

template <typename T> class array_2d
{
    public:
        array_2d() {};
        void add_row()
        {
            ll_node<T> new_node;
            nodes.push_back(new_node);
            ll newlist(nodes.size() - 1, nodes.size() - 1, nodes);
            lists.push_back(newlist);
            size++;
        }
        ll<T>& operator[](int loc)
        {
            return lists[loc];
        }

        int size = 0;
    private:
    std::vector<ll<T>> lists;
    std::vector<ll_node<T>> nodes;
};
