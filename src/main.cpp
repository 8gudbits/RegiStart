#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

struct AutoStartEntry
{
    std::wstring name;
    std::wstring path;
    std::wstring location;
    std::wstring parameters;
};

class RegistryAutoStartScanner
{
private:
    std::vector<AutoStartEntry> entries;

public:
    void ScanAllLocations()
    {
        ScanHKCURun();
        ScanHKLMRun();
        ScanHKCURunOnce();
        ScanHKLMRunOnce();
        ScanHKLMWow6432Run();
        ScanHKCUWow6432Run();
        ScanExplorerRun();
        ScanPoliciesExplorerRun();
    }

    void DisplayResults()
    {
        if (entries.empty())
        {
            std::wcout << L"\n";
            std::wcout << L"No auto-start programs found.\n";
            return;
        }

        for (size_t i = 0; i < entries.size(); ++i)
        {
            const auto &entry = entries[i];
            std::wcout << L"\n";
            std::wcout << L"[" << i + 1 << L"] " << entry.name << L"\n";
            std::wcout << L"    Location: " << entry.location << L"\n";
            std::wcout << L"    Path: " << entry.path << L"\n";
            if (!entry.parameters.empty())
            {
                std::wcout << L"    Parameters: " << entry.parameters << L"\n";
            }
        }
    }

    bool RemoveEntry(int index)
    {
        if (index < 1 || static_cast<size_t>(index) > entries.size())
        {
            std::wcout << L"Invalid index: " << index << L"\n";
            return false;
        }

        const AutoStartEntry &entry = entries[index - 1];

        HKEY hRoot = GetRootKeyFromLocation(entry.location);
        std::wstring subKey = GetSubKeyFromLocation(entry.location);

        if (hRoot == NULL)
        {
            std::wcout << L"Unsupported registry location: " << entry.location << L"\n";
            return false;
        }

        HKEY hKey;
        LONG result = RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);

        if (result != ERROR_SUCCESS)
        {
            std::wcout << L"Failed to open registry key: " << subKey << L" (Error: " << result << L")\n";
            return false;
        }

        result = RegDeleteValueW(hKey, entry.name.c_str());
        RegCloseKey(hKey);

        if (result == ERROR_SUCCESS)
        {
            std::wcout << L"Successfully removed: " << entry.name << L"\n";
            return true;
        }
        else
        {
            std::wcout << L"Failed to remove: " << entry.name << L" (Error: " << result << L")\n";
            return false;
        }
    }

    bool AddEntry(const std::wstring &location, const std::wstring &name, const std::wstring &path, const std::wstring &parameters = L"")
    {
        HKEY hRoot = GetRootKeyFromLocation(location);
        std::wstring subKey = GetSubKeyFromLocation(location);

        if (hRoot == NULL)
        {
            std::wcout << L"Unsupported registry location: " << location << L"\n";
            return false;
        }

        HKEY hKey;
        LONG result = RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_SET_VALUE, &hKey);

        if (result != ERROR_SUCCESS)
        {
            // Try to create the key if it doesn't exist
            result = RegCreateKeyExW(hRoot, subKey.c_str(), 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);
            if (result != ERROR_SUCCESS)
            {
                std::wcout << L"Failed to open/create registry key: " << subKey << L" (Error: " << result << L")\n";
                return false;
            }
        }

        // Build the full command line
        std::wstring fullCommand = path;
        if (!parameters.empty())
        {
            // If path has spaces, quote it
            if (path.find(L' ') != std::wstring::npos)
            {
                fullCommand = L"\"" + path + L"\" " + parameters;
            }
            else
            {
                fullCommand = path + L" " + parameters;
            }
        }
        else if (path.find(L' ') != std::wstring::npos)
        {
            // Quote paths with spaces even without parameters
            fullCommand = L"\"" + path + L"\"";
        }

        result = RegSetValueExW(hKey, name.c_str(), 0, REG_SZ,
                                (BYTE *)fullCommand.c_str(),
                                (fullCommand.length() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);

        if (result == ERROR_SUCCESS)
        {
            std::wcout << L"Successfully added: " << name << L" to " << location << L"\n";
            return true;
        }
        else
        {
            std::wcout << L"Failed to add: " << name << L" (Error: " << result << L")\n";
            return false;
        }
    }

    void ShowAvailableLocations()
    {
        std::wcout << L"\nAvailable registry locations:\n";
        std::wcout << L"1. HKEY_CURRENT_USER\\Run\n";
        std::wcout << L"2. HKEY_LOCAL_MACHINE\\Run\n";
        std::wcout << L"3. HKEY_CURRENT_USER\\RunOnce\n";
        std::wcout << L"4. HKEY_LOCAL_MACHINE\\RunOnce\n";
        std::wcout << L"5. HKEY_CURRENT_USER\\WOW6432Node\\Run\n";
        std::wcout << L"6. HKEY_LOCAL_MACHINE\\WOW6432Node\\Run\n";
        std::wcout << L"7. HKEY_CURRENT_USER\\Explorer\\Run\n";
        std::wcout << L"8. HKEY_CURRENT_USER\\Policies\\Explorer\\Run\n";
        std::wcout << L"9. HKEY_LOCAL_MACHINE\\Policies\\Explorer\\Run\n";
    }

    const std::vector<AutoStartEntry> &GetEntries() const
    {
        return entries;
    }

private:
    HKEY GetRootKeyFromLocation(const std::wstring &location)
    {
        if (location.find(L"HKEY_CURRENT_USER") == 0)
            return HKEY_CURRENT_USER;
        else if (location.find(L"HKEY_LOCAL_MACHINE") == 0)
            return HKEY_LOCAL_MACHINE;
        else
            return NULL;
    }

    std::wstring GetSubKeyFromLocation(const std::wstring &location)
    {
        if (location == L"HKEY_CURRENT_USER\\Run")
            return L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        else if (location == L"HKEY_LOCAL_MACHINE\\Run")
            return L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        else if (location == L"HKEY_CURRENT_USER\\RunOnce")
            return L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
        else if (location == L"HKEY_LOCAL_MACHINE\\RunOnce")
            return L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
        else if (location == L"HKEY_CURRENT_USER\\WOW6432Node\\Run")
            return L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";
        else if (location == L"HKEY_LOCAL_MACHINE\\WOW6432Node\\Run")
            return L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";
        else if (location == L"HKEY_CURRENT_USER\\Explorer\\Run")
            return L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Run";
        else if (location == L"HKEY_CURRENT_USER\\Policies\\Explorer\\Run")
            return L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run";
        else if (location == L"HKEY_LOCAL_MACHINE\\Policies\\Explorer\\Run")
            return L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run";
        else
            return L"";
    }

    void ScanRegistryKey(HKEY hKey, const std::wstring &keyPath, const std::wstring &locationName)
    {
        HKEY hSubKey;
        LONG result = RegOpenKeyExW(hKey, keyPath.c_str(), 0, KEY_READ, &hSubKey);

        if (result != ERROR_SUCCESS)
            return;

        DWORD index = 0;
        WCHAR valueName[256];
        WCHAR valueData[1024];
        DWORD valueNameSize, valueDataSize, valueType;

        while (true)
        {
            valueNameSize = sizeof(valueName) / sizeof(valueName[0]);
            valueDataSize = sizeof(valueData);

            result = RegEnumValueW(hSubKey, index, valueName, &valueNameSize,
                                   NULL, &valueType, (LPBYTE)valueData, &valueDataSize);

            if (result != ERROR_SUCCESS)
                break;

            if (valueType == REG_SZ || valueType == REG_EXPAND_SZ)
            {
                AutoStartEntry entry;
                entry.name = valueName;
                entry.path = valueData;
                entry.location = locationName;
                ParseCommandLine(entry);
                entries.push_back(entry);
            }
            index++;
        }
        RegCloseKey(hSubKey);
    }

    void ParseCommandLine(AutoStartEntry &entry)
    {
        std::wstring &cmdLine = entry.path;
        if (cmdLine.length() > 1 && cmdLine[0] == L'"')
        {
            size_t endQuote = cmdLine.find(L'"', 1);
            if (endQuote != std::wstring::npos)
            {
                entry.parameters = cmdLine.substr(endQuote + 1);
                size_t firstNonSpace = entry.parameters.find_first_not_of(L" ");
                if (firstNonSpace != std::wstring::npos)
                {
                    entry.parameters = entry.parameters.substr(firstNonSpace);
                }
                entry.path = cmdLine.substr(1, endQuote - 1);
            }
        }
        else if (cmdLine.find(L"%") != std::wstring::npos)
        {
            WCHAR expandedPath[MAX_PATH];
            DWORD result = ExpandEnvironmentStringsW(cmdLine.c_str(), expandedPath, MAX_PATH);
            if (result > 0 && result <= MAX_PATH)
            {
                entry.path = expandedPath;
            }
        }
    }

    void ScanHKCURun()
    {
        ScanRegistryKey(HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                        L"HKEY_CURRENT_USER\\Run");
    }

    void ScanHKLMRun()
    {
        ScanRegistryKey(HKEY_LOCAL_MACHINE,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                        L"HKEY_LOCAL_MACHINE\\Run");
    }

    void ScanHKCURunOnce()
    {
        ScanRegistryKey(HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
                        L"HKEY_CURRENT_USER\\RunOnce");
    }

    void ScanHKLMRunOnce()
    {
        ScanRegistryKey(HKEY_LOCAL_MACHINE,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
                        L"HKEY_LOCAL_MACHINE\\RunOnce");
    }

    void ScanHKLMWow6432Run()
    {
        ScanRegistryKey(HKEY_LOCAL_MACHINE,
                        L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",
                        L"HKEY_LOCAL_MACHINE\\WOW6432Node\\Run");
    }

    void ScanHKCUWow6432Run()
    {
        ScanRegistryKey(HKEY_CURRENT_USER,
                        L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",
                        L"HKEY_CURRENT_USER\\WOW6432Node\\Run");
    }

    void ScanExplorerRun()
    {
        ScanRegistryKey(HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Run",
                        L"HKEY_CURRENT_USER\\Explorer\\Run");
    }

    void ScanPoliciesExplorerRun()
    {
        ScanRegistryKey(HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run",
                        L"HKEY_CURRENT_USER\\Policies\\Explorer\\Run");

        ScanRegistryKey(HKEY_LOCAL_MACHINE,
                        L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run",
                        L"HKEY_LOCAL_MACHINE\\Policies\\Explorer\\Run");
    }
};

void ShowBanner()
{
    std::cout << "\n";
    std::cout << "RegiStart 1.0 (x64) : (c) 8gudbits - All rights reserved.\n";
    std::cout << "Source - \"https://github.com/8gudbits/8gudbitsKit\"\n";
}

void ShowHelp()
{
    ShowBanner();
    std::cout << "\n";
    std::cout << "Usage: RegiStart [OPTIONS]\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                 Show this help message\n";
    std::cout << "  -s, --scan                 Scan and display auto-start programs (default)\n";
    std::cout << "  -n, --nobanner             Suppress banner display\n";
    std::cout << "  -r, --remove <index>       Remove an entry by index number\n";
    std::cout << "  -a, --add <location> <name> <path> [parameters]\n";
    std::cout << "                             Add a new auto-start entry\n";
    std::cout << "  -l, --locations            Show available registry locations\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  RegiStart -a \"HKEY_CURRENT_USER\\Run\" \"MyApp\" \"C:\\MyApp.exe\" \"--silent\"\n";
    std::cout << "  RegiStart -r 5\n";
    std::cout << "  RegiStart -l\n";
}

std::wstring StringToWString(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

int main(int argc, char *argv[])
{
    bool showBanner = true;
    int removeIndex = -1;
    bool showLocations = false;
    std::vector<std::string> addArgs;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            ShowHelp();
            return 0;
        }
        else if (arg == "-n" || arg == "--nobanner")
        {
            showBanner = false;
        }
        else if (arg == "-l" || arg == "--locations")
        {
            showLocations = true;
        }
        else if (arg == "-r" || arg == "--remove")
        {
            if (i + 1 < argc)
            {
                removeIndex = std::atoi(argv[++i]);
            }
            else
            {
                std::cout << "Error: -r option requires an index number\n";
                return 1;
            }
        }
        else if (arg == "-a" || arg == "--add")
        {
            if (i + 3 < argc)
            {
                addArgs.push_back(argv[++i]); // location
                addArgs.push_back(argv[++i]); // name
                addArgs.push_back(argv[++i]); // path
                // Check for optional parameters
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    addArgs.push_back(argv[++i]); // parameters
                }
            }
            else
            {
                std::cout << "Error: -a option requires at least 3 arguments: location, name, path\n";
                return 1;
            }
        }
    }

    if (showBanner)
    {
        ShowBanner();
    }

    RegistryAutoStartScanner scanner;

    if (showLocations)
    {
        scanner.ShowAvailableLocations();
        return 0;
    }

    if (!addArgs.empty())
    {
        std::wstring location = StringToWString(addArgs[0]);
        std::wstring name = StringToWString(addArgs[1]);
        std::wstring path = StringToWString(addArgs[2]);
        std::wstring parameters = addArgs.size() > 3 ? StringToWString(addArgs[3]) : L"";

        return scanner.AddEntry(location, name, path, parameters) ? 0 : 1;
    }

    scanner.ScanAllLocations();

    if (removeIndex != -1)
    {
        if (scanner.GetEntries().empty())
        {
            std::wcout << L"No entries found to remove.\n";
            return 1;
        }
        return scanner.RemoveEntry(removeIndex) ? 0 : 1;
    }
    else
    {
        scanner.DisplayResults();
    }

    return 0;
}

