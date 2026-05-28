#ifndef TASK_H
#define TASK_H

#include "ToDoItem.h"

class Project;

class Task : public ToDoItem {
    Project *project{nullptr};
protected:
    char t{'t'};
public:
    Task() = default;
    Task(const std::string &n, const std::string &d, int p, time_t due): ToDoItem(n,d,p,due) {}

    char typeTag() const override { return t; }

    void setProject(Project *pr) { project = pr; }
    Project* getProject() const { return project; }

    void setPriority(int p) override {
        priority = p;
        dueDatePriority();
        if (priority < 0) priority = 0;
    }

    std::string getDetails() const override {
        std::ostringstream out;
        out << "Type: Task\n";
        out << this->printDetails();
        return out.str();
    }
};

#endif // TASK_H
