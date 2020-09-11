#!/bin/bash

testdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
url="http://127.0.0.1:9998/Service/"
curl_opts="--silent -X POST ${url} --header \"Content-Type: application/json\""
container_id="TestContainer"

bundlesdir="${testdir}/TestBundles/"
specsdir="${testdir}/DobbySpecs/"

# Start container with OCI bundle from ./TestBundles directory
start_container_from_bundle()
{
    echo ""
    # Check if path to bundle or bundle name
    if grep -q "/" <<< "$1"; then
        container_id="$(echo ${1} | sed 's/.*\///')"
        bundle_path="${1}"
    else
        container_id="${1}"
        bundle_path="${bundlesdir}${1}"
        bundle_tarball="${bundlesdir}${1}.tar.gz"

        # Extract tarball if it hasn't already been extracted
        if [ ! -d "${bundle_path}" ] && [ -f "${bundle_tarball}" ]; then
            tar -xzf "${bundle_tarball}" -C "${bundlesdir}"
        fi
    fi

    if [ -d "${bundle_path}" ]; then
        response=$(curl ${curl_opts} -d '{
            "jsonrpc": "2.0",
            "id": 3,
            "method": "OCIContainer.1.startContainer",
            "params":{
                "containerId": "'${container_id}'",
                "bundlePath": "'${bundle_path}'"
            }
        }')
        echo ${response}
    else
        echo "ERROR: bundle not found in ${bundle_path}/"
    fi
    echo ""
}

# Start container with Dobby spec from ./DobbySpecs directory
start_container_from_spec()
{
    echo ""
    dobby_spec="${specsdir}/${1}.json"

    if [ -f "${dobby_spec}" ]; then
        response=$(curl ${curl_opts} -d '{
            "jsonrpc": "2.0",
            "id": 3,
            "method": "OCIContainer.1.startContainerFromDobbySpec",
            "params":{
                "containerId": "'${1}'",
                "dobbySpec": '"$(<${dobby_spec})"'
            }
        }')
        echo ${response}
    else
        echo "ERROR: Dobby spec '${1}.json' not found in ${specsdir}/"
    fi
    echo ""
}

# Stop a running container
stop_container()
{
    echo ""
    response=$(curl ${curl_opts} -d '{
        "jsonrpc": "2.0",
        "id": 3,
        "method": "OCIContainer.1.stopContainer",
        "params":{
            "containerId": "'${1}'"
        }
    }')

    echo ${response}
    echo ""
}

# List all running containers
list_containers()
{
    echo ""
    response=$(curl ${curl_opts} -d '{
        "jsonrpc": "2.0",
        "id": 3,
        "method": "OCIContainer.1.listContainers"
    }')

    echo ${response}
    echo ""
}

# Get a container's info
get_container_info()
{
    echo ""
    response=$(curl ${curl_opts} -d '{
        "jsonrpc": "2.0",
        "id": 3,
        "method": "OCIContainer.1.getContainerInfo",
        "params":{
            "containerId": "'${1}'"
        }
    }')

    echo ${response}
    echo ""
}

# Get a container's state
get_container_state()
{
    echo ""
    response=$(curl ${curl_opts} -d '{
        "jsonrpc": "2.0",
        "id": 3,
        "method": "OCIContainer.1.getContainerState",
        "params":{
            "containerId": "'${1}'"
        }
    }')

    echo ${response}
    echo ""
}

# Pause a container
pause_container()
{
    echo ""
    response=$(curl ${curl_opts} -d '{
        "jsonrpc": "2.0",
        "id": 3,
        "method": "OCIContainer.1.pauseContainer",
        "params":{
            "containerId": "'${1}'"
        }
    }')

    echo ${response}
    echo ""
}

# Resume a paused container
resume_container()
{
    echo ""
    response=$(curl ${curl_opts} -d '{
        "jsonrpc": "2.0",
        "id": 3,
        "method": "OCIContainer.1.resumeContainer",
        "params":{
            "containerId": "'${1}'"
        }
    }')

    echo ${response}
    echo ""
}

# Run the executeCommand method with input params
execute_command()
{
    args=("$@")
    options=""
    command=""
    options_empty=""

    # Move options and command into their respective strings
    for i in "${args[@]:1:${#args[@]}}"; do # args from args[1] (=containerId) until the end of the array
        if [[ $i  == -* && -z "$options_empty" ]]; then
            options="$options$i "
        else
            options_empty="1"
            command="$command$i "
        fi
    done

    # Remove trailing spaces
    options=$(sed 's/ *$//g' <<< $options)
    command=$(sed 's/ *$//g' <<< $command)

    echo ""
    response=$(curl ${curl_opts} -d '{
        "jsonrpc": "2.0",
        "id": 3,
        "method": "OCIContainer.1.executeCommand",
        "params":{
            "containerId": "'${1}'",
            "options": "'"${options}"'",
            "command": "'"${command}"'"
        }
    }')

    echo ${response}
    echo ""
}

# Available commands
ocicontainer_help()
{
    echo ""
    echo ""
    echo "Commands:"
    echo ""
    echo "s <container name or bundle path> - start container from an OCI bundle in ./TestBundles or from given path."
    echo "ss <container name> - start container from a dobby spec in ./DobbySpecs."
    echo "t <container name> - stop container."
    echo "l <container name> - list containers."
    echo "i <container name> - get container info."
    echo "g <container name> - get container state."
    echo "p <container name> - pause container."
    echo "r <container name> - resume paused container."
    echo "e <container name> [options...] <command> - execute a command in container."
    echo "h - help."
    echo "x - exit the script."
    echo ""
}

main()
{
    # Read user input and create an args array out of it
    read -p "Command:" input
    read -a args <<< ${input}

    case ${args[0]} in
    s)
        if [ ${args[1]} ]; then
            start_container_from_bundle ${args[1]}
        else
            echo "ERROR: Missing arguments. Try 's <container name or bundle path>.'"
            echo ""
        fi
    ;;
    ss)
        if [ ${args[1]} ]; then
            start_container_from_spec ${args[1]}
        else
            echo "ERROR: Missing arguments. Try 'ss <container name>.'"
            echo ""
        fi
    ;;
    t)
        if [ ${args[1]} ]; then
            stop_container ${args[1]}
        else
            echo "ERROR: Missing arguments. Try 't <container name>.'"
            echo ""
        fi
    ;;
    l)
        list_containers ;;
    i)
        if [ ${args[1]} ]; then
            get_container_info ${args[1]}
        else
            echo "ERROR: Missing arguments. Try 'i <container name>.'"
            echo ""
        fi
    ;;
    g)
        if [ ${args[1]} ]; then
            get_container_state ${args[1]}
        else
            echo "ERROR: Missing arguments. Try 'g <container name>.'"
            echo ""
        fi
    ;;
    p)
        if [ ${args[1]} ]; then
            pause_container ${args[1]}
        else
            echo "ERROR: Missing arguments. Try 'p <container name>.'"
            echo ""
        fi
    ;;

    r)
        if [ ${args[1]} ]; then
            resume_container ${args[1]}
        else
            echo "ERROR: Missing arguments. Try 'r <container name>.'"
            echo ""
        fi
    ;;
    e)
        if [[ (${args[1]} && ${args[2]}) ]]; then
            execute_command ${args[@]:1:${#args[@]}} # args from args[1] until the end of the array
        else
            echo "ERROR: Missing arguments. Try 'e [options...] <container name> <command>.'"
            echo ""
        fi
    ;;
    h)
        ocicontainer_help ;;
    x)
        exit ;;
    X)
        exit ;;
    *)
        echo "Invalid selection."
        echo ""
    ;;
    esac
    main
}

echo "OCIContainer test tool"
echo "------------------------"
ocicontainer_help
main
