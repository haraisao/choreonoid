/**
   @author Shizuko Hattori
   @author Shin'ichiro Nakaoka
*/

#ifndef PA10PosControllerRTC_H
#define PA10PosControllerRTC_H

#include <rtm/Manager.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/CorbaPort.h>
#include <rtm/DataInPort.h>
#include <rtm/DataOutPort.h>
#include "Interpolator.h"
#include <cnoid/Body>
#include <cnoid/JointPath>

class PA10PosControllerRTC : public RTC::DataFlowComponentBase
{
public:
    PA10PosControllerRTC(RTC::Manager* manager);
    ~PA10PosControllerRTC();

    virtual RTC::ReturnCode_t onInitialize();
    virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);
    virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

protected:
    // DataInPort declaration
    RTC::TimedDoubleSeq m_target_angle;
    RTC::InPort<RTC::TimedDoubleSeq> m_target_angleIn;

	RTC::TimedString m_command;
    RTC::InPort<RTC::TimedString> m_commandIn;

    RTC::TimedDoubleSeq m_angle;
    RTC::InPort<RTC::TimedDoubleSeq> m_angleIn;
  
    RTC::TimedDoubleSeq m_torque_in;
    RTC::InPort<RTC::TimedDoubleSeq> m_torqueIn;

    RTC::TimedDoubleSeq m_torque_out;
    RTC::OutPort<RTC::TimedDoubleSeq> m_torqueOut;

	RTC::TimedDoubleSeq m_angle_out;
    RTC::OutPort<RTC::TimedDoubleSeq> m_angleOut;

 private:
    cnoid::BodyPtr body;
	unsigned int n;
	unsigned int rightHand_id;
	unsigned int leftHand_id;
    int close_hand;

    cnoid::Interpolator<cnoid::VectorXd> wristInterpolator;
    cnoid::Interpolator<cnoid::VectorXd> jointInterpolator;
    cnoid::Link* wrist;
    cnoid::JointPathPtr baseToWrist;
    double dq_hand;
    cnoid::VectorXd qref, qold, qref_old;

	double time;

	int m_mode, m_mode_prev;
};

extern "C"
{
    DLL_EXPORT void PA10PosControllerRTCInit(RTC::Manager* manager);
};

#endif
