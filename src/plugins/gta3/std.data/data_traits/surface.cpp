/*
 * Copyright (C) 2014  LINK/2012 <dma_2012@hotmail.com>
 * Licensed under GNU GPL v3, see LICENSE at top level directory.
 * 
 */
#include <stdinc.hpp>
#include "../data.hpp"
using namespace modloader;

//
struct surface_traits : public data_traits
{
    static const bool can_cache         = true;     // Can this store get cached?
    static const bool is_reversed_kv    = false;    // Does the key contains the data instead of the value in the key-value pair?
    static const bool has_sections      = false;    // Does this data file contains sections?
    static const bool per_line_section  = false;    // Is the sections of this data file different on each line?

    // Detouring traits
    struct dtraits : modloader::dtraits::OpenFile
    {
        static const char* what() { return "surface adhesion limits"; }
    };
    
    // Detouring type
    using detour_type = modloader::OpenFileDetour<0x55D100, dtraits>;

    // Dominance Flags
    using domflags_fn = datalib::domflags_fn<flag_RemoveIfNotExistInOneCustomButInDefault>;

    // The first string from the line doesn't matter, it's just a helper to the human reader.
    // The following values are real numbers, '---' like values or nothing
    using key_type      = int;
    using value_type    = data_slice<dummy_string, std::list<either<real_t, std::string>>>; // (XXX list because of a move issue with either)

    key_type key_from_value(const value_type&)
    {
        return adhesion_line++;
    }

public:
    int adhesion_line = 0;

    template<class Archive>
    void serialize(Archive& archive)
    { archive(this->adhesion_line); }
};

//
using surface_store = gta3::data_store<surface_traits, std::map<
                        surface_traits::key_type, surface_traits::value_type
                        >>;


//
static auto xinit = initializer([](DataPlugin* plugin_ptr)
{
    auto ReloadSurfaceInfo = std::bind(injector::thiscall<void(void*)>::call<0x55F420>, mem_ptr(0xB79538).get<void>());
    plugin_ptr->AddMerger<surface_store>("surface.dat", true, false, false, reinstall_since_start, gdir_refresh(ReloadSurfaceInfo));
});
