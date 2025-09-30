# RegiStart

CLI tool that can be used to manage Windows registry auto-start programs. It displays all applications configured to start automatically with Windows and provides options to add or remove entries from registry locations.

## Usage

```bash
Usage: RegiStart [OPTIONS]
Options:
  -h, --help                 Show this help message
  -s, --scan                 Scan and display auto-start programs (default)
  -n, --nobanner             Suppress banner display
  -r, --remove <index>       Remove an entry by index number
  -a, --add <location> <name> <path> [parameters]
                             Add a new auto-start entry
  -l, --locations            Show available registry locations

Examples:
  RegiStart -a "HKEY_CURRENT_USER\Run" "MyApp" "C:\MyApp.exe" "--silent"
  RegiStart -r 5
  RegiStart -l
```

## Download exe for Windows

This tool is part of the [8gudbitsKit](https://github.com/8gudbits/8gudbitsKit) project. To download the executable for Windows, visit the [8gudbitsKit](https://github.com/8gudbits/8gudbitsKit) repository.

## For the Tech People

- Scans multiple Windows Registry locations including Run, RunOnce, WOW6432Node, and Explorer keys
- Uses Windows Registry API (advapi32) for direct registry access
- Parses command-line arguments with quoting and parameter separation
- Handles REG_SZ and REG_EXPAND_SZ value types with environment variable expansion
- Supports both HKEY_CURRENT_USER and HKEY_LOCAL_MACHINE registry hives

