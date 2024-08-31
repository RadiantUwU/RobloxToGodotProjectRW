#ifndef SECURITY_HPP
#define SECURITY_HPP

#include "macros.hpp"

namespace gdrblx {
enum SecurityContext : char {
    SEC_None = 0x0,
    SEC_Plugin = 0x1,
    SEC_RobloxPlace = 0x2,
    SEC_WritePlayer = 0x4,
    SEC_LocalUser = 0x10,
    SEC_RobloxScript = 0x1D,
    SEC_Roblox = 0x1F,
    SEC_TestLocalUser = 0x10
};
enum ThreadIdentityType : char {
    IDEN_ANON,
    IDEN_USERINIT,
    IDEN_SCRIPT,
    IDEN_SCRIPTINROBLOXPLACE,
    IDEN_SCRIPTBYROBLOX,
    IDEN_STUDIOCOMMANDBAR,
    IDEN_STUDIOPLUGIN,
    IDEN_WEBSERV,
    IDEN_REPL
};
inline char getSecurityContextForIdentity(ThreadIdentityType t) {
    switch (t) {
        case IDEN_ANON:
            return SEC_None;
        case IDEN_USERINIT:
            return SEC_Plugin | SEC_RobloxPlace | SEC_LocalUser;
        case IDEN_SCRIPT:
            return SEC_None;
        case IDEN_SCRIPTINROBLOXPLACE:
            return SEC_RobloxPlace;
        case IDEN_SCRIPTBYROBLOX:
            return SEC_Plugin | SEC_RobloxPlace | SEC_LocalUser | SEC_RobloxScript;
        case IDEN_STUDIOCOMMANDBAR:
            return SEC_Plugin | SEC_RobloxPlace | SEC_LocalUser;
        case IDEN_STUDIOPLUGIN:
            return SEC_Plugin;
        case IDEN_WEBSERV:
            return SEC_Roblox;
        case IDEN_REPL:
            return SEC_WritePlayer | SEC_RobloxPlace | SEC_RobloxScript;
        default:
            UNREACHABLE();
    }
}
} // namespace gdrblx

#endif