function(add_unit_test_project ProjectName)
	set(gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE)
	file(GLOB_RECURSE TestSources "test/*.cpp" "test/*.h")
	add_executable("${ProjectName}.UnitTests" ${TestSources})
	add_test(NAME "${ProjectName}.UnitTests" COMMAND "${ProjectName}.UnitTests")
	target_link_libraries("${ProjectName}.UnitTests" PUBLIC ${ProjectName} gtest gmock)
endfunction()
