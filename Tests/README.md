# Unit Tests #

## How to run locally ##

Install [act](https://github.com/nektos/act) and docker:

##### Ubuntu #####

```shell script
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash
sudo apt update
sudo apt -y install docker.io
```

##### macOS #####

```shell script
brew install act
brew install --cask docker
```

Create a [personal access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token).

Invoke the workflow:

```shell script
act -W .github/workflows/unit-tests.yml -s GITHUB_TOKEN=[token]
```

`-r, --reuse` to reuse the container.

Get a bash shell in the container, if needed:

```shell script
docker ps
docker exec -it <container name> /bin/bash
```

## FAQ ##

1. The commands to build and run tests are in [unit-tests.yml](../.github/workflows/unit-tests.yml).
 For the queries on syntax please refer to the [Workflow syntax for GitHub Actions](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions).

2. External dependencies need to be mocked.
 For each external header, add one into the [tests.cmake](../tests.cmake) (preserve the original path parts, if needed).
 For each external declaration, add one into the [mocks folder](./mocks).
 For the queries on mocks please refer to the [gMock Cookbook](http://google.github.io/googletest/gmock_cook_book.html).

3. For the queries on how to write tests please refer to the [GoogleTest User’s Guide](https://google.github.io/googletest/).
 Common recommendations:
   - Tests should be fast.
   - Tests should be independent and repeatable.
   - If two or more tests operate on similar data, use a test fixture.
   - Fixture constructor/destructor should be preferred over `SetUp()/TearDown()` unless using `ASSERT_xx`.
   - `EXPECT_*` are preferred, as they allow more than one failure to be reported in a test.
     However, use `ASSERT_*` if it doesn’t make sense to continue when the assertion in question fails.
   - Having more constraints than necessary is bad.
     If a test over-specifies, it doesn’t leave enough freedom to the implementation.
     Use `ON_CALL` by default, and only use `EXPECT_CALL` when intend to verify that the call is made.
     Suppress GMOCK WARNING-s with NiceMock.

4. Before review:
   - See the [common mistakes](https://github.com/rdkcentral/rdkservices/issues/3711).
   - Enable [ClangFormat](./.clang-format) and make sure the tests code is formatted.
   - For the tests, the code is built with flags `-Wall -Werror`.
     Make sure the code builds without warnings.
   - At the bottom of the workflow summary page on GitHub, there is a section with artifacts.
     Artifacts include coverage report and valgrind log.
     They help to understand how much code is covered by tests and whether there are memory leaks.
     Make sure you check both.
