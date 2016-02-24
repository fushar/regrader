# Change Log

## [2.2.0] - 2016-02-21

### Added
- First-to-solve flag for each problem in contest.
- MathJax support for problem description.
- `.env` support.
- Custom table prefix and names. Now you can have more than one Regrader instances running with only one database.
- Multiple grader instances support.

### Fixed
- Institution logo size bug. Now the logo would fit inside scoreboard's cell even if it's bigger than the cell.

### Changed
- MOE now should be configured manually instead of configured along with Regrader installation.

## [2.1.0] - 2015-08-09

### Added
- This CHANGELOG.
- #58: A publish script that removes unused files from dependencies

### Fixed
- #57: Incorrect assets reference.
- #25: Cannot edit user without changing password.

## 2.0.0 - 2015-07-11

### Added
- #29, #53: Dependency manager(s) for the assets.
- #54: Download script for moe.

### Changed
- #15: Upgrade to CodeIgniter 3.
- #55: Change license to MIT.

[2.1.0]: https://github.com/fushar/regrader/compare/v2.0.0...v2.1.0
