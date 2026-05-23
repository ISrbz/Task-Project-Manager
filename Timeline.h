#ifndef TIMELINE_H
#define TIMELINE_H

#include "ToDoItem.h"
#include <vector>
#include <algorithm>
#include <iostream>

class Timeline {
    std::vector<ToDoItem*> items;
public:
    void addItem(ToDoItem *it){ if(it) items.push_back(it); }

    std::vector<ToDoItem*> filterByPriority(int minPriority) const{
        std::vector<ToDoItem*> res;
        for (auto *it: items) if (it->getPriority() >= minPriority) res.push_back(it);
        return res;
    }

    void sortByPriorityDesc(){
        std::sort(items.begin(), items.end(), [](ToDoItem *a, ToDoItem *b){ return a->getPriority() > b->getPriority(); });
    }

    void sortByDueDateAsc(){
        std::sort(items.begin(), items.end(), [](ToDoItem *a, ToDoItem *b){ return a->getDueDate() < b->getDueDate(); });
    }

    void sortByStatusAsc(){
        std::sort(items.begin(), items.end(), [](ToDoItem *a, ToDoItem *b){ return a->getStatus() < b->getStatus(); });
    }

    std::vector<ToDoItem*> filterByStatus(int wantedStatus) const {
        std::vector<ToDoItem*> res;
        for (auto *it: items) if (it->getStatus() == wantedStatus) res.push_back(it);
        return res;
    }

    std::vector<ToDoItem*> filterByDueInDays(int days) const {
        std::vector<ToDoItem*> res;
        time_t now = time(nullptr);
        time_t limit = now + static_cast<time_t>(days) * 24 * 60 * 60;
        for (auto *it: items) if (it->getDueDate() <= limit) res.push_back(it);
        return res;
    }

    size_t size() const { return items.size(); }

    ToDoItem* getItem(size_t idx) const {
        if (idx >= items.size()) return nullptr;
        return items[idx];
    }

    void showIndexed() const {
        for (size_t i = 0; i < items.size(); ++i) {
            std::cout << "[" << i << "] " << items[i]->getName() << " (priority=" << items[i]->getPriority()
                 << ", status=" << statusToString(items[i]->getStatus()) << ")\n";
        }
    }

    void showAll() const{
        for (auto *it: items){
            std::cout << "----\n" << it->getDetails();
        }
    }
};

#endif // TIMELINE_H
