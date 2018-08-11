#ifndef TEST_HTTPROUTER2_H
#define TEST_HTTPROUTER2_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <map>
#include <vector>
#include <cstring>
#include <functional>
#include <iostream>
#include <initializer_list>

#include "HttpRouter.h"

template <typename UserData>
class HttpRouter2 {
public:
    typedef std::function<void(UserData, std::vector<string_view> &)> callback_type;

    struct NodeHeader {
        unsigned short length;
        unsigned short nameLength;
        short handlerIndex;
    };

    struct Node {
        std::string name;
        std::map<std::string, Node *> children;
        short handler;

        Node(short _handler = -1) : handler(_handler) {}
        Node(const std::string & _name, short _handler = -1) : name(_name), handler(_handler) {}
        Node(const Node & src) {
            this->name = src.name;
            this->children = src.children;
            this->handler = src.handler;
        }
        Node(Node && src) {
            this->swap(std::forward<Node>(src));
        }

        ~Node() {
            for (auto child : children) {
                Node * node = child.second;
                if (node != nullptr) {
                    delete node;
                    child.second = nullptr;
                }
            }
        }

        void swap(Node & others) {
            this->name.swap(others.name);
            this->children.swap(others.children);
            std::swap(this->handler, others.handler);
        }
    };

private:
    std::vector<callback_type> handlers_;
    std::vector<string_view> params_;
    Node * tree_root_;
    std::string compiled_tree_;

    void add(std::vector<std::string> route, short handler) {
        Node * parent = tree_root_;
        for (std::string node : route) {
            if (parent->children.find(node) == parent->children.end()) {
                parent->children[node] = new Node(node, handler);
            }
            parent = parent->children[node];
        }
    }

    unsigned short compile_tree(Node * node) {
        // Header: nodeLength, nodeNameLength, sizeof(node->handler)
        static const size_t kHeaderLen = sizeof(NodeHeader);

        assert(node != nullptr);
        unsigned short nodeLength = (unsigned short)(kHeaderLen + node->name.length());
        for (auto children : node->children) {
            nodeLength += compile_tree(children.second);
        }

        unsigned short nodeNameLength = (unsigned short)(node->name.length());

        std::string compiledNode;
        compiledNode.append((char *)&nodeLength, sizeof(nodeLength));
        compiledNode.append((char *)&nodeNameLength, sizeof(nodeNameLength));
        compiledNode.append((char *)&node->handler, sizeof(node->handler));
        compiledNode.append(node->name.data(), node->name.length());

        compiled_tree_ = compiledNode + compiled_tree_;
        return nodeLength;
    }

    inline const NodeHeader * find_node(const NodeHeader * parent_node,
                                        const char * name, std::size_t name_length) {
        // Header: nodeLength, nodeNameLength, sizeof(node->handler)
        static const size_t kHeaderLen = sizeof(NodeHeader);

        assert(parent_node != nullptr);
        unsigned short nodeLength = parent_node->length;
        unsigned short nodeNameLength = parent_node->nameLength;

        //std::cout << "Finding node: <" << std::string(name, name_length) << ">" << std::endl;

        const char * stop_ptr = (const char *)parent_node + nodeLength;
        for (const char * candidate = (const char *)parent_node + kHeaderLen + nodeNameLength; candidate < stop_ptr; ) {
            unsigned short nodeLength = ((NodeHeader *)candidate)->length;
            unsigned short nodeNameLength = ((NodeHeader *)candidate)->nameLength;

            // whildcard, parameter, equal
            if (nodeNameLength == 0) {
                return (const NodeHeader *)candidate;
            }
            else if (candidate[kHeaderLen] == ':') {
                // parameter

                // TODO: push this pointer on the stack of args!
                params_.push_back(string_view(name, name_length));

                return (const NodeHeader *)candidate;
            }
            else if (nodeNameLength == name_length &&
                    !memcmp(candidate + kHeaderLen, name, name_length)) {
                return (const NodeHeader *)candidate;
            }

            candidate = candidate + nodeLength;
        }

        return nullptr;
    }

    // Returns next slash from start or end
    inline const char * getNextSegment(const char * start, const char * end) {
        assert(end >= start);
        const char * stop = (const char *)memchr(start, '/', end - start);
        return ((stop != nullptr) ? stop : end);
    }

    // Should take method also!
    inline int lookup(const char * url, std::size_t length) {       
        const NodeHeader * treeStart = (const NodeHeader *)compiled_tree_.data();

        // All urls start with "/"
        const char * stop, * start = url + sizeof(char), * end_ptr = url + length;

        do {
            stop = getNextSegment(start, end_ptr);

            //std::cout << "Matching(" << std::string(start, stop - start) << ")" << std::endl;

            if ((treeStart = find_node(treeStart, start, stop - start)) == nullptr) {
                return -1;
            }

            start = stop + sizeof('/');
        } while (stop != end_ptr);

        return (int)(treeStart->handlerIndex);
    }

public:
    HttpRouter2() : tree_root_(nullptr) {
        // Tree root node
        tree_root_ = new Node("GET", -1);
        // maximum 100 parameters
        params_.reserve(100);
    }

    ~HttpRouter2() {
        Node * node = tree_root_;
        if (node != nullptr) {
            delete tree_root_;
            tree_root_ = nullptr;
        }
    }    

    HttpRouter2 & add(const char * method, const char * pattern, callback_type handler) {
        // Step over any initial slash
        if (pattern[0] == '/') {
            pattern++;
        }

        std::vector<std::string> nodes;
        //nodes.push_back(method);

        const char * stop, * start = pattern, * end_ptr = pattern + strlen(pattern);
        do {
            stop = getNextSegment(start, end_ptr);

            //std::cout << "Segment(" << std::string(start, stop - start) << ")" << std::endl;

            nodes.push_back(std::string(start, stop - start));

            start = stop + 1;
        } while (stop != end_ptr);

        // If pattern starts with / then move 1+ and run inline slash parser
        add(nodes, (short)handlers_.size());
        handlers_.push_back(handler);

        compile();
        return *this;
    }

    void compile() {
        compiled_tree_.clear();
        compile_tree(tree_root_);
    }

    void route(const char * method, std::size_t method_length,
               const char * url, std::size_t url_length, UserData user_data) {
        int index = lookup(url, url_length);
        if (index != -1) {
            handlers_[index](user_data, params_);
        }
        params_.clear();
    }
};

template <typename UserData>
inline void swap(typename HttpRouter2<UserData>::Node & lhs, typename HttpRouter2<UserData>::Node & rhs) {
    lhs.swap(rhs);
}

#endif // TEST_HTTPROUTER2_H
