#ifndef GAMEAPI_H
#define GAMEAPI_H

#include <map>
#include <variant>
#include "json.hpp"
#include <memory>
#include <functional>
#include <vector>

struct object
{
    ///maps a key to a type
    ///so there are fundamentally 3 datatypes here, one is a raw value store in the nlohmann::json
    ///this will contain actual datatypes
    ///the second is a function pointer that returns a datatype, this should be used for interop with other languages
    ///the third is a game type type so we can store recursively and model objects
    ///maybe instead of the function pointer we need to model js style getter/setters/apply
    std::map<std::string, std::variant<nlohmann::json, std::function<std::shared_ptr<object>()>, std::shared_ptr<object>>> type;
};

#define SERIALISE_FUNC() virtual void handle_serialise(object& o, bool ser)

struct serialisable
{
    virtual void handle_serialise(object& o, bool ser){}
};

#define SER(obj) serialise(o, obj, #obj, ser);

///ok so
///gameapi will be the object we interact with from c++
///we want to be able to serialise objects to this easily, WHOLE objects as well
///so the c++ friend injection hack might become viable here
struct gameapi
{

};

template<typename T, typename = std::enable_if_t<!std::is_base_of_v<serialisable, T>>>
inline
void serialise(object& obj, T& t, const std::string& key, bool ser)
{
    if(ser)
        obj.type[key] = nlohmann::json(t);
    else
    {
        if(!std::holds_alternative<nlohmann::json>(obj.type[key]))
            throw std::runtime_error("bad key type");

        t = (T)std::get<nlohmann::json>(obj.type[key]);
    }
}

template<typename T, typename = std::enable_if_t<std::is_base_of_v<serialisable, T>>>
inline
void serialise(object& obj, serialisable& v, const std::string& key, bool ser)
{
    if(ser)
    {
        object no;

        v.handle_serialise(no, ser);

        obj.type[key] = std::make_shared<object>(no);
    }
    else
    {
        v.handle_serialise(obj, ser);
    }
}

#endif // GAMEAPI_H
