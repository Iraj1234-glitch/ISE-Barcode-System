#pragma once
enum class UserRole {
    None,
    Cashier,
    Manager
};

inline UserRole g_currentRole = UserRole::None;   // global for the whole app
