#include <cstring>
#include <cstddef>
#include <cassert>
#include <cstdlib>

#include "LinkedList.h"

LinkedList::LinkedList(): head(nullptr), size(0)
{
}

LinkedList::~LinkedList()
{
    Clear();
}

void LinkedList::Add(const char* message)
{
    const auto new_node = new Node();
    const auto memory_size = strlen(message) * sizeof(char);
    new_node->message = static_cast<char*>(malloc(memory_size));
    strcpy(new_node->message, message);
    new_node->next = head;
    head = new_node;
    size++;
}

const char* LinkedList::Pop()
{
    Node* node = head;
    const char* result = node->message;
    head = node->next;
    delete node;
    size--;
    return result;
}

void LinkedList::Clear()
{
    Node* next = head;
    while (next)
    {
        Node* node_to_delete = next;
        next = next->next;
        delete node_to_delete;
    }
    head = nullptr;
    size = 0;
}

bool LinkedList::HasNext() const
{
    return head != nullptr;
}

std::size_t LinkedList::Size() const
{
    return size;
}

#if LINKED_LIST_TEST

int main()
{
    LinkedList list;

    assert(list.head == nullptr);
    assert(list.size == 0);

    list.Add("Hello, World!");

    assert(strcmp(list.head->message, "Hello, World!") == 0);
    assert(list.head->next == nullptr);
    assert(list.size == 1);

    const auto* prev = list.head;

    list.Add("Who am I?");

    assert(strcmp(list.head->message, "Who am I?") == 0);
    assert(list.head->next == prev);
    assert(list.size == 2);

    assert(strcmp(list.Pop(), "Who am I?") == 0);
    assert(list.size == 1);

    assert(strcmp(list.Pop(), "Hello, World!") == 0);
    assert(list.size == 0);

    list.Add("Hello, World!");
    list.Add("Who am I?");
    list.Clear();

    assert(list.size == 0);
    assert(list.head == nullptr);

    for (int i = 0; i < 5; i++)
    {
        list.Add(std::to_string(i).c_str());
    }
    assert(strcmp(list.Pop(), "4") == 0);
    assert(strcmp(list.Pop(), "3") == 0);
    assert(strcmp(list.Pop(), "2") == 0);
    assert(strcmp(list.Pop(), "1") == 0);
    assert(strcmp(list.Pop(), "0") == 0);

    assert(list.HasNext() == false);

    list.Add("Hello, World!");
    assert(list.HasNext() == true);

    list.Pop();
    assert(list.HasNext() == false);

    return EXIT_SUCCESS;
}

#endif
