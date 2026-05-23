#ifndef TODOITEM_H
#define TODOITEM_H

#include <string>
#include <ctime>
#include <stdexcept>
#include <sstream>

enum Status { NotStarted = 0, InProgress = 1, Completed = 2 };

inline std::string statusToString(int status) {
    switch (status) {
        case NotStarted: return "NotStarted";
        case InProgress: return "InProgress";
        case Completed: return "Completed";
        default: return "Unknown";
    }
}

class ToDoItem {
protected:
    std::string name;
    std::string description;
    int priority{0};
    int status{NotStarted};
    time_t dueDate{0};
public:
    ToDoItem() = default;
    ToDoItem(const std::string &n, const std::string &d, int p, time_t due):
        name(n), description(d), priority(p), dueDate(due) {}
    virtual ~ToDoItem() = default;

    const std::string &getName() const { return name; }
    void setName(const std::string &n) {
        if (n.empty()) throw std::invalid_argument("Name cannot be empty");
        name = n;
    }

    const std::string &getDescription() const { return description; }
    void setDescription(const std::string &d) {
        if (d.empty()) throw std::invalid_argument("Description cannot be empty");
        description = d;
    }

    int getPriority() const { return priority; }
    void setPriorityManual(int p) { priority = p; }

    int getStatus() const { return status; }
    void setStatus(int s) {
        if (s < NotStarted || s > Completed) throw std::invalid_argument("Invalid status value");
        status = s;
    }

    time_t getDueDate() const { return dueDate; }
    void setDueDate(time_t t) { dueDate = t; }

    virtual void setPriority(int priority) = 0;

    void dueDatePriority(){
        if (dueDate == 0) return;
        time_t now = time(nullptr);
        double days = difftime(dueDate, now) / (60.0*60.0*24.0);
        if (days <= 0) priority += 3;
        else if (days <= 1) priority += 2;
        else if (days <= 7) priority += 1;
    }

    std::string printDetails() const {
        std::ostringstream out;
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

    virtual std::string getDetails() const = 0;
};

#endif // TODOITEM_H
