#pragma once

#include "ll/api/mod/NativeMod.h"
#include "mod/server/app.h"

namespace voxesis_levilamina {

class VoxesisLevilamina {

public:
    static VoxesisLevilamina& getInstance();

    VoxesisLevilamina() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    // TODO: Implement this method if you need to unload the mod.
    // /// @return True if the mod is unloaded successfully.
    // bool unload();

private:
    ll::mod::NativeMod& mSelf;
    static std::unique_ptr<app> serverApp;
};

} // namespace my_mod
