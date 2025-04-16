//
// Created by Guy Friedman on 24/01/2025.
//

#ifndef DEQUENODE_H
#define DEQUENODE_H


#include <cassert>

bool can_use_fstream = false;
#if can_use_fstream
#include <fstream>
#include <iostream>
using std::cout;
using std::ostream;
using std::endl;
#endif



template <typename T>
class DeQue;

template <typename T>
class DeQueNode {
private:

    // Declare the templated DeQue as a friend
    template <typename U>
    friend class DeQue;

    inline bool initialNode() const {
        return (this->next == nullptr) && (this->prev == nullptr);
    }

    inline bool noData() const {
        return this->data == nullptr;
    }

    inline bool canBecomeHead() {
        return (this->initialNode()) && (this->noData());
    }

    inline bool canBecomeTail() {
        return (this->initialNode()) && (this->noData());
    }

protected:

    T* data;
    DeQueNode<T>* next;
    DeQueNode<T>* prev;

public:

    DeQueNode() : data(nullptr), next(nullptr), prev(nullptr) {}

    DeQueNode(T* data) : data(data), next(nullptr), prev(nullptr) {}

    ~DeQueNode() {
        delete next;
        delete data;
    }

    void nullify() {
        this->next = nullptr;
        this->prev = nullptr;
        this->data = nullptr;
    }

    void deCouple() {
        this->next->prev = this->prev;
        this->prev->next = this->next;
        this->next = nullptr;
        this->prev = nullptr;
    }

    void verifyDeCouple() {
        if (this->next == nullptr && this->prev == nullptr) {
            return;
        }
        this->deCouple();
    }

    T* extractAndDelete() {
        this->deCouple();
        T* temp = this->data;
        this->data = nullptr;
        this->nullify();
        //delete this;
        return temp;
    }

    T* extract() {
        T* temp = this->data;
        this->data = nullptr;
        return temp;
    }


    inline bool hasNext() const { //fixme problem
        return !((this->next != nullptr) && (this->next->isTail()));
    }

    inline bool isTail() const {
        return (this->prev != nullptr) && (this->data == nullptr) && (this->next == nullptr);
    }

    inline bool isHead() const {
        return (this->next != nullptr) && (this->data == nullptr) && (this->prev == nullptr);
    }

    DeQueNode<T>* popNext() {
        assert(this->hasNext());
        assert(this->next != nullptr && this->next->next != nullptr);
        DeQueNode<T>* temp = this->next;
        temp->deCouple();
        return temp;
    }

    void queueAdd(DeQueNode<T>* node) {
        assert(node != nullptr);
        assert(!this->isHead());
        this->prev->next = node;
        node->prev = this->prev;
        node->next = this;
        this->prev = node;
    }

    void stackAdd(DeQueNode<T>* node) {
        assert(node != nullptr);
        assert(!this->isTail());
        this->next->prev = node;
        node->next = this->next;
        node->prev = this;
        this->next = node;
    }

    void addInitial(DeQueNode<T>* node) {
        assert(node != nullptr);
        //make sure that 'this' node is in a state required to become head
        assert(this->canBecomeHead());
        //make sure that 'node' node is in a state required to become tail
        assert(node->canBecomeTail());
        this->next = node;
        node->prev = this;
    }

    T* getData() const {
        return this->data;
    }

    DeQueNode<T>* find(T& toFind) {
        if(this->isTail()) {
            return nullptr;
        }
        if (this->isHead()) {
            return this->next->find(toFind);
        }
        if (*(this->data) == toFind) {
            return this;
        }
        return this->next->find(toFind);
    }

    DeQueNode<T>* remove(T& toFind) {
        if(this->isTail()) {
            return nullptr;
        }
        if (this->isHead()) {
            return this->next->remove(toFind);
        }
        if (*(this->data) == toFind) {
            this->deCouple();
            return this;
        }
        return this->next->find(toFind);
    }


    #if can_use_fstream

    void printHelper(ostream& os,int node_number = 0) const {
        char node_seperator_start = '[';
        char node_seperator_end = ']';
        auto node_seperator = "<->";
        cout << node_seperator_start;
        if (this->isTail()) {
            cout << "Tail Node." << node_seperator_end;
            return;
        }
        if (this->isHead()) {
            cout << "Head Node." << node_seperator_end << node_seperator;
            this->next->printHelper(os,node_number + 1);
            return;
        }
        cout << "Node number: " << node_number;
        cout << ", Value : {";
        if (this->data == nullptr) {
            cout << "nullptr";
        } else {
            cout << (*(this->data));
        }
        cout<< "}";
        cout << node_seperator_end << node_seperator;
        this->next->printHelper(os,node_number + 1);
    }

    // Overloaded ostream operator
    friend ostream& operator<<(ostream& os, const DeQueNode<T>& deque) {
        deque.printHelper(os);
        return os;
    }
    #endif


};



#endif //DEQUENODE_H
