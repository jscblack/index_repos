#include <assert.h>
#include <algorithm>

#include "N.h"
#include "N4.cpp"
#include "N16.cpp"
#include "N48.cpp"
#include "N256.cpp"

namespace ART_unsynchronized {

    void N::setType(NTypes type) {
        this->type = type;
    }

    NTypes N::getType() const {
        return type;
    }

    N *N::getAnyChild(const N *node) {
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<const N4 *>(node);
                return n->getAnyChild();
            }
            case NTypes::N16: {
                auto n = static_cast<const N16 *>(node);
                return n->getAnyChild();
            }
            case NTypes::N48: {
                auto n = static_cast<const N48 *>(node);
                return n->getAnyChild();
            }
            case NTypes::N256: {
                auto n = static_cast<const N256 *>(node);
                return n->getAnyChild();
            }
        }
        assert(false);
        __builtin_unreachable();
    }

    void N::change(N *node, uint8_t key, N *val) {
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                n->change(key, val);
                return;
            }
            case NTypes::N16: {
                auto n = static_cast<N16 *>(node);
                n->change(key, val);
                return;
            }
            case NTypes::N48: {
                auto n = static_cast<N48 *>(node);
                n->change(key, val);
                return;
            }
            case NTypes::N256: {
                auto n = static_cast<N256 *>(node);
                n->change(key, val);
                return;
            }
        }
        assert(false);
        __builtin_unreachable();
    }

    template<typename curN, typename biggerN>
    void N::insertGrow(curN *n, N *parentNode, uint8_t keyParent, uint8_t key, N *val) {
        if (n->insert(key, val)) {
            return;
        }

        auto nBig = new biggerN(n->getPrefix(), n->getPrefixLength());
        n->copyTo(nBig);
        nBig->insert(key, val);

        N::change(parentNode, keyParent, nBig);

        delete n;
    }

    void N::insertA(N *node, N *parentNode, uint8_t keyParent, uint8_t key, N *val) {
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                insertGrow<N4, N16>(n, parentNode, keyParent, key, val);
                return;
            }
            case NTypes::N16: {
                auto n = static_cast<N16 *>(node);
                insertGrow<N16, N48>(n, parentNode, keyParent, key, val);
                return;
            }
            case NTypes::N48: {
                auto n = static_cast<N48 *>(node);
                insertGrow<N48, N256>(n, parentNode, keyParent, key, val);
                return;
            }
            case NTypes::N256: {
                auto n = static_cast<N256 *>(node);
                n->insert(key, val);
                return;
            }
        }
        assert(false);
        __builtin_unreachable();
    }

    N *N::getChild(const uint8_t k, N *node) {
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                return n->getChild(k);
            }
            case NTypes::N16: {
                auto n = static_cast<N16 *>(node);
                return n->getChild(k);
            }
            case NTypes::N48: {
                auto n = static_cast<N48 *>(node);
                return n->getChild(k);
            }
            case NTypes::N256: {
                auto n = static_cast<N256 *>(node);
                return n->getChild(k);
            }
        }
        assert(false);
        __builtin_unreachable();
    }

    void N::deleteChildren(N *node) {
        if (N::isLeaf(node)) {
            return;
        }
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                n->deleteChildren();
                return;
            }
            case NTypes::N16: {
                auto n = static_cast<N16 *>(node);
                n->deleteChildren();
                return;
            }
            case NTypes::N48: {
                auto n = static_cast<N48 *>(node);
                n->deleteChildren();
                return;
            }
            case NTypes::N256: {
                auto n = static_cast<N256 *>(node);
                n->deleteChildren();
                return;
            }
        }
        assert(false);
        __builtin_unreachable();
    }

    template<typename curN, typename smallerN>
    void N::removeAndShrink(curN *n, N *parentNode, uint8_t keyParent, uint8_t key) {
        if (n->remove(key, parentNode == nullptr)) {
            return;
        }

        auto nSmall = new smallerN(n->getPrefix(), n->getPrefixLength());


        n->remove(key, true);
        n->copyTo(nSmall);
        N::change(parentNode, keyParent, nSmall);

        delete n;
    }

    void N::removeA(N *node, uint8_t key, N *parentNode, uint8_t keyParent) {
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                n->remove(key, false);
                return;
            }
            case NTypes::N16: {
                auto n = static_cast<N16 *>(node);
                removeAndShrink<N16, N4>(n, parentNode, keyParent, key);
                return;
            }
            case NTypes::N48: {
                auto n = static_cast<N48 *>(node);
                removeAndShrink<N48, N16>(n, parentNode, keyParent, key);
                return;
            }
            case NTypes::N256: {
                auto n = static_cast<N256 *>(node);
                removeAndShrink<N256, N48>(n, parentNode, keyParent, key);
                return;
            }
        }
        assert(false);
        __builtin_unreachable();
    }

    uint32_t N::getPrefixLength() const {
        return prefixCount;
    }

    bool N::hasPrefix() const {
        return prefixCount > 0;
    }

    uint32_t N::getCount() const {
        return count;
    }

    const uint8_t *N::getPrefix() const {
        return prefix;
    }

    void N::setPrefix(const uint8_t *prefix, uint32_t length) {
        if (length > 0) {
            memcpy(this->prefix, prefix, std::min(length, maxStoredPrefixLength));
            prefixCount = length;
        } else {
            prefixCount = 0;
        }
    }

    void N::addPrefixBefore(N *node, uint8_t key) {
        uint32_t prefixCopyCount = std::min(maxStoredPrefixLength, node->getPrefixLength() + 1);
        memmove(this->prefix + prefixCopyCount, this->prefix,
                std::min(this->getPrefixLength(), maxStoredPrefixLength - prefixCopyCount));
        memcpy(this->prefix, node->prefix, std::min(prefixCopyCount, node->getPrefixLength()));
        if (node->getPrefixLength() < maxStoredPrefixLength) {
            this->prefix[prefixCopyCount - 1] = key;
        }
        this->prefixCount += node->getPrefixLength() + 1;
    }


    bool N::isLeaf(const N *n) {
        return (reinterpret_cast<uint64_t>(n) & (static_cast<uint64_t>(1) << 63)) == (static_cast<uint64_t>(1) << 63);
    }

    N *N::setLeaf(TID tid) {
        return reinterpret_cast<N *>(tid | (static_cast<uint64_t>(1) << 63));
    }

    TID N::getLeaf(const N *n) {
        return (reinterpret_cast<uint64_t>(n) & ((static_cast<uint64_t>(1) << 63) - 1));
    }

    std::tuple<N *, uint8_t> N::getSecondChild(N *node, const uint8_t key) {
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                return n->getSecondChild(key);
            }
            default: {
                assert(false);
                __builtin_unreachable();
            }
        }
    }

    void N::deleteNode(N *node) {
        if (N::isLeaf(node)) {
            return;
        }
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                delete n;
                return;
            }
            case NTypes::N16: {
                auto n = static_cast<N16 *>(node);
                delete n;
                return;
            }
            case NTypes::N48: {
                auto n = static_cast<N48 *>(node);
                delete n;
                return;
            }
            case NTypes::N256: {
                auto n = static_cast<N256 *>(node);
                delete n;
                return;
            }
        }
        delete node;
    }


    TID N::getAnyChildTid(N *n) {
        N *nextNode = n;
        N *node = nullptr;

        nextNode = getAnyChild(nextNode);

        assert(nextNode != nullptr);
        if (isLeaf(nextNode)) {
            return getLeaf(nextNode);
        }

        while (true) {
            node = nextNode;

            nextNode = getAnyChild(node);

            assert(nextNode != nullptr);
            if (isLeaf(nextNode)) {
                return getLeaf(nextNode);
            }
        }
    }

    void N::getChildren(const N *node, uint8_t start, uint8_t end, std::tuple<uint8_t, N *> children[],
                        uint32_t &childrenCount) {
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<const N4 *>(node);
                n->getChildren(start, end, children, childrenCount);
                return;
            }
            case NTypes::N16: {
                auto n = static_cast<const N16 *>(node);
                n->getChildren(start, end, children, childrenCount);
                return;
            }
            case NTypes::N48: {
                auto n = static_cast<const N48 *>(node);
                n->getChildren(start, end, children, childrenCount);
                return;
            }
            case NTypes::N256: {
                auto n = static_cast<const N256 *>(node);
                n->getChildren(start, end, children, childrenCount);
                return;
            }
        }
    }

    long N::size(N *node) {
        if (node == nullptr) {
            return 0;
        }
        if (N::isLeaf(node)) {
            return sizeof(N::getLeaf(node));
        }
        switch (node->getType()) {
            case NTypes::N4: {
                auto n = static_cast<N4 *>(node);
                return n->size();
            }
            case NTypes::N16: {
                auto n = static_cast<N16 *>(node);
                return n->size();
            }
            case NTypes::N48: {
                auto n = static_cast<N48 *>(node);
                return n->size();
            }
            case NTypes::N256: {
                auto n = static_cast<N256 *>(node);
                return n->size();
            }
            default: {
                assert(false);
                __builtin_unreachable();
            }
        }
    }

    void N::collect_stats(N *node, size_t depth, std::vector<size_t> &depth_distribution, std::vector<size_t> &type_distribution) {
        if (node == nullptr) {
            return;
        }
        
        if (N::isLeaf(node)) {
            if (depth_distribution.size() <= depth) {
                depth_distribution.resize(depth + 1, 0);
            }
            depth_distribution[depth]++;
            return;
        }

        switch (node->getType()) {
            case NTypes::N4: {
                type_distribution[static_cast<int>(NTypes::N4)]++;
                auto n = static_cast<N4 *>(node);
                for (uint8_t i = 0; i < n->getCount(); ++i) {
                    collect_stats(n->get_child(i), depth + 1, depth_distribution, type_distribution);
                }
                // uint8_t start = 0, end = 255;
                // std::tuple<uint8_t, N *> *children = new std::tuple<uint8_t, N *>[256];
                // uint32_t childrenCount = 0;
                // n->getChildren(start, end, children, childrenCount);
                // for (uint32_t i = 0; i < childrenCount; ++i) {
                //     collect_stats(std::get<1>(children[i]), depth + 1, depth_distribution, type_distribution);
                // }
            }
            case NTypes::N16: {
                type_distribution[static_cast<int>(NTypes::N16)]++;
                auto n = static_cast<N16 *>(node);
                for (uint8_t i = 0; i < n->getCount(); ++i) {
                    collect_stats(n->get_child(i), depth + 1, depth_distribution, type_distribution);
                }
                // uint8_t start = 0, end = 255;
                // std::tuple<uint8_t, N *> *children = new std::tuple<uint8_t, N *>[256];
                // uint32_t childrenCount = 0;
                // n->getChildren(start, end, children, childrenCount);
                // for (uint32_t i = 0; i < childrenCount; ++i) {
                //     collect_stats(std::get<1>(children[i]), depth + 1, depth_distribution, type_distribution);
                // }
            }
            case NTypes::N48: {
                type_distribution[static_cast<int>(NTypes::N48)]++;
                auto n = static_cast<N48 *>(node);
                for (uint8_t i = 0; i < 48; ++i) {
                    collect_stats(n->get_child(i), depth + 1, depth_distribution, type_distribution);
                }
                // uint8_t start = 0, end = 255;
                // std::tuple<uint8_t, N *> *children = new std::tuple<uint8_t, N *>[256];
                // uint32_t childrenCount = 0;
                // n->getChildren(start, end, children, childrenCount);
                // for (uint32_t i = 0; i < childrenCount; ++i) {
                //     collect_stats(std::get<1>(children[i]), depth + 1, depth_distribution, type_distribution);
                // }
            }
            case NTypes::N256: {
                type_distribution[static_cast<int>(NTypes::N256)]++;
                auto n = static_cast<N256 *>(node);
                for (uint8_t i = 0; i < 256; ++i) {
                    collect_stats(n->get_child(i), depth + 1, depth_distribution, type_distribution);
                }
                // uint8_t start = 0, end = 255;
                // std::tuple<uint8_t, N *> *children = new std::tuple<uint8_t, N *>[256];
                // uint32_t childrenCount = 0;
                // n->getChildren(start, end, children, childrenCount);
                // for (uint32_t i = 0; i < childrenCount; ++i) {
                //     collect_stats(std::get<1>(children[i]), depth + 1, depth_distribution, type_distribution);
                // }
            }
            default: {
                __builtin_unreachable();
            }
        }
        return ;
    }
}