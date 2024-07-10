#!/bin/bash 

OUTOFRANGEVALUE=110
UNDIFINEDVALUE="NotDefined"
pqparams=("Backlight" "Brightness" "Contrast" "Sharpness" "Saturation" "Hue")
modes=("DolbyVisionMode")
videoParameters=("VideoSource" "VideoFormat" "VideoResolution" "VideoFrameRate")
pictureModes=()
videoSources=()
videoFormats=()
optionsArray=()
from_value=0
to_value=0
platformSupport=true
OUTOFRANGEVALUE=110
UNDIFINEDVALUE="NotDefined"
count=0
Cont="Content-Type: application/json"
Auth="Authorization: Bearer"
TOKEN_tmp=`/usr/bin/WPEFrameworkSecurityUtility | cut -d'"' -f4`

getParameter()
{
        parameterName=$1
        apiName="get${parameterName}"
        #echo $apiName
        if [ $parameterName = "DolbyVisionMode" ] || [ $parameterName = "HDR10Mode" ] || [ $parameterName = "HLGMode" ]; then
            pictureMode=$2
            videoSource=$3
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":'$pictureMode',"videoSource":'$videoSource'}}' http://127.0.0.1:9998/jsonrpc)
        elif [ $parameterName = "VideoResolution" ] || [ $parameterName = "VideoSource" ] || [ $parameterName = "VideoFormat" ] || [ $parameterName = "VideoFrameRate" ]; then
            echo "    API Name: $apiName"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'"}' http://127.0.0.1:9998/jsonrpc)
        elif [ $parameterName = "PictureMode" ]; then
            videoSource=$2
            videoFormat=$3
            echo "    API Name: $apiName, videoSource: $videoSource, videoFormat: $videoFormat"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"videoSource":'$videoSource',"videoFormat":'$videoFormat'}}' http://127.0.0.1:9998/jsonrpc)
        else
            pictureMode=$2
            videoSource=$3
            videoFormat=$4
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource, videoFormat: $videoFormat"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":'$pictureMode',"videoSource":'$videoSource',"videoFormat":'$videoFormat'}}' http://127.0.0.1:9998/jsonrpc)
        fi
        echo "Response: "${ret}
}


setParameter()
{
        parameterName=$1
        apiName="set${parameterName}"
        param="$(echo "${parameterName:0:1}" | tr '[:upper:]' '[:lower:]')${parameterName:1}"
        if [ $parameterName = "DolbyVisionMode" ] || [ $parameterName = "HDR10Mode" ] || [ $parameterName = "HLGMode" ]; then
            pictureMode=$2
            videoSource=$3
            value=$4
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource, Value: $value"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":['$pictureMode'],"videoSource":['$videoSource'],"'$param'":'$value'}}' http://127.0.0.1:9998/jsonrpc)
        elif [ $parameterName = "PictureMode" ]; then
            pictureMode=$2
            videoSource=$3
            videoFormat=$4
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource, videoFormat: $videoFormat"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":'$pictureMode',"videoSource":['$videoSource'],"videoFormat":['$videoFormat']}}' http://127.0.0.1:9998/jsonrpc)
        elif [ $parameterName = "ZoomMode" ] || [ $parameterName = "ColorTemperature" ]; then
            pictureMode=$2
            videoSource=$3
            videoFormat=$4
            value=$5
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource, videoFormat: $videoFormat value: $value"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":['$pictureMode'],"videoSource":['$videoSource'],"videoFormat":['$videoFormat'],"'$param'":'"$value"'}}' http://127.0.0.1:9998/jsonrpc)
        else
            pictureMode=$2
            videoSource=$3
            videoFormat=$4
            value=$5
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource, videoFormat: $videoFormat value: $value"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":['$pictureMode'],"videoSource":['$videoSource'],"videoFormat":['$videoFormat'],"'$param'":'$value'}}' http://127.0.0.1:9998/jsonrpc)
        fi
        echo "Response: "${ret}
}

resetParameter()
{
        parameterName=$1
        apiName="reset${parameterName}"

        if [ $parameterName = "DolbyVisionMode" ] || [ $parameterName = "HDR10Mode" ] || [ $parameterName = "HLGMode" ]; then
            pictureMode=$2
            videoSource=$3
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":['$pictureMode'],"videoSource":['$videoSource']}}' http://127.0.0.1:9998/jsonrpc)
        elif [ $parameterName = "PictureMode" ]; then
            videoSource=$2
            videoFormat=$3
            echo "    API Name: $apiName, videoSource: $videoSource, videoFormat: $videoFormat"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"videoSource":['$videoSource'],"videoFormat":['$videoFormat']}}' http://127.0.0.1:9998/jsonrpc)
        else
            pictureMode=$2
            videoSource=$3
            videoFormat=$4
            echo "    API Name: $apiName, pictureMode: $pictureMode, videoSource: $videoSource, videoFormat: $videoFormat"
            ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{"pictureMode":['$pictureMode'],"videoSource":['$videoSource'],"videoFormat":['$videoFormat']}}' http://127.0.0.1:9998/jsonrpc)
        fi
        echo "Response: "${ret}
}


getParameterCaps()
{
        parameterName=$1
        apiName="get${parameterName}Caps"
        ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"org.rdk.AVOutput.'${apiName}'","params":{}}' http://127.0.0.1:9998/jsonrpc)
        echo "Response: "${ret}
}

extractParameterCapsOutput()
{
        capsoutput=$1


        # Extract 'platformSupport' value from the response
        platformSupport=$(echo "$capsoutput" | grep -o '"platformSupport":[^,}]*' | cut -d ':' -f 2 | tr -d '[:space:]":')

        # Extract 'from' value of the range from the response
        from_value=$(echo "$capsoutput" | grep -o '"from":*[0-9]*' | sed 's/"from":\([0-9]*\)/\1/')

        # Extract 'to' value of the range from the response
        to_value=$(echo "$capsoutput" | grep -o '"to":[0-9]*' | sed 's/"to":\([0-9]*\)/\1/')

        # Extract 'options' array from the response
        options=$(echo "$capsoutput" | grep -o '"options":*\[[^]]*' | sed 's/"options":\[\(.*\)/\1/' )
        IFS=',' read -ra optionsArray <<< "$options"

        # Extract 'pictureModeInfo' array from the response
        pictureModeInfo=$(echo "$capsoutput" | grep -o '"pictureModeInfo":\[[^]]*' | sed 's/"pictureModeInfo":\[\(.*\)/\1/' | sed 's/,/ /g')
        read -a pictureModes <<< "$pictureModeInfo"

        # Extract 'videoSourceInfo' array from the response
        videoSourceInfo=$(echo "$capsoutput" | grep -o '"videoSourceInfo":\[[^]]*' | sed 's/"videoSourceInfo":\[\(.*\)/\1/' | sed 's/,/ /g')
        read -a videoSources <<< "$videoSourceInfo"

        # Extract 'videoFormatInfo' array from the response
        videoFormatInfo=$(echo "$capsoutput" | grep -o '"videoFormatInfo":\[[^]]*' | sed 's/"videoFormatInfo":\[\(.*\)/\1/' | sed 's/,/ /g')
        read -a videoFormats <<< "$videoFormatInfo"

        # Extract 'maxVideoResolution' from the response
        maxVideoResolution=$(echo "$capsoutput" | grep -o '"maxResolution":[^,}]*' | cut -d ':' -f 2 | tr -d '[:space:]":' )

}

retval=0
checkPlugin()
{
        sub='deactivated'
        sub1='null'
        ret=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"Controller.1.status@org.rdk.AVOutput"}' http://127.0.0.1:9998/jsonrpc)
        if [[ "$ret" = *"$sub"* ]]; then
                rtrn=$(curl -H "$Cont" -H "$Auth $TOKEN_tmp" -X POST -d '{"jsonrpc":"2.0","id":1,"method":"Controller.1.activate","params":{"callsign":"org.rdk.AVOutput"}}' http://127.0.0.1:9998/jsonrpc)
                if [[ "$rtrn" = *"$sub1"* ]]; then
                        echo "Plugin is activated."
                        retval=0
                elif [[ "$rtrn" = "ERROR_UNKNOWN_KEY" ]]; then
                        echo "Plugin is not available"
                        retval=1
                else
                        retval=0
                fi
        fi
}


main()
{
        # Check whether plugin is activated or not
        checkPlugin
        if [[ $retval = 1 ]]; then
                echo "Plugin not supported"
                exit 1
        fi

        #Handling PQ params "Backlight" "Brightness" "Contrast" "Sharpness" "Saturation" "Hue"
        for pqparam in ${pqparams[@]}; do
            echo ""
            echo "--------- Testing $pqparam--------"
            getcaps=$(getParameterCaps $pqparam)
	    echo " $getcaps"
            extractParameterCapsOutput "$getcaps"
            if [[ $platformSupport = "false" ]]; then
                echo "    $pqparam is not supported"
            else
                if ! [ "$pictureModes" ]; then
                    pictureModes=('"Global"')
                fi

                if ! [ "$videoSources" ]; then
                    videoSources=('"Global"')
                fi

                if ! [ "$videoFormats" ]; then
                    videoFormats=('"Global"')
                fi
                echo "    Capability of the parameter: $pqparam"
                echo "        platformSupport = $platformSupport"
                echo "        videoFormats = ${videoFormats[@]}"
                echo "        videoSources = ${videoSources[@]}"
                echo "        pictureModes = ${pictureModes[@]}"
                echo "        From = $from_value"
                echo "        To = $to_value"

                # Handle parameters for pqmode/source/format
                for pqmode in ${pictureModes[@]}; do
                    for source in ${videoSources[@]}; do
                            for format in ${videoFormats[@]}; do
                                pictureMode_get=$pqmode
                                videoSource_get=$source
                                videoFormat_get=$format
                                echo "    1.1.Testing get${pqparam}"
                                if [ $pqmode = '"Global"' ]; then
                                    pictureMode_get='"Current"'
                                fi

                                if [ $source = '"Global"' ]; then
                                    videoSource_get='"Current"'
                                fi
                                if [ $format = '"Global"' ]; then
                                    videoFormat_get='"Current"'
                                fi
                                default=$(getParameter $pqparam $pictureMode_get $videoSource_get $videoFormat_get)
				echo "$default"
                                if [[ "$default" != *"true"* ]]; then
                                    echo "        get${pqparam} failed"
                                else
                                    echo "        get${pqparam} is successful"
                                fi
                                echo "    1.2.Testing set${pqparam}"
                                for ((i=from_value;i<=to_value;i+=5));
                                do
                                        echo "        set${pqparam} value - "$i
                                        setval=$(setParameter $pqparam $pqmode $source $format $i)
                                        echo "$setval"
                                        getval=$(getParameter $pqparam $pictureMode_get $videoSource_get $videoFormat_get)
                                        echo "$getval"
                                        if [[ "$setval" != *"true"* ]]; then
                                                echo "        set${pqparam} failed"
                                        elif [[ "$getval" != *"$i"* ]]; then
                                                echo "        Not set correct value"
                                        else
                                                echo "        set${pqparam} successful"
                                        fi
                                done
                                echo "    1.3.Testing reset${pqparam}"
                                resetval=$(resetParameter $pqparam $pqmode $source $format)
				echo "$resetval"
                                getval=$(getParameter $pqparam $pictureMode_get $videoSource_get $videoFormat_get)
				echo "$getval"
                                if [[ "$getval" != *"$default"* ]]; then
                                    echo "        reset to incorrect default value"
                                elif [[ "$resetval" != *"true"* ]]; then
                                    echo "        reset${pqparam} failed"
                                else
                                    echo "        reset ${pqparam} is scuccessful. Reset to correct default value"
                                fi
                            done
                    done
                done
                if [[ $1 = "--extended" ]]; then
                    echo "    1.4.Testing set${pqparam} with a value out of the permitted range:" $OUTOFRANGEVALUE
                    setval=$(setParameter $pqparam $pqmode $source $format $OUTOFRANGEVALUE)
		    echo "$setval"
                    if [[ "$setval" != *"true"* ]]; then
                        echo "        ${pqparam} is not set for value $OUTOFRANGEVALUE which is Expected behaviour"
                    else
                        echo "        ${pqparam} is set for value $OUTOFRANGEVALUE which is Unexpected behaviour"
                    fi
                    echo ""
                fi

            fi
        done

        #Handling Color Temperature
        echo ""
        echo "--------- Testing ColorTemperature--------"
        getcaps=$(getParameterCaps "ColorTemperature")
	echo "$getcaps"
        extractParameterCapsOutput "$getcaps"
        if [[ $platformSupport = "false" ]]; then
            echo "    ColorTemperature is not supported"
        else
            if ! [ "$pictureModes" ]; then
                pictureModes=('"Global"')
            fi
            if ! [ "$videoSources" ]; then
                videoSources=('"Global"')
            fi
            if ! [ "$videoFormats" ]; then
                videoFormats=('"Global"')
            fi
            echo "    Capability of the parameter: ColorTemperature"
            echo "        platformSupport = $platformSupport"
            echo "        videoFormats = ${videoFormats[@]}"
            echo "        videoSources = ${videoSources[@]}"
            echo "        pictureModes = ${pictureModes[@]}"
            echo "        optionsArray = ${optionsArray[@]}"


            # Handle parameters for pqmode/source/format
            for pqmode in ${pictureModes[@]}; do
                for source in ${videoSources[@]}; do
                    for format in ${videoFormats[@]}; do
                        echo "    1.1.Testing getColorTemperature"
                        pictureMode_get=$pqmode
                        videoSource_get=$source
                        videoFormat_get=$format
                        if [ $pqmode = '"Global"' ]; then
                            pictureMode_get='"Current"'
                        fi

                        if [ $source = '"Global"' ]; then
                            videoSource_get='"Current"'
                        fi

                        if [ $format = '"Global"' ]; then
                            videoFormat_get='"Current"'
                        fi

                        default=$(getParameter "ColorTemperature" $pictureMode_get $videoSource_get $videoFormat_get)
			echo "$default"
                        if [[ "$default" != *"true"* ]]; then
                            echo "        getColorTemperature failed"
                        else
                            echo "        getColorTemperature is successful"
                        fi

                        echo "    1.2.Testing setColorTemperature"
                        for colorTemperature in "${optionsArray[@]}"; do
                            echo "        setColorTemperature option - " "$colorTemperature"
                            setval=$(setParameter "ColorTemperature" "$pqmode" "$source" "$format" "$colorTemperature")
                            echo "$setval"
                            getval=$(getParameter "ColorTemperature" $pictureMode_get $videoSource_get $videoFormat_get)
			    echo "$getval"
                            if [[ "$setval" != *"true"* ]]; then
                                echo "        setColorTemperature failed"
                            elif [[ "$getval" != *"$colorTemperature"* ]]; then
                                echo "        Not set correct value"
                            else
                                echo "        setColorTemperature successful"
                            fi
                        done
                        echo "    1.3.Testing resetColorTemperature"
                        resetval=$(resetParameter "ColorTemperature" "$pqmode" "$source" "$format")
			echo "$resetval"
                        getval=$(getParameter "ColorTemperature" $pictureMode_get $videoSource_get $videoFormat_get)
			echo "$getval"
                        if [[ "$getval" != *"$default"* ]]; then
                            echo "        reset to incorrect default value"
                        elif [[ "$resetval" != *"true"* ]]; then
                            echo "        resetColorTemperature failed"
                        else
                            echo "        reset ColorTemperature is scuccessful. Reset to correct default value"
                        fi
                    done
                done
            done
            if [[ $1 = "--extended" ]]; then
                echo "    6.4.Testing setColorTemperature with a value out of the permitted range:"
                setval=$(setParameter "ColorTemperature" "$pqmode" "$source" "$format" $UNDIFINEDVALUE)
		echo "$setval"
                if [[ "$setval" != *"true"* ]]; then
                    echo "        Colortemperature is not set for value $UNDIFINEDVALUE which is Expected behaviour"
                else
                    echo "        Colortemperature is set for value $UNDIFINEDVALUE which is Unexpected behaviour"
                fi
                echo ""
            fi
        fi

        #Handling Zoom mode
        echo ""
        echo "--------- Testing Zoom Mode--------"
        getcaps=$(getParameterCaps "ZoomMode")
	echo "$getcaps"
        extractParameterCapsOutput "$getcaps"
        if [[ $platformSupport = "false" ]]; then
            echo "    ZoomMode is not supported"
        else
            if ! [ "$pictureModes" ]; then
                pictureModes=('"Global"')
            fi
            if ! [ "$videoSources" ]; then
                videoSources=('"Global"')
            fi
            if ! [ "$videoFormats" ]; then
                videoFormats=('"Global"')
            fi
            echo "    Capability of the parameter: ColorTemperature"
            echo "        platformSupport = $platformSupport"
            echo "        videoFormats = ${videoFormats[@]}"
            echo "        videoSources = ${videoSources[@]}"
            echo "        pictureModes = ${pictureModes[@]}"
            echo "        optionsArray = ${optionsArray[@]}"

            # Handle parameters for pqmode/source/format
            for pqmode in ${pictureModes[@]}; do
                for source in ${videoSources[@]}; do
                    for format in ${videoFormats[@]}; do
                        echo "    1.1.Testing getZoomMode"
                        pictureMode_get=$pqmode
                        videoSource_get=$source
                        videoFormat_get=$format
                        if [ $pqmode = '"Global"' ]; then
                            pictureMode_get='"Current"'
                        fi

                        if [ $source = '"Global"' ]; then
                            videoSource_get='"Current"'
                        fi

                        if [ $format = '"Global"' ]; then
                            videoFormat_get='"Current"'
                        fi

                        default=$(getParameter "ZoomMode" $pictureMode_get $videoSource_get $videoFormat_get)
			echo "$default"
                        if [[ "$default" != *"true"* ]]; then
                            echo "        getZoomMode failed"
                        else
                            echo "        getZoomMode is successful"
                        fi

                        echo "    1.2.Testing setZoomMode"
                        for zoomMode in "${optionsArray[@]}"; do
                            echo "        setZoomMode option - $zoomMode"
                            setval=$(setParameter "ZoomMode" "$pqmode" "$source" "$format" "$zoomMode")
                            echo "$setval"
                            getval=$(getParameter "ZoomMode" $pictureMode_get $videoSource_get $videoFormat_get)
			    echo "$getval"
                            if [[ "$setval" != *"true"* ]]; then
                                echo "        setZoomMode failed"
                            elif [[ "$getval" != *"$zoomMode"* ]]; then
                                echo "        Not set correct value"
                            else
                                echo "        setZoomMode successful"
                            fi
                        done
                        echo "    1.3.Testing resetZoomMode"
                        resetval=$(resetParameter "ZoomMode" "$pqmode" "$source" "$format")
			echo "$resetval"
                        getval=$(getParameter "ZoomMode" $pictureMode_get $videoSource_get $videoFormat_get)
			echo "$getval"
                       if [[ "$getval" != *"$default"* ]]; then
                            echo "        reset to incorrect default value"
                        elif [[ "$resetval" != *"true"* ]]; then
                            echo "        resetZoomMode failed"
                        else
                            echo "        reset ZoomMode is scuccessful. Reset to correct default value"
                        fi
                    done
                done
            done      
            if [[ $1 = "--extended" ]]; then      
                echo "    6.4.Testing setZoomMode with a value out of the permitted range:"
                setval=$(setParameter "ZoomMode" "$pqmode" "$source" "$format" $UNDIFINEDVALUE)
		echo "$setval"
                if [[ "$setval" != *"true"* ]]; then
                    echo "        ZoomMode is not set for value $UNDIFINEDVALUE which is Expected behaviour"
                else
                    echo "        ZoomMode is set for value $UNDIFINEDVALUE which is Unexpected behaviour"
                fi
                echo ""
            fi
        fi

        #Handling modes "DolbyVisionMode" "HDR10Mode" "HLGMode"
        for mode in ${modes[@]}; do
            echo ""
            echo "--------- Testing $mode--------"
            getcaps=$(getParameterCaps $mode)
	    echo "$getcaps"
            extractParameterCapsOutput "$getcaps"
            echo "    optionsArray = ${optionsArray[@]}"
            if [[ $platformSupport = "false" ]]; then
                echo "    $mode is not supported"
            else
                if ! [ "$pictureModes" ]; then
                    pictureModes=('"Global"')
                fi
                echo "    pictureModes = ${pictureModes[@]}"

                if ! [ "$videoSources" ]; then
                    videoSources=('"Global"')
                fi
                echo "    videoSources = ${videoSources[@]}"
                if ! [ "$videoFormats" ]; then
                    videoFormats=('"Global"')
                fi
                echo "    videoFormats = ${videoFormats[@]}"

                echo "    Capability of the parameter: $mode"
                echo "        platformSupport = $platformSupport"
                echo "        videoFormats = ${videoFormats[@]}"
                echo "        videoSources = ${videoSources[@]}"
                echo "        pictureModes = ${pictureModes[@]}"
                echo "        optionsArray = ${optionsArray[@]}"

                # Handle parameters for pqmode/source
                for pqmode in ${pictureModes[@]}; do
                    for source in ${videoSources[@]}; do
                        echo "    1.1.Testing get${mode}"
                        pictureMode_get=$pqmode
                        videoSource_get=$source
                        if [ $pqmode = '"Global"' ]; then
                            pictureMode_get='"Current"'
                        fi

                        if [ $source = '"Global"' ]; then
                            videoSource_get='"Current"'
                        fi

                        default=$(getParameter $mode $pictureMode_get $videoSource_get)
			echo "Request $default"
                        if [[ "$default" != *"true"* ]]; then
                            echo "        get${mode} failed"
                        else
                            echo "        get${mode} is successful"
                        fi
                        echo "    1.2.Testing set${mode}"
                        for option in "${optionsArray[@]}"; do
                            echo "        set${mode} value - "$option
                            setval=$(setParameter $mode $pqmode $source $option)
			    echo "$setval"
                            getval=$(getParameter $mode $pictureMode_get $videoSource_get)
			    echo "$getval"
                            if [[ "$setval" != *"true"* ]]; then
                                echo "        set${mode} failed"
                            elif [[ "$getval" != *"$option"* ]]; then
                                echo "        Not set correct value"
                            else
                                echo "        set${mode} successful"
                            fi
                        done
                        echo "    1.3.Testing reset${mode}"
                        resetval=$(resetParameter $mode $pqmode $source)
			echo "$resetval"
                        getval=$(getParameter $mode $pictureMode_get $videoSource_get)
			echo "$getval"
                        if [[ "$getval" != *"$default"* ]]; then
                            echo "        reset to incorrect default value"
                        elif [[ "$resetval" != *"true"* ]]; then
                            echo "        reset${mode} failed"
                        else
                            echo "        reset ${mode} is scuccessful. Reset to correct default value"
                        fi
                    done
                done
                if [[ $1 = "--extended" ]]; then
                    echo "    6.4.Testing set${mode} with a value out of the permitted range:"
                    setval=$(setParameter $mode "$pqmode" "$source" "$format" $UNDIFINEDVALUE)
		    echo "$setval"
                    if [[ "$setval" != *"true"* ]]; then
                        echo "        ${mode} is not set for value $UNDIFINEDVALUE which is Expected behaviour"
                    else
                        echo "        ${mode} is set for value $UNDIFINEDVALUE which is Unexpected behaviour"
                    fi
                    echo ""
                fi
            fi
        done

        #Handling videoSource/videoFormat/videoFrameRate/videoResolution
        for videoParameter in ${videoParameters[@]}; do
            echo ""
            echo "--------- Testing $videoParameter--------"
            getcaps=$(getParameterCaps $videoParameter)
	    echo "$getcaps"
            extractParameterCapsOutput "$getcaps"
            if [[ $platformSupport = "false" ]]; then
                echo "    $mode is not supported"
            else
                if [[ $videoParameter = "VideoSource" ]]; then
                    echo "    Capability of the parameter: $videoParameter"
                    echo "        videoSources = ${optionsArray[@]}"
                    echo "        Tesing get${videoParameter}"
                    default=$(getParameter $videoParameter)
		    echo "$default"
                    currentVideoSource=$(echo "$default" | grep -o '"currentVideoSource":[^,}]*' | cut -d ':' -f 2 | tr -d '[:space:]":' )
                    if [[ "$default" = *"false"* ]]; then
                        echo "        get${videoParameter} failed"
                    elif [[ "${optionsArray[@]}" != *"$currentVideoSource"* ]]; then
                        echo "        get${videoParameter} returned a value which is not the capability. Get value is $currentVideoSource"
                    else
                        echo "        get${videoParameter} is successful. VideoSource is $currentVideoSource"
                    fi

                elif [[ $videoParameter = "VideoFormat" ]]; then
                    echo "    Capability of the parameter: $videoParameter"
                    echo "        videoFormats = ${optionsArray[@]}"
                    echo "        Tesing get${videoParameter}"
                    default=$(getParameter $videoParameter)
		    echo "$default"
                    currentVideoFormat=$(echo "$default" | grep -o '"currentVideoFormat":[^,}]*' | cut -d ':' -f 2 | tr -d '[:space:]":' )
                    if [[ "$default" = *"false"* ]]; then
                        echo "        get${videoParameter} failed"
                    elif [[ "${optionsArray[@]}" != *"$currentVideoFormat"* ]]; then
                        echo "        get${videoParameter} returned a value which is not the capability. Get value is $currentVideoFormat"
                    else
                        echo "        get${videoParameter} is successful. VideoFormat is $currentVideoFormat"
                    fi
                elif [[ $videoParameter = "VideoResolution" ]]; then
                    echo "    Capability of the parameter: $videoParameter"
                    echo "        maxResolution = $maxVideoResolution"
                    echo "        Tesing get${videoParameter}"
                    default=$(getParameter $videoParameter)
		    echo "$default"
                    currentVideoResolution=$(echo "$default" | grep -o '"currentVideoResolution":[^,}]*' | cut -d ':' -f 2 | tr -d '[:space:]":' )
                    if [[ "$default" = *"false"* ]]; then
                        echo "        get${videoParameter} failed"
                    else
                        echo "        get${videoParameter} is successful. VideoResolution is $currentVideoResolution"
                    fi
                else
                    echo "    Capability of the parameter: $videoParameter"
                    echo "        videoFrameRates = ${optionsArray[@]}"
                    echo "        Tesing get${videoParameter}"
                    default=$(getParameter $videoParameter)
		    echo "$default"
                    currentVideoFrameRate=$(echo "$default" | grep -o '"currentVideoFrameRate":[^,}]*' | cut -d ':' -f 2 | tr -d '[:space:]":' )
                    if [[ "$default" = *"false"* ]]; then
                        echo "        get${videoParameter} failed"
                    elif [[ "${optionsArray[@]}" != *"$currentVideoFrameRate"* ]]; then
                        echo "        get${videoParameter} returned a value which is not the capability. Get value is $currentVideoFrameRate"
                    else
                        echo "        get${videoParameter} is successful. VideoFramerate is $currentVideoFrameRate"
                    fi
                fi
            fi
        done

        #Handling PictureMode
        echo ""
        echo "--------- Testing PictureMode--------"
        getcaps=$(getParameterCaps "PictureMode")
	echo "$getcaps"
        extractParameterCapsOutput "$getcaps"
        if [[ $platformSupport = "false" ]]; then
            echo "    PictureMode is not supported"
        else
            if ! [ "$pictureModes" ]; then
                pictureModes=('"Global"')
            fi
            if ! [ "$videoSources" ]; then
                videoSources=('"Global"')
            fi
            if ! [ "$videoFormats" ]; then
                videoFormats=('"Global"')
            fi

            echo "    Capability of the parameter: PictureMode"
            echo "        platformSupport = $platformSupport"
            echo "        videoFormats = ${videoFormats[@]}"
            echo "        videoSources = ${videoSources[@]}"
            echo "        pictureModes = ${pictureModes[@]}"
            echo "        optionsArray = ${optionsArray[@]}"
            # Handle parameters for pqmode/source/format
            for source in ${videoSources[@]}; do
                for format in ${videoFormats[@]}; do
                    echo "    1.1.Testing getPictureMode"
                    videoSource_get=$source
                    videoFormat_get=$format

                    if [ $source = '"Global"' ]; then
                        videoSource_get='"Current"'
                    fi

                    if [ $format = '"Global"' ]; then
                        videoFormat_get='"Current"'
                    fi
                    default=$(getParameter "PictureMode" $videoSource_get $videoFormat_get)
		    echo "$default"
                    if [[ "$default" != *"true"* ]]; then
                        echo "        getPictureMode failed"
                    else
                        echo "        getPictureMode is successful"
                    fi
                    echo "    1.2.Testing setPictureMode"
                    for pqmode in ${optionsArray[@]}; do
                        echo "        setPictureMode - $pqmode"
                        setval=$(setParameter "PictureMode" "$pqmode" "$source" "$format")
			echo "$setval"
                        getval=$(getParameter "PictureMode" $videoSource_get $videoFormat_get)
			echo "$getval"
                        if [[ "$setval" != *"true"* ]]; then
                           echo "        setPictureMode failed"
                        elif [[ "$getval" != *"$pqmode"* ]]; then
                           echo "        Not set correct value"
                        else
                           echo "        setPictureMode successful"
                        fi
                    done
                    echo "    1.3.Testing resetPictureMode"
                    resetval=$(resetParameter "PictureMode" "$source" "$format")
		    echo "$resetval"
                    getval=$(getParameter "PictureMode" $videoSource_get $videoFormat_get)
		    echo "$getval"
                    if [[ "$getval" != *"$default"* ]]; then
                        echo "        reset to incorrect default value"
                    elif [[ "$resetval" != *"true"* ]]; then
                        echo "        resetPictureMode failed"
                    else
                        echo "        resetPictureMode is scuccessful. Reset to correct default value"
                    fi
                done
            done
            if [[ $1 = "--extended" ]]; then
                echo "    6.4.Testing setPictureMode with a value out of the permitted range:"
                setval=$(setParameter "PictureMode" $UNDIFINEDVALUE "$source" "$format")
		echo "$setval"
                if [[ "$setval" != *"true"* ]]; then
                    echo "        PictureMode is not set for value $UNDIFINEDVALUE which is Expected behaviour"
                else
                    echo "        PictureMode is set for value $UNDIFINEDVALUE which is Unexpected behaviour"
                fi
                echo ""
            fi
        fi

}

echo "TV AVOUTPUT PLUGIN TEST TOOL"
echo "---------------------"
main $@

