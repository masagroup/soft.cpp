# soft.cpp: contributing

> How to set up a development environment, the Git and Jira workflow and the conventions of `soft.cpp`. Back to the [README](README.md) · [Architecture](ARCHITECTURE.md)

## Development environment

The build of the repository is described in the [README](README.md#build); the tooling of the ecosystem is on the transverse pages:

- Windows, Ant and sw.poney: Apache Ant, JDK 8, Visual Studio 2022 Professional with the MSVC v140 toolset and the Windows SDK 10.0.19041.0, `gh` authenticated, access to `\\file1` or a local `sw.poney` clone in `PONEY_HOME`; initialise the MSVC environment before any `ant` (see [Build on Windows](https://github.com/masagroup/wiki/wiki/build-windows)).
- Linux and Docker Compose: Docker with BuildKit, the GitHub PAT file named by `SECRET_GITHUB_PAT` (`~/.masabot/github.pat`) or the SSH key of `SECRET_GITHUB_SSH` for the private Go modules and npm packages, the profiles `prod`, `test`, `pack` and `dev` (see [Build on Linux](https://github.com/masagroup/wiki/wiki/build-linux)); never push an image, the CI does it.
- C++: the Windows toolset above, or the development container of the repository on Linux (`docker compose --profile dev up -d`, then `cmake` and `ninja` inside it).
- Editor rules for Claude Code: the shared `claude/` folder of [cicd](https://github.com/masagroup/cicd) installs the same `CLAUDE.md` for every developer (rules, roles, build pages, conventions).

## Git and Jira workflow

- The organisation repository `masagroup/soft.cpp` is the reference (CI, releases); work on a fork or on a branch of it, never directly on `master` nor on a release branch (`6.28`...).
- Branch name: `<TICKET>-<short-description>` (for example `SWNG-421-tests-and-fixes`), created from the target branch of the ticket.
- Commit message: `<type>(<scope>): <TICKET-ID>: <imperative description in English>`, with `type` in `feat`, `fix`, `refactor`, `test`, `build`, `ci`, `perf`, `docs`, `chore`; the body says what and why, and the CI impact when there is one. One commit is one coherent change; no `Co-Authored-By` nor other automatic signature.
- Pull request: masabot reads the Jira keys (`SW`, `SWNG`, `IO`, `AI`, `GEFORA`, `COA`) in the title, the branch, the body and the commits, comments the Jira issue and moves it (*In Progress*, *In Review* with `[DONE]`, *In QA* or *In Validation* after the merge). The PR is built rebased on its base branch; the comment commands `/rebase`, `/rebase-all` and `/automerge` are available. See [CI/CD pipelines](https://github.com/masagroup/wiki/wiki/cicd-pipelines).
- Never push to `master` by hand, never create a release or push an image: the CI does it.

## Conventions

- Code, comments, commit messages and file names in English; documentation of the repository in English.
- Formatting of the repository (`clang-format`, `gofmt`/`goimports`, `prettier`/`eslint`) on the lines you touch; no reformatting of a whole file for a one-line change; no dead code, no `TODO` without a ticket; logs at the right level, never a secret.
- C++: C++17 as accepted by both MSVC v140 (`vc140_x64`) and recent GCC/Clang; never replace a forward declaration by an include in a header; the frozen libraries (Boost 1.56, Qt 4.8.7, protobuf 2.x on the old gen) bound what is available; determinism of the simulation (no iteration over unordered containers for observable results, no dependency on addresses, randomness and clock from the simulation only); modern CMake, `std::filesystem`, no hard-coded path.
- Shared contracts (`.proto`, `.ecore`, OpenAPI, exported Go modules, file formats) are changed with the consumers in mind: grep the workspace, state the compatibility strategy, update the dependants.
- Documentation: this README set (README, ARCHITECTURE, CONTRIBUTING, README.LEGACY) is the source of the [SWORD wiki](https://github.com/masagroup/wiki/wiki/soft.cpp); keep the metadata block at the top of the README up to date (the `apps` list feeds the Interfaces section).

## CI/CD

| Item | State |
|---|---|
| Workflow files | none: there is no `.github/` directory and no `uses: masagroup/cicd/...` reference |
| Docker image | none (no Dockerfile, no `packages-latest` image) |
| Artifacts | only the local `dist/ecore-<ecore.version>-win64.zip` from `ant ecore.dist` |
| Publication | `build.xml` has no `<update>`, `<update-gh>` or `<upload>` task, so there is no upload to `file1` or to GitHub release assets from this repo |

Tags `v1.0.0` to `v1.2.4` exist (`v1.0.0`..`v1.2.1` annotated; `v1.2.2`..`v1.2.4` lightweight). The `Readme.md` says binaries (VC 2019) are on the releases page of `soft.gen`. See [cicd](https://github.com/masagroup/cicd) and the ecosystem wiki page [CI/CD pipelines](https://github.com/masagroup/wiki/wiki/cicd-pipelines) for the shared pipelines this repo does not use.
