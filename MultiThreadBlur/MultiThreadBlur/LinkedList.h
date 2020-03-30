#pragma once

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
};
