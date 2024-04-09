#RDK services L2 Tests #

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
act -W .github/workflows/L2-tests.yml -s GITHUB_TOKEN=[token]
```

`-r, --reuse` to reuse the container.

Get a bash shell in the container, if needed:

```shell script
docker ps
docker exec -it <container name> /bin/bash
```

## FAQ ##

1. The commands to build and run tests are in [L2-tests.yml](../../.github/workflows/L2-tests.yml).
 For the queries on syntax please refer to the [Workflow syntax for GitHub Actions](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions).

2. External dependencies need to be mocked.
 For each external header, add one into the [l2tests.cmake](../../l2tests.cmake) (preserve the original path parts, if needed).
 For each external declaration, add one into the [mocks folder](../mocks).
 For the queries on mocks please refer to the [gMock Cookbook](http://google.github.io/googletest/gmock_cook_book.html).

3. For the queries on how to write tests please refer to the [GoogleTest User’s Guide](https://google.github.io/googletest/).
 Common recommendations:
   - Tests should be fast.
   - Tests should be independent and repeatable.
   - Find the L2 usecase/logic from your plugin which you can test.  Isolate each use case from other use cases.
   - Implement the Arrange-Act-Assert patterns for each step in a use case.
     If two or more tests operate on similar data, use a test fixture.
     Use `ON_CALL` by default, and only use `EXPECT_CALL` when intend to verify that the call is made.

4. To add L2 tests for a RDK service
 Required steps:
   - Set plugin ON by default in [l2tests.cmake](../../l2tests.cmake)
  	  Example : set(PLUGIN_SYSTEMSERVICES ON)

   - Add required empty headers,definitions and mock files in [l2tests.cmake](../../l2tests.cmake) to pass compilation for the added plugin
   - Update yml file as well to enable the plugin.
      Example : -DPLUGIN_SYSTEMSERVICES=ON

   - Add new L2 test file for the plugin in folder rdkservices/L2Tests/tests with name Pluginname_L2tests
       Example : SystemServices_L2test.cpp

   - Add a class (with pluginname_L2tests as name) which inherits the mock class for getting all the mocks required by plugin.
       Example :class Systemservice_L2test : public L2TestMocks

   -  Add Test fixtures for each usecase.

5. To run L2 tests for specific suite (plugin)
    During test/code development, we often do not want to run all the tests. Instead, to save time, we can use the following method to run specified test suite(s).

 Required steps:
   - In yml file in the step where it runs the L2test binary RDKServiceL2test add the test suite name you want to run
   Example :
       a. If we want to run single suite
          RDKServicesL2test System (this is internally considered as 'System*' and gtest framework runs all tests added with suite name starting with 'System')
       b. If we want to run multiple suites
          RDKServicesL2test System Usb (this is internally considered as 'System*:Usb*' and gtest framework runs all tests added with suite name starting with 'System' and all tests with suite name starting with 'Usb')


  Note: TEST_COMPLETION_TIMEOUT specifies a time limit for completing the specified tests.  This can be increased/decreased by changing the value in  [CMakeLists.txt](./CMakeLists.txt).


