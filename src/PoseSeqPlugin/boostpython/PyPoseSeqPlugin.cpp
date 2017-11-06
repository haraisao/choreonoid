/*!
  @author Shin'ichiro Nakaoka
*/

#include "../PoseSeqItem.h"
#include <cnoid/PyUtil>

using namespace cnoid;
namespace py = boost::python;

CNOID_PYTHON_DEFINE_GET_POINTER(PoseSeqItem)

BOOST_PYTHON_MODULE(PoseSeqPlugin)
{
    py::class_<PoseSeqItem, PoseSeqItemPtr, py::bases<Item>>("PoseSeqItem");
}
