#include <cstring>
#include <cstddef>
#include <cstdlib>

#include "catch.hpp"
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

SCENARIO("LinkedList")
{
    GIVEN("An empty LinkedList")
    {
        LinkedList list;

        REQUIRE(list.Size() == 0);

        WHEN("Message has been added")
        {
            list.Add("Hello, World!");

            THEN("Size is 1")
            {
                REQUIRE(list.Size() == 1);
            }

            AND_THEN("HasNext is true")
            {
                REQUIRE(list.HasNext() == true);
            }

            AND_WHEN("Message has been popped")
            {
                const char* message = list.Pop();

                THEN("Message equals to \"Hello, World!\"")
                {
                    REQUIRE(strcmp(message, "Hello, World!") == 0);
                }

                AND_THEN("Size is 0")
                {
                    REQUIRE(list.Size() == 0);
                }

                AND_THEN("HasNext is false")
                {
                    REQUIRE(list.HasNext() == false);
                }
            }
        }
    }

    AND_GIVEN("LinkedList of 5 elements")
    {
        LinkedList list;

        for (std::size_t i = 0; i < 5; i++)
        {
            list.Add(std::to_string(i).c_str());
        }


        WHEN("First pop")
        {
            const char* message = list.Pop();

            THEN("Message equals to \"4\"")
            {
                REQUIRE(strcmp(message, "4") == 0);
            }

            AND_THEN("Size is 4")
            {
                REQUIRE(list.Size() == 4);
            }

            AND_THEN("HasNext is true")
            {
                REQUIRE(list.HasNext() == true);
            }

            AND_WHEN("Second pop")
            {
                const char* message = list.Pop();

                THEN("Message equals to \"3\"")
                {
                    REQUIRE(strcmp(message, "3") == 0);
                }

                AND_THEN("Size is 3")
                {
                    REQUIRE(list.Size() == 3);
                }

                AND_THEN("HasNext is true")
                {
                    REQUIRE(list.HasNext() == true);
                }

                AND_WHEN("Third pop")
                {
                    const char* message = list.Pop();

                    THEN("Message equals to  \"2\"")
                    {
                        REQUIRE(strcmp(message, "2") == 0);
                    }

                    AND_THEN("Size is 2")
                    {
                        REQUIRE(list.Size() == 2);
                    }

                    AND_THEN("HasNext is true")
                    {
                        REQUIRE(list.HasNext() == true);
                    }

                    AND_WHEN("Fourth pop")
                    {
                        const char* message = list.Pop();

                        THEN("Message equals to \"1\"")
                        {
                            REQUIRE(strcmp(message, "1") == 0);
                        }

                        AND_THEN("Size is 1")
                        {
                            REQUIRE(list.Size() == 1);
                        }

                        AND_THEN("HasNext is true")
                        {
                            REQUIRE(list.HasNext() == true);
                        }

                        AND_WHEN("Fifth pop")
                        {
                            const char* message = list.Pop();

                            THEN("Message equals to  \"0\"")
                            {
                                REQUIRE(strcmp(message, "0") == 0);
                            }

                            AND_THEN("Size is 0")
                            {
                                REQUIRE(list.Size() == 0);
                            }

                            AND_THEN("HasNext is false")
                            {
                                REQUIRE(list.HasNext() == false);
                            }
                        }
                    }
                }
            }
        }
    }

    AND_GIVEN("LinkedList of 5 elements")
    {
        LinkedList list;

        for (std::size_t i = 0; i < 5; i++)
        {
            list.Add(std::to_string(i).c_str());
        }

        WHEN("List has been cleared")
        {
            list.Clear();

            THEN("List size is 0")
            {
                REQUIRE(list.Size() == 0);
            }

            AND_THEN("HasNext is false")
            {
                REQUIRE(list.HasNext() == false);
            }
        }
    }
}
