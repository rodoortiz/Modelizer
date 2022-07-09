include(FetchContent)

macro(get_repository name repository branch)
    FetchContent_Declare(${name}
            GIT_REPOSITORY ${repository}
            GIT_TAG ${branch}
            GIT_PROGRESS 1
            GIT_SHALLOW 1)

    FetchContent_GetProperties(${name})

    if (NOT ${name}_POPULATED)
        message("Fetching ${name}...")
        FetchContent_Populate(${name})
        message("Fetch of ${name} finished")
    endif()
endmacro()