if (MSVC)
  add_compile_options("$<$<CONFIG:RELEASE>:/O2>")
  add_compile_options("$<$<CONFIG:RELEASE>:/MD>")
  add_compile_options("$<$<CONFIG:DEBUG>:/MDd>")
  add_compile_options("/Zi" "/EHsc" "/GR")

  add_definitions(
    /EHsc
    /MP
    /permissive-
    /Zi
    /W4
    /WX
  )
endif()