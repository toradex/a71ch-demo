--
-- (c) NXP Semiconductors
--

project "A71CHConfig"
    kind "ConsoleApp"
    symbols "On"
    files {
        "*.h",
        "../../tstUtil/axCliUtil.c",
        "../../tstUtil/axEccRefPem.c",
        "config*.h",
        "configCli.c",
        "configCliApdu.c",
        "configCliConnect.c",
        "configCliDebug.c",
        "configCliEcrt.c",
        "configCliErase.c",
        "configCliGen.c",
        "configCliGet.c",
        "configCliInfo.c",
        "configCliInteractive.c",
        "configCliLock.c",
        "configCliObj.c",
        "configCliRcrt.c",
        "configCliRefpem.c",
        "configCliScp.c",
        "configCliScript.c",
        "configCliSet.c",
        "configCliTransport.c",
        "configCliWcrt.c",
        "configCmd*.c",
        "configState.c",
        "mainA71chConfig.c",
        -- "configCliInteractive_ln.c",
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
        "ax_api",
        "a71ch_src",
        "smCom",
        "scp",
        "tstUtil",
        "platform",
        "hostCrypto",
        "infra",
    }

    AddCommonOptionsForBinary("../../..")

    if IsWindows() then
        postbuildcommands {
            "copy /Y %{cfg.buildtarget.directory}A71CHConfig.exe ..\\..\\tools\\A71CHConfig_" .. _OPTIONS["CONN"] .. ".exe",
        }
    end
