/**
   @author Shizuko Hattori
   @author Shin'ichiro Nakaoka
   @author Isao Hara
*/

#include "PA10JointControllerRTC.h"
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
		"implementation_id", "PA10JointControllerRTC",
		"type_name",         "PA10JointControllerRTC",
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

    
PA10JointControllerRTC::PA10JointControllerRTC(RTC::Manager* manager)
	: RTC::DataFlowComponentBase(manager),
      m_target_angleIn("target_q", m_target_angle),
	  m_d_angleIn("d_q", m_d_angle),
      m_angleIn("q", m_angle),
      m_torqueIn("u_in", m_torque_in),
      m_torqueOut("u_out", m_torque_out),
	  m_angleOut("q_out", m_angle_out)
{

}


PA10JointControllerRTC::~PA10JointControllerRTC()
{

}


RTC::ReturnCode_t PA10JointControllerRTC::onInitialize()
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


RTC::ReturnCode_t PA10JointControllerRTC::onActivated(RTC::UniqueId ec_id)
{
    time = 0.0;
    qref.resize(n);
    qref_old.resize(n);
    qold.resize(n);

    wrist = body->link("J7");
    Link* base = body->rootLink();
 
    base->p().setZero();
    base->R().setIdentity();
        
	if(m_angleIn.isNew()){
		m_angleIn.read();
	}

	VectorXd p(n);

    for(int i=0; i < n; ++i){
        double q = m_angle.data[i];
        qold[i] = q;
        body->joint(i)->q() = q;

		p[i] = m_angle.data[i];
    }
    qref = qold;
    qref_old = qold;

    jointInterpolator.clear();
    jointInterpolator.appendSample(0.0, p);
    jointInterpolator.update();
	/*
	for(int i=0; i < n; ++i){
        qref[i] = p[i];
    }
	*/

    close_hand = 0;
    dq_hand = 0.0;
	
	m_torque_out.data.length(n);
	m_angle_out.data.length(n);

    return RTC::RTC_OK;
}


RTC::ReturnCode_t PA10JointControllerRTC::onDeactivated(RTC::UniqueId ec_id)
{
	return RTC::RTC_OK;
}


RTC::ReturnCode_t PA10JointControllerRTC::onExecute(RTC::UniqueId ec_id)
{
	if(m_angleIn.isNew()){
		m_angleIn.read();
	}
	if(m_torqueIn.isNew()){
		m_torqueIn.read();
	}

	VectorXd p(n);
	VectorXd p0(n);

    p = jointInterpolator.interpolate(time);
	
	for(int i=0; i < 7; ++i){
        qref[i] = p[i];
    }

	for(int i=0; i < 7; ++i){
		m_angle_out.data[i] = m_angle.data[i];
	}
	m_angle_out.data[7] = close_hand; 

	if(close_hand == 1){
	  if(fabs(m_torque_in.data[0]) < 40.0 || fabs(m_torque_in.data[1]) < 40.0){ // not holded ?
            dq_hand = std::min(dq_hand + 0.00001, 0.0005);
			if (qref[rightHand_id] > 0){	qref[rightHand_id] -= radian(dq_hand); }
			if (qref[leftHand_id] < 0){	qref[leftHand_id]  += radian(dq_hand); }
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

		if (m_target_angle.data[7] >= 1){ close_hand=1;}
		if (m_target_angle.data[7] <= -1){ close_hand=0;}

	    for(int i=0; i < 7; ++i){
			p0[i] = m_target_angle.data[i];
        }
		p0[rightHand_id] = qref[rightHand_id];
        p0[leftHand_id] =  qref[leftHand_id];

        jointInterpolator.clear();
		jointInterpolator.appendSample(time, p);
		jointInterpolator.appendSample(time+m_target_angle.data[8], p0);
        jointInterpolator.update();
	}

	if(m_d_angleIn.isNew()){		
		m_d_angleIn.read();

		if (m_d_angle.data[7] >= 1){ close_hand=1;}
		if (m_d_angle.data[7] <= -1){ close_hand=0;}

	    for(int i=0; i < 7; ++i){
			p0[i] = qref[i] + m_d_angle.data[i];
        }
		p0[rightHand_id] = qref[rightHand_id];
        p0[leftHand_id] =  qref[leftHand_id];

        jointInterpolator.clear();
		jointInterpolator.appendSample(time, p);
		jointInterpolator.appendSample(time+m_d_angle.data[8], p0);
        jointInterpolator.update();
	}

    for(int i=0; i < n; ++i){
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
	DLL_EXPORT void PA10JointControllerRTCInit(RTC::Manager* manager)
    {
		coil::Properties profile(samplepd_spec);
		manager->registerFactory(profile,
                             RTC::Create<PA10JointControllerRTC>,
                             RTC::Delete<PA10JointControllerRTC>);
	}
};
