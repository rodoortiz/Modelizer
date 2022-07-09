macro(create_plugin target name companyName manufacturerCode pluginCode version)
    juce_add_plugin(${target}
            PLUGIN_MANUFACTURER_CODE ${manufacturerCode}
            PLUGIN_CODE ${pluginCode}
            FORMATS AU VST3
            PRODUCT_NAME ${name}
            COMPANY_NAME ${companyName}
            IS_SYNTH FALSE
            NEEDS_MIDI_INPUT FALSE
            COPY_PLUGIN_AFTER_BUILD TRUE
            VERSION ${version})

    juce_generate_juce_header(${target})

    target_compile_definitions(${target}
            PUBLIC
            # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
            JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
            JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
            JUCE_VST3_CAN_REPLACE_VST2=0
            JUCE_DISPLAY_SPLASH_SCREEN=0
            DONT_SET_USING_JUCE_NAMESPACE=1
            )

    target_link_libraries(${target}
            PRIVATE
            juce::juce_audio_basics
            juce::juce_audio_devices
            juce::juce_audio_formats
            juce::juce_audio_plugin_client
            juce::juce_audio_processors
            juce::juce_audio_utils
            juce::juce_core
            juce::juce_data_structures
            juce::juce_dsp
            juce::juce_events
            juce::juce_graphics
            juce::juce_gui_basics
            juce::juce_gui_extra
            PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
            "${TORCH_LIBRARIES}")

    file(GLOB_RECURSE RESOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/Assets/
    )
    
    if(RESOURCES)
      juce_add_binary_data(plugin_resources SOURCES ${RESOURCES})
      target_link_libraries(${target} PUBLIC pligin_resources)
    endif()

endmacro()