/*!
  @author Shin'ichiro Nakaoka
*/

#include "../PoseSeqItem.h"
#include <cnoid/PyUtil>

using namespace boost::python;
using namespace cnoid;

CNOID_PYTHON_DEFINE_GET_POINTER(PoseSeqItem)

BOOST_PYTHON_MODULE(PoseSeqPlugin)
{
    class_< PoseSeqItem, PoseSeqItemPtr, bases<Item> >("PoseSeqItem");
}
