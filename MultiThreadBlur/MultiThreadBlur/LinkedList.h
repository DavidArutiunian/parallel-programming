#pragma once

#define ENABLE_TESTS false
#define LINKED_LIST_TEST ENABLE_TESTS

class LinkedList
{
    struct Node
    {
        char* message;
        Node* next;
    };

public:
    LinkedList(const LinkedList&) = delete;

    LinkedList& operator=(const LinkedList&) = delete;

    LinkedList(LinkedList&&) = delete;

    LinkedList& operator=(LinkedList&&) = delete;

    LinkedList();

    ~LinkedList();

    void Add(const char* message);

    const char* Pop();

    void Clear();

    bool HasNext() const;

    std::size_t Size() const;

private:
    Node* head;
    std::size_t size;

#if LINKED_LIST_TEST

    friend int main();

#endif
};
