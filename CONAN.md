# Conan Setup for IDEs

In a 'off source' folder run both these commands before opening the ide for the first time:
```
conan install .. -s build_type=Release -o testing=True --build=missing
conan install .. -s build_type=Debug -o testing=True --build=missing
```