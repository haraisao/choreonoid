/**
   @author Isao Hara
*/

#include <cnoid/BodyIoRTC>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>

using namespace std;
using namespace cnoid;

namespace {

class RobotIoRTC2 : public BodyIoRTC
{
public:
    RobotIoRTC2(RTC::Manager* manager);
    ~RobotIoRTC2();

    virtual bool initializeIO(ControllerIO* io) override;
    virtual bool initializeSimulation(ControllerIO* io) override;
    virtual void inputFromSimulator() override;
    virtual void outputToSimulator() override;

    BodyPtr body;
    
    // DataInPort declaration
    RTC::TimedDoubleSeq torques_in;
    RTC::InPort<RTC::TimedDoubleSeq> torquesIn;

    // DataOutPort declaration
    RTC::TimedDoubleSeq angles;
    RTC::OutPort<RTC::TimedDoubleSeq> anglesOut;

    RTC::TimedDoubleSeq torques_out;
    RTC::OutPort<RTC::TimedDoubleSeq> torquesOut;

};

const char* spec[] =
{
    "implementation_id", "RobotIoRTC2",
    "type_name",         "RobotIoRTC2",
    "description",       "Robot I/O",
    "version",           "1.0",
    "vendor",            "AIST",
    "category",          "Generic",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    ""
};

}


RobotIoRTC2::RobotIoRTC2(RTC::Manager* manager)
    : BodyIoRTC(manager),
      torquesIn("u_in", torques_in),
      torquesOut("u_out", torques_out),
      anglesOut("q", angles)
{

}


RobotIoRTC2::~RobotIoRTC2()
{

}


bool RobotIoRTC2::initializeIO(ControllerIO* io)
{
    // Set InPort buffers
    addInPort("u_in", torquesIn);
    
    // Set OutPort buffer
    addOutPort("q", anglesOut);
    angles.data.length(io->body()->numJoints());

    addOutPort("u_out", torquesOut);
    torques_out.data.length(io->body()->numJoints());

    return true;
}


bool RobotIoRTC2::initializeSimulation(ControllerIO* io)
{
    body = io->body();

    for(auto joint : body->joints()){
        if(joint->isRevoluteJoint() || joint->isPrismaticJoint()){
            joint->setActuationMode(Link::JOINT_TORQUE);
        }
    }
    
    return true;
}


void RobotIoRTC2::inputFromSimulator()
{
    for(auto joint : body->joints()){
        int index = joint->jointId();
        angles.data[index] = joint->q();
        torques_out.data[index] = joint->u();
    }
    anglesOut.write();
    torquesOut.write();
}


void RobotIoRTC2::outputToSimulator()
{
    if(torquesIn.isNew()){
        torquesIn.read();
        int n = torques_in.data.length();
        for(int i=0; i < n; ++i){
            if(i < body->numJoints()){
                body->joint(i)->u() = torques_in.data[i];
            }
        }
    }
}


extern "C"
{
    DLL_EXPORT void RobotIoRTC2Init(RTC::Manager* manager)
    {
        coil::Properties profile(spec);
        manager->registerFactory(
            profile, RTC::Create<RobotIoRTC2>, RTC::Delete<RobotIoRTC2>);
    }
};
