#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <stdexcept>
#include <algorithm>
#include <sstream>

using namespace std;

enum Status { NotStarted = 0, InProgress = 1, Completed = 2 };

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

class Project; // forward

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
    vector<ToDoItem*> items; // owns? not owning here, just references
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

    void showAll() const{
        for (auto *it: items){
            cout << "----\n" << it->getDetails();
        }
    }
};

int main(){
    // create a project due in 5 days
    time_t now = time(nullptr);
    time_t in5 = now + 5*24*60*60;
    Project proj("Website", "Build marketing website", 1, in5);

    // tasks
    Task t1("Design", "Create mockups", 1, now + 2*24*60*60);
    Task t2("Implementation", "Implement pages", 1, now + 6*24*60*60);
    t1.setStatus(InProgress);
    t2.setStatus(NotStarted);

    proj.addTask(t1);
    proj.addTask(t2);

    // project priority influenced by tasks
    proj.setPriority(2);

    // timeline
    Timeline tl;
    tl.addItem(&proj);

    // standalone quick task due today
    Task quick("Hotfix", "Fix critical bug", 3, now);
    quick.setStatus(InProgress);
    tl.addItem(&quick);

    cout << "All items:\n";
    tl.showAll();

    cout << "\nSorted by priority:\n";
    tl.sortByPriorityDesc();
    tl.showAll();

    cout << "\nFiltered (priority >= 2):\n";
    auto filtered = tl.filterByPriority(2);
    for (auto *it: filtered) cout << it->getDetails() << "----\n";

    return 0;
}