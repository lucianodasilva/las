# las
Idioms and tools I noticed I tend to re-write on every single project i'm in.

# vcpkg - Update package version
1. Update *vcpkg.json* file with new version.
2. Commit and send to repository
3. Gather full SHA number for release "commit"
4. Update registry *portfile.cmake* file with SHA in the REF parameter of vcpkg_from_github function call
5. Run ```vcpkg install --overlay-ports=[local registry path]/ports las```
6. If execution fails with "error download from... had an unexpected hash", get the actual hash and replace the vcpkg_from_github's SHA512 argument in the *portfile.cmake*
7. Copy the vcpkg.json file from the project into the registrie's ports/las/vcpkg.json file
8. Run ```vcpkg format-manifest ports/las/vcpkg.json```
9. Commit the package changes to the repository
10. Run ```vcpkg x-add-version --x-builtin-ports-root=./ports --x-builtin-registry-versions-dir=./versions las``` to update versions
11. Commit the registries changes and push to server