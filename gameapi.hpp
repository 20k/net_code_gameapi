#ifndef GAMEAPI_H
#define GAMEAPI_H

#include <map>
#include <variant>
#include <memory>
#include <functional>
#include <vector>

///nlohmann not a good choice for this as its very heavily exception based

///this is a simple nlohmann replacement
///hmm actually we dont need a nlohmann replacement
///what I just really need is a way to iterate through every member of a struct, and call a host function on it

///so what we need is a method to iterate through an object, and be provided with the corresponding concrete type

#define WASM_EXPORT __attribute__ ((visibility ("default"), used)) extern "C"

/*template<typename T>
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

#define SER(obj) to_gameapi(o, obj, #obj, ser)

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

extern "C" void serialise_basic_u32(game_api_t gapi, uint32_t* u, const char* key, bool ser);
extern "C" void serialise_basic_u64(game_api_t gapi, uint64_t* u, const char* key, bool ser);
extern "C" void serialise_basic_float(game_api_t gapi, float* u, const char* key, bool ser);
extern "C" void serialise_basic_double(game_api_t gapi, double* u, const char* key, bool ser);
extern "C" void serialise_basic_string(game_api_t gapi, char* u, uint32_t l, const char* key, bool ser);
extern "C" void serialise_basic_string_length(game_api_t gapi, uint32_t* len, const char* key);
extern "C" void serialise_basic_function(game_api_t gapi, uint32_t* address, const char* key, bool ser);

extern "C" void serialise_object_begin(game_api_t gapi, const char* key, bool ser);
extern "C" void serialise_object_end(game_api_t gapi, const char* key, bool ser);

extern "C" void serialise_object_begin_base(game_api_t gapi, bool ser);
extern "C" void serialise_object_end_base(game_api_t gapi, bool ser);

extern "C" game_api_t get_next_id();

//template<typename T, typename = std::enable_if_t<std::is_base_of_v<serialisable, T>>>
void to_gameapi(game_api_t gapi, serialisable& s, const std::string& key, bool ser)
{
    serialise_object_begin(gapi, key.c_str(), ser);

    s.handle_serialise(gapi, ser);

    serialise_object_end(gapi, key.c_str(), ser);
}

void to_gameapi(game_api_t gapi, std::string& str, const std::string& key, bool ser)
{
    uint32_t len = str.length();

    if(ser)
    {
        serialise_basic_string(gapi, const_cast<char*>(str.c_str()), len, key.c_str(), ser);
    }
    else
    {
        serialise_basic_string_length(gapi, &len, key.c_str());

        str.resize(len);

        if(len == 0)
            return;

        serialise_basic_string(gapi, &str[0], len, key.c_str(), ser);
    }
}

/*template<typename T, typename = std::enable_if_t<!std::is_base_of_v<serialisable, T>>>
void to_gameapi(game_api_t gapi, T& s, const std::string& key, bool ser)
{
    c_str ckey(key);

    serialise_basic(gapi, &s, &ckey, ser);
}*/

void to_gameapi(game_api_t gapi, uint32_t& u, const std::string& key, bool ser)
{
    serialise_basic_u32(gapi, &u, key.c_str(), ser);
}

void to_gameapi(game_api_t gapi, uint64_t& u, const std::string& key, bool ser)
{
    serialise_basic_u64(gapi, &u, key.c_str(), ser);
}

void to_gameapi(game_api_t gapi, float& u, const std::string& key, bool ser)
{
    serialise_basic_float(gapi, &u, key.c_str(), ser);
}

void to_gameapi(game_api_t gapi, double& u, const std::string& key, bool ser)
{
    serialise_basic_double(gapi, &u, key.c_str(), ser);
}

template<typename T>
void to_gameapi(game_api_t gapi, game_api_t (T::*ptr)(game_api_t), const std::string& key, bool ser)
{

}

using game_func = game_api_t (*)(game_api_t);

extern
game_api_t invoke_host_function(game_api_t args)

void to_gameapi(game_api_t gapi, game_func& ptr, const std::string& key, bool ser)
{
    uint32_t address = (uint32_t)ptr;

    serialise_basic_function(gapi, &address, key.c_str(), ser);

    if(ser == false)
    {
        //ptr = (game_func)address;
        ptr = invoke_host_function;
    }
    else
    {

    }
}

template<typename T>
void serialise_root(game_api_t gapi, T& t, bool ser)
{
    serialise_object_begin_base(gapi, ser);

    t.handle_serialise(gapi, ser);

    serialise_object_end_base(gapi, ser);
}

WASM_EXPORT
game_api_t invoke_address(uint32_t address, game_api_t args)
{
    game_api_t (*ptr)(game_api_t) = (game_api_t (*)(game_api_t))address;

    return ptr(args);
}

#endif // GAMEAPI_H
