# Security Policy

## Reporting vulnerabilities

To report a vulnerability in MapLibre Native, create a [security advisory](https://github.com/maplibre/maplibre-native/security/advisories/new). You will be able to privately discuss the issue with the maintainers of MapLibre Native and we can collaborate on a fix.

We highly appreciate your reports, but we do not pay out bug bounties at this time.

We will acknowledge your report in our newsletter with your consent.

## Mitigation

We are actively trying to prevent security incidents using the following methods:

- Static analysis with the use of [clang-tidy](https://clang.llvm.org/extra/clang-tidy/).
- C++ [code scanning](https://github.com/maplibre/maplibre-native/security/code-scanning) with CodeQL.

MapLibre Native relies on several [external open-source libraries](https://github.com/maplibre/maplibre-native/issues/990). We currently do not monitor these dependencies automatically for vulnerabilities.

If you have any suggestions how to improve our security mitigation strategies, feel free to open an issue or start a Discussion. Be sure to check out the [open issues](https://github.com/maplibre/maplibre-native/labels/security) tagged with the security label.
