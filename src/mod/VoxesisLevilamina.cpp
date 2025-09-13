#include "mod/VoxesisLevilamina.h"

#include "ll/api/mod/RegisterHelper.h"
#include "mod/server/app.h"

#include <ll/api/Config.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/data/KeyValueDB.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>
#include <ll/api/service/Bedrock.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/world/actor/player/Player.h>
#include <chrono>
#include <thread>

namespace voxesis_levilamina {

std::unique_ptr<app> VoxesisLevilamina::serverApp = nullptr;

VoxesisLevilamina& VoxesisLevilamina::getInstance() {
    static VoxesisLevilamina instance;
    return instance;
}

bool VoxesisLevilamina::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool VoxesisLevilamina::enable() {
    getSelf().getLogger().debug("Enabling...");

    auto commandRegistry = ll::service::getCommandRegistry();
    if (!commandRegistry) {
        throw std::runtime_error("failed to get command registry");
    }

    auto& command = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand("suicide", "Commits suicide.", CommandPermissionLevel::Any);
    command.overload().execute([this](CommandOrigin const& origin, CommandOutput& output) {
        auto* entity = origin.getEntity();
        if (entity == nullptr || !entity->isType(ActorType::Player)) {
            output.error("Only players can commit suicide");
            return;
        }

        auto* player = static_cast<Player*>(entity); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        player->kill();

        getSelf().getLogger().info("{} killed themselves", player->getRealName());

        // 处理来自服务器线程的日志消息
        if (serverApp) {
            serverApp->processLogMessages(this);
        }
    });

    serverApp = std::make_unique<app>();

    serverApp->init(8081);

    serverApp->run(this);

    // 短暂等待以处理初始日志消息
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    serverApp->processLogMessages(this);

    return true;
}

bool VoxesisLevilamina::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    if (serverApp) {
        serverApp->stop();
        serverApp->processLogMessages(this);  // 处理停止日志
        serverApp.reset();
    }
    return true;
}

} // namespace voxesis_levilamina

LL_REGISTER_MOD(voxesis_levilamina::VoxesisLevilamina, voxesis_levilamina::VoxesisLevilamina::getInstance());
