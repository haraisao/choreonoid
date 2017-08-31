/**
  @author Shin'ichiro Nakaoka
*/

#include <boost/python.hpp>
#include "omniORBpy.h"
#include <cnoid/CorbaUtil>

using namespace cnoid;

namespace {

omniORBpyAPI* api;

PyObject* getPyORB()
{
    CORBA::ORB_ptr orb = getORB();
    PyObject* pyOrb = api->cxxObjRefToPyObjRef(orb, true);
    return pyOrb;
}

}

BOOST_PYTHON_MODULE(Corba)
{
    PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
    PyObject* pyapi = PyObject_GetAttrString(omnipy, (char*)"API");

#ifdef __PYTHON3__
	api = (omniORBpyAPI*)PyCapsule_GetPointer(pyapi, "omniORBpyApi");
#else
    api = (omniORBpyAPI*)PyCObject_AsVoidPtr(pyapi);
#endif

    Py_DECREF(pyapi);

    boost::python::def("getORB", &getPyORB);
}
