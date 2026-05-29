#ifndef TASK_H
#define TASK_H

#include "ToDoItem.h"

class Project;

class Task : public ToDoItem {
    Project *project{nullptr};
    std::string projectName;
protected:
    char t{'t'};
public:
    Task() = default;
    Task(const std::string &n, const std::string &d, int p, time_t due): ToDoItem(n,d,p,due) {}

    char typeTag() const override { return t; }

    void setProject(Project *pr);
    Project* getProject() const { return project; }
    const std::string &getProjectName() const { return projectName; }
    void setProjectName(const std::string &name) { projectName = name; }

    void setPriority(int p) override {
        priority = p;
        dueDatePriority();
        if (priority < 0) priority = 0;
    }

    void writeExtraFields(std::ostream &out) const override {
        out << "|" << projectName;
    }

    std::string getDetails() const override {
        std::ostringstream out;
        out << "Type: Task\n";
        if (!projectName.empty()) {
            out << "Project: " << projectName << "\n";
        }
        out << this->printDetails();
        return out.str();
    }
};

#endif // TASK_H
