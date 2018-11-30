#ifndef GAMEAPI_H
#define GAMEAPI_H

#include <map>
#include <variant>
#include "json.hpp"
#include <memory>
#include <functional>

struct gametype
{
    std::variant<nlohmann::json, std::function<std::shared_ptr<gametype>()>, std::shared_ptr<gametype>> type;
};

struct gameapi
{

};

#endif // GAMEAPI_H
