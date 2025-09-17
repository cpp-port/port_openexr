cmake_minimum_required(VERSION 3.16)
project(port_openexr CXX)


if (MSVC)
    #set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endif ()

list(APPEND library_source
        #Half/main_eLut.cpp
        Half/half.cpp
        #Half/main_toFloat.cpp
        Iex/IexBaseExc.cpp
        Iex/IexThrowErrnoExc.cpp
        IexMath/IexMathFloatExc.cpp
        IexMath/IexMathFpu.cpp
        #IlmImf/b44ExpLogTable.cpp
        #IlmImf/dwaLookups.cpp
        IlmImf/ImfAcesFile.cpp
        IlmImf/ImfAttribute.cpp
        IlmImf/ImfB44Compressor.cpp
        IlmImf/ImfBoxAttribute.cpp
        IlmImf/ImfChannelList.cpp
        IlmImf/ImfChannelListAttribute.cpp
        IlmImf/ImfChromaticities.cpp
        IlmImf/ImfChromaticitiesAttribute.cpp
        IlmImf/ImfCompositeDeepScanLine.cpp
        IlmImf/ImfCompressionAttribute.cpp
        IlmImf/ImfCompressor.cpp
        IlmImf/ImfConvert.cpp
        IlmImf/ImfCRgbaFile.cpp
        IlmImf/ImfDeepCompositing.cpp
        IlmImf/ImfDeepFrameBuffer.cpp
        IlmImf/ImfDeepImageStateAttribute.cpp
        IlmImf/ImfDeepScanLineInputFile.cpp
        IlmImf/ImfDeepScanLineInputPart.cpp
        IlmImf/ImfDeepScanLineOutputFile.cpp
        IlmImf/ImfDeepScanLineOutputPart.cpp
        IlmImf/ImfDeepTiledInputFile.cpp
        IlmImf/ImfDeepTiledInputPart.cpp
        IlmImf/ImfDeepTiledOutputFile.cpp
        IlmImf/ImfDeepTiledOutputPart.cpp
        IlmImf/ImfDoubleAttribute.cpp
        IlmImf/ImfDwaCompressor.cpp
        IlmImf/ImfEnvmap.cpp
        IlmImf/ImfEnvmapAttribute.cpp
        IlmImf/ImfFastHuf.cpp
        IlmImf/ImfFloatAttribute.cpp
        IlmImf/ImfFloatVectorAttribute.cpp
        IlmImf/ImfFrameBuffer.cpp
        IlmImf/ImfFramesPerSecond.cpp
        IlmImf/ImfGenericInputFile.cpp
        IlmImf/ImfGenericOutputFile.cpp
        IlmImf/ImfHeader.cpp
        IlmImf/ImfHuf.cpp
        IlmImf/ImfInputFile.cpp
        IlmImf/ImfInputPart.cpp
        IlmImf/ImfInputPartData.cpp
        IlmImf/ImfIntAttribute.cpp
        IlmImf/ImfIO.cpp
        IlmImf/ImfKeyCode.cpp
        IlmImf/ImfKeyCodeAttribute.cpp
        IlmImf/ImfLineOrderAttribute.cpp
        IlmImf/ImfLut.cpp
        IlmImf/ImfMatrixAttribute.cpp
        IlmImf/ImfMisc.cpp
        IlmImf/ImfMultiPartInputFile.cpp
        IlmImf/ImfMultiPartOutputFile.cpp
        IlmImf/ImfMultiView.cpp
        IlmImf/ImfOpaqueAttribute.cpp
        IlmImf/ImfOutputFile.cpp
        IlmImf/ImfOutputPart.cpp
        IlmImf/ImfOutputPartData.cpp
        IlmImf/ImfPartType.cpp
        IlmImf/ImfPizCompressor.cpp
        IlmImf/ImfPxr24Compressor.cpp
        IlmImf/ImfRational.cpp
        IlmImf/ImfRationalAttribute.cpp
        IlmImf/ImfRgbaFile.cpp
        IlmImf/ImfRgbaYca.cpp
        IlmImf/ImfRle.cpp
        IlmImf/ImfRleCompressor.cpp
        IlmImf/ImfScanLineInputFile.cpp
        IlmImf/ImfStandardAttributes.cpp
        IlmImf/ImfStdIO.cpp
        IlmImf/ImfStringAttribute.cpp
        IlmImf/ImfStringVectorAttribute.cpp
        IlmImf/ImfSystemSpecific.cpp
        IlmImf/ImfTestFile.cpp
        IlmImf/ImfThreading.cpp
        IlmImf/ImfThumbnailImage.cpp
        IlmImf/ImfThumbnailImageAttribute.cpp
        IlmImf/ImfTileDescriptionAttribute.cpp
        IlmImf/ImfTiledInputFile.cpp
        IlmImf/ImfTiledInputPart.cpp
        IlmImf/ImfTiledMisc.cpp
        IlmImf/ImfTiledOutputFile.cpp
        IlmImf/ImfTiledOutputPart.cpp
        IlmImf/ImfTiledRgbaFile.cpp
        IlmImf/ImfTileOffsets.cpp
        IlmImf/ImfTimeCode.cpp
        IlmImf/ImfTimeCodeAttribute.cpp
        IlmImf/ImfVecAttribute.cpp
        IlmImf/ImfVersion.cpp
        IlmImf/ImfWav.cpp
        IlmImf/ImfZip.cpp
        IlmImf/ImfZipCompressor.cpp
        IlmThread/IlmThreadPool.cpp
        IlmThread/IlmThread.cpp
        IlmThread/IlmThreadMutex.cpp
        IlmThread/IlmThreadSemaphore.cpp
        Imath/ImathBox.cpp
        Imath/ImathColorAlgo.cpp
        Imath/ImathFun.cpp
        Imath/ImathMatrixAlgo.cpp
        Imath/ImathRandom.cpp
        Imath/ImathShear.cpp
        Imath/ImathVec.cpp

)

if(${WIN32})

    list(APPEND library_source
            #Half/main_eLut.cpp
            IlmThread/IlmThreadMutexWin32.cpp
            IlmThread/IlmThreadSemaphoreWin32.cpp
            IlmThread/IlmThreadWin32.cpp

    )
endif()

if(${LINUX} OR ${ANDROID} OR ${__APPLE__} OR ${__BSD__})
message(STATUS "IlmThreadPosix at OpenEXR")
    list(APPEND library_source
            #Half/main_eLut.cpp
            IlmThread/IlmThreadPosix.cpp
            IlmThread/IlmThreadMutexPosix.cpp
            IlmThread/IlmThreadSemaphorePosix.cpp

    )

endif()

if(${UNIX_COMPAT})

    list(APPEND library_source
            IlmThread/IlmThreadSemaphorePosixCompat.cpp

    )
endif()

#if(${OPENEXR_NO_THREADING})
#
#    list(APPEND library_source
#            #Half/main_eLut.cpp
#
#    )
#
#endif()


add_library(${PROJECT_NAME} SHARED ${library_source})


if(MSVC)
target_compile_definitions(${PROJECT_NAME} PRIVATE OPENEXR_DLL ILMIMF_EXPORTS)
target_compile_definitions(${PROJECT_NAME} PRIVATE HALF_EXPORTS)
target_compile_definitions(${PROJECT_NAME} PRIVATE IMATH_EXPORTS)
target_compile_definitions(${PROJECT_NAME} PRIVATE ILMTHREAD_EXPORTS)
target_compile_definitions(${PROJECT_NAME} PRIVATE IEX_EXPORTS)
elseif(${LINUX} OR ${ANDROID} OR ${__BSD__})

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_PTHREAD=1)

endif()


if(${LINUX})
    target_link_libraries(${PROJECT_NAME} PRIVATE z)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE zlib)
endif()


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../operating_system/operating_system-${OPERATING_SYSTEM_LOWERED_NAME}/configuration/${PROJECT_NAME})



