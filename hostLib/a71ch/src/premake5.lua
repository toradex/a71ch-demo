--
-- (c) NXP Semiconductors
--

project "a71ch_src"
    kind "StaticLib"
    symbols "On"
    files {
        "*.c",
        "*.h",
    }
    if IsInternalBuild() then
        files "*.lua"
    end
    includedirs {
        "../../libCommon/infra",
        "../../platform/inc",
        "../../libCommon/smCom",
        "../../libCommon/scp",
        "../../libCommon/hostCrypto",
        "../../api/inc",
        "../inc",
    }
AddCommonOptionsForLibrary("../../..")
