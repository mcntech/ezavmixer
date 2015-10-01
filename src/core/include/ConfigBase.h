#ifndef _CONFIG_BASE_H_
#define _CONFIG_BASE_H_
#include <string>

class CConfigBase
{
public:
    virtual bool getbool(const std::string& Section, const std::string& Key, bool DefValue=false) const{return DefValue;};
    virtual long getl(const std::string& Section, const std::string& Key, long DefValue=0) const{return DefValue;};
    virtual int geti(const std::string& Section, const std::string& Key, int DefValue=0) const{return DefValue;};
    virtual std::string gets(const std::string& Section, const std::string& Key, const std::string& DefValue="") const{return DefValue;};
    virtual float getf(const std::string& Section, const std::string& Key, float DefValue=0) const{return DefValue;};

    virtual bool put(const std::string& Section, const std::string& Key, long Value) const{return false;};
    virtual bool put(const std::string& Section, const std::string& Key, int Value) const{return false;};
    virtual bool put(const std::string& Section, const std::string& Key, bool Value) const{return false;};
    virtual bool put(const std::string& Section, const std::string& Key, const char* Value) const{return false;};

    virtual bool put(const std::string& Section, const std::string& Key, float Value) const{return false;};
    virtual bool del(const std::string& Section, const std::string& Key) const{return false;};
};

#endif /* _CONFIG_BASE_H_MININI_H */
