#ifndef PROJECT_H
#define PROJECT_H

#include "ToDoItem.h"
#include "Task.h"
#include <vector>
#include <algorithm>

class Project : public ToDoItem {
    std::vector<Task*> tasks;
protected:
    char t{'p'};
public:
    Project() = default;
    Project(const std::string &n, const std::string &d, int p, time_t due): ToDoItem(n,d,p,due) {}

    char typeTag() const override { return t; }

    void addTask(Task &task){
        Project *currentProject = task.getProject();
        if (currentProject != nullptr && currentProject != this) {
            currentProject->removeTask(task.getName());
        }

        task.setProject(this);

        if (std::find(tasks.begin(), tasks.end(), &task) == tasks.end()) {
            tasks.push_back(&task);
        }

        updateStatusFromTasks();
    }

    bool removeTask(const std::string &taskName){
        auto it = std::remove_if(tasks.begin(), tasks.end(), [&](Task *task){
            if (task == nullptr || task->getName() != taskName) return false;
            if (task->getProject() == this) {
                task->setProject(nullptr);
            }
            return true;
        });
        if (it == tasks.end()) return false;
        tasks.erase(it, tasks.end());
        updateStatusFromTasks();
        return true;
    }

    const std::vector<Task*>& getAllTasks() const { return tasks; }

    void setPriority(int p) override {
        priority = p;
        for (auto *task : tasks){
            if (task != nullptr && task->getPriority() < priority) task->setPriority(priority);
        }
    }

    void updateStatusFromTasks(){
        if (tasks.empty()) {
            status = NotStarted;
            return;
        }
        bool anyStarted = false;
        bool allCompleted = true;
        for (auto *task : tasks){
            if (task == nullptr) continue;
            if (task->getStatus() == InProgress) anyStarted = true;
            if (task->getStatus() != Completed) allCompleted = false;
        }
        if (allCompleted) status = Completed;
        else if (anyStarted) status = InProgress;
        else status = NotStarted;
    }

    void writeExtraFields(std::ostream &out) const override {
        out << "|" << tasks.size();
        for (auto *task : tasks) {
            out << "|" << (task != nullptr ? task->getName() : "");
        }
    }

    std::string getDetails() const override {
        std::ostringstream out;
        out << "Type: " << colorize("Project", ansiCyan()) << "\n";
        out << "Tasks: " << tasks.size() << "\n";
        out << this->printDetails();
        return out.str();
    }
};

inline void Task::setProject(Project *pr) {
    project = pr;
    projectName = pr != nullptr ? pr->getName() : "";
}

#endif // PROJECT_H
