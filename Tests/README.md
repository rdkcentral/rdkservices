# Unit Tests #

Supply the CMake options or other steps for your plugins in [unit-tests.yml](../.github/workflows/unit-tests.yml).

Read the [GoogleTest User’s Guide](https://google.github.io/googletest/) and write the tests.

Enable [ClangFormat](./.clang-format).

- [headers](./headers) - third party headers for the plugins
- [mocks](./mocks) - mocks for the tests
- [source](./source) - other sources for the tests
- [tests](./tests) - the tests

## How to run locally ##

Install act and docker.

##### Ubuntu #####

```shell script
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash
sudo apt remove docker docker-engine docker.io
sudo apt update
sudo apt -y install docker.io
sudo snap install docker
```

##### macOS #####

```shell script
brew install act
brew install --cask docker
```

Create a [personal access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token).

Invoke the workflow.

```shell script
act -W .github/workflows/unit-tests.yml -s GITHUB_TOKEN=[token]
```

`-r, --reuse` to reuse the container.

Get a bash shell in the container.

```shell script
docker ps
docker exec -it <container name> /bin/bash
```

## Tips and tricks ##

### GCC ###

`-I dir` adds the dir to the list of directories to be searched for headers.\
`-Wl,-wrap,symbol` uses a wrapper function for symbol.

### CMake ###

`-DCMAKE_DISABLE_FIND_PACKAGE_<PackageName>=ON` disables non-REQUIRED `find_package` call.

### GitHub ###

At the bottom of the workflow summary page on GitHub, there is a section with artifacts. [unit-tests.yml](../.github/workflows/unit-tests.yml) uploads: coverage, valgrind_log.
