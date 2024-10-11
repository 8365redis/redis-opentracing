#include <string>

class VersionManager {
public:
    static VersionManager& GetInstance() {
        static VersionManager instance;  
        return instance;
    }

    VersionManager(const VersionManager&) = delete;
    VersionManager& operator=(const VersionManager&) = delete;

    void Set_Module_Version(const char* version_string) {
        version_str = std::string(version_string);
        version = ParseVersion(version_string); 
    }
    int Get_Module_Version() const {
        return version;
    }
    std::string Get_Module_Version_Str() const {
        return version_str;
    }

private:
    VersionManager() : version(0) {}
    int ParseVersion(const char* version_string) {
        int major = 0, minor = 0, patch = 0;
        sscanf(version_string, "%d.%d.%d", &major, &minor, &patch);
        return major * 10000 + minor * 100 + patch;
    }
    int version;
    std::string version_str;
};
