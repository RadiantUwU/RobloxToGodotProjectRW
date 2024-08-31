#ifndef VM_HPP
#define VM_HPP

#include <godot_cpp/templates/vector.hpp>

#include "core/object.hpp"
#include "templates/rc.hpp"
#include "core/state.hpp"

namespace gdrblx {

class Workspace;
class ReplicatedFirst;
class DataModel;
class GlobalTaskScheduler;
class Actor;

class RobloxVM {
public:
    LuauState* main_state;
    GlobalTaskScheduler* task;

    Arc<DataModel> game;
    Arc<Workspace> workspace;
    Arc<ReplicatedFirst> replicated_first;

    Vec<Arc<Actor>> actors;

    RobloxVM();
    ~RobloxVM();

}; // class RobloxVM

} // namespace gdrblx

#endif 