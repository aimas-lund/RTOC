#ifndef PROCESSINTERFACE_H
#define PROCESSINTERFACE_H

#include <QDebug>
#include <typeinfo>
#include "../lib/process.h"

class ProcessInterface {
public:
    enum class Action { Up, Down, Remove, Create, GetName };
    ProcessInterface(std::vector<ProcessBase*>* processContainer);

    std::string info(const std::string& typeName, Action action);
    const std::vector<std::string>& getProcessTypes() { return ProcessBase::get_processes(); }

private:
    std::vector<ProcessBase*>* m_container;
    void push_back(ProcessBase* p) { m_container->push_back(p); }
    void clear() { m_container->clear(); }
};

#endif  // PROCESSINTERFACE_H
