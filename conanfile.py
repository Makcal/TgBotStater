import os

from conan import ConanFile
from conan.tools.files import copy
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMake

required_conan_version = ">=2.0"


class TgBotStaterConan(ConanFile):
    name = "tgbotstater"
    version = "0.1.0"
    description = "A C++ library for constructing Telegram bots in compile-time!"
    author = "Maxim Fomin fominmaxim3721@gmail.com"
    license = "MIT"

    url = "https://github.com/Makcal/TgBotStater"
    topics = ("telegram", "telegram-bot")

    package_type = "header-library"
    exports_sources = "include/*", "CMakeLists.txt"
    no_copy_source = True
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("tgbot/1.8")
        self.requires("brigand/cpp11-1.3.0", options={"with_boost": False})

    def validate(self):
        check_min_cppstd(self, 23)

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.libs = ["TgBotStater"]
        self.cpp_info.includedirs = ['include']
        self.cpp_info.bindirs = []

    def package_id(self):
        self.info.clear()
