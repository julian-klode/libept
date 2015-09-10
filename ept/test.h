#ifndef EPT_TEST_H
#define EPT_TEST_H

#include <ept/utils/tests.h>
#include <ept/config.h>
#include <apt-pkg/pkgcache.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/error.h>
#include <apt-pkg/policy.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/init.h>
#include <cstdlib>

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
