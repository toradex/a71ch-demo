--
-- (c) NXP Semiconductors
--

project "a71ch_ex"
    kind "StaticLib"
    symbols "On"

    if "mbedtls" == _OPTIONS["CRYPTO"] then
        files {
            "ex_aes.c",
            "ex_boot.c",
            "ex_psk.c",
            "ex_walkthrough.c",
            "ex_config.c",
            "ex_debug.c",
            "ex_ecc_nohc.c",
            "ex_gpstorage.c",
            "ex_misc.c",
            "ex_scp.c",
            "ex_sst.c",
            "ex_sst_kp.c",
        }
    else
        files {
            "ex_*.c",
        }
    end
    files {
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


project "mainA71CH"
    kind "ConsoleApp"
    files {
        "mainA71CH.c",
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
        "a71ch_ex",
        "a71ch_src",
        "tstUtil",
        "scp",
        "ax_api",
    }
if "mbedtls" == _OPTIONS["CRYPTO"] then
    links "mbedtls"
end
    links {
        "smCom",
        "platform",
        "hostCrypto",
        "infra",
    }

    AddCommonOptionsForBinary("../../..")
