# Settings Testing

## Documented Configuration Settings

File: initConfig_01.json

All the documented configuration settings apart from "offset". The values are supplied in the different supported format.

    curl -d '{"jsonrpc": "2.0", "id": "4", "method": "org.rdk.FireboltMediaPlayer.1.create", "params": { "id": "mainplayer" }}' http://127.0.0.1:9998/jsonrpc
    curl -d @initConfig_01.json http://127.0.0.1:9998/jsonrpc

Check the logging to see the values being set e.g. 

    [7103] INFO [AampMediaStream.cpp:389] operator(): Invoking PlayerInstanceAAMP::SetInitialBitrate(2000000)
    1623315461:978 : [AAMP-PLAYER]SetValue: defaultBitrate New Owner[3]
    [7103] INFO [AampMediaStream.cpp:390] operator(): Invoking PlayerInstanceAAMP::SetInitialBitrate4K(5000000)
    1623315461:978 : [AAMP-PLAYER]SetValue: defaultBitrate4K New Owner[3]
    [7103] INFO [AampMediaStream.cpp:401] operator(): Invoking PlayerInstanceAAMP::SetNetworkTimeout(11.000000)
    1623315461:978 : [AAMP-PLAYER]SetValue: networkTimeout New Owner[3]
    [7103] INFO [AampMediaStream.cpp:402] operator(): Invoking PlayerInstanceAAMP::SetManifestTimeout(12.000000)
    1623315461:978 : [AAMP-PLAYER]SetValue: manifestTimeout New Owner[3]


## offset

File: initConfig_02.json

This configuration setting is done separately as arguably it's a command. Load an asset but don't automatically play before using the setting.

    curl -d '{"jsonrpc": "2.0", "id": "4", "method": "org.rdk.FireboltMediaPlayer.1.create", "params": { "id": "mainplayer" }}' http://127.0.0.1:9998/jsonrpc
    curl -d '{"jsonrpc": "2.0", "id": "5", "method": "org.rdk.FireboltMediaPlayer.1.load", "params": { "id": "mainplayer", "url": "https://multiplatform-f.akamaihd.net/i/multi/will/bunny/big_buck_bunny_,640x360_400,640x360_700,640x360_1000,950x540_1500,.f4v.csmil/master.m3u8", "autoplay": false }}' http://127.0.0.1:9998/jsonrpc
    curl -d @rdkservices/FireboltMediaPlayer/test/settings/initConfig_02.json http://127.0.0.1:9998/jsonrpc
    curl -d '{"jsonrpc": "2.0", "id": "8", "method": "org.rdk.FireboltMediaPlayer.1.play", "params": { "id": "mainplayer" }}' http://127.0.0.1:9998/jsonrpc

The asset should start playing 45s in.

## Incorrect settings

File: initConfig_03.json

These are incorrect settings: misspelt, wrong case, wrong value type. 

    curl -d '{"jsonrpc": "2.0", "id": "4", "method": "org.rdk.FireboltMediaPlayer.1.create", "params": { "id": "mainplayer" }}' http://127.0.0.1:9998/jsonrpc
    curl -d @initConfig_03.json http://127.0.0.1:9998/jsonrpc

You should see errors in the logs e.g

    [7103] ERROR [AampMediaStream.cpp:129] apply: Settings::apply - failed to find setting 'PreferredAudioLanguage'
    [7103] ERROR [AampMediaStream.cpp:129] apply: Settings::apply - failed to find setting 'preferedAudioLanguage'
    [7103] ERROR [AampMediaStream.cpp:255] extractSetting: Settings::extraSetting - 'networkProxy' setting is not a string
    [7103] ERROR [AampMediaStream.cpp:255] extractSetting: Settings::extraSetting - 'licenseProxy' setting is not a string
    [7103] ERROR [AampMediaStream.cpp:208] extractSetting: Settings::extraSetting - 'initialBitrate' setting is not numeric or string

## Undocumented Settings

File: initConfig_04.json

These are settings that UVE-JS supports but are not in its documentation.

    curl -d '{"jsonrpc": "2.0", "id": "4", "method": "org.rdk.FireboltMediaPlayer.1.create", "params": { "id": "mainplayer" }}' http://127.0.0.1:9998/jsonrpc
    curl -d @initConfig_04.json http://127.0.0.1:9998/jsonrpc

You should see these settings being applied in the logs e.g.

    [7103] WARN [AampMediaStream.cpp:452] operator(): The configuration setting 'playbackBuffer' is currently unsupported
    [7103] INFO [AampMediaStream.cpp:453] operator(): Invoking PlayerInstanceAAMP::SetPreferredRenditions("commentary")
    [7103] INFO [AampMediaStream.cpp:454] operator(): Invoking PlayerInstanceAAMP::SetPreferredCodec("aac")

## Clearing Settings

File: initConfig_05.json

Certain aamp settings can be cleared by providing a NULL C string. This is done in the FireboltMediaPlayer by giving an empty string or 'null' literal. 

    curl -d '{"jsonrpc": "2.0", "id": "4", "method": "org.rdk.FireboltMediaPlayer.1.create", "params": { "id": "mainplayer" }}' http://127.0.0.1:9998/jsonrpc
    curl -d @initConfig_05.json http://127.0.0.1:9998/jsonrpc

You should see these settings being applied in the logs e.g.

    [7103] INFO [AampMediaStream.cpp:598] InitConfig: InitConfig with config={"id":"mainplayer","preferredAudioRendition":"","preferredAudioCodec":null}
    [7103] INFO [AampMediaStream.cpp:453] operator(): Invoking PlayerInstanceAAMP::SetPreferredRenditions(NULL)
    [7103] INFO [AampMediaStream.cpp:454] operator(): Invoking PlayerInstanceAAMP::SetPreferredCodec(NULL)
    [7100] INFO [FireboltMediaPlayer.cpp:359] initConfig: response={"success":true}

## DRMConfig

File: setDRMConfig_01.json

Less testing here as the DRM settings share common code with the configuration settings. Just want to test they're all recognised.

    curl -d '{"jsonrpc": "2.0", "id": "4", "method": "org.rdk.FireboltMediaPlayer.1.create", "params": { "id": "mainplayer" }}' http://127.0.0.1:9998/jsonrpc
    curl -d @setDRMConfig_01.json http://127.0.0.1:9998/jsonrpc
