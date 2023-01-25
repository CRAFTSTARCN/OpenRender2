#pragma once
#include <stack>
#include <unordered_map>

template <typename T>
class GUIDAllocator
{
    size_t IdStart;
    size_t Current;
    std::stack<size_t> Reuse;

    std::unordered_map<T, size_t> Table;
    std::unordered_map<size_t, T> ReverseTable;

    size_t GetNew()
    {
        if(Reuse.empty())
        {
            return Current++;
        }
        
        size_t Ans = Reuse.top();
        Reuse.pop();
        return Ans;
    }
    
public:

    GUIDAllocator(size_t Start) : IdStart(Start), Current(Start)
    {
        
    }

    /*
     * Return pair of id and if it's new allocated
     */
    auto GetOrAllocateNew(const T& Entity) 
    {
        auto Iter = Table.find(Entity);
        
        if(Iter != Table.end())
        {
            return std::make_pair(Iter->second, false);
        }

        size_t N = GetNew();
        Table.emplace(Entity, N);
        ReverseTable.emplace(N, Entity);
        return std::make_pair(N, true);
    }

    /*
     * Only return id, don't care if it's new
     */
    size_t GetAllocateNew(const T& Entity)
    {
        auto &&[Id, New] = GetOrAllocateNew(Entity);
        return Id;
    }

    bool HasElement(const T& Entity)
    {
        auto Iter = Table.find(Entity);
        return Iter != Table.end();
    }

    auto TryGetElement(const T& Entity)
    {
        auto Iter = Table.find(Entity);
        if(Iter != Table.end())
        {
            return std::make_pair(Iter->second, true);
        }
        return std::make_pair(SIZE_MAX, false);
    }

    void Deallocate(size_t Id)
    {
        auto Iter = ReverseTable.find(Id);
        if(Iter == ReverseTable.end())
        {
            return;
        }

        Table.erase(Iter->second);
        ReverseTable.erase(Iter);

        Reuse.push(Id);
    }

    void DeallocateAll()
    {
        Current = IdStart;
        std::stack<size_t>().swap(Reuse);
        Table.clear();
        ReverseTable.clear();
    }
    
};
