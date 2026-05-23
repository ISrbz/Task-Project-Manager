#ifndef PROJECT_H
#define PROJECT_H

#include "ToDoItem.h"
#include "Task.h"
#include <vector>
#include <algorithm>

class Project : public ToDoItem {
    std::vector<Task> tasks;
public:
    Project() = default;
    Project(const std::string &n, const std::string &d, int p, time_t due): ToDoItem(n,d,p,due) {}

    void addTask(Task t){
        t.setProject(this);
        tasks.push_back(std::move(t));
        updateStatusFromTasks();
    }

    bool removeTask(const std::string &taskName){
        auto it = std::remove_if(tasks.begin(), tasks.end(), [&](const Task &t){ return t.getName() == taskName; });
        if (it == tasks.end()) return false;
        tasks.erase(it, tasks.end());
        updateStatusFromTasks();
        return true;
    }

    const std::vector<Task>& getAllTasks() const { return tasks; }

    void setPriority(int p) override {
        priority = p;
        for (auto &t : tasks){
            if (t.getPriority() < priority) t.setPriority(priority);
        }
    }

    void updateStatusFromTasks(){
        if (tasks.empty()) return;
        bool anyStarted = false;
        bool allCompleted = true;
        for (auto &t : tasks){
            if (t.getStatus() == InProgress) anyStarted = true;
            if (t.getStatus() != Completed) allCompleted = false;
        }
        if (allCompleted) status = Completed;
        else if (anyStarted) status = InProgress;
        else status = NotStarted;
    }

    std::string getDetails() const override {
        std::ostringstream out;
        out << "Type: Project\n";
        out << this->printDetails();
        return out.str();
    }
};

#endif // PROJECT_H
