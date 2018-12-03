#ifndef GAMEAPI_H
#define GAMEAPI_H

#include <map>
#include <variant>
#include "json.hpp"
#include <memory>
#include <functional>
#include <vector>

///nlohmann not a good choice for this as its very heavily exception based

///this is a simple nlohmann replacement
///hmm actually we dont need a nlohmann replacement
///what I just really need is a way to iterate through every member of a struct, and call a host function on it
/*struct datatype
{

};

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

///so what we need is a method to iterate through an object, and be provided with the corresponding concrete type

template<typename T>
inline
void for_each_recursive(object& o, const T& func)
{
    for(auto& i : o.type)
    {
        if(std::holds_alternative<nlohmann::json>(i.second))
        {
            func(i.first, std::get<nlohmann::json>(i.second));
        }

        if(std::holds_alternative<std::function<std::shared_ptr<object>()>>(i.second))
        {
            func(i.first, std::get<std::function<std::shared_ptr<object>()>>(i.second));
        }

        if(std::holds_alternative<std::shared_ptr<object>>(i.second))
        {
            func(i.first, std::get<std::shared_ptr<object>>(i.second));

            for_each_recursive(*std::get<std::shared_ptr<object>>(i.second), func);
        }
    }
}*/

using game_api_t = uint32_t;

#define SERIALISE_FUNC() virtual void handle_serialise(game_api_t& o, bool ser)

struct serialisable
{
    virtual void handle_serialise(game_api_t& o, bool ser){}
};

//#define SER(obj) serialise(o, obj, #obj, ser);

#define SER(obj) to_gameapi(o, obj, #obj, ser)

/*template<typename T, typename = std::enable_if_t<!std::is_base_of_v<serialisable, T>>>
inline
void serialise(object& obj, T& t, const std::string& key, bool ser)
{
    if(ser)
        obj.type[key] = nlohmann::json(t);
    else
    {
        //if(!std::holds_alternative<nlohmann::json>(obj.type[key]))
        //    throw std::runtime_error("bad key type");

        t = (T)std::get<nlohmann::json>(obj.type[key]);
    }
}

//template<typename T, typename = std::enable_if_t<std::is_base_of_v<serialisable, T>>>
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
}*/

struct c_str
{
    c_str(const std::string& str)
    {
        len = str.size();
        ptr = str.c_str();
    }

    uint32_t len;
    const char* ptr;
};

extern void serialise_basic(game_api_t gapi, uint32_t* u, c_str* key, bool ser);
extern void serialise_basic(game_api_t gapi, uint64_t* u, c_str* key, bool ser);
extern void serialise_basic(game_api_t gapi, float* u, c_str* key, bool ser);
extern void serialise_basic(game_api_t gapi, double* u, c_str* key, bool ser);
extern void serialise_basic(game_api_t gapi, c_str* u, c_str* key, bool ser);

extern void serialise_object_begin(game_api_t gapi, c_str* key);
extern void serialise_object_end(game_api_t gapi, c_str* key);

//template<typename T, typename = std::enable_if_t<std::is_base_of_v<serialisable, T>>>
void to_gameapi(game_api_t gapi, serialisable& s, const std::string& key, bool ser)
{
    c_str ckey(key);

    serialise_object_begin(gapi, &ckey);

    s.handle_serialise(gapi, ser);

    serialise_object_end(gapi, &ckey);
}

void to_gameapi(game_api_t gapi, std::string& str, const std::string& key, bool ser)
{
    c_str cstr(str);
    c_str ckey(key);

    serialise_basic(gapi, &cstr, &ckey, ser);
}

template<typename T, typename = std::enable_if_t<!std::is_base_of_v<serialisable, T>>>
void to_gameapi(game_api_t gapi, T& s, const std::string& key, bool ser)
{
    c_str ckey(key);

    serialise_basic(gapi, &s, &ckey, ser);
}

#endif // GAMEAPI_H
