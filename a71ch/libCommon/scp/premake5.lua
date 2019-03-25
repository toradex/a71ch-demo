--
-- (c) NXP Semiconductors
--

project "scp"
    kind "StaticLib"
    symbols "On"

    files {
        "scp.c",
    }
    if IsInternalBuild() then
        files "*.lua"
        files "*.h"
    end

    includedirs {
        ".",
        "../infra",
        "../hostCrypto",
        "../../platform/inc",
        "../../api/inc",
        "../../a71ch/inc",
        -- "../scp",
        "../smCom",
    }

    AddCommonOptionsForLibrary("../../..")
