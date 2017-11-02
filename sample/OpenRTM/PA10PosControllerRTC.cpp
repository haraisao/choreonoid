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
		// Configuration variables
		"conf.default.mode", "0",
	    ""
	};

	Vector3 toRadianVector3(double x, double y, double z){
        return Vector3(radian(x), radian(y), radian(z));
    }
}

    
PA10PosControllerRTC::PA10PosControllerRTC(RTC::Manager* manager)
	: RTC::DataFlowComponentBase(manager),
      m_target_angleIn("target_q", m_target_angle),
      m_commandIn("command", m_command),
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
	addInPort("command", m_commandIn);
	addInPort("target_q", m_target_angleIn);
	addInPort("q", m_angleIn);
	addInPort("u_in", m_torqueIn);
  
	// Set OutPort buffer
	addOutPort("u_out", m_torqueOut);
	addOutPort("q_out", m_angleOut);

	///// Configuration
	bindParameter("mode", m_mode, "0");


	//////


	string modelfile = getNativePathString(
        boost::filesystem::path(shareDirectory()) / "model/PA10/PA10.body");
            
    	BodyLoader loader;
    	loader.setMessageSink(cout);
    	loader.setShapeLoadingEnabled(false);
    	body = loader.load(modelfile);
            
    	if(!body){
        	cout << modelfile << " cannot be loaded." << endl;
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
	m_angle_out.data.length(9);

	m_mode_prev = m_mode;

    return RTC::RTC_OK;
}


RTC::ReturnCode_t PA10PosControllerRTC::onDeactivated(RTC::UniqueId ec_id)
{
	return RTC::RTC_OK;
}


RTC::ReturnCode_t PA10PosControllerRTC::onExecute(RTC::UniqueId ec_id)
{
	int i,j;

	VectorXd p0(6);
	VectorXd p1(6);

	VectorXd p2(n);
	VectorXd p3(n);

	if(m_angleIn.isNew()){
		m_angleIn.read();
	}
	if(m_torqueIn.isNew()){
		m_torqueIn.read();
	}

	if(m_commandIn.isNew()){
		m_commandIn.read();

		if(strcmp(m_command.data._ptr,"Open") == 0){
			close_hand = 0;

		}else if(strcmp(m_command.data._ptr,"Close") == 0){
			close_hand = 1;

		}else if(strcmp(m_command.data._ptr,"Mode0") == 0){
			m_mode = 0;
		}else if(strcmp(m_command.data._ptr,"Mode1") == 0){
			m_mode = 1;
		}else if(strcmp(m_command.data._ptr,"Mode2") == 0){
			m_mode = 2;
		}else if(strcmp(m_command.data._ptr,"Mode3") == 0){
			m_mode = 3;
		}
	}

	if (m_mode_prev == 0 || m_mode_prev == 1){
		if(m_mode == 2 || m_mode == 3){
			for(i=0; i < n; ++i){
				p2[i] = m_angle.data[i];
			}

			jointInterpolator.clear();
			jointInterpolator.appendSample(time, p2);
			jointInterpolator.update();
		}

	}else if (m_mode_prev == 2 || m_mode_prev == 3){
		if(m_mode == 0 || m_mode == 1){
			for(int i=0; i < n; ++i){
				body->joint(i)->q() = m_angle.data[i];
			}
			baseToWrist->calcForwardKinematics();
			p0.head<3>() = wrist->p();
			p0.tail<3>() = rpyFromRot(wrist->attitude());

			wristInterpolator.clear();
			wristInterpolator.appendSample(time, p0);
			wristInterpolator.update();
		}
	}


	switch(m_mode){
		case 0:
		case 1:
			p0 = wristInterpolator.interpolate(time);
			if(baseToWrist->calcInverseKinematics(
				Vector3(p0.head<3>()), wrist->calcRfromAttitude(rotFromRpy(Vector3(p0.tail<3>()))))){
				for(i=0; i < baseToWrist->numJoints(); ++i){
					Link* joint = baseToWrist->joint(i);
					qref[joint->jointId()] = joint->q();
				}
			}
			for(i=0; i < 3; ++i){
				m_angle_out.data[i] = p0.head<3>()[i];
			}
			for(j=0; j < 3; ++j){
				m_angle_out.data[j+3] = p0.tail<3>()[j]/3.141592*180.0;
			}
			m_angle_out.data[6]=0;
			m_angle_out.data[7] = close_hand;
			m_angle_out.data[8] = time; 
			break;

		case 2:
		case 3:
			p2 = jointInterpolator.interpolate(time);
			for(i=0; i < 7; ++i){
				qref[i] = p2[i];
			}
			for(i=0; i < 7; ++i){
				m_angle_out.data[i] = m_angle.data[i];
			}
			m_angle_out.data[7] = close_hand;
			m_angle_out.data[8] = time; 
			break;

		default:
			break;
	}



	if(close_hand == 1){
	  if(fabs(m_torque_in.data[0]) < 40.0 || fabs(m_torque_in.data[1]) < 40.0){ // not holded ?
            dq_hand = std::min(dq_hand + 0.00001, 0.0005);
			if(qref[rightHand_id] > 0) { qref[rightHand_id] -= radian(dq_hand); }
			if(qref[leftHand_id] < 0)  { qref[leftHand_id]  += radian(dq_hand); }
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

		switch(m_mode){
			case 0:
				if (m_target_angle.data.length() < 7) { break; }
				p1.head<3>() = Vector3(m_target_angle.data[0], m_target_angle.data[1], m_target_angle.data[2]);
				p1.tail<3>() = toRadianVector3(m_target_angle.data[3], m_target_angle.data[4], m_target_angle.data[5]);

				wristInterpolator.clear();
				wristInterpolator.appendSample(time, p0);
				wristInterpolator.appendSample(time+m_target_angle.data[6], p1);
				wristInterpolator.update();
				break;

			case 1:
				if (m_target_angle.data.length() < 7) { break; }
				p1.head<3>() = Vector3(m_angle_out.data[0] + m_target_angle.data[0],
					m_angle_out.data[1]+m_target_angle.data[1],
					m_angle_out.data[2]+m_target_angle.data[2]);
				p1.tail<3>() = toRadianVector3(m_angle_out.data[3]+m_target_angle.data[3],
					m_angle_out.data[4]+m_target_angle.data[4],
					m_angle_out.data[5]+m_target_angle.data[5]);

				wristInterpolator.clear();
				wristInterpolator.appendSample(time, p0);
				wristInterpolator.appendSample(time+m_target_angle.data[6], p1);
				wristInterpolator.update();
				break;

			case 2:
				if (m_target_angle.data.length() < 8) { break; }

				for(int i=0; i < 7; ++i){
					p3[i] = m_target_angle.data[i];
				}
				p3[rightHand_id] = qref[rightHand_id];
				p3[leftHand_id] =  qref[leftHand_id];

				jointInterpolator.clear();
				jointInterpolator.appendSample(time, p2);
				jointInterpolator.appendSample(time+m_target_angle.data[7], p3);
				jointInterpolator.update();
				break;

			case 3:
				if (m_target_angle.data.length() < 8) { break; }

				for(int i=0; i < 7; ++i){
					p3[i] = qref[i] + m_target_angle.data[i];
				}
				p3[rightHand_id] = qref[rightHand_id];
				p3[leftHand_id] =  qref[leftHand_id];

				jointInterpolator.clear();
				jointInterpolator.appendSample(time, p2);
				jointInterpolator.appendSample(time+m_target_angle.data[7], p3);
				jointInterpolator.update();
				break;

			default:
				break;
		}

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

	m_mode_prev = m_mode;

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
