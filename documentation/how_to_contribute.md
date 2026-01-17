# How to Contribute

**Version:** 2.0 | **Last Updated:** January 2026

Welcome to the R-Type project! This guide will help you get started as a new developer.

---

## Getting Started

### Prerequisites

Make sure you have installed:

- Git
- CMake (v3.20+)
- A C++20 compatible compiler (g++ 10+, clang 12+, or MSVC 19.28+)
- clang-format

> **Note:** Dependencies (SFML, ASIO, Catch2) are automatically managed via vcpkg. No manual library installation required.

### Clone the Repository

```bash
git clone git@github.com:EpitechPGE3-2025/G-CPP-500-PAR-5-2-rtype-3.git
cd G-CPP-500-PAR-5-2-rtype-3
```

### Build the Project (Automated)

```bash
chmod +x ./build.sh
./build.sh
```

### Build the Project (Manual)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --parallel
```

### Clean the Build

```bash
chmod +x ./clean.sh
./clean.sh
```

### Run the Project

**Server:**

```bash
./bin/linux/r-type_server 4242
```

**Client:**

```bash
./bin/linux/r-type_client 127.0.0.1 4242
```

**Tests:**

```bash
./unit_tests
```

---

## Branching Convention

We use strict branch naming conventions. Both `-` and `/` separators are accepted:

| Type          | Format                                 | Example                                          |
| ------------- | -------------------------------------- | ------------------------------------------------ |
| Feature       | `feat-<name>` or `feat/<name>`         | `feat-spectator-mode` or `feat/spectator-mode`   |
| Bug Fix       | `fix-<name>` or `fix/<name>`           | `fix-collision-bug` or `fix/collision-bug`       |
| Documentation | `docs-<name>` or `docs/<name>`         | `docs-readme-update` or `docs/readme-update`     |
| Refactor      | `refactor-<name>` or `refactor/<name>` | `refactor-ecs-cleanup` or `refactor/ecs-cleanup` |

---

## Workflow

### 1. Create a Branch

```bash
git checkout dev
git pull origin dev
git checkout -b feat-my-new-feature
# or: git checkout -b feat/my-new-feature
```

### 2. Make Your Changes

- Respect **Clean Includes** — No relative backtracking (e.g., `../../`)
- Use CMake `target_include_directories` pattern
- Correct: `#include "components/Position.hpp"`
- Incorrect: `#include "../../ecs/include/components/Position.hpp"` ❌

### 3. Commit Your Changes

Use [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/):

```bash
git add .
git commit -m "feat(ecs): add SpectatorComponent for observer mode"
```

**Commit types:** `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

### 4. Push Your Branch

```bash
git push origin feat-my-new-feature
```

### 5. Create a Pull Request

Open a PR on GitHub targeting the `dev` branch.

> **⚠️ IMPORTANT:** CI must pass on **both Windows AND Linux** before your PR can be merged.

---

## CI Requirements

Before your PR can be merged:

- ✅ **Linux Build** — Must compile successfully
- ✅ **Windows Build (MSVC)** — Must compile successfully
- ✅ **Commit Lint** — Messages must follow conventional commits
- ✅ **Clang-Format** — Code must be properly formatted

Check the CI status on your PR before requesting a review.

---

## Code Guidelines

- Follow the **Clean Include Strategy** (no `../../` paths)
- Write clear and maintainable code
- Add comments where necessary
- Ensure your code compiles without warnings

### Pull Request Checklist

Before submitting:

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] Code formatted with clang-format
- [ ] Commit messages follow convention
- [ ] No conflicts with target branch
- [ ] Tests added for new features
- [ ] Documentation updated if needed
- [ ] **CI passes (Windows & Linux)**

---

## Release Process (Maintainers Only)

To trigger a new automatic release on GitHub:

1. **Switch to main branch:**

   ```bash
   git checkout main
   git pull origin main
   ```

2. **Create a Tag** (Semantic Versioning):

   ```bash
   git tag v1.0.1
   ```

3. **Push the Tag** (triggers GitHub Actions Release):

   ```bash
   git push origin v1.0.1
   ```

This will build Windows binaries and create a GitHub Release with attached artifacts.

---

## Testing

- Run the provided unit tests
- Make sure new features are tested
- Verify the project still builds and runs correctly

---

## Communication

- Discuss your ideas on the project's issue tracker
- Ask for help if you get stuck

---

## Additional Resources

- [Developer Documentation](./DEVELOPER_DOCUMENTATION.md)
- [User Guide](./USER_GUIDE.md)
- [Conventional Commits](https://www.conventionalcommits.org/)
