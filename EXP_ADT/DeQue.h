//
// Created by Guy Friedman on 24/01/2025.
//

#ifndef DEQUE_H
#define DEQUE_H

#include "DeQueNode.h"
#define EMPTY (0)

//todo remove for mivnei
/*
#include <fstream>
#include <iostream>
using std::cout;
using std::ostream;
using std::endl;
*/
//todo remove for mivnei


template <typename T>
class DeQue {
protected:
    DeQueNode<T>* head;
    DeQueNode<T>* tail;
    int size;
public:

    DeQue() : head(new DeQueNode<T>()), tail(new DeQueNode<T>()), size(EMPTY) {
        //head->addInitial(this->tail);
        this->head->next = this->tail;
        this->tail->prev = this->head;
    }

    ~DeQue() {
        delete head;
    }

    int getSize() const {
        return size;
    }

    void append(T* item) {
        auto newNode = new DeQueNode<T>(item);
        this->tail->queueAdd(newNode);
        ++this->size;
    }

    void insert(T* item) {
        auto newNode = new DeQueNode<T>(item);
        this->head->stackAdd(newNode);
        ++this->size;
    }

    T* pop() {
        assert(this->head->hasNext());
        auto newNode = this->head->popNext();
        T* tempVal = newNode->getData();
        newNode->nullify();
        delete newNode;
        --this->size;
        return tempVal;
    }

    T* find(T& value) {
        auto node = this->head->find(value);
        return (node == nullptr)?nullptr:node->getData();
    }

    T* remove(T& item) { //fixme יש פה השוואת כתובות במקום השוואת ערכים
        DeQueNode<T>* temp = this->head->remove(item);
        if (temp == nullptr) {
            return nullptr;
        }
        T* newTemp = temp->extract();
        temp->verifyDeCouple(); //fixme
        temp->nullify();
        delete temp;
        --this->size;
        return newTemp;
    }

    void verifyInitialisation() {
        if(this->size != EMPTY) {
            return;
        }
        this->head->next = this->tail;
        this->tail->prev = this->head;
    }



    //todo remove for mivnei

    // Overloaded ostream operator
    friend ostream& operator<<(ostream& os, const DeQue<T>& deque) {
        cout << "linked List of size: " << deque.getSize();
        cout << " : ";
        cout << *(deque.head);
        return os;
    }

    //todo remove for mivnei


};



#endif //DEQUE_H
