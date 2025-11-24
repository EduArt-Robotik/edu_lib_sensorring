# Installation


<div class="tabbed">

- <b class="tab-title">Base Theme</b><div class="darkmode_inverted_image">
    ![](img/theme-variants-base.drawio.svg)
    </div>
    Comes with the typical Doxygen titlebar. Optionally the treeview in the sidebar can be enabled.

    Required files: `doxygen-awesome.css`

    Required `Doxyfile` configuration:
    ```
    GENERATE_TREEVIEW      = YES # optional. Also works without treeview
    DISABLE_INDEX = NO
    FULL_SIDEBAR = NO
    HTML_EXTRA_STYLESHEET  = doxygen-awesome-css/doxygen-awesome.css
    HTML_COLORSTYLE        = LIGHT # required with Doxygen >= 1.9.5
    ```

- <b class="tab-title">Sidebar-Only Theme</b><div class="darkmode_inverted_image">
    ![](img/theme-variants-sidebar-only.drawio.svg)
    </div>
    Hides the top titlebar to give more space to the content. The treeview must be enabled in order for this theme to work.

    Required files: `doxygen-awesome.css`, `doxygen-awesome-sidebar-only.css`

    Required `Doxyfile` configuration:
    ```

    GENERATE_TREEVIEW      = YES # required!
    DISABLE_INDEX          = NO
    FULL_SIDEBAR           = NO
    HTML_EXTRA_STYLESHEET  = doxygen-awesome-css/doxygen-awesome.css \
                            doxygen-awesome-css/doxygen-awesome-sidebar-only.css
    HTML_COLORSTYLE        = LIGHT # required with Doxygen >= 1.9.5
    ```

</div>




## 5. Building and installing the library
The library is built with a standard CMake workflow.
To build the library execute the following commands starting in the root of the repository:
```
mkdir build
cd build
cmake ..
cmake --build . -j
```

Use this command to install the library to a custom location:
```
cmake --install . --prefix <install-path>
```

If the library is installed to a non-standard location, the directory should be added to the `CMAKE_PREFIX_PATH` environment variable. This enables cmake to find and include the library automatically in other projects. Add an export command to your .bashrc script or edit the environment variable file of your OS to set the path permanently.

```
export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:<install-path>
```



For an automatic build and automatic installation the above commands are also available as convenience scripts [make_release.bash](cmake/make_release.bash) and [install_release.bash](cmake/install_release.bash). These scripts should be called from the root of the repository.

```
cmake/make_release.bash
```
```
cmake/install_release.bash
```
> **Note on the convenience scripts**:<br>If the bash script does not execute it may need to be converted to a file with unix style line endings. Run `dos2unix ./cmake/*` for an automatic conversion before executing the script.

> **Notes on the installation script:**<br>  - The installation script automatically runs the build script before installing the library. <br>- The installation path in the script is hard-coded and should be changed if desired. <br> - Don't forget to set the `CMAKE_PREFIX_PATH` when using the script.

In addition to the make and install scripts there is a [remove_installation.bash](cmake/remove_installation.bash) cleanup script that removes the library when it is installed at the default installation location.
```
cmake/remove_installation.bash

```

<div class="section_buttons"> 

| Read Previous | Read Next |
|:--|--:|
| [Hardware](01_hardware.md) | [Software](03_software.md) |

</div>
