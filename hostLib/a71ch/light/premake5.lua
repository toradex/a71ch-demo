--
-- (c) NXP Semiconductors
--

if not "stub" == _OPTIONS["CRYPTO"] then
    error "This application is supported only without Crypto"
end


project "a71ch_light"
    kind "StaticLib"
    symbols "On"
    files {
        "ex_light.c",
        "*.h",
    }
    if IsInternalBuild() then
        files "*.lua"
    end
    includedirs {
        "../../libCommon/infra",
        "../../libCommon/smCom",
        "../../libCommon/scp",
        "../../platform/inc",
        "../../tstUtil",
        "../../libCommon/hostCrypto",
        "../../api/inc",
        "../inc",
    }
    AddCommonOptionsForLibrary("../../..")

project "mainA71CHLight"
    kind "ConsoleAPP"
    symbols "On"
    files {
         "mainA71CHLight.c",
         "../../tstUtil/tst_utils_rtos.c",
        "*.h",
    }
    if IsInternalBuild() then
        files "*.lua"
    end
    includedirs {
        "../../libCommon/infra",
        "../../libCommon/smCom",
        "../../libCommon/scp",
        "../../platform/inc",
        "../../tstUtil",
        "../../libCommon/hostCrypto",
        "../../api/inc",
        "../inc",
    }
    links {
        "a71ch_light",
        "tstUtil",
        "a71ch_src",
        "smCom",
        "hostCrypto",
        "infra",
        "scp",
        "platform",
    }

    AddCommonOptionsForBinary("../../..")
