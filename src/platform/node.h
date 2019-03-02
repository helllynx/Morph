#pragma once

#include "knob.h"

#include "base/immutable/map.h"

#include <functional>
#include <string>

namespace platform
{
using NodeId            = std::uint64_t;
using KnobCollection    = immutable::Map<std::string, Knob>;
using KnobRefCollection = immutable::Map<std::string, KnobRef>;

struct NodeState
{
    KnobCollection    input_knobs;
    KnobRefCollection input_knob_refs;
    KnobCollection    output_knobs;
};

class Node
{
  public:
    //
    //  Returns initial state for this node.
    //
    virtual NodeState initial() const = 0;

    //
    //  Transforms input knobs into output ones.
    //
    virtual KnobCollection compute(KnobCollection    input_knobs,
                                   KnobRefCollection input_knob_refs) const = 0;

    virtual ~Node() = default;
};

struct NodeContext
{
    bool operator==(const NodeContext& other) const
    {
        // TODO: implement this.
        return false;
    }

    NodeId    id;
    NodeState state;
    Node*     node;
};

} // namespace platform