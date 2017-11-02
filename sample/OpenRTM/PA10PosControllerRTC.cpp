/**
   @author Shizuko Hattori
   @author Shin'ichiro Nakaoka
   @author Isao Hara
*/

#include "PA10PosControllerRTC.h"
#include <cnoid/ExecutablePath>
#include <cnoid/BodyLoader>
#include <cnoid/Link>
#include <cnoid/EigenUtil>
#include <cnoid/FileUtil>

using namespace std;
using namespace cnoid;

namespace {

    const double TIMESTEP = 0.001;

    const double pgain[] = {
        35000.0, 35000.0, 35000.0, 35000.0, 35000.0, 35000.0, 35000.0,
        17000.0, 17000.0 };

    const double dgain[] = {
        220.0, 220.0, 220.0, 220.0, 220.0, 220.0, 220.0,
        220.0, 220.0 };

    const char* samplepd_spec[] =
	{
		"implementation_id", "PA10PosControllerRTC",
		"type_name",         "PA10PosControllerRTC",
		"description",       "OpenRTM_PA10 Position Controller component",
		"version",           "0.1",
		"vendor",            "AIST",
		"category",          "Generic",
		"activity_type",     "DataFlowComponent",
		"max_instance",      "10",
		"language",          "C++",
		"lang_type",         "compile",
	    ""
	};

	Vector3 toRadianVector3(double x, double y, double z){
        return Vector3(radian(x), radian(y), radian(z));
    }
}

    
PA10PosControllerRTC::PA10PosControllerRTC(RTC::Manager* manager)
	: RTC::DataFlowComponentBase(manager),
      m_target_angleIn("target_q", m_target_angle),
	  m_d_angleIn("d_q", m_d_angle),
      m_angleIn("q", m_angle),
      m_torqueIn("u_in", m_torque_in),
      m_torqueOut("u_out", m_torque_out),
	  m_angleOut("q_out", m_angle_out)
{

}


PA10PosControllerRTC::~PA10PosControllerRTC()
{

}


RTC::ReturnCode_t PA10PosControllerRTC::onInitialize()
{
	// Set InPort buffers
	addInPort("target_q", m_target_angleIn);
	addInPort("d_q", m_d_angleIn);
	addInPort("q", m_angleIn);
	addInPort("u_in", m_torqueIn);
  
	// Set OutPort buffer
	addOutPort("u_out", m_torqueOut);
	addOutPort("q_out", m_angleOut);


	string modelfile = getNativePathString(
        boost::filesystem::path(shareDirectory()) / "model/PA10/PA10.body");
            
    BodyLoader loader;
    //loader.enableShapeLoading(false);
    loader.setShapeLoadingEnabled(false);
    body = loader.load(modelfile);
            
    if(!body){
        cout << modelfile << " cannot be loaded." << endl;
//        cout << loader.errorMessage() << endl;
		return RTC::RTC_ERROR;
    }

	n = body->numJoints();
	leftHand_id  = body->link("HAND_L")->jointId();
	rightHand_id = body->link("HAND_R")->jointId();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t PA10PosControllerRTC::onActivated(RTC::UniqueId ec_id)
{
	time = 0.0;
    qref.resize(n);
    qref_old.resize(n);
    qold.resize(n);

    wrist = body->link("J7");
    Link* base = body->rootLink();
    baseToWrist = getCustomJointPath(body, base, wrist);
    base->p().setZero();
    base->R().setIdentity();
        
	if(m_angleIn.isNew()){
		m_angleIn.read();
	}
    for(unsigned int i=0; i < n; ++i){
        double q = m_angle.data[i];
        qold[i] = q;
        body->joint(i)->q() = q;
    }
    qref = qold;
    qref_old = qold;
    baseToWrist->calcForwardKinematics();

    VectorXd p0(6);
    p0.head<3>() = wrist->p();
    p0.tail<3>() = rpyFromRot(wrist->attitude());

    wristInterpolator.clear();
    wristInterpolator.appendSample(0.0, p0);
    wristInterpolator.update();

    close_hand = 0;
    dq_hand = 0.0;
	
	m_torque_out.data.length(n);
	m_angle_out.data.length(6);

    return RTC::RTC_OK;
}


RTC::ReturnCode_t PA10PosControllerRTC::onDeactivated(RTC::UniqueId ec_id)
{
	return RTC::RTC_OK;
}


RTC::ReturnCode_t PA10PosControllerRTC::onExecute(RTC::UniqueId ec_id)
{
	if(m_angleIn.isNew()){
		m_angleIn.read();
	}
	if(m_torqueIn.isNew()){
		m_torqueIn.read();
	}

	VectorXd p(6);
	VectorXd p2(6);

    p = wristInterpolator.interpolate(time);
    if(baseToWrist->calcInverseKinematics(
           Vector3(p.head<3>()), wrist->calcRfromAttitude(rotFromRpy(Vector3(p.tail<3>()))))){
        for(int i=0; i < baseToWrist->numJoints(); ++i){
            Link* joint = baseToWrist->joint(i);
            qref[joint->jointId()] = joint->q();
        }
    }            
 	for(int i=0; i < 3; ++i){
		m_angle_out.data[i] = p.head<3>()[i];
	}
	for(int j=0; j < 3; ++j){
		m_angle_out.data[j+3] = p.tail<3>()[j]/3.141592*180.0;
	}

	if(close_hand == 1){
	  if(fabs(m_torque_in.data[0]) < 40.0 || fabs(m_torque_in.data[1]) < 40.0){ // not holded ?
            dq_hand = std::min(dq_hand + 0.00001, 0.0005);
			if(qref[rightHand_id] < 0) { qref[rightHand_id] -= radian(dq_hand); }
			if(qref[leftHand_id] > 0)  { qref[leftHand_id]  += radian(dq_hand); }
	  }
	}else{
	  if(qref[rightHand_id] < 0.028 || qref[leftHand_id] > -0.028){
            dq_hand = std::min(dq_hand + 0.00001, 0.002);
            qref[rightHand_id] += radian(dq_hand);
            qref[leftHand_id]  -= radian(dq_hand);
      }
	}

	if(m_target_angleIn.isNew()){		
		m_target_angleIn.read();

        p2.head<3>() = Vector3(m_target_angle.data[0], m_target_angle.data[1], m_target_angle.data[2]);
        p2.tail<3>() = toRadianVector3(m_target_angle.data[3], m_target_angle.data[4], m_target_angle.data[5]);
			
		if (m_target_angle.data[6] >= 1){ close_hand=1;}
		else { close_hand=0; }

        wristInterpolator.clear();
        wristInterpolator.appendSample(time, p);
		wristInterpolator.appendSample(time+m_target_angle.data[7], p2);
        wristInterpolator.update();
	}

	if(m_d_angleIn.isNew()){		
		m_d_angleIn.read();
 
        p2.head<3>() = Vector3(m_angle_out.data[0] + m_d_angle.data[0], m_angle_out.data[1]+m_d_angle.data[1], m_angle_out.data[2]+m_d_angle.data[2]);
        p2.tail<3>() = toRadianVector3(m_angle_out.data[3]+m_d_angle.data[3], m_angle_out.data[4]+m_d_angle.data[4], m_angle_out.data[5]+m_d_angle.data[5]);
			
		if (m_d_angle.data[6] >= 1){ close_hand=1;}
		if (m_d_angle.data[6] <= -1){ close_hand=0;}

        wristInterpolator.clear();
        wristInterpolator.appendSample(time, p);
		wristInterpolator.appendSample(time+m_d_angle.data[7], p2);
        wristInterpolator.update();
	}


    for(unsigned int i=0; i < n; ++i){
        double q = m_angle.data[i];
        double dq = (q - qold[i]) / TIMESTEP;
        double dq_ref = (qref[i] - qref_old[i]) / TIMESTEP;
        m_torque_out.data[i] = (qref[i] - q) * pgain[i] + (dq_ref - dq) * dgain[i];

        qold[i] = q;
    }

    qref_old = qref;
    time += TIMESTEP;
    
	m_torqueOut.write();
    m_angleOut.write();

    return RTC::RTC_OK;
}


extern "C"
{
	DLL_EXPORT void PA10PosControllerRTCInit(RTC::Manager* manager)
    {
		coil::Properties profile(samplepd_spec);
		manager->registerFactory(profile,
                             RTC::Create<PA10PosControllerRTC>,
                             RTC::Delete<PA10PosControllerRTC>);
	}
};
