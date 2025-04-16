//
// Created by Guy Friedman on 24/01/2025.
//

#ifndef PAIR_H
#define PAIR_H

#include <fstream>

constexpr int DEFAULT_KEY = 0;

template <typename T>
class Pair {
public:
    int key;
    T* value;

    Pair(int key, T* value) : Pair(key) {
        this->value = value;
    }

    Pair(int key) : Pair() {
        this->key = key;
    }

    Pair() : key(DEFAULT_KEY), value(nullptr) {}

    ~Pair() {
        delete this->value;
    }

    T* extract() {
        auto value = this->value;
        this->value = nullptr;
        return value;
    }

    void nullify() {
        this->key = DEFAULT_KEY;
        this->value = nullptr;
    }


    bool operator ==(const Pair& other) const {
        return this->key == other.key;
    }

    // Overloaded operator== as a member function
    //bool operator==(int otherKey) const {return this->key == otherKey;}

    // Friend operator== (int == Pair<int>)
    friend bool operator==(int lhs, const Pair& rhs) {
        return rhs == lhs;
    }

    // Declare the friend function
    friend std::ostream& operator<<(std::ostream& os, const Pair& pair) {
        os << "Key: " << pair.key;
        os << ", Value: ";
        if (pair.value) {
            os << *(pair.value);
        } else {
            os << "nullptr";
        }
        os << "." << std::endl;
        return os;
    }


};



#endif //PAIR_H
