from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class LasRecipe(ConanFile):

    name = "las"
    version = "1.0"
    package_type = "library"

    # Optional metadata
    license = "MIT"
    author = "Luciano da Silva luciano.silva@live.com.pt"
    url = "https://github.com:lucianodasilva/las.git"
    description = "Tools, idioms and that kind of stuff"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "testing": [True, False]}

    default_options = {"shared": False, "fPIC": True, "testing": False}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = \
        "CMakeLists.txt", \
        "las/*", \
        "las-test/*"

    generators = "CMakeDeps"

    def requirements(self):
        if self.options.testing:
            self.requires("catch2/3.3.2")

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)

        tc.variables["LAS_UNIT_TEST"] = self.options.testing

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["las", "las-test"]