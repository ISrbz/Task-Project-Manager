#ifndef TIMELINE_H
#define TIMELINE_H

#include "ToDoItem.h"
#include "Task.h"
#include "Project.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <unordered_map>

class Timeline {
    std::vector<ToDoItem*> items;
    static std::vector<std::string> splitFields(const std::string &line) {
        std::vector<std::string> fields;
        size_t start = 0;
        while (start <= line.size()) {
            size_t pos = line.find('|', start);
            if (pos == std::string::npos) {
                fields.push_back(line.substr(start));
                break;
            }
            fields.push_back(line.substr(start, pos - start));
            start = pos + 1;
            if (start == line.size()) {
                fields.emplace_back("");
                break;
            }
        }
        return fields;
    }
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

    void writeToFile(const std::string& filePath){
        std::ofstream fs(filePath, std::ios::out);
        if(!fs.is_open()) throw std::runtime_error("Couldn't open the file: " + filePath);
        for(auto i : this->items) fs << *i << "\n";
    }

    void readFromFile(const std::string& filePath){
        std::ifstream fs(filePath, std::ios::in);
        if(!fs.is_open()) throw std::runtime_error("Couldn't open the file: " + filePath);

        items.clear();

        std::unordered_map<std::string, Project*> projectsByName;
        std::unordered_map<std::string, Task*> tasksByName;
        std::unordered_map<std::string, std::string> taskProjectNames;
        std::unordered_map<std::string, std::vector<std::string>> projectTaskNames;

        std::string ln;
        while(std::getline(fs, ln)){
            if (ln.empty()) continue;

            auto fields = splitFields(ln);
            if (fields.size() < 6) {
                throw std::runtime_error("invalid stored item format");
            }

            std::unique_ptr<ToDoItem> item;
            switch(fields[0].empty() ? '\0' : fields[0][0]){
                case 'p': item = std::make_unique<Project>(); break;
                case 't': item = std::make_unique<Task>(); break;
                default: throw std::runtime_error("stored item neither project nor task");
            }

            item->setName(fields[1]);
            item->setDescription(fields[2]);
            item->setPriorityManual(std::stoi(fields[3]));
            item->setStatus(std::stoi(fields[4]));
            item->setDueDate(static_cast<time_t>(std::stoll(fields[5])));

            if (auto *task = dynamic_cast<Task*>(item.get())) {
                if (fields.size() >= 7) {
                    task->setProjectName(fields[6]);
                    taskProjectNames[task->getName()] = fields[6];
                }
            } else if (auto *project = dynamic_cast<Project*>(item.get())) {
                std::vector<std::string> taskNames;
                if (fields.size() >= 7) {
                    int storedCount = 0;
                    try {
                        storedCount = std::stoi(fields[6]);
                    } catch (...) {
                        throw std::runtime_error("failed to parse stored project tasks");
                    }
                    for (size_t i = 7; i < fields.size(); ++i) {
                        taskNames.push_back(fields[i]);
                    }
                    if (storedCount >= 0 && static_cast<size_t>(storedCount) != taskNames.size()) {
                        // Keep the stored names even if the count is off; the names are the source of truth here.
                    }
                }
                projectTaskNames[project->getName()] = std::move(taskNames);
            }

            ToDoItem *rawItem = item.release();
            items.push_back(rawItem);

            if (auto *task = dynamic_cast<Task*>(rawItem)) {
                tasksByName[task->getName()] = task;
            } else if (auto *project = dynamic_cast<Project*>(rawItem)) {
                projectsByName[project->getName()] = project;
            }
        }

        for (auto &[taskName, task] : tasksByName) {
            auto projectNameIt = taskProjectNames.find(taskName);
            if (projectNameIt == taskProjectNames.end() || projectNameIt->second.empty()) {
                continue;
            }

            auto projectIt = projectsByName.find(projectNameIt->second);
            if (projectIt != projectsByName.end()) {
                task->setProject(projectIt->second);
            }
        }

        for (auto &[projectName, project] : projectsByName) {
            auto taskNamesIt = projectTaskNames.find(projectName);
            if (taskNamesIt != projectTaskNames.end() && !taskNamesIt->second.empty()) {
                for (const auto &taskName : taskNamesIt->second) {
                    auto taskIt = tasksByName.find(taskName);
                    if (taskIt != tasksByName.end()) {
                        project->addTask(*taskIt->second);
                    }
                }
                continue;
            }

            for (auto &[taskName, task] : tasksByName) {
                if (task->getProject() == project) {
                    project->addTask(*task);
                }
            }
        }
    }
};

#endif // TIMELINE_H
