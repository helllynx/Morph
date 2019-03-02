#pragma once

#include "node.h"

#include <cstdint>
#include <string>
#include <variant>

namespace platform
{

struct CreateNode
{
    NodeId      id;
    std::string model;
};

struct RemoveNode
{
    NodeId id;
};

struct UpdateNodeOutput
{
    NodeId         id;
    KnobCollection output_knobs;
};

using NodeStorageAction = std::variant<
    CreateNode,
    RemoveNode, 
    UpdateNodeOutput>;

} // namespace platform
