#pragma once

#include <cstdint>
#include <string>

namespace platform
{

using NodeId = std::uint64_t;

//
//  Knob is a value container that could be modified
//  by user directly.
//
class Knob
{};

struct KnobRef
{
    NodeId      node_id;
    std::string knob_name;
};

} // namespace platform