# Contributing

When contributing to this repository, please first discuss the change you wish to make via issue,
email, or any other method with the owners of this repository before making a change.

Please note we have a code of conduct, please follow it in all your interactions with the project.

## Pull Request Process

1. Ensure any install or build dependencies are removed before the end of the layer when doing a
   build.
2. Create a new branch with a keyword of your contribution or main issue.
3. Update the README.md with details of changes to the new features or fixed settings done.
4. Commit your additions and deletions, or new files to the branch created previously.
5. Run `just check` and `just test`. The first compiles every example and checks the generated pages; the second runs the unit tests. CI runs both on your pull request.
6. When the changes are stable and run effectively, make a pull request to main branch.

## Review of pull request

The maintainer named in `library.properties` reviews each pull request and merges it to main once CI passes. Changes are released from main in batches.

## Versioning

The library follows [Semantic Versioning](https://semver.org/). Leave the `version` in `library.properties` alone in a pull request; it is set when a release is cut. Say in the pull request whether the change is a fix, a new feature, or a change to existing behaviour, which decides whether the next release is a patch, minor or major version.
