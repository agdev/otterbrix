#pragma once

#include <components/logical_plan/node.hpp>
#include <components/physical_plan/collection/operators/operator.hpp>
#include <logical_plan/node_limit.hpp>
#include <services/memory_storage/context_storage.hpp>

namespace services::collection::planner::impl {

    operators::operator_ptr create_plan_aggregate(const context_storage_t& context,
                                                  const components::logical_plan::node_ptr& node,
                                                  components::logical_plan::limit_t limit);

}
