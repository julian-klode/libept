//#include <ept/core/apt.h>
#include <ept/config.h>
#include <ept/debtags/maint/path.h>

#include <wibble/test.h>

#include <apt-pkg/pkgcache.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/error.h>
#include <apt-pkg/policy.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/init.h>
#include <cstdlib>


#ifndef EPT_TEST_H
#define EPT_TEST_H

struct AptTestEnvironment {
    //ept::core::AptDatabase db;
    AptTestEnvironment() {
        pkgInitConfig (*_config);
        _config->Set("Initialized", 1);
        _config->Set("Dir", TEST_ENV_DIR);
        _config->Set("Dir::Cache", "cache");
        _config->Set("Dir::State", "state");
        _config->Set("Dir::Etc", "etc");
        _config->Set("Dir::Etc::sourcelist", "sources.list");
        _config->Set("Dir::State::status", TEST_ENV_DIR "dpkg-status");
        pkgInitSystem (*_config, _system);
    }
};

struct DebtagsTestEnvironment : AptTestEnvironment {
    ept::debtags::Path::OverrideDebtagsSourceDir odsd;
    ept::debtags::Path::OverrideDebtagsIndexDir odid;
    ept::debtags::Path::OverrideDebtagsUserSourceDir odusd;
    ept::debtags::Path::OverrideDebtagsUserIndexDir oduid;

    DebtagsTestEnvironment()
        : odsd( TEST_ENV_DIR "debtags/"),
          odid( TEST_ENV_DIR "debtags/"),
          odusd( TEST_ENV_DIR "debtags/"),
          oduid( TEST_ENV_DIR "debtags/")
    {}
};

struct EnvOverride
{
    const char* name;
    bool old_value_set;
    std::string old_value;

    EnvOverride(const char* name, const char* value)
        : name(name)
    {
        const char* old = getenv(name);
        if (old)
        {
            old_value_set = true;
            old_value = old;
        } else
            old_value_set = false;
        setenv(name, value, 1);
    }
    ~EnvOverride()
    {
        if (old_value_set)
            setenv(name, old_value.c_str(), 1);
        else
            unsetenv(name);
    }
};

#endif
