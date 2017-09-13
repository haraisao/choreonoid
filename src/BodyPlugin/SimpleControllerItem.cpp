/**
   @author Shin'ichiro Nakaoka
*/

#include "SimpleControllerItem.h"
#include <cnoid/SimpleController>
#include <cnoid/Body>
#include <cnoid/Link>
#include <cnoid/Archive>
#include <cnoid/MessageView>
#include <cnoid/ExecutablePath>
#include <cnoid/FileUtil>
#include <cnoid/ConnectionSet>
#include <cnoid/ProjectManager>
#include <cnoid/ItemManager>
#include <QLibrary>
#include <boost/dynamic_bitset.hpp>
#include "gettext.h"

using namespace std;
using namespace cnoid;
namespace filesystem = boost::filesystem;

namespace {

enum {
    INPUT_JOINT_DISPLACEMENT = 0,
    INPUT_JOINT_FORCE = 1,
    INPUT_JOINT_VELOCITY = 2,
    INPUT_JOINT_ACCELERATION = 3,
    INPUT_LINK_POSITION = 4,
    INPUT_NONE = 5
};


struct SharedInfo : public Referenced
{
    BodyPtr ioBody;
    ScopedConnectionSet inputDeviceStateConnections;
    boost::dynamic_bitset<> inputEnabledDeviceFlag;
    boost::dynamic_bitset<> inputDeviceStateChangeFlag;
};

typedef ref_ptr<SharedInfo> SharedInfoPtr;

}

namespace cnoid {

class SimpleControllerItemImpl : public SimulationSimpleControllerIO
{
public:
    SimpleControllerItem* self;
    SimpleController* controller;
    Body* simulationBody;
    Body* ioBody;
    ControllerItemIO* io;
    SharedInfoPtr sharedInfo;

    vector<unsigned short> inputLinkIndices;
    vector<char> inputStateTypes;

    vector<bool> outputLinkFlags;
    vector<unsigned short> outputLinkIndices;

    ConnectionSet outputDeviceStateConnections;
    boost::dynamic_bitset<> outputDeviceStateChangeFlag;

    vector<SimpleControllerItemPtr> childControllerItems;

    vector<char> linkIndexToInputStateTypeMap;
        
    MessageView* mv;

    std::string controllerModuleName;
    std::string controllerModuleFileName;
    QLibrary controllerModule;
    bool doReloading;
    Selection pathBase;

    enum PathBase {
        CONTROLLER_DIRECTORY = 0,
        PROJECT_DIRECTORY,
        N_PATH_BASE
    };

    Signal<void()> sigControllerChanged;

    SimpleControllerItemImpl(SimpleControllerItem* self);
    SimpleControllerItemImpl(SimpleControllerItem* self, const SimpleControllerItemImpl& org);
    ~SimpleControllerItemImpl();
    void unloadController(bool doNotify);
    void initializeIoBody();
    void updateInputEnabledDevices();
    SimpleController* initialize(ControllerItemIO* io, SharedInfo* info);
    void updateIOStateTypes();
    void input();
    void onInputDeviceStateChanged(int deviceIndex);
    void onOutputDeviceStateChanged(int deviceIndex);
    void output();
    bool onReloadingChanged(bool on);
    void doPutProperties(PutPropertyFunction& putProperty);
    bool store(Archive& archive);
    bool restore(const Archive& archive);

    // virtual functions of SimpleControllerIO
    virtual std::string optionString() const override;
    virtual std::vector<std::string> options() const override;
    virtual Body* body() override;
    virtual double timeStep() const override;
    virtual std::ostream& os() const override;

    virtual void enableIO(Link* link) override;
    virtual void enableInput(Link* link) override;
    virtual void enableInput(Link* link, int stateTypes) override;
    virtual void enableInput(Device* device) override;
    virtual void enableOutput(Link* link) override;

    // deprecated virtual functions
    virtual void setLinkInput(Link* link, int stateTypes) override;
    virtual void setJointInput(int stateTypes) override;
    virtual void setLinkOutput(Link* link, int stateTypes) override;
    virtual void setJointOutput(int stateTypes) override;

    virtual bool isImmediateMode() const override;
    virtual void setImmediateMode(bool on) override;
};

}


void SimpleControllerItem::initializeClass(ExtensionManager* ext)
{
    ItemManager& itemManager = ext->itemManager();
    itemManager.registerClass<SimpleControllerItem>(N_("SimpleControllerItem"));
    itemManager.addCreationPanel<SimpleControllerItem>();
}


SimpleControllerItem::SimpleControllerItem()
{
    setName("SimpleController");
    impl = new SimpleControllerItemImpl(this);
}


SimpleControllerItemImpl::SimpleControllerItemImpl(SimpleControllerItem* self)
    : self(self),
      pathBase(N_PATH_BASE, CNOID_GETTEXT_DOMAIN_NAME)
{
    controller = 0;
    io = 0;
    mv = MessageView::instance();
    doReloading = true;
    pathBase.setSymbol(CONTROLLER_DIRECTORY, N_("Controller directory"));
    pathBase.setSymbol(PROJECT_DIRECTORY, N_("Project directory"));
    pathBase.select(CONTROLLER_DIRECTORY);
}


SimpleControllerItem::SimpleControllerItem(const SimpleControllerItem& org)
    : ControllerItem(org)
{
    impl = new SimpleControllerItemImpl(this, *org.impl);
}


SimpleControllerItemImpl::SimpleControllerItemImpl(SimpleControllerItem* self, const SimpleControllerItemImpl& org)
    : self(self),
      pathBase(org.pathBase)
{
    controller = 0;
    io = 0;
    mv = MessageView::instance();
    controllerModuleName = org.controllerModuleName;
    doReloading = org.doReloading;
}


SimpleControllerItem::~SimpleControllerItem()
{
    delete impl;
}


SimpleControllerItemImpl::~SimpleControllerItemImpl()
{
    unloadController(false);
    outputDeviceStateConnections.disconnect();
}


void SimpleControllerItem::onDisconnectedFromRoot()
{
    if(!isActive()){
        impl->unloadController(false);
    }
    impl->childControllerItems.clear();
}


Item* SimpleControllerItem::doDuplicate() const
{
    return new SimpleControllerItem(*this);
}


void SimpleControllerItem::setController(const std::string& name)
{
    impl->unloadController(true);
    impl->controllerModuleName = name;
    impl->controllerModuleFileName.clear();
}


SimpleController* SimpleControllerItem::controller()
{
    return impl->controller;
}


void SimpleControllerItemImpl::unloadController(bool doNotify)
{
    if(controller){
        delete controller;
        controller = 0;

        if(doNotify){
            sigControllerChanged();
        }
    }

    if(controllerModule.unload()){
        mv->putln(fmt(_("The controller module \"%2%\" of %1% has been unloaded."))
                  % self->name() % controllerModuleFileName);
    }
}


SignalProxy<void()> SimpleControllerItem::sigControllerChanged()
{
    return impl->sigControllerChanged;
}


void SimpleControllerItemImpl::initializeIoBody()
{
    ioBody = simulationBody->clone();

    outputDeviceStateConnections.disconnect();
    const DeviceList<>& ioDevices = ioBody->devices();
    outputDeviceStateChangeFlag.resize(ioDevices.size());
    outputDeviceStateChangeFlag.reset();
    for(size_t i=0; i < ioDevices.size(); ++i){
        outputDeviceStateConnections.add(
            ioDevices[i]->sigStateChanged().connect(
                [this, i](){ onOutputDeviceStateChanged(i); }));
    }

    sharedInfo->inputEnabledDeviceFlag.resize(simulationBody->numDevices());
    sharedInfo->inputEnabledDeviceFlag.reset();

    sharedInfo->ioBody = ioBody;
}


void SimpleControllerItemImpl::updateInputEnabledDevices()
{
    const DeviceList<>& devices = simulationBody->devices();
    sharedInfo->inputDeviceStateChangeFlag.resize(devices.size());
    sharedInfo->inputDeviceStateChangeFlag.reset();
    sharedInfo->inputDeviceStateConnections.disconnect();

    const boost::dynamic_bitset<>& flag = sharedInfo->inputEnabledDeviceFlag;
    for(size_t i=0; i < devices.size(); ++i){
        if(flag[i]){
            sharedInfo->inputDeviceStateConnections.add(
                devices[i]->sigStateChanged().connect(
                    [this, i](){ onInputDeviceStateChanged(i); }));
        } else {
            sharedInfo->inputDeviceStateConnections.add(Connection()); // null connection
        }
    }
}


bool SimpleControllerItem::initialize(ControllerItemIO* io)
{
    if(impl->initialize(io, new SharedInfo)){
        impl->updateInputEnabledDevices();
        return true;
    }
    return false;
}


SimpleController* SimpleControllerItemImpl::initialize(ControllerItemIO* io, SharedInfo* info)
{
    this->io = io;
    simulationBody = io->body();
    sharedInfo = info;
    
    bool result = false;

    if(!controller){

        filesystem::path dllPath(controllerModuleName);
        if(!checkAbsolute(dllPath)){
            if (pathBase.is(CONTROLLER_DIRECTORY))
            dllPath =
                filesystem::path(executableTopDirectory()) /
                CNOID_PLUGIN_SUBDIR / "simplecontroller" / dllPath;
            else {
                string projectFile = ProjectManager::instance()->currentProjectFile();
                if(projectFile.empty()){
                    mv->putln(_("Please save the project."));
                    return 0;
                } else {
                    dllPath = boost::filesystem::path(projectFile).parent_path() / dllPath;
                }
            }
        }
        controllerModuleFileName = getNativePathString(dllPath);
        controllerModule.setFileName(controllerModuleFileName.c_str());
        
        if(controllerModule.isLoaded()){
            mv->putln(fmt(_("The controller module of %1% has already been loaded.")) % self->name());
            
            // This should be called to make the reference to the DLL.
            // Otherwise, QLibrary::unload() unloads the DLL without considering this instance.
            controllerModule.load();
            
        } else {
            mv->put(fmt(_("Loading the controller module \"%2%\" of %1% ... "))
                    % self->name() % controllerModuleFileName);
            if(!controllerModule.load()){
                mv->put(_("Failed.\n"));
                mv->putln(controllerModule.errorString());
            } else {                
                mv->putln(_("OK!"));
            }
        }
        
        if(controllerModule.isLoaded()){
            SimpleController::Factory factory =
                (SimpleController::Factory)controllerModule.resolve("createSimpleController");
            if(!factory){
                mv->putln(_("The factory function \"createSimpleController()\" is not found in the controller module."));
            } else {
                controller = factory();
                if(!controller){
                    mv->putln(_("The factory failed to create a controller instance."));
                } else {
                    mv->putln(_("A controller instance has successfully been created."));
                    sigControllerChanged();
                }
            }
        }
    }

    childControllerItems.clear();

    if(controller){
        if(!sharedInfo->ioBody){
            initializeIoBody();
        } else {
            ioBody = sharedInfo->ioBody;
        }
        
        controller->setIO(this);

        inputLinkIndices.clear();
        inputStateTypes.clear();
        outputLinkFlags.clear();
        
        result = controller->initialize(this);

        // try the old API
        if(!result){
            for(auto joint : ioBody->joints()){
                enableIO(joint);
            }
            result = controller->initialize();
            if(result){
                for(auto device : ioBody->devices()){
                    enableInput(device);
                }
            }
        }
        
        if(!result){
            mv->putln(fmt(_("%1%'s initialize method failed.")) % self->name());
            if(doReloading){
                self->stop();
            }
        } else {
            for(Item* child = self->childItem(); child; child = child->nextItem()){
                SimpleControllerItem* childControllerItem = dynamic_cast<SimpleControllerItem*>(child);
                if(childControllerItem){
                    SimpleController* childController = childControllerItem->impl->initialize(io, sharedInfo);
                    if(childController){
                        childControllerItems.push_back(childControllerItem);
                    }
                }
            }
        }
    }

    updateIOStateTypes();

    return controller;
}


static int getInputStateTypeIndex(int type)
{
    switch(type){
    case SimpleControllerIO::JOINT_DISPLACEMENT: return INPUT_JOINT_DISPLACEMENT;
    case SimpleControllerIO::JOINT_VELOCITY:     return INPUT_JOINT_VELOCITY;
    case SimpleControllerIO::JOINT_ACCELERATION: return INPUT_JOINT_ACCELERATION;
    case SimpleControllerIO::JOINT_FORCE:        return INPUT_JOINT_FORCE;
    case SimpleControllerIO::LINK_POSITION:      return INPUT_LINK_POSITION;
    default:
        return INPUT_NONE;
    }
}


void SimpleControllerItemImpl::updateIOStateTypes()
{
    // Input
    inputLinkIndices.clear();
    inputStateTypes.clear();
    for(size_t i=0; i < linkIndexToInputStateTypeMap.size(); ++i){
        bitset<5> types(linkIndexToInputStateTypeMap[i]);
        if(types.any()){
            const int n = types.count();
            inputLinkIndices.push_back(i);
            inputStateTypes.push_back(n);
            for(int j=0; j < 5; ++j){
                if(types.test(j)){
                    inputStateTypes.push_back(getInputStateTypeIndex(1 << j));
                }
            }
        }
    }

    // Output
    outputLinkIndices.clear();
    for(size_t i=0; i < outputLinkFlags.size(); ++i){
        if(outputLinkFlags[i]){
            outputLinkIndices.push_back(i);
            simulationBody->link(i)->setActuationMode(ioBody->link(i)->actuationMode());
        }
    }
}


std::string SimpleControllerItemImpl::optionString() const
{
    if(io){
        const std::string& opt1 = io->optionString();
        const std::string& opt2 = self->optionString();
        if(!opt1.empty()){
            if(opt2.empty()){
                return opt1;
            } else {
                return opt1 + " " + opt2;
            }
        }
    }
    return self->optionString();
}


std::vector<std::string> SimpleControllerItemImpl::options() const
{
    vector<string> options;
    self->splitOptionString(optionString(), options);
    return options;
}


Body* SimpleControllerItemImpl::body()
{
    return ioBody;
}


double SimpleControllerItem::timeStep() const
{
    return impl->io ? impl->io->timeStep() : 0.0;
}


double SimpleControllerItemImpl::timeStep() const
{
    return io->timeStep();
}


std::ostream& SimpleControllerItemImpl::os() const
{
    return mv->cout();
}


void SimpleControllerItemImpl::enableIO(Link* link)
{
    enableInput(link);
    enableOutput(link);
}
        

void SimpleControllerItemImpl::enableInput(Link* link)
{
    int defaultInputStateTypes = 0;

    switch(link->actuationMode()){

    case Link::JOINT_EFFORT:
    case Link::JOINT_SURFACE_VELOCITY:
        defaultInputStateTypes = SimpleControllerIO::JOINT_ANGLE;
        break;

    case Link::JOINT_DISPLACEMENT:
    case Link::JOINT_VELOCITY:
        defaultInputStateTypes = SimpleControllerIO::JOINT_ANGLE | SimpleControllerIO::JOINT_TORQUE;
        break;

    case Link::LINK_POSITION:
        defaultInputStateTypes = SimpleControllerIO::LINK_POSITION;
        break;
    }

    enableInput(link, defaultInputStateTypes);
}        


void SimpleControllerItemImpl::enableInput(Link* link, int stateTypes)
{
    if(link->index() >= linkIndexToInputStateTypeMap.size()){
        linkIndexToInputStateTypeMap.resize(link->index() + 1, 0);
    }
    linkIndexToInputStateTypeMap[link->index()] |= stateTypes;
}        


void SimpleControllerItemImpl::setLinkInput(Link* link, int stateTypes)
{
    enableInput(link, stateTypes);
}


void SimpleControllerItemImpl::setJointInput(int stateTypes)
{
    for(Link* joint : ioBody->joints()){
        setLinkInput(joint, stateTypes);
    }
}


void SimpleControllerItemImpl::enableOutput(Link* link)
{
    int index = link->index();
    if(outputLinkFlags.size() <= index){
        outputLinkFlags.resize(index + 1, false);
    }
    outputLinkFlags[index] = true;
}


void SimpleControllerItemImpl::setLinkOutput(Link* link, int stateTypes)
{
    Link::ActuationMode mode = Link::JOINT_TORQUE;
    
    if(stateTypes & SimpleControllerIO::LINK_POSITION){
        mode = Link::LINK_POSITION;
    } else if(stateTypes & SimpleControllerIO::JOINT_DISPLACEMENT){
        mode = Link::JOINT_DISPLACEMENT;
    } else if(stateTypes & SimpleControllerIO::JOINT_VELOCITY){
        mode = Link::JOINT_VELOCITY;
    }
    
    link->setActuationMode(mode);
    enableOutput(link);
}


void SimpleControllerItemImpl::setJointOutput(int stateTypes)
{
    const int nj = ioBody->numJoints();
    for(int i=0; i < nj; ++i){
        setLinkOutput(ioBody->joint(i), stateTypes);
    }
}
    

void SimpleControllerItemImpl::enableInput(Device* device)
{
    sharedInfo->inputEnabledDeviceFlag.set(device->index());
}


bool SimpleControllerItemImpl::isImmediateMode() const
{
    return self->isImmediateMode();
}


void SimpleControllerItemImpl::setImmediateMode(bool on)
{
    self->setImmediateMode(on);
}


bool SimpleControllerItem::start()
{
    if(impl->controller->start()){
        for(size_t i=0; i < impl->childControllerItems.size(); ++i){
            if(!impl->childControllerItems[i]->start()){
                return false;
            }
        }
    }
    return true;
}


void SimpleControllerItem::input()
{
    impl->input();
    
    for(size_t i=0; i < impl->childControllerItems.size(); ++i){
        impl->childControllerItems[i]->impl->input();
    }
}


void SimpleControllerItemImpl::input()
{
    int typeArrayIndex = 0;
    for(size_t i=0; i < inputLinkIndices.size(); ++i){
        const int linkIndex = inputLinkIndices[i];
        const Link* simLink = simulationBody->link(linkIndex);
        Link* ioLink = ioBody->link(linkIndex);
        const int n = inputStateTypes[typeArrayIndex++];
        for(int j=0; j < n; ++j){
            switch(inputStateTypes[typeArrayIndex++]){
            case INPUT_JOINT_DISPLACEMENT:
                ioLink->q() = simLink->q();
                break;
            case INPUT_JOINT_FORCE:
                ioLink->u() = simLink->u();
                break;
            case INPUT_LINK_POSITION:
                ioLink->T() = simLink->T();
                break;
            case INPUT_JOINT_VELOCITY:
                ioLink->dq() = simLink->dq();
                break;
            case INPUT_JOINT_ACCELERATION:
                ioLink->ddq() = simLink->ddq();
                break;
            default:
                break;
            }
        }
    }

    boost::dynamic_bitset<>& inputDeviceStateChangeFlag = sharedInfo->inputDeviceStateChangeFlag;
    if(inputDeviceStateChangeFlag.any()){
        const DeviceList<>& devices = simulationBody->devices();
        const DeviceList<>& ioDevices = ioBody->devices();
        boost::dynamic_bitset<>::size_type i = inputDeviceStateChangeFlag.find_first();
        while(i != inputDeviceStateChangeFlag.npos){
            Device* ioDevice = ioDevices[i];
            ioDevice->copyStateFrom(*devices[i]);
            outputDeviceStateConnections.block(i);
            ioDevice->notifyStateChange();
            outputDeviceStateConnections.unblock(i);
            i = inputDeviceStateChangeFlag.find_next(i);
        }
        inputDeviceStateChangeFlag.reset();
    }
}


void SimpleControllerItemImpl::onInputDeviceStateChanged(int deviceIndex)
{
    sharedInfo->inputDeviceStateChangeFlag.set(deviceIndex);
}


bool SimpleControllerItem::control()
{
    bool result = impl->controller->control();

    for(size_t i=0; i < impl->childControllerItems.size(); ++i){
        if(impl->childControllerItems[i]->impl->controller->control()){
            result = true;
        }
    }
        
    return result;
}


void SimpleControllerItemImpl::onOutputDeviceStateChanged(int deviceIndex)
{
    outputDeviceStateChangeFlag.set(deviceIndex);
}


void SimpleControllerItem::output()
{
    impl->output();
    
    for(size_t i=0; i < impl->childControllerItems.size(); ++i){
        impl->childControllerItems[i]->impl->output();
    }
}


void SimpleControllerItemImpl::output()
{
    for(size_t i=0; i < outputLinkIndices.size(); ++i){
        const int index = outputLinkIndices[i];
        const Link* ioLink = ioBody->link(index);
        Link* simLink = simulationBody->link(index);
        switch(ioLink->actuationMode()){
        case Link::JOINT_EFFORT:
            simLink->u() = ioLink->u();
            break;
        case Link::JOINT_DISPLACEMENT:
            simLink->q() = ioLink->q();
            break;
        case Link::JOINT_VELOCITY:
        case Link::JOINT_SURFACE_VELOCITY:
            simLink->dq() = ioLink->dq();
            simLink->dq() = ioLink->dq();
            break;
        case Link::LINK_POSITION:
            simLink->T() = ioLink->T();
            break;
        default:
            break;
        }
    }
        
    if(outputDeviceStateChangeFlag.any()){
        const DeviceList<>& devices = simulationBody->devices();
        const DeviceList<>& ioDevices = ioBody->devices();
        boost::dynamic_bitset<>::size_type i = outputDeviceStateChangeFlag.find_first();
        while(i != outputDeviceStateChangeFlag.npos){
            Device* device = devices[i];
            device->copyStateFrom(*ioDevices[i]);
            sharedInfo->inputDeviceStateConnections.block(i);
            device->notifyStateChange();
            sharedInfo->inputDeviceStateConnections.unblock(i);
            i = outputDeviceStateChangeFlag.find_next(i);
        }
        outputDeviceStateChangeFlag.reset();
    }
}


void SimpleControllerItem::stop()
{
    if(impl->doReloading || !findRootItem()){
        impl->unloadController(true);
    }

    for(size_t i=0; i < impl->childControllerItems.size(); ++i){
        impl->childControllerItems[i]->stop();
    }
    impl->childControllerItems.clear();

    impl->io = 0;
}


bool SimpleControllerItemImpl::onReloadingChanged(bool on)
{
    doReloading = on;
    return true;
}


void SimpleControllerItem::doPutProperties(PutPropertyFunction& putProperty)
{
    ControllerItem::doPutProperties(putProperty);
    impl->doPutProperties(putProperty);
}


void SimpleControllerItemImpl::doPutProperties(PutPropertyFunction& putProperty)
{
    putProperty(_("Relative Path Base"), pathBase, changeProperty(pathBase));

    FileDialogFilter filter;
    filter.push_back( string(_(" Dynamic Link Library ")) + DLLSFX );
    string dir;
    if(!controllerModuleName.empty() && checkAbsolute(filesystem::path(controllerModuleName)))
        dir = filesystem::path(controllerModuleName).parent_path().string();
    else{
        if(pathBase.is(CONTROLLER_DIRECTORY))
            dir = (filesystem::path(executableTopDirectory()) / CNOID_PLUGIN_SUBDIR / "simplecontroller").string();
    }
    putProperty(_("Controller module"), FilePath(controllerModuleName, filter, dir),
                [&](const string& name){ self->setController(name); return true; });
    putProperty(_("Reloading"), doReloading,
                [&](bool on){ return onReloadingChanged(on); });
}


bool SimpleControllerItem::store(Archive& archive)
{
    if(!ControllerItem::store(archive)){
        return false;
    }
    return impl->store(archive);
}


bool SimpleControllerItemImpl::store(Archive& archive)
{
    archive.writeRelocatablePath("controller", controllerModuleName);
    archive.write("reloading", doReloading);
    archive.write("RelativePathBase", pathBase.selectedSymbol(), DOUBLE_QUOTED);
    return true;
}


bool SimpleControllerItem::restore(const Archive& archive)
{
    if(!ControllerItem::restore(archive)){
        return false;
    }
    return impl->restore(archive);
}


bool SimpleControllerItemImpl::restore(const Archive& archive)
{
    string value;
    if(archive.read("controller", value)){
        controllerModuleName = archive.expandPathVariables(value);
    }
    archive.read("reloading", doReloading);
    string symbol;
    if (archive.read("RelativePathBase", symbol)){
        pathBase.select(symbol);
    }
    return true;
}
