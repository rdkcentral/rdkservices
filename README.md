# RDK Services #

RDK services are a set of JSON-RPC based RESTful services for accessing component functionality that is available on set-top devices. RDK Services are managed and accessed through the [Thunder](https://github.com/rdkcentral/Thunder) framework. Thunder supports both HTTP and Websocket requests making the services easily accessible to [Lightning](https://github.com/rdkcentral/Lightning), Web, and native client applications. 

## Contributing to RDKServices ##

### License Requirements ###
Before RDK accepts your code into the project you must sign the RDK [Contributor License Agreement (CLA)](https://developer.rdkcentral.com/source/contribute/contribute/before_you_contribute/).

Each new file should include the latest [RDKM license header](https://developer.rdkcentral.com/source/source-code/source-code/coding_guideline/).


License for this project is included in the root directory and there shouldn't be any additional license file in any of the subfolders.

### How to contribute? ###
1. [Fork](https://docs.github.com/en/github/getting-started-with-github/quickstart/fork-a-repo) the repository, commit your changes, build and test it in at least one approved test platform/device.
2. To test it in a RDKV device, update SRC_URI and SRCREV in the [rdkservices_git.bb](https://gerrit.teamccp.com/plugins/gitiles/rdk/yocto_oe/layers/meta-rdk-video/+/2103_sprint/recipes-extended/rdkservices/rdkservices_git.bb) recipe to point to your fork.
3. Submit your changes as a [pull request](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request-from-a-fork) to the latest sprint branch.
4. If more than one developer has to work on a particular feature, request for a dev branch to be created.

### Pull request Checklist ###
1. When a pull request is submitted, Blackduck and other checks will automatically be triggered. Ensure these checks should pass (turn into green).
2. At least one reviewer needs to **review and approve** the pull request.For Metro owned plugins, at least 1 reviewer from Comcast CPE team and 1 reviewer from Metro need to review and approve.
3. For tracking and release management purpose, each pull request and all the commits in the pull request shall include **RDK ticket number(s) or Github issue number(s)** and “reason for change”.

4. Any pull request from Comcast developers should include a link to successful gerrit verification (in the comment section).
5. To verify your changeset in gerrit, submit a test gerrit change to [rdkservices_git.bb](https://gerrit.teamccp.com/plugins/gitiles/rdk/yocto_oe/layers/meta-rdk-video/+/2103_sprint/recipes-extended/rdkservices/rdkservices_git.bb) with the SRC_URI and SRCREV pointing to your fork.
6. If the changes to RDKServices require any Thunder framework changes, the contributor has to plan for a limited regression testing (with the Thunder and RDKServices changes) before submitting the pull request.

### RDKServices branches - sprint vs main (Specific to RDKV Builds) ###
1. Comcast gerrit sprint branch will point to rdkservices sprint branch.
    1. For example [2103_sprint](https://gerrit.teamccp.com/plugins/gitiles/rdk/yocto_oe/layers/meta-rdk-video/+/2103_sprint/recipes-extended/rdkservices/rdkservices_git.bb#11) branch gerrit recipe will point to [sprint/2103](https://github.com/rdkcentral/rdkservices/tree/sprint/2103) branch in github.
2. Comcast gerrit [stable2](https://gerrit.teamccp.com/plugins/gitiles/rdk/yocto_oe/layers/meta-rdk-video/+/stable2/recipes-extended/rdkservices/rdkservices_git.bb#11) branch will point to rdkservices [main](https://github.com/rdkcentral/rdkservices/tree/main) branch in github.

### Sprint/stable2 git hash updates (Specific to RDKV Builds) ###
1. Git hash in [rdkservices_git.bb (sprint_branch)](https://gerrit.teamccp.com/plugins/gitiles/rdk/yocto_oe/layers/meta-rdk-video/+/2103_sprint/recipes-extended/rdkservices/rdkservices_git.bb) for will be updated periodically (at least once a week).
    1. Dev owner to follow up as needed and update the JIRA ticket to “ready for sprint testing”.
2. Once a change is verified in the sprint branch and approved by RM, the developer can contact the maintainers to cherry-pick the change set to the [main branch](https://github.com/rdkcentral/rdkservices/tree/main) or submit the cherry picked change set to the main branch.
    1. Git hash in [rdkservices_git.bb (stable2)](https://gerrit.teamccp.com/plugins/gitiles/rdk/yocto_oe/layers/meta-rdk-video/+/stable2/recipes-extended/rdkservices/rdkservices_git.bb) will be updated periodically.
    2. Dev owner to follow up as needed and update the ticket to “ready for release testing”.
3. What if a changeset in the sprint branch fails sprint testing?
    1. The developer has to submit a pull request to undo the commit before the end of the sprint cycle or
    2. The changeset will be left in the sprint branch and won’t make into the stable2

### Upstream Vs Patch ###
1. Patches will increase the chances of build failures when the git hash is moved to a newer version.
2. We encourage everyone to upstream all the changes to GitHub instead of using patches
3. On a need basis, a developer can request the maintainers for an approval to use a patch in RDKV build (as a stop-gap measure). An unapproved patch will be rejected.

### Documentation ###
1. Each RDK Service must be fully documented in github and in [rdkcentral wiki](https://wiki.rdkcentral.com/pages/viewpage.action?pageId=98961092). 
2. Each RDK Service should have a /doc folder, and contain a markdown file of the same name as the plugin.
3. The markdown file should document each of the public methods and events, and any enumerations or other pertinent information required by a user of that API.
    1. As an alternative, this markdown file can link to the documentation at rdkcentral wiki. For example refer to [DisplaySettings](https://github.com/rdkcentral/rdkservices/blob/main/DisplaySettings/doc/DisplaySettings.md).

### Questions? ###
If you have any questions or concerns reach out to the RDKServices maintainers - [Vijay Selvaraj](mailto:VijayAnand_Selvaraj@cable.comcast.com) / [Anand Kandasamy](mailto:anand_kandasamy@comcast.com)

For a plugin specific question, maintainers might refer you to the plugin owner(s).

## Coding Guidelines ##
1. **Be Consistent**
    - If you’re editing code, take a few minutes to look at the code around you and determine its style.
    - If they use spaces around all their arithmetic operators, you should too.
    - If their comments have little boxes of hash marks around them, make your comments have little boxes of hash marks around them too.
    - The point of having style guidelines is to have a common vocabulary of coding so people can concentrate on what you’re saying rather than on how you’re saying it.
    - If code you add to a file looks drastically different from the existing code around it, it throws readers out of their rhythm when they go to read it. Avoid this.
2. RDK services are implemented as Thunder Plugins and must adhere to the [PluginHost::IPlugin](https://github.com/rdkcentral/Thunder/blob/master/Source/plugins/IPlugin.h) interface. This interface is accessible by extending the [AbstractPlugin](https://github.com/rdkcentral/rdkservices/blob/main/helpers/AbstractPlugin.h) helper class.
3. All RDK Services must have a callsign with a prefix of **org.rdk**. RDK Service name must be CamelCase and start with a capital letter.
4. All method, parameter and event names must be camelCase and start with a lower case letter.
5. Versioning :- All RDK Services by default will start with version 1. Any client facing changes (Removing APIs, Adding new APIs) to RDKServices should be made by incrementing the version to the next whole number (2,3,4...). This will ensure that clients using existing versions are not affected. Use the AbstractPlugin [registerMethod](https://github.com/rdkcentral/rdkservices/blob/main/helpers/AbstractPlugin.h#L76) helper function to register APIs to specific versions. Here is an [example](https://github.com/rdkcentral/rdkservices/commit/3692632373e8e82dba92bec56f9e6082b430a829#diff-f6cd28bb8911a0253a4601f823d3777089aecd54eb214bc6cdc227961da7b13f).
