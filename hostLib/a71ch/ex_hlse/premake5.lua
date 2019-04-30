--
-- (c) NXP Semiconductors
--

project "ex_hlse"
    kind "StaticLib"
    symbols "On"

    files {
        "ex_hlse_*.c",
    }


    if IsInternalBuild() then
        files "*.lua"
    end
    files {
        "*.h",
    }
    includedirs {
        "../../libCommon/infra",
        "../../libCommon/smCom",
        "../../libCommon/scp",
        "../../platform/inc",
        "../../tstUtil",
        "../../libCommon/hostCrypto",
        "../../api/inc",
        "../inc",
        "../ex",
    }
    AddCommonOptionsForLibrary("../../..")

project "hlseA71CH"
    kind "ConsoleApp"
    files {
        "mainA71CH_hlse.c",
    }
    if IsInternalBuild() then
        files "*.lua"
    end
    includedirs {
        "../../api/inc",
        "../../libCommon/hostCrypto",
        "../../libCommon/infra",
        "../../libCommon/scp",
        "../../libCommon/smCom",
        "../../platform/inc",
        "../../tstUtil",
        "../inc",
    }
    links {
        "ex_hlse",
        "a71ch_ex",
        "ax_api",
        "a71ch_src",
    }
if "mbedtls" == _OPTIONS["CRYPTO"] then
    links "mbedtls"
end
    links {
        "tstUtil",
        "scp",
        "smCom",
        "platform",
        "hostCrypto",
        "infra",
    }

    AddCommonOptionsForBinary("../../..")
