#pragma once
#include <cassert>
#include "AVLNode.h"
#include <new>
#define NULL_ID (-1)


template <typename Value>
class AVLTree {
protected:
    AVLNode<Value>* head;

public:
    AVLTree(Value* value, int id):head(AVLNode<Value>(value,id)){}
    AVLTree():head(nullptr){}
    virtual ~AVLTree(){
        delete head;
        head = nullptr;
    }

    bool insert(Value* value, int id){
        if (this->head == nullptr){
            this->head = new AVLNode<Value>(value,id);
            return true;
        }
        Value* exists = this->head->find(id);
        if (exists != nullptr){
            return false;
        }
        this->head = this->head->insert(value,id);
        assert(this->verifyTree());
        return true;
    }

    bool remove(int index){
        if (this->head == nullptr){
            return false;
        }
        Value* exists = this->head->find(index);
        if (exists == nullptr){
            return false;
        }
        this->head = this->head->deleteNode(index);
        assert(this->verifyTree());
        return true;
    }

    Value* get(int index){
        //assert(this->verifyTree());
        if (this->head == nullptr){
            return nullptr;
        }
        return this->head->find(index);
    }

    /**
     * this function will be used for debugging
     */
    bool verifyTree(){
        if (this->head == nullptr){return true;}
        bool heightVerified = this->head->heightVerified();
        bool balanceVerified = this->head->isBalanced();
        return heightVerified && balanceVerified;
    }

};

