/*!
  @author Shin'ichiro Nakaoka
*/

#include "PyBase.h"
#include "../Item.h"
#include "../RootItem.h"
#include "../FolderItem.h"
#include "../AbstractTextItem.h"
#include "../ScriptItem.h"
#include "../ExtCommandItem.h"
#include "../MultiValueSeqItem.h"
#include "../MultiAffine3SeqItem.h"
#include "../MultiSE3SeqItem.h"
#include "../Vector3SeqItem.h"
#include "../SceneItem.h"
#include "../PointSetItem.h"
#include "../MultiPointSetItem.h"
#include <cnoid/PyUtil>

namespace python = boost::python;
using namespace boost::python;
using namespace cnoid;

// for MSVC++2015 Update3
CNOID_PYTHON_DEFINE_GET_POINTER(Item)
CNOID_PYTHON_DEFINE_GET_POINTER(RootItem)
CNOID_PYTHON_DEFINE_GET_POINTER(MultiPointSeqItem)
CNOID_PYTHON_DEFINE_GET_POINTER(PointSetItem)
CNOID_PYTHON_DEFINE_GET_POINTER(SceneItem)
CNOID_PYTHON_DEFINE_GET_POINTER(MultiSeqItem<class MultiSE3Seq>)
CNOID_PYTHON_DEFINE_GET_POINTER(MultiSeqItem<class MultAffineSeq>)
CNOID_PYTHON_DEFINE_GET_POINTER(MultiSeqItem<class MultiValueSeq>)
CNOID_PYTHON_DEFINE_GET_POINTER(Vector3SeqItem)
CNOID_PYTHON_DEFINE_GET_POINTER(ExtCommandItem)
CNOID_PYTHON_DEFINE_GET_POINTER(FolderItem)
CNOID_PYTHON_DEFINE_GET_POINTER(AbstractMultiSeqItem)
CNOID_PYTHON_DEFINE_GET_POINTER(AbstractSeqItem)
CNOID_PYTHON_DEFINE_GET_POINTER(ScriptItem)
CNOID_PYTHON_DEFINE_GET_POINTER(AcstractTextItem)


namespace {

template<typename ItemType>
struct ItemList_to_pylist_converter {
    static PyObject* convert(const ItemList<ItemType>& items){
        python::list retval;
        for(int i=0; i < items.size(); ++i){
            retval.append(items[i]);
        }
        return python::incref(retval.ptr());
    }
};

ItemPtr Item_childItem(Item& self) { return self.childItem(); }
ItemPtr Item_prevItem(Item& self) { return self.prevItem(); }
ItemPtr Item_nextItem(Item& self) { return self.nextItem(); }
ItemPtr Item_parentItem(Item& self) { return self.parentItem(); }
ItemPtr Item_find(const std::string& path) { return Item::find(path); }
RootItemPtr Item_findRootItem(Item& self) { return self.findRootItem(); }
ItemPtr Item_findItem(Item& self, const std::string& path) { return self.findItem(path); };
ItemPtr Item_findChildItem(Item& self, const std::string& path) { return self.findChildItem(path); }
ItemPtr Item_findSubItem(Item& self, const std::string& path) { return self.findSubItem(path); }
ItemPtr Item_headItem(Item& self) { return self.headItem(); }

ItemList<Item> Item_getDescendantItems1(Item& self){
    ItemList<Item> items;
    items.extractChildItems(&self);
    return items;
}

python::object Item_getDescendantItems2(Item& self, python::object itemClass){
    ItemList<Item> items;
    items.extractChildItems(&self);
    return getPyNarrowedItemList(items, itemClass);
}

ItemPtr Item_duplicate(Item& self) { return self.duplicate(); }
ItemPtr Item_duplicateAll(Item& self) { return self.duplicateAll(); }


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Item_addChildItem_overloads, addChildItem, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Item_insertChildItem, insertChildItem, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Item_setTemporal, setTemporal, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Item_load1_overloads, load, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Item_load2_overloads, load, 2, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Item_save, load, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Item_overwrite, overwrite, 0, 2)

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(ScriptItem_waitToFinish, waitToFinish, 0, 1)

RootItemPtr RootItem_Instance() { return RootItem::instance(); }


SgPosTransformPtr SceneItem_topNode(SceneItem& self){
    return self.topNode();
}

Affine3 PointSetItem_offsetTransform(const PointSetItem& self){
    return self.offsetTransform();
}

Vector3 PointSetItem_attentionPoint(PointSetItem& self, int index){
    return self.attentionPoint(index);
}

PointSetItemPtr MultiPointSetItem_pointSetItem(MultiPointSetItem& self, int index)
{
    return self.pointSetItem(index);
}

PointSetItemPtr MultiPointSetItem_activePointSetItem(MultiPointSetItem& self, int index)
{
    return self.activePointSetItem(index);
}

} // namespace


namespace cnoid {

void exportPyItems()
{
    to_python_converter<ItemList<Item>, ItemList_to_pylist_converter<Item> >();
    to_python_converter<ItemList<RootItem>, ItemList_to_pylist_converter<RootItem> >();
    to_python_converter<ItemList<FolderItem>, ItemList_to_pylist_converter<FolderItem> >();
    to_python_converter<ItemList<ScriptItem>, ItemList_to_pylist_converter<ScriptItem> >();
    to_python_converter<ItemList<ExtCommandItem>, ItemList_to_pylist_converter<ExtCommandItem> >();
    to_python_converter<ItemList<MultiValueSeqItem>, ItemList_to_pylist_converter<MultiValueSeqItem> >();
    to_python_converter<ItemList<MultiAffine3SeqItem>, ItemList_to_pylist_converter<MultiAffine3SeqItem> >();
    to_python_converter<ItemList<MultiSE3SeqItem>, ItemList_to_pylist_converter<MultiSE3SeqItem> >();
    to_python_converter<ItemList<Vector3SeqItem>, ItemList_to_pylist_converter<Vector3SeqItem> >();

    bool (Item::*Item_load1)(const std::string& filename, const std::string& formatId) = &Item::load;
    bool (Item::*Item_load2)(const std::string& filename, Item* parent, const std::string& formatId) = &Item::load;
    
    class_<Item, ItemPtr, boost::noncopyable> itemClass("Item", no_init);
    
    itemClass
        .def("find", Item_find).staticmethod("find")
        .def("name", &Item::name, return_value_policy<copy_const_reference>())
        .def("setName", &Item::setName)
        .def("hasAttribute", &Item::hasAttribute)
        .def("childItem", Item_childItem)
        .def("prevItem", Item_prevItem)
        .def("nextItem", Item_nextItem)
        .def("parentItem", Item_parentItem)
        .def("addChildItem", &Item::addChildItem, Item_addChildItem_overloads())
        .def("addSubItem", &Item::addSubItem)
        .def("isSubItem", &Item::isSubItem)
        .def("detachFromParentItem", &Item::detachFromParentItem)
        .def("emitSigDetachedFromRootForSubTree", &Item::emitSigDetachedFromRootForSubTree)
        .def("insertChildItem", &Item::insertChildItem, Item_insertChildItem())
        .def("insertSubItem", &Item::insertSubItem)
        .def("isTemporal", &Item::isTemporal)
        .def("setTemporal", &Item::setTemporal, Item_setTemporal())
        .def("findRootItem", Item_findRootItem)
        .def("findItem", Item_findItem)
        .def("findChildItem", Item_findChildItem)
        .def("findSubItem", Item_findSubItem)
        .def("headItem", Item_headItem)
        .def("getDescendantItems", Item_getDescendantItems1)
        .def("getDescendantItems", Item_getDescendantItems2)
        .def("duplicate", Item_duplicate)
        .def("duplicateAll", Item_duplicateAll)
        .def("assign", &Item::assign)
        .def("load", Item_load1, Item_load1_overloads())
        .def("load", Item_load2, Item_load2_overloads())
        .def("save", &Item::save, Item_save())
        .def("overwrite", &Item::overwrite, Item_overwrite())
        .def("filePath", &Item::filePath, return_value_policy<copy_const_reference>())
        .def("fileFormat", &Item::fileFormat, return_value_policy<copy_const_reference>())
        .def("clearFileInformation", &Item::clearFileInformation)
        .def("suggestFileUpdate", &Item::suggestFileUpdate)
        .def("notifyUpdate", &Item::notifyUpdate)
        .def("sigNameChanged", &Item::sigNameChanged)
        .def("sigUpdated", &Item::sigUpdated)
        .def("sigPositionChanged", &Item::sigPositionChanged)
        .def("sigDisconnectedFromRoot", &Item::sigDisconnectedFromRoot)
        .def("sigSubTreeChanged", &Item::sigSubTreeChanged);
    
    {
        scope itemScope = itemClass;
        
        enum_<Item::Attribute>("Attribute")
            .value("SUB_ITEM", Item::SUB_ITEM) 
            .value("TEMPORAL", Item::TEMPORAL)
            .value("LOAD_ONLY", Item::LOAD_ONLY)
            .value("NUM_ATTRIBUTES", Item::NUM_ATTRIBUTES);
    }

    implicitly_convertible<ItemPtr, ReferencedPtr>();
    PyItemList<Item>("ItemList");

    class_< RootItem, RootItemPtr, bases<Item> >("RootItem")
        .def("instance", RootItem_Instance).staticmethod("instance");

    implicitly_convertible<RootItemPtr, ItemPtr>();
    PyItemList<RootItem>("RootItemList");

    class_< FolderItem, FolderItemPtr, bases<Item> >("FolderItem");

    implicitly_convertible<FolderItemPtr, ItemPtr>();
    PyItemList<FolderItem>("FolderItemList");

    class_< AbstractTextItem, AbstractTextItemPtr, bases<Item>, boost::noncopyable >
        ("AbstractTextItem", no_init)
        .def("textFilename", &AbstractTextItem::textFilename, return_value_policy<copy_const_reference>());
            
    implicitly_convertible<AbstractTextItemPtr, ItemPtr>();
    //PyItemList<AbstractTextItem>("AbstractTextItemList");
    
    class_< ScriptItem, ScriptItemPtr, bases<AbstractTextItem>, boost::noncopyable >
        ("ScriptItem", no_init)
        .def("scriptFilename", &ScriptItem::scriptFilename, return_value_policy<copy_const_reference>())
        .def("identityName", &ScriptItem::identityName)
        .def("setBackgroundMode", &ScriptItem::setBackgroundMode)
        .def("isBackgroundMode", &ScriptItem::isBackgroundMode)
        .def("isRunning", &ScriptItem::isRunning)
        .def("execute", &ScriptItem::execute)
        .def("waitToFinish", &ScriptItem::waitToFinish, ScriptItem_waitToFinish())
        .def("resultString", &ScriptItem::resultString)
        .def("sigScriptFinished", &ScriptItem::sigScriptFinished)
        .def("terminate", &ScriptItem::terminate)
        ;

    implicitly_convertible<ScriptItemPtr, AbstractTextItemPtr>();
    //PyItemList<ScriptItem>("ScriptItemList");

    class_< ExtCommandItem, ExtCommandItemPtr, bases<Item> >("ExtCommandItem")
        .def("setCommand", &ExtCommandItem::setCommand)
        .def("command", &ExtCommandItem::command, return_value_policy<copy_const_reference>())
        .def("waitingTimeAfterStarted", &ExtCommandItem::waitingTimeAfterStarted)
        .def("setWaitingTimeAfterStarted", &ExtCommandItem::setWaitingTimeAfterStarted)
        .def("execute", &ExtCommandItem::execute)
        .def("terminate", &ExtCommandItem::terminate);
    
    implicitly_convertible<ExtCommandItemPtr, ItemPtr>();
    PyItemList<ExtCommandItem>("ExtCommandItemList");

    // seq items
    class_< AbstractSeqItem, AbstractSeqItemPtr, bases<Item>, boost::noncopyable >
        abstractSeqItemClass("AbstractSeqItem", no_init);
    abstractSeqItemClass.def("abstractSeq", &AbstractSeqItem::abstractSeq);

    implicitly_convertible<AbstractSeqItemPtr, ItemPtr>();    
    PyItemList<AbstractSeqItem>("AbstractSeqItemList", abstractSeqItemClass);

    class_< Vector3SeqItem, Vector3SeqItemPtr, bases<AbstractSeqItem> >("Vector3SeqItem")
        .def("seq", &Vector3SeqItem::seq);
    
    implicitly_convertible<Vector3SeqItemPtr, AbstractSeqItemPtr>();
    PyItemList<Vector3SeqItem>("Vector3SeqItemList");

    // multi seq items
    class_< AbstractMultiSeqItem, AbstractMultiSeqItemPtr, bases<AbstractSeqItem>, boost::noncopyable >
        abstractMultiSeqItemClass("AbstractMultiSeqItem", no_init);
    abstractMultiSeqItemClass.def("abstractMultiSeq", &AbstractMultiSeqItem::abstractMultiSeq);

    implicitly_convertible<AbstractMultiSeqItemPtr, AbstractSeqItemPtr>();
    //PyItemList<AbstractMultiSeqItem>("AbstractMultiSeqItemList", abstractMultiSeqItemClass);
    
    class_< MultiValueSeqItem, MultiValueSeqItemPtr, bases<AbstractMultiSeqItem> >("MultiValueSeqItem")
        .def("abstractMultiSeq", &MultiValueSeqItem::abstractMultiSeq)
        .def("seq", &MultiValueSeqItem::seq);

    implicitly_convertible<MultiValueSeqItemPtr, AbstractMultiSeqItemPtr>();
    PyItemList<MultiValueSeqItem>("MultiValueSeqItemList");

    class_< MultiAffine3SeqItem, MultiAffine3SeqItemPtr, bases<AbstractMultiSeqItem> >("MultiAffine3SeqItem")
        .def("abstractMultiSeq", &MultiAffine3SeqItem::abstractMultiSeq)
        .def("seq", &MultiAffine3SeqItem::seq);

    implicitly_convertible<MultiAffine3SeqItemPtr, AbstractMultiSeqItemPtr>();
    PyItemList<MultiAffine3SeqItem>("MultiAffine3SeqItemList");

    class_< MultiSE3SeqItem, MultiSE3SeqItemPtr, bases<AbstractMultiSeqItem> >("MultiSE3SeqItem")
        .def("abstractMultiSeq", &MultiSE3SeqItem::abstractMultiSeq)
        .def("seq", &MultiSE3SeqItem::seq);
    
    implicitly_convertible<MultiSE3SeqItemPtr, AbstractMultiSeqItemPtr>();
    PyItemList<MultiSE3SeqItem>("MultiSE3SeqItemList");

    class_< SceneItem, SceneItemPtr, bases<Item, SceneProvider> >("SceneItem")
        .def("topNode", SceneItem_topNode)
        ;

    implicitly_convertible<SceneItemPtr, ItemPtr>();
    implicitly_convertible<SceneItemPtr, SceneProvider*>();

    class_< PointSetItem, PointSetItemPtr, bases<Item, SceneProvider> >("PointSetItem")
        .def("offsetTransform", PointSetItem_offsetTransform)
        .def("setOffsetTransform", &PointSetItem::setOffsetTransform)
        .def("sigOffsetTransformChanged", &PointSetItem::sigOffsetTransformChanged)
        .def("notifyOffsetTransformChange", &PointSetItem::notifyOffsetTransformChange)
        .def("numAttentionPoints", &PointSetItem::numAttentionPoints)
        .def("attentionPoint", PointSetItem_attentionPoint)
        .def("clearAttentionPoints", &PointSetItem::clearAttentionPoints)
        .def("addAttentionPoint", &PointSetItem::addAttentionPoint)
        .def("sigAttentionPointsChanged", &PointSetItem::sigAttentionPointsChanged)
        .def("notifyAttentionPointChange", &PointSetItem::notifyAttentionPointChange)
        ;

    implicitly_convertible<PointSetItemPtr, ItemPtr>();
    implicitly_convertible<PointSetItemPtr, SceneProvider*>();
    PyItemList<PointSetItem>("PointSetItemList");

    class_< MultiPointSetItem, MultiPointSetItemPtr, bases<PointSetItem, SceneProvider> >("MultiPointSetItem")
        .def("numPointSetItems", &MultiPointSetItem::numPointSetItems)
        .def("pointSetItem", MultiPointSetItem_pointSetItem)
        .def("numActivePointSetItems", &MultiPointSetItem::numActivePointSetItems)
        .def("activePointSetItem", MultiPointSetItem_activePointSetItem)
        .def("sigPointSetItemAdded", &MultiPointSetItem::sigPointSetItemAdded)
        .def("sigPointSetUpdated", &MultiPointSetItem::sigPointSetUpdated)
        .def("topOffsetTransform", &MultiPointSetItem::topOffsetTransform, return_value_policy<copy_const_reference>())
        .def("setTopOffsetTransform", &MultiPointSetItem::setTopOffsetTransform)
        .def("sigTopOffsetTransformChanged", &MultiPointSetItem::sigTopOffsetTransformChanged)
        .def("notifyTopOffsetTransformChange", &MultiPointSetItem::notifyTopOffsetTransformChange)
        .def("offsetTransform", &MultiPointSetItem::offsetTransform)
        .def("getTransformedPointSet", &MultiPointSetItem::getTransformedPointSet)
        .def("numAttentionPoints", &MultiPointSetItem::numAttentionPoints)
        .def("attentionPoint", &MultiPointSetItem::attentionPoint)
        .def("clearAttentionPoints", &MultiPointSetItem::clearAttentionPoints)
        .def("addAttentionPoint", &MultiPointSetItem::addAttentionPoint)
        .def("sigAttentionPointsChanged", &MultiPointSetItem::sigAttentionPointsChanged)
        .def("notifyAttentionPointChange", &MultiPointSetItem::notifyAttentionPointChange)
        .def("startAutomaticSave", &MultiPointSetItem::startAutomaticSave)
        .def("stopAutomaticSave", &MultiPointSetItem::stopAutomaticSave);
        ;

    implicitly_convertible<MultiPointSetItemPtr, ItemPtr>();
    implicitly_convertible<MultiPointSetItemPtr, SceneProvider*>();
    PyItemList<MultiPointSetItem>("MultiPointSetItemList");


	REGISTER_PTR_TO_PYTHON(ItemPtr)
	REGISTER_PTR_TO_PYTHON(RootItemPtr)

}

} // namespace cnoid
