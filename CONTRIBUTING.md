## How to contribute to Gravitymon

#### **Did you find a bug?**

* **Ensure the bug was not already reported** by searching on GitHub under [Issues](https://github.com/mp-se/gravitymon/issues). Dont forget to look under closed issues. There might be a fix but not yet included in the released version.

* If you're unable to find an open issue addressing the problem, [open a new one](https://github.com/mp-se/gravitymon/issues/new). Be sure to include a **title and clear description**, as much relevant information as possible, Use the function on the device to extract configuration and device information (does not contain any sensitive data). This can help to pinpoint the issue. 

#### **Did you write a patch that fixes a bug?**

* Open a new GitHub pull request with the patch.

* Ensure the PR description clearly describes the problem and solution. Include the relevant issue number if applicable.

* Before submitting, please use `pre-commit` to validate that your code contribution complies with the formatting standards for C++ and C. 

* Check the contribution section under the documentation for additional information.

#### **Do you intend to add a new feature or change an existing one?**

* Suggest your change in the [Discussion forums](https://github.com/mp-se/gravitymon/discussions) and start writing code.

* Do not open an issue on GitHub until you have collected positive feedback about the change. GitHub issues are primarily intended for bug reports and fixes.

#### **Do you have questions about the source software?**

* Start a discussion in the [Discussion forums](https://github.com/mp-se/gravitymon/discussions) and start writing code.

#### **Do you want to contribute to the documentation?**

* Open a new GitHub pull request with the updated documentation changes.

* Ensure the PR description clearly describes the problem and solution. Include the relevant issue number if applicable.

#### **Adding a new board**

It's now possible to add new boards to Gravitymon, these will not be tested as part of the release process since I dont really have access to all of them. This should be a showstopper since I test all the supported board types. In order to add a new board type do the following:

* If the board is not supported by Arduino/Platformio create the defintions and add them to the boards root directory. Place the pin defintions in a subfolder with the board id.
* Add a new target to the platformio.ini file with the board defintion and configuration needed. Use the naming standard [env:gravity-boardid] for example [env:gravity-d1_mini], the result of this build will be copied to bin/custom-boardid.bin for example bin/custom-d1_mini.bin to indicate this is a custom board.
* Update the documentation at src_docs/custom.rst
* Test and verify
* Create a PR

Thanks! 
