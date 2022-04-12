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

```shell script
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash
```

```shell script
sudo apt remove docker docker-engine docker.io
sudo apt update
sudo apt -y install docker.io
sudo snap install docker
```

Create a [personal access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token).

Invoke the workflow.

```shell script
sudo ./bin/act -W .github/workflows/unit-tests.yml -s GITHUB_TOKEN=[token]
```

`-r, --reuse` to reuse the container.

Get a bash shell in the container.

```shell script
sudo docker ps
sudo docker exec -it <container name> /bin/bash
```
