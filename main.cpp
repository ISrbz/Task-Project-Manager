#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include <ctime>

#include "ToDoItem.h"
#include "Task.h"
#include "Project.h"
#include "Timeline.h"

using namespace std;
time_t makeDueDateFromDaysHours(int daysFromNow, int hourOfDay) {
    time_t now = time(nullptr);
    std::tm tm = *std::localtime(&now);
    tm.tm_mday += daysFromNow; // advance days
    tm.tm_hour = hourOfDay;    // set clock hour on that date (0-23)
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return std::mktime(&tm);
}

// Backwards-compatible helper
time_t makeDueDateFromDays(int daysFromNow) {
    return makeDueDateFromDaysHours(daysFromNow, 0);
}

void printMenu() {
    std::cout << "\n=== Task & Project Manager ===\n";
    std::cout << "1. New task\n";
    std::cout << "2. New project\n";
    std::cout << "3. Add existing task to project\n";
    std::cout << "4. Check project status\n";
    std::cout << "5. Filter\n";
    std::cout << "6. Sort\n";
    std::cout << "7. Show all (timeline)\n";
    std::cout << "8. Show details by index\n";
    std::cout << "9. Change task status\n";
    std::cout << "q. Quit\n";
    std::cout << "Choice: ";
}

string readLine(const string &prompt) {
    std::cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

int readInt(const string &prompt) {
    while (true) {
        std::cout << prompt;
        string line;
        getline(cin, line);
        stringstream ss(line);
        int value;
        if (ss >> value && ss.eof()) return value;
        std::cout << "Invalid number. Try again.\n";
    }
}

int main(){
    Timeline timeline;
    vector<unique_ptr<Task>> allTasks;
    vector<unique_ptr<Project>> allProjects;
    timeline.readFromFile("storage.txt");

    while (true) {
        printMenu();
        string choice;
        getline(cin, choice);

        if (choice == "q" || choice == "Q") {
            std::cout << "Goodbye!\n";
            break;
        }

        try {
            //new task
            if (choice == "1") {
                auto task = make_unique<Task>();
                task->setName(readLine("Task name: "));
                task->setDescription(readLine("Task description: "));
                int priority = readInt("Task priority (0+): ");
                int status = readInt("Task status (0=NotStarted, 1=InProgress, 2=Completed): ");
                int days = readInt("Due in how many days (0=today): ");
                int hours = readInt("Due at what time (0-23): ");
                while (hours < 0 || hours > 23) {
                    std::cout << "Invalid hours. Enter a value between 0 and 23.\n";
                    hours = readInt("Due at what time (0-23): ");
                }

                task->setPriority(priority);
                task->setStatus(status);
                task->setDueDate(makeDueDateFromDaysHours(days, hours));

                timeline.addItem(task.get());
                allTasks.push_back(move(task));
                timeline.writeToFile("storage.txt");
                std::cout << "Task created and placed on timeline.\n";
            }
            //new project
            else if (choice == "2") {
                auto project = make_unique<Project>();
                project->setName(readLine("Project name: "));
                project->setDescription(readLine("Project description: "));
                int priority = readInt("Project priority (0+): ");
                int days = readInt("Due in how many days (0=today): ");
                int hours = readInt("Additional hours (0-23): ");
                while (hours < 0 || hours > 23) {
                    std::cout << "Invalid hours. Enter a value between 0 and 23.\n";
                    hours = readInt("Additional hours (0-23): ");
                }

                project->setPriority(priority);
                project->setDueDate(makeDueDateFromDaysHours(days, hours));

                timeline.addItem(project.get());
                allProjects.push_back(move(project));
                timeline.writeToFile("storage.txt");
                std::cout << "Project created and placed on timeline.\n";
            }
            //add task to project
            else if (choice == "3") {
                if (allTasks.empty() || allProjects.empty()) {
                    std::cout << "Need at least one task and one project first.\n";
                    continue;
                }

                std::cout << "Tasks:\n";
                for (size_t i = 0; i < allTasks.size(); ++i) {
                    std::cout << "[" << i << "] " << allTasks[i]->getName() << "\n";
                }
                size_t taskIdx = static_cast<size_t>(readInt("Choose task index: "));

                std::cout << "Projects:\n";
                for (size_t i = 0; i < allProjects.size(); ++i) {
                    std::cout << "[" << i << "] " << allProjects[i]->getName() << "\n";
                }
                size_t projIdx = static_cast<size_t>(readInt("Choose project index: "));

                if (taskIdx >= allTasks.size() || projIdx >= allProjects.size()) {
                    std::cout << "Invalid index.\n";
                    continue;
                }

                allTasks[taskIdx]->setProject(allProjects[projIdx].get());
                allProjects[projIdx]->addTask(*allTasks[taskIdx]);
                timeline.writeToFile("storage.txt");
                std::cout << "Task added to project.\n";
            }
            //check project status
            else if (choice == "4") {
                if (allProjects.empty()) {
                    std::cout << "No projects available.\n";
                    continue;
                }

                for (size_t i = 0; i < allProjects.size(); ++i) {
                    std::cout << "[" << i << "] " << allProjects[i]->getName() << "\n";
                }
                size_t projIdx = static_cast<size_t>(readInt("Choose project index: "));
                if (projIdx >= allProjects.size()) {
                    std::cout << "Invalid index.\n";
                    continue;
                }

                allProjects[projIdx]->updateStatusFromTasks();
                std::cout << "Project status: " << statusToString(allProjects[projIdx]->getStatus()) << "\n";
            }
            //filter
            else if (choice == "5") {
                std::cout << "Filter by:\n";
                std::cout << "1. Priority (>= value)\n";
                std::cout << "2. Status\n";
                std::cout << "3. Due in next N days\n";
                std::cout << "4. Project name (tasks only)\n";
                string f = readLine("Choose filter: ");

                vector<ToDoItem*> filtered;
                if (f == "1") {
                    int minP = readInt("Min priority: ");
                    filtered = timeline.filterByPriority(minP);
                } else if (f == "2") {
                    int s = readInt("Status (0/1/2): ");
                    filtered = timeline.filterByStatus(s);
                } else if (f == "3") {
                    int d = readInt("Days: ");
                    filtered = timeline.filterByDueInDays(d);
                } else if (f == "4") {
                    string projectName = readLine("Project name: ");
                    for (size_t i = 0; i < timeline.size(); ++i) {
                        ToDoItem *item = timeline.getItem(i);
                        Task *task = dynamic_cast<Task*>(item);
                        if (task != nullptr && task->getProject() != nullptr && task->getProject()->getName() == projectName) {
                            filtered.push_back(task);
                        }
                    }
                } else {
                    std::cout << "Invalid filter.\n";
                    continue;
                }

                std::cout << "Filtered results: " << filtered.size() << "\n";
                for (auto *it : filtered) {
                    std::cout << "----\n" << it->getDetails();
                }
            }
            //sort
            else if (choice == "6") {
                std::cout << "Sort by:\n";
                std::cout << "1. Priority\n";
                std::cout << "2. Due date\n";
                std::cout << "3. Status\n";
                string s = readLine("Choose sort: ");

                if (s == "1") timeline.sortByPriorityDesc();
                else if (s == "2") timeline.sortByDueDateAsc();
                else if (s == "3") timeline.sortByStatusAsc();
                else {
                    std::cout << "Invalid sort.\n";
                    continue;
                }

                std::cout << "Sorted timeline:\n";
                timeline.showAll();
            }
            //show all
            else if (choice == "7") {
                if (timeline.size() == 0) {
                    std::cout << "Timeline is empty.\n";
                    continue;
                }
                timeline.showAll();
            }
            //get details
            else if (choice == "8") {
                if (timeline.size() == 0) {
                    std::cout << "Timeline is empty.\n";
                    continue;
                }
                timeline.showIndexed();
                size_t idx = static_cast<size_t>(readInt("Item index: "));
                ToDoItem *item = timeline.getItem(idx);
                if (!item) {
                    std::cout << "Invalid index.\n";
                    continue;
                }
                std::cout << "----\n" << item->getDetails();
            }
            //change status
            else if (choice == "9") {
                // list tasks from timeline with compact indices
                vector<size_t> taskTimelineIndices;
                for (size_t i = 0; i < timeline.size(); ++i) {
                    ToDoItem *it = timeline.getItem(i);
                    if (dynamic_cast<Task*>(it) != nullptr) {
                        taskTimelineIndices.push_back(i);
                    }
                }
                if (taskTimelineIndices.empty()) {
                    std::cout << "No tasks on the timeline.\n";
                    continue;
                }

                std::cout << "Tasks on timeline:\n";
                for (size_t idx = 0; idx < taskTimelineIndices.size(); ++idx) {
                    ToDoItem *it = timeline.getItem(taskTimelineIndices[idx]);
                    std::cout << "[" << idx << "] " << it->getName() << " (status=" << statusToString(it->getStatus()) << ")\n";
                }

                size_t pick = static_cast<size_t>(readInt("Choose task number: "));
                if (pick >= taskTimelineIndices.size()) {
                    std::cout << "Invalid choice.\n";
                    continue;
                }

                ToDoItem *chosen = timeline.getItem(taskTimelineIndices[pick]);
                Task *task = dynamic_cast<Task*>(chosen);
                if (!task) {
                    std::cout << "Selected item is not a task.\n";
                    continue;
                }

                int newStatus = readInt("New status (0=NotStarted,1=InProgress,2=Completed): ");
                task->setStatus(newStatus);
                if (task->getProject() != nullptr) task->getProject()->updateStatusFromTasks();
                timeline.writeToFile("storage.txt");
                std::cout << "Task status updated.\n";
            }
            else {
                std::cout << "Unknown choice.\n";
            }
        } catch (const exception &ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    }

    return 0;
}