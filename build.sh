conan install . -if=build -b=missing -s build_type=Debug -s cppstd=17
conan build -if=build -bf=build .
conan package -if=build -bf=build -pf=bin .
