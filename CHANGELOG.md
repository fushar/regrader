# Change Log

## [3.0.0] - 2018-11-30

### Breaking
- #83 Update PHP requirement from >=5.4 to >=5.6
- #61 Different CSS for "first-to-solve"
  - Requires new column `accepted_time datetime DEFAULT NULL` in `scoreboard_admin` and `scoreboard_contestant` tables.

### Added
- #86 Highlight current user in scoreboard

### Fixed
- #80 Fix now-invalid default MySQL datetime value: '0000-00-00 00:00:00'

## [2.2.2] - 2017-04-22

### Fixed
- #77 Add missing `.env.example` in publishing flow

## [2.2.1] - 2016-03-23

### Fixed
- #70 Add missing Indonesian system language
- #71 Add missing published files for phpdotenv support

## [2.2.0] - 2016-03-19

### Added
- #63 Use phpdotenv for better env variable support

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
