#pragma once
#include <cstdint>

namespace mp::gui {
bool startGUI(std::uint16_t port = 7777);
void stopGUI();
void tickGUI();
} // namespace mp::gui
