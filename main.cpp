#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <memory>
#include <limits>

using namespace std;

enum Status { NotStarted = 0, InProgress = 1, Completed = 2 };

string statusToString(int status) {
    switch (status) {
        case NotStarted: return "NotStarted";
        case InProgress: return "InProgress";
        case Completed: return "Completed";
        default: return "Unknown";
    }
}

class ToDoItem {
protected:
    string name;
    string description;
    int priority{0};
    int status{NotStarted};
    time_t dueDate{0};
public:
    ToDoItem() = default;
    ToDoItem(const string &n, const string &d, int p, time_t due):
        name(n), description(d), priority(p), dueDate(due) {}
    virtual ~ToDoItem() = default;

    // getters / setters
    const string &getName() const { return name; }
    void setName(const string &n) {
        if (n.empty()) throw invalid_argument("Name cannot be empty");
        name = n;
    }

    const string &getDescription() const { return description; }
    void setDescription(const string &d) {
        if (d.empty()) throw invalid_argument("Description cannot be empty");
        description = d;
    }

    int getPriority() const { return priority; }
    void setPriorityManual(int p) { priority = p; }

    int getStatus() const { return status; }
    void setStatus(int s) { 
        if (s < NotStarted || s > Completed) throw invalid_argument("Invalid status value");
        status = s; 
    }

    time_t getDueDate() const { return dueDate; }
    void setDueDate(time_t t) { dueDate = t; }

    // pure virtual: derived classes should implement how priority is set
    virtual void setPriority(int priority) = 0;

    // adjust priority based on due date: nearby deadlines increase priority
    void dueDatePriority(){
        if (dueDate == 0) return;
        time_t now = time(nullptr);
        double days = difftime(dueDate, now) / (60*60*24);
        if (days <= 0) priority += 3; // overdue or due today
        else if (days <= 1) priority += 2; // due tomorrow
        else if (days <= 7) priority += 1; // due this week
    }

    string getDetails() const {
        ostringstream out;
        out << "Name: " << name << "\n";
        out << "Description: " << description << "\n";
        out << "Priority: " << priority << "\n";
        out << "Status: ";
        switch(status){
            case NotStarted: out << "NotStarted"; break;
            case InProgress: out << "InProgress"; break;
            case Completed: out << "Completed"; break;
            default: out << "Unknown"; break;
        }
        out << "\n";
        if (dueDate != 0) {
            char buf[64];
            struct tm *tm = localtime(&dueDate);
            strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
            out << "Due: " << buf << "\n";
        }
        return out.str();
    }
};

class Project;

class Task : public ToDoItem {
    Project *project{nullptr};
public:
    Task() = default;
    Task(const string &n, const string &d, int p, time_t due): ToDoItem(n,d,p,due) {}

    void setProject(Project *pr) { project = pr; }
    Project* getProject() const { return project; }

    void setPriority(int p) override {
        priority = p;
        dueDatePriority();
        if (priority < 0) priority = 0;
    }
};

class Project : public ToDoItem {
    vector<Task> tasks;
public:
    Project() = default;
    Project(const string &n, const string &d, int p, time_t due): ToDoItem(n,d,p,due) {}

    void addTask(Task t){
        t.setProject(this);
        tasks.push_back(move(t));
        // update project status/priority
        updateStatusFromTasks();
    }

    bool removeTask(const string &taskName){
        auto it = remove_if(tasks.begin(), tasks.end(), [&](const Task &t){ return t.getName() == taskName; });
        if (it == tasks.end()) return false;
        tasks.erase(it, tasks.end());
        updateStatusFromTasks();
        return true;
    }

    const vector<Task>& getAllTasks() const { return tasks; }

    void setPriority(int p) override {
        priority = p;
        // also push priority to tasks if they have lower priority
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
};

class Timeline {
    vector<ToDoItem*> items;
public:
    void addItem(ToDoItem *it){ if(it) items.push_back(it); }

    vector<ToDoItem*> filterByPriority(int minPriority) const{
        vector<ToDoItem*> res;
        for (auto *it: items) if (it->getPriority() >= minPriority) res.push_back(it);
        return res;
    }

    void sortByPriorityDesc(){
        sort(items.begin(), items.end(), [](ToDoItem *a, ToDoItem *b){ return a->getPriority() > b->getPriority(); });
    }

    void sortByDueDateAsc(){
        sort(items.begin(), items.end(), [](ToDoItem *a, ToDoItem *b){ return a->getDueDate() < b->getDueDate(); });
    }

    void sortByStatusAsc(){
        sort(items.begin(), items.end(), [](ToDoItem *a, ToDoItem *b){ return a->getStatus() < b->getStatus(); });
    }

    vector<ToDoItem*> filterByStatus(int wantedStatus) const {
        vector<ToDoItem*> res;
        for (auto *it: items) if (it->getStatus() == wantedStatus) res.push_back(it);
        return res;
    }

    vector<ToDoItem*> filterByDueInDays(int days) const {
        vector<ToDoItem*> res;
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
            cout << "[" << i << "] " << items[i]->getName() << " (priority=" << items[i]->getPriority()
                 << ", status=" << statusToString(items[i]->getStatus()) << ")\n";
        }
    }

    void showAll() const{
        for (auto *it: items){
            cout << "----\n" << it->getDetails();
        }
    }
};

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
            else {
                cout << "Unknown choice.\n";
            }
        } catch (const exception &ex) {
            cout << "Error: " << ex.what() << "\n";
        }
    }

    return 0;
}