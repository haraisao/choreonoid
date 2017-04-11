/*!
  @author Shizuko Hattori
  @author Shin'ichiro Nakaoka
 */

#include "../Task.h"
#include "../AbstractTaskSequencer.h"
#include "../ValueTree.h"
#include "PyUtil.h"
#include <cnoid/PythonUtil>
#include <boost/ref.hpp>
#include <pybind11/stl.h>
#include <set>
#include <map>

using namespace std;
namespace py = pybind11;
using namespace cnoid;

// for MSVC++2015 Update3
CNOID_PYTHON_DEFINE_GET_POINTER(TaskProc)
CNOID_PYTHON_DEFINE_GET_POINTER(TaskMenu)
CNOID_PYTHON_DEFINE_GET_POINTER(AbstractTaskSequencer)
        
namespace {

/**
   \todo Currently boost::python::object is used for storing the callback function object,
   but this generates a circular reference between the task object and the function object
   because callback functions are ususally instance methods of the task object and the reference
   to the task object (self) is contained in the function objcets. In this case, the task object
   is never released even if the task is removed from the task sequencer and there is no
   varibale that refers to the task in Python. Using the weakref module may solve this problem.
*/
struct PyTaskFunc
{
    py::object func;
    PyTaskFunc(py::object f) : func(f) {
        if(!PyFunction_Check(f.ptr()) && !PyMethod_Check(f.ptr())){
            PyErr_SetString(PyExc_TypeError, "Task command must be a function type object");
            throw py::error_already_set();
        }
    }
    void operator()(TaskProc* proc) {
        PyGILock lock;
        try {
            int numArgs = func.attr("func_code").attr("co_argcount").cast<int>();
            if(numArgs == 0){
                func();
            } else {
                func(boost::ref(proc));
            }
        } catch(py::error_already_set const& ex) {
            handlePythonException();
        }
    }
};

struct PyMenuItemFunc
{
    py::object func;
    PyMenuItemFunc(py::object f) : func(f) { }
    void operator()() {
        PyGILock lock;
        try {
            func();
        } catch(py::error_already_set const& ex) {
            handlePythonException();
        }
    }
};

struct PyCheckMenuItemFunc
{
    py::object func;
    PyCheckMenuItemFunc(py::object f) : func(f) { }
    void operator()(bool on) {
        PyGILock lock;
        try {
            func(on);
        } catch(py::error_already_set const& ex) {
            handlePythonException();
        }
    }
};


TaskCommandPtr TaskPhase_addCommandExMain(TaskPhase* self, const std::string& caption, py::dict kw) {

    TaskCommandPtr command = self->addCommand(caption);

    for (auto item : kw){
        std::string key = item.first.cast<std::string>();
        py::handle value = item.second;

        if(key == "default" && PyBool_Check(value.ptr())){
            if(value.cast<bool>()){
                command->setDefault();
            }
        } else if(key == "function"){
            //;
        }
    }
    return command;
}


TaskCommandPtr TaskPhase_addCommandEx(py::tuple args_, py::dict kw) {
    TaskPhasePtr self = args_[0].cast<TaskPhasePtr>();
    const std::string caption = args_[1].cast<std::string>();
    return TaskPhase_addCommandExMain(self, caption, kw);
}    

class PyTask : public Task
{
public:
    /* Inherit the constructors */
    using Task::Task;

    void onMenuRequest(TaskMenu& menu) override{
        PyGILock lock;
        try {
            PYBIND11_OVERLOAD( void, Task, onMenuRequest, boost::ref(menu));
        } catch(py::error_already_set const& ex) {
            cnoid::handlePythonException();
        }
    }

    void onActivated(AbstractTaskSequencer* sequencer){
        PyGILock lock;
        try {
            PYBIND11_OVERLOAD( void, Task, onActivated, boost::ref(sequencer));
        } catch(py::error_already_set const& ex) {
            cnoid::handlePythonException();
        }
    }

    void onDeactivated(AbstractTaskSequencer* sequencer){
        PyGILock lock;
        try {
            PYBIND11_OVERLOAD( void, Task, onDeactivated, boost::ref(sequencer));
        } catch(py::error_already_set const& ex) {
            cnoid::handlePythonException();
        }
    }

    void storeState(AbstractTaskSequencer* sequencer, Mapping& archive){
        PyGILock lock;
        try {
            //MappingPtr a = &archive;
            PYBIND11_OVERLOAD( void, Task, storeState, boost::ref(sequencer), archive);
        } catch(py::error_already_set const& ex) {
            cnoid::handlePythonException();
        }
    }
    
    void restoreState(AbstractTaskSequencer* sequencer, const Mapping& archive){
        PyGILock lock;
        try {
            //MappingPtr a = const_cast<Mapping*>(&archive);
            PYBIND11_OVERLOAD( void, Task, restoreState, boost::ref(sequencer), archive);
        } catch(py::error_already_set const& ex) {
            cnoid::handlePythonException();
        }
    }

};
typedef ref_ptr<PyTask> PyTaskPtr;


TaskCommandPtr Task_addCommandEx(py::tuple args_, py::dict kw){
    PyTaskPtr self = args_[0].cast<PyTaskPtr>();
    const std::string caption = args_[1].cast<std::string>();
    return TaskPhase_addCommandExMain(self->lastPhase(), caption, kw);
}


typedef std::set<AbstractTaskSequencer*> TaskSequencerSet;
TaskSequencerSet taskSequencers;

typedef std::map<TaskPtr, py::object> PyTaskMap;
PyTaskMap pyTasks;

void onTaskRemoved(Task* task)
{
    PyTaskMap::iterator p  = pyTasks.find(task);
    if(p != pyTasks.end()){
        PyGILock lock;
        pyTasks.erase(p);
    }
}


TaskPtr registerTask(AbstractTaskSequencer* sequencer, py::object& pyTask)
{
    PyGILock lock;
    TaskPtr task = pyTask.cast<TaskPtr>();
    if(task){
        if(taskSequencers.find(sequencer) == taskSequencers.end()){
            sequencer->sigTaskRemoved().connect(onTaskRemoved);
            taskSequencers.insert(sequencer);
        }
        pyTasks[task] = pyTask;
        return task;
    }
    return TaskPtr();
}
    
#if 0

TaskPtr AbstractTaskSequencer_task(AbstractTaskSequencer& self, int index)
{
    return self.task(index);
}
#endif
}

namespace cnoid {

void exportPyTaskTypes(py::module& m)
{
    py::class_<TaskProc>(m, "TaskProc")
        .def("currentPhaseIndex", &TaskProc::currentPhaseIndex)
        .def("isAutoMode", &TaskProc::isAutoMode)
        .def("breakSequence", &TaskProc::breakSequence)
        .def("setNextCommand", &TaskProc::setNextCommand)
        .def("setNextPhase", &TaskProc::setNextPhase)
        .def("setCommandLinkAutomatic", &TaskProc::setCommandLinkAutomatic)
        .def("executeCommand", &TaskProc::executeCommand)
        .def("wait", &TaskProc::wait)
        .def("waitForCommandToFinish", [](TaskProc& self, double timeout = 0.0) {
            bool ret;
            Py_BEGIN_ALLOW_THREADS
            ret = self.waitForCommandToFinish(timeout);
            Py_END_ALLOW_THREADS
            return ret;
        })
        .def("waitForCommandToFinish", [](TaskProc& self, Connection connectionToDisconnect, double timeout = 0.0) {
            bool ret;
            Py_BEGIN_ALLOW_THREADS
            ret = self.waitForCommandToFinish(connectionToDisconnect, timeout);
            Py_END_ALLOW_THREADS
            return ret;
        })
        .def("notifyCommandFinish", &TaskProc:: notifyCommandFinish, py::arg("isCompleted")=true)
        .def("notifyCommandFinish_true", &TaskProc:: notifyCommandFinish, py::arg("isCompleted")=true)
        .def("waitForSignal", [](py::object self, py::object signalProxy, double timeout = 0.0){
            py::object notifyCommandFinish = self.attr("notifyCommandFinish")(true);
            py::object connection = signalProxy.attr("connect")(notifyCommandFinish);
            return self.attr("waitForCommandToFinish")(connection, timeout).cast<bool>();
        })
        .def("waitForBooleanSignal", [](py::object self, py::object signalProxy, double timeout = 0.0){
            py::object notifyCommandFinish = self.attr("notifyCommandFinish")();
            py::object connection = signalProxy.attr("connect")(notifyCommandFinish);
            return self.attr("waitForCommandToFinish")(connection, timeout).cast<bool>();
        })
        ;

    py::class_<TaskFunc>(m, "TaskFunc")
        .def("__call__", &TaskFunc::operator())
        ;

    py::class_<TaskToggleState, TaskToggleStatePtr, Referenced>(m, "TaskToggleState")
        .def("isChecked", &TaskToggleState::isChecked)
        .def("setChecked", &TaskToggleState::setChecked)
        .def("sigToggled", &TaskToggleState::sigToggled)
        ;

    py::implicitly_convertible<TaskToggleStatePtr, ReferencedPtr>();

    py::class_<TaskCommand, TaskCommandPtr, Referenced>(m, "TaskCommand")
        .def(py::init<const std::string&>())
        .def("caption", &TaskCommand::caption, py::return_value_policy::reference)
        .def("setCaption", [](TaskCommand& self, const std::string& caption){
            return TaskCommandPtr(self.setCaption(caption));
        })
        .def("description", &TaskCommand::description, py::return_value_policy::reference)
        .def("setDescription", [](TaskCommand& self, const std::string& description){
            return TaskCommandPtr(self.setDescription(description));
        })
        .def("function", &TaskCommand::function)
        .def("setFunction", [](TaskCommand& self, py::object func){
            return TaskCommandPtr(self.setFunction(PyTaskFunc(func)));
        })
        .def("setDefault", [](TaskCommand& self){
            return TaskCommandPtr(self.setDefault());
        })
        .def("setDefault", [](TaskCommand& self, bool on){
             return TaskCommandPtr(self.setDefault(on));
        })
        .def("isDefault", &TaskCommand::isDefault)
        .def("setCheckable", [](TaskCommand& self){
            return TaskCommandPtr(self.setCheckable());
        })
        .def("setCheckable", [](TaskCommand& self, bool on){
            return TaskCommandPtr(self.setCheckable(on));
        })
        .def("setToggleState", [](TaskCommand& self,  TaskToggleState* state){
            return TaskCommandPtr(self.setToggleState(state));
        })
        .def("toggleState",  [](TaskCommand& self){
            return TaskToggleStatePtr(self.toggleState());
        })
        .def("setChecked",  [](TaskCommand& self, bool on){
            return TaskCommandPtr(self.setChecked(on));
        })
        .def("isChecked", &TaskCommand::isChecked)
        .def("nextPhaseIndex", &TaskCommand::nextPhaseIndex)
        .def("setPhaseLink", [](TaskCommand& self, int phaseIndex) {
            return TaskCommandPtr(self.setPhaseLink(phaseIndex));
        })
        .def("setPhaseLinkStep", [](TaskCommand& self, int phaseIndexStep) {
            return TaskCommandPtr(self.setPhaseLinkStep(phaseIndexStep));
        })
        .def("linkToNextPhase", [](TaskCommand& self) {
            return TaskCommandPtr(self.linkToNextPhase());
        })
        .def("nextCommandIndex", &TaskCommand::nextCommandIndex)
        .def("setCommandLink", [](TaskCommand& self, int commandIndex){
            return TaskCommandPtr(self.setCommandLink(commandIndex));
        })
        .def("setCommandLinkStep", [](TaskCommand& self, int commandIndexStep){
            return TaskCommandPtr(self.setCommandLinkStep(commandIndexStep));
        })
        .def("linkToNextCommand", [](TaskCommand& self){
            return TaskCommandPtr(self.linkToNextCommand());
        })
        .def("isCommandLinkAutomatic", &TaskCommand::isCommandLinkAutomatic)
        .def("setCommandLinkAutomatic", [](TaskCommand& self) {
            return TaskCommandPtr(self.setCommandLinkAutomatic());
        })
        .def("setCommandLinkAutomatic", [](TaskCommand& self, bool on){
            return TaskCommandPtr(self.setCommandLinkAutomatic(on));
        })
        .def("setLevel", [](TaskCommand& self, int level){
            return TaskCommandPtr(self.setLevel(level));
        })
        .def("level", &TaskCommand::level)
        ;
    
    py::implicitly_convertible<TaskCommandPtr, ReferencedPtr>();

    py::class_<TaskPhase, TaskPhasePtr, Referenced>(m, "TaskPhase")
        .def(py::init<const std::string&>())
        .def(py::init<const TaskPhase&>())
        .def(py::init<const TaskPhase&, bool>())
        .def("clone", [](TaskPhase& self){
            return TaskPhasePtr(self.clone());
        })
        .def("clone", [](TaskPhase& self, bool doDeepCopy){
            return TaskPhasePtr(self.clone(doDeepCopy));
        })
        .def("caption", &TaskPhase::caption, py::return_value_policy::reference)
        .def("setCaption", &TaskPhase::setCaption)
        .def("isSkipped", &TaskPhase::isSkipped)
        .def("setSkipped", &TaskPhase::setSkipped)
        .def("setPreCommand", [](TaskPhase& self, py::object func){
            return self.setPreCommand(PyTaskFunc(func));
        })
        .def("setPreCommand", &TaskPhase::setPreCommand)
        .def("preCommand", &TaskPhase::preCommand)
        .def("addCommand", [](TaskPhase& self){
            return TaskCommandPtr(self.addCommand());
        })
        .def("addCommand", [](TaskPhase& self, const std::string& caption) {
            return TaskCommandPtr(self.addCommand(caption));
        })
        .def("addToggleCommand", [](TaskPhase& self) {
            return TaskCommandPtr(self.addToggleCommand());
        })
        .def("addToggleCommand", [](TaskPhase& self, const std::string& caption){
            return TaskCommandPtr(self.addToggleCommand(caption));
        })
        .def("addCommandEx", &TaskPhase_addCommandEx)
        .def("numCommands", &TaskPhase::numCommands)
        .def("command", [](TaskPhase& self, int index) {
            return TaskCommandPtr(self.command(index));
        })
        .def("lastCommandIndex", &TaskPhase::lastCommandIndex)
        .def("lastCommand", [](TaskPhase& self) {
            return TaskCommandPtr(self.lastCommand());
        })
        .def("commandLevel", &TaskPhase::commandLevel)
        .def("maxCommandLevel", &TaskPhase::maxCommandLevel)
        ;

    py::implicitly_convertible<TaskPhasePtr, ReferencedPtr>();

    py::class_<TaskPhaseProxy, TaskPhaseProxyPtr, Referenced >(m, "TaskPhaseProxy")
        .def("setCommandLevel", &TaskPhaseProxy::setCommandLevel)
        .def("commandLevel", &TaskPhaseProxy::commandLevel)
        .def("addCommand", [](TaskPhaseProxy& self) {
            return TaskCommandPtr(self.addCommand());
        })
        .def("addCommand", [](TaskPhaseProxy& self, const std::string& caption) {
            return TaskCommandPtr(self.addCommand(caption));
        })
        .def("addToggleCommand", [](TaskPhaseProxy& self) {
            return TaskCommandPtr(self.addToggleCommand());
        })
        .def("addToggleCommand", [](TaskPhaseProxy& self, const std::string& caption) {
            return TaskCommandPtr(self.addToggleCommand(caption));
        })
        ;
    
    py::implicitly_convertible<TaskPhaseProxyPtr, ReferencedPtr>();

    py::class_<TaskMenu>(m, "TaskMenu")
        .def("addMenuItem", [](TaskMenu& self, const std::string& caption, py::object func) {
            self.addMenuItem(caption, PyMenuItemFunc(func));
        })
        .def("addCheckMenuItem", [](TaskMenu& self, const std::string& caption,
                bool isChecked, py::object func){
            self.addCheckMenuItem(caption, isChecked, PyCheckMenuItemFunc(func));
        })
        .def("addMenuSeparator", [](TaskMenu& self) {
            self.addMenuSeparator();
        })
        ;

    py::class_<Task, TaskPtr, PyTask >(m, "Task")
        .def(py::init<>())
        .def(py::init<const std::string&, const std::string&>())
        .def(py::init<const Task&, bool>(), py::arg("org"), py::arg("doDeepCopy")=true)
        .def("name", &Task::name, py::return_value_policy::reference)
        .def("setName", &Task::setName)
        .def("caption", &Task::caption, py::return_value_policy::reference)
        .def("setCaption", &Task::setCaption)
        .def("numPhases", &Task::numPhases)
        .def("phase", [](Task& self, int index){
            return TaskPhasePtr(self.phase(index));
        })
        .def("addPhase", [](Task& self, TaskPhase* phase) {
            return TaskPhasePtr(self.addPhase(phase));
        })
        .def("addPhase",  [](Task& self, const std::string& caption) {
            return TaskPhasePtr(self.addPhase(caption));
        })
        .def("lastPhase", [](Task& self) {
            return TaskPhasePtr(self.lastPhase());
        })
        .def("setPreCommand", [](Task& self, py::object func) {
            return self.setPreCommand(PyTaskFunc(func));
        })
        .def("setPreCommand", &Task::setPreCommand)
        .def("addCommand", [](Task& self) {
            return TaskCommandPtr(self.addCommand());
        })
        .def("addCommand", [](Task& self, const std::string& caption){
            return TaskCommandPtr(self.addCommand(caption));
        })
        .def("addToggleCommand", [](Task& self) {
            return TaskCommandPtr(self.addToggleCommand());
        })
        .def("addToggleCommand", [](Task& self, const std::string& caption) {
            return TaskCommandPtr(self.addToggleCommand(caption));
        })
        .def("addCommandEx", &Task_addCommandEx)
        .def("lastCommand", [](Task& self) {
            return TaskCommandPtr(self.lastCommand());
        })
        .def("lastCommandIndex", &Task::lastCommandIndex)
        .def("funcToSetCommandLink", &Task::funcToSetCommandLink)


        .def("onMenuRequest", &Task::onMenuRequest)
        .def("onActivated", &Task::onActivated)
        .def("onDeactivated", &Task::onDeactivated)
        .def("storeState", &Task::storeState)
        .def("restoreState", &Task::restoreState)
        .def("commandLevel", &Task::commandLevel)
        .def("maxCommandLevel", &Task::maxCommandLevel)
        ;

    py::implicitly_convertible<TaskPtr, ReferencedPtr>();
    py::implicitly_convertible<PyTaskPtr, TaskPtr>();

    py::class_<AbstractTaskSequencer>(m, "AbstractTaskSequencer")
        .def("activate", &AbstractTaskSequencer::activate)
        .def("isActive", &AbstractTaskSequencer::isActive)
        .def("addTask", [](AbstractTaskSequencer& self, py::object pyTask) {
            if(TaskPtr task = registerTask(&self, pyTask)){
                self.addTask(task);
            }
        })
        .def("updateTask", [](AbstractTaskSequencer& self, py::object pyTask) {
            if(TaskPtr task = registerTask(&self, pyTask)){
                self.updateTask(task);
            }
            return false;
        })
        .def("removeTask", &AbstractTaskSequencer::removeTask)
        .def("sigTaskAdded", &AbstractTaskSequencer::sigTaskAdded)
        .def("sigTaskRemoved", &AbstractTaskSequencer::sigTaskRemoved)
        .def("clearTasks", &AbstractTaskSequencer::clearTasks)
        .def("numTasks", &AbstractTaskSequencer::numTasks)
        .def("task",  [](AbstractTaskSequencer& self, int index) {
            return TaskPtr(self.task(index));
        })
        .def("currentTaskIndex", &AbstractTaskSequencer::currentTaskIndex)
        .def("setCurrentTask", &AbstractTaskSequencer::setCurrentTask)
        .def("sigCurrentTaskChanged", &AbstractTaskSequencer::sigCurrentTaskChanged)
        .def("currentPhaseIndex", &AbstractTaskSequencer::currentPhaseIndex)
        .def("setCurrentPhase", &AbstractTaskSequencer::setCurrentPhase)
        .def("sigCurrentPhaseChanged", &AbstractTaskSequencer::sigCurrentPhaseChanged)
        .def("currentCommandIndex", &AbstractTaskSequencer::currentCommandIndex)
        .def("executeCommand", &AbstractTaskSequencer::executeCommand)
        .def("sigCurrentCommandChanged", &AbstractTaskSequencer::sigCurrentCommandChanged)
        .def("isBusy", &AbstractTaskSequencer::isBusy)
        .def("sigBusyStateChanged", &AbstractTaskSequencer::sigBusyStateChanged)
        .def("isAutoMode", &AbstractTaskSequencer::isAutoMode)
        .def("setAutoMode", &AbstractTaskSequencer::setAutoMode)
        .def("sigAutoModeToggled", &AbstractTaskSequencer::sigAutoModeToggled)
        ;
}

}