/*
 * Copyright (C) 2015  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under the MIT License, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data_traits.hpp"
using namespace modloader;

struct timecyc_traits
{
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "time cycle properties"; }
    };
};

using OpenTimecycDetourSA = modloader::OpenFileDetour<0x5BBADE, timecyc_traits::dtraits>;
using OpenTimecycDetour3VC = modloader::LoadFileDetour<xVc(0x4D0614), timecyc_traits::dtraits>;

static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadTimeCycle = []
    {
        injector::cstd<void()>::call<0x5BBAC0>();   // CTimeCycle::Initialise
        // XXX there are some vars at the end of that function body which should not be reseted while the game runs...
        // doesn't cause serious issues but well... shall we take care of them?
    };

    if(gvm.IsIII() || gvm.IsVC())
    {
        plugin_ptr->AddDetour("timecyc.dat", reinstall_since_start, OpenTimecycDetour3VC(), gdir_refresh(ReloadTimeCycle));
    }
    else if(gvm.IsSA())
    {
        auto& timecyc_ov = plugin_ptr->AddDetour("timecyc.dat", reinstall_since_start, OpenTimecycDetourSA(), gdir_refresh(ReloadTimeCycle));
        if(GetModuleHandleA("samp"))
        {
            //
            // SAMP changes the path from any file named timecyc.dat to a custom path. This hook takes place
            // at kernel32:CreateFileA, so the only way to fix it is telling SAMP to load the timecyc using a different filename.
            //
            // So, the approach used here to get around this issue is, when the game is running under SAMP copy the
            // custom timecyc (which is somewhere at the modloader directory) into the gta3.std.data cache with a different extension and voil�. 
            //
            auto& timecyc_detour = static_cast<OpenTimecycDetourSA&>(timecyc_ov.GetInjection(0));
            timecyc_detour.OnPosTransform([plugin_ptr](std::string file) -> std::string
            {
                if(file.size())
                {
                    std::string fullpath = get<2>(plugin_ptr->cache.AddCacheFile("timecyc.samp", true));

                    if(!CopyFileA(
                        std::string(plugin_ptr->loader->gamepath).append(file).c_str(),
                        (fullpath).c_str(),
                        FALSE))
                        plugin_ptr->Log("Warning: Failed to make timecyc for SAMP.");
                    else
                        file = std::move(fullpath);
                }
                return file;
            });
        }
    }
});



