/**
   \file
   \author Shin'ichiro Nakaoka
*/

#ifndef CNOID_BASE_SCRIPT_ITEM_H
#define CNOID_BASE_SCRIPT_ITEM_H

#include "AbstractTextItem.h"
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT ScriptItem : public AbstractTextItem
{
public:
    ScriptItem();
    ScriptItem(const ScriptItem& org);

    virtual const std::string& textFilename() const;
#if _MSC_VER == 1900
	virtual const std::string& scriptFilename() { return ""; };
#else
    virtual const std::string& scriptFilename() const = 0;
#endif
    virtual std::string identityName() const;

#if _MSC_VER == 1900
	virtual void setBackgroundMode(bool on) { return; };
#else
    virtual void setBackgroundMode(bool on) = 0;
#endif
    virtual bool isBackgroundMode() const;
    virtual bool isRunning() const;
      
#if _MSC_VER == 1900
	virtual bool execute() { return true };
#else
    virtual bool execute() = 0;
#endif

    /**
       This function executes the code in the same namespace as that of the script exection.
       @note Implementing this function is optional.
    */
    virtual bool executeCode(const char* code);

    /**
       This function waits for the script to finish.
       @return True if the script is actually finished, or false if timeout happens.
       @note Implementing this function is optional.
       The function returns false if the function is not implemented.
    */
    virtual bool waitToFinish(double timeout = 0.0);
        
    virtual std::string resultString() const;
#if _MSC_VER == 1900
	virtual SignalProxy<void()> sigScriptFinished() { return __sigScriptFiniushed__; };
        
	virtual bool terminate() { return true; };
#else
	virtual SignalProxy<void()> sigScriptFinished() = 0;

	virtual bool terminate() = 0;
#endif

protected:
    virtual ~ScriptItem();
#if _MSC_VER == 1900
	 SignalProxy __sigScriptFinished__;
#endif
};

typedef ref_ptr<ScriptItem> ScriptItemPtr;
}

#endif
