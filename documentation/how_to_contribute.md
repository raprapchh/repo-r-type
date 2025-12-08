# How to Contribute

Welcome to the R-Type project! This guide will help you get started as a new developer.

## Getting Started

### Prerequisites
Make sure you have installed:
- Git
- CMake
- A C++20 compatible compiler
- SFML (for client)
- ASIO (networking)
- Catch2 (unit tests)
- clang-format

### Clone the Repository
``` bash
git clone https://github.com/raprapchh/rtype.git
cd rtype
```
### Build the Project (Automated)
``` bash
chmod +x ./build.sh
./build.sh
```

### Build the Project (Manual)
``` bash
mkdir build
cd build
cmake ..
make
```

### To clean up the build directory
``` bash
chmod +x ./clean.sh
./clean.sh
```

### Run the Project
- Server:
``` bash
./r-type_server
```
- Client:
./r-type_client
``` bash
./r-type_client
```
- Tests:
``` bash
./unit_tests
```

## Workflow
1. **Create a branch**
       git checkout -b feature/my-new-feature
2. **Make your changes**
3. **Commit your changes**
``` bash
git add .
git commit -m "Add a short, meaningful commit message and respect the norme"
```
[Conventional Commit norme](https://www.conventionalcommits.org/en/v1.0.0/)
4. **Push your branch**
``` bash
git push origin feature/my-new-feature
```
5. **Create a Pull Request** on GitHub

## Code Guidelines
- Follow consistent naming conventions.
- Write clear and maintainable code.
- Add comments where necessary.
- Ensure your code compiles without warnings.

## Testing
- Run the provided unit tests.
- Make sure new features are tested.
- Verify the project still builds and runs correctly.

## Communication
- Discuss your ideas on the project’s issue tracker or Slack/Discord channel.
- Ask for help if you get stuck.

## Additional Resources
- [Project Wiki](https://github.com/raprapchh/rtype/wiki)
- [Coding Standards](https://github.com/raprapchh/rtype/blob/main/CODING_STANDARD.md)
