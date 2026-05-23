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
time_t makeDueDateFromDays(int daysFromNow) {
    return time(nullptr) + static_cast<time_t>(daysFromNow) * 24 * 60 * 60;
}

void printMenu() {
    cout << "\n=== Task & Project Manager ===\n";
    cout << "1. New task\n";
    cout << "2. New project\n";
    cout << "3. Add existing task to project\n";
    cout << "4. Check project status\n";
    cout << "5. Filter\n";
    cout << "6. Sort\n";
    cout << "7. Show all (timeline)\n";
    cout << "8. Show details by index\n";
    cout << "9. Change task status\n";
    cout << "q. Quit\n";
    cout << "Choice: ";
}

string readLine(const string &prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

int readInt(const string &prompt) {
    while (true) {
        cout << prompt;
        string line;
        getline(cin, line);
        stringstream ss(line);
        int value;
        if (ss >> value && ss.eof()) return value;
        cout << "Invalid number. Try again.\n";
    }
}

int main(){
    Timeline timeline;
    vector<unique_ptr<Task>> allTasks;
    vector<unique_ptr<Project>> allProjects;

    while (true) {
        printMenu();
        string choice;
        getline(cin, choice);

        if (choice == "q" || choice == "Q") {
            cout << "Goodbye!\n";
            break;
        }

        try {
            if (choice == "1") {
                auto task = make_unique<Task>();
                task->setName(readLine("Task name: "));
                task->setDescription(readLine("Task description: "));
                int priority = readInt("Task priority (0+): ");
                int status = readInt("Task status (0=NotStarted, 1=InProgress, 2=Completed): ");
                int days = readInt("Due in how many days (0=today): ");

                task->setPriority(priority);
                task->setStatus(status);
                task->setDueDate(makeDueDateFromDays(days));

                timeline.addItem(task.get());
                allTasks.push_back(move(task));
                cout << "Task created and placed on timeline.\n";
            }
            else if (choice == "2") {
                auto project = make_unique<Project>();
                project->setName(readLine("Project name: "));
                project->setDescription(readLine("Project description: "));
                int priority = readInt("Project priority (0+): ");
                int days = readInt("Due in how many days (0=today): ");

                project->setPriority(priority);
                project->setDueDate(makeDueDateFromDays(days));

                timeline.addItem(project.get());
                allProjects.push_back(move(project));
                cout << "Project created and placed on timeline.\n";
            }
            else if (choice == "3") {
                if (allTasks.empty() || allProjects.empty()) {
                    cout << "Need at least one task and one project first.\n";
                    continue;
                }

                cout << "Tasks:\n";
                for (size_t i = 0; i < allTasks.size(); ++i) {
                    cout << "[" << i << "] " << allTasks[i]->getName() << "\n";
                }
                size_t taskIdx = static_cast<size_t>(readInt("Choose task index: "));

                cout << "Projects:\n";
                for (size_t i = 0; i < allProjects.size(); ++i) {
                    cout << "[" << i << "] " << allProjects[i]->getName() << "\n";
                }
                size_t projIdx = static_cast<size_t>(readInt("Choose project index: "));

                if (taskIdx >= allTasks.size() || projIdx >= allProjects.size()) {
                    cout << "Invalid index.\n";
                    continue;
                }

                allTasks[taskIdx]->setProject(allProjects[projIdx].get());
                allProjects[projIdx]->addTask(*allTasks[taskIdx]);
                cout << "Task added to project.\n";
            }
            else if (choice == "4") {
                if (allProjects.empty()) {
                    cout << "No projects available.\n";
                    continue;
                }

                for (size_t i = 0; i < allProjects.size(); ++i) {
                    cout << "[" << i << "] " << allProjects[i]->getName() << "\n";
                }
                size_t projIdx = static_cast<size_t>(readInt("Choose project index: "));
                if (projIdx >= allProjects.size()) {
                    cout << "Invalid index.\n";
                    continue;
                }

                allProjects[projIdx]->updateStatusFromTasks();
                cout << "Project status: " << statusToString(allProjects[projIdx]->getStatus()) << "\n";
            }
            else if (choice == "5") {
                cout << "Filter by:\n";
                cout << "1. Priority (>= value)\n";
                cout << "2. Status\n";
                cout << "3. Due in next N days\n";
                cout << "4. Project name (tasks only)\n";
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
                    cout << "Invalid filter.\n";
                    continue;
                }

                cout << "Filtered results: " << filtered.size() << "\n";
                for (auto *it : filtered) {
                    cout << "----\n" << it->getDetails();
                }
            }
            else if (choice == "6") {
                cout << "Sort by:\n";
                cout << "1. Priority\n";
                cout << "2. Due date\n";
                cout << "3. Status\n";
                string s = readLine("Choose sort: ");

                if (s == "1") timeline.sortByPriorityDesc();
                else if (s == "2") timeline.sortByDueDateAsc();
                else if (s == "3") timeline.sortByStatusAsc();
                else {
                    cout << "Invalid sort.\n";
                    continue;
                }

                cout << "Sorted timeline:\n";
                timeline.showAll();
            }
            else if (choice == "7") {
                if (timeline.size() == 0) {
                    cout << "Timeline is empty.\n";
                    continue;
                }
                timeline.showAll();
            }
            else if (choice == "8") {
                if (timeline.size() == 0) {
                    cout << "Timeline is empty.\n";
                    continue;
                }
                timeline.showIndexed();
                size_t idx = static_cast<size_t>(readInt("Item index: "));
                ToDoItem *item = timeline.getItem(idx);
                if (!item) {
                    cout << "Invalid index.\n";
                    continue;
                }
                cout << "----\n" << item->getDetails();
            }
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
                    cout << "No tasks on the timeline.\n";
                    continue;
                }

                cout << "Tasks on timeline:\n";
                for (size_t idx = 0; idx < taskTimelineIndices.size(); ++idx) {
                    ToDoItem *it = timeline.getItem(taskTimelineIndices[idx]);
                    cout << "[" << idx << "] " << it->getName() << " (status=" << statusToString(it->getStatus()) << ")\n";
                }

                size_t pick = static_cast<size_t>(readInt("Choose task number: "));
                if (pick >= taskTimelineIndices.size()) {
                    cout << "Invalid choice.\n";
                    continue;
                }

                ToDoItem *chosen = timeline.getItem(taskTimelineIndices[pick]);
                Task *task = dynamic_cast<Task*>(chosen);
                if (!task) {
                    cout << "Selected item is not a task.\n";
                    continue;
                }

                int newStatus = readInt("New status (0=NotStarted,1=InProgress,2=Completed): ");
                task->setStatus(newStatus);
                if (task->getProject() != nullptr) task->getProject()->updateStatusFromTasks();
                cout << "Task status updated.\n";
            }
            else {
                cout << "Unknown choice.\n";
            }
        } catch (const exception &ex) {
            cout << "Error: " << ex.what() << "\n";
        }
    }

    return 0;
}