## Contributing In General
Our project welcomes external contributions. If you have an itch, please feel
free to scratch it.

To contribute code or documentation, please submit a [pull request](https://github.com/DS4SD/deepsearch-glm/pulls).

A good way to familiarize yourself with the codebase and contribution process is
to look for and tackle low-hanging fruit in the [issue tracker](https://github.com/DS4SD/deepsearch-glm/issues).
Before embarking on a more ambitious contribution, please quickly [get in touch](#communication) with us.

For general questions or support requests, please refer to the [discussion section](https://github.com/DS4SD/deepsearch-glm/discussions).

**Note: We appreciate your effort, and want to avoid a situation where a contribution
requires extensive rework (by you or by us), sits in backlog for a long time, or
cannot be accepted at all!**

### Proposing new features

If you would like to implement a new feature, please [raise an issue](https://github.com/DS4SD/deepsearch-glm/issues)
before sending a pull request so the feature can be discussed. This is to avoid
you wasting your valuable time working on a feature that the project developers
are not interested in accepting into the code base.

### Fixing bugs

If you would like to fix a bug, please [raise an issue](https://github.com/DS4SD/deepsearch-glm/issues) before sending a
pull request so it can be tracked.

### Merge approval

The project maintainers use LGTM (Looks Good To Me) in comments on the code
review to indicate acceptance. A change requires LGTMs from two of the
maintainers of each component affected.

For a list of the maintainers, see the [MAINTAINERS.md](MAINTAINERS.md) page.


## Legal

Each source file must include a license header for the MIT
Software. Using the SPDX format is the simplest approach.
e.g.

```
/*
Copyright IBM Inc. All rights reserved.

SPDX-License-Identifier: MIT
*/
```

We have tried to make it as easy as possible to make contributions. This
applies to how we handle the legal aspects of contribution. We use the
same approach - the [Developer's Certificate of Origin 1.1 (DCO)](https://github.com/hyperledger/fabric/blob/master/docs/source/DCO1.1.txt) - that the Linux® Kernel [community](https://elinux.org/Developer_Certificate_Of_Origin)
uses to manage code contributions.

We simply ask that when submitting a patch for review, the developer
must include a sign-off statement in the commit message.

Here is an example Signed-off-by line, which indicates that the
submitter accepts the DCO:

```
Signed-off-by: John Doe <john.doe@example.com>
```

You can include this automatically when you commit a change to your
local git repository using the following command:

```
git commit -s
```


## Communication

Please feel free to connect with us using the [discussion section](https://github.com/DS4SD/deepsearch-glm/discussions).



## Developing

### Usage of uv

We use uv to manage dependencies.


#### Install

To install, see the documentation here: https://docs.astral.sh/uv/getting-started/installation/

1. Install uv globally on your machine
    ```bash
    curl -LsSf https://astral.sh/uv/install.sh | sh
    ```

2. Make sure uv is in your `$PATH`
    - for `zsh`
        ```sh
        echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
        ```
    - for `bash`
        ```sh
        echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
        ```
3. The official guidelines linked above include useful details on shell completion.


#### Create a Virtual Environment and Install Dependencies

To create the virtual environment and install dependencies, run:

```bash
uv sync --all-extras
```

To run commands inside the environment, use `uv run`, for example:

```bash
uv run pytest
```

**(Advanced) Use a Specific Python Version**

This project supports Python 3.10 and newer. To use a specific supported Python version, run:

```bash
uv python pin 3.10
uv sync --all-extras
```


#### Add a new dependency

```bash
uv add NAME
```


## Updating the Generated Swagger Client

**Note:** This requires Docker.

To update the Swagger Client with the new API specifications, run:

```bash
tools/swagger-client-generator <URL to the DS>
```

Example:

```bash
tools/swagger-client-generator.sh https://DEEPSEARCH_HOST
```

If you don't want to download the Swagger Specification, and just want to use the local files in `tools/swagger-client-generator`, run:

```bash
tools/swagger-client-generator.sh .
```


## Coding style guidelines

We use the following tools to enforce code style:

- iSort, to sort imports
- Black, to format code
- Pylint, to lint code
- Mypy, to check typing specs


We run a series of checks on the code base on every commit, using `pre-commit`. To install the hooks, run:

```bash
pre-commit install
```

To run the checks on-demand, run:

```
pre-commit run --all-files
```

Note: Checks like `Black` and `isort` will "fail" if they modify files. This is because `pre-commit` doesn't like to see files modified by their Hooks. In these cases, `git add` the modified files and `git commit` again.



## Documentation

We use [MkDocs](https://www.mkdocs.org/) to write documentation.

To run the documentation server, do:

```bash
mkdocs serve
```

The server will be available on [http://localhost:8000](http://localhost:8000).

### Pushing Documentation to GitHub pages

Run the following:

```bash
mkdocs gh-deploy
```
