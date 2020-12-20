#!/usr/bin/python

import os
import json

path = os.getcwd()
with open(path + "/LICENSE.md", 'w') as licenseFile:
    licenseFile.write("<!-- This file was generated. Use `make android-license` to update. -->  \n")
    licenseFile.write("mapbox-gl-native-android copyright 2014-2020 Mapbox. \n\n"
    "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following "
    "conditions are met:  \n\n"
    "1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.  \n\n"
    "2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following "
    "disclaimer in the documentation and/or other materials provided with the distribution.  \n\n"
    "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED "
    "WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A "
    "PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR "
    "ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED "
    "TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) "
    "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING "
    "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY "
    "OF SUCH DAMAGE.")
    licenseFile.write("\n\n===========================================================================\n\n")
    with open(path + "/MapboxGLAndroidSDK/build/reports/licenses/licenseReleaseReport.json", 'r') as dataFile:
        data = json.load(dataFile)

        gradleLicensePlugin ="""
        {
            "project": "Gradle License Plugin",
            "url": "https://github.com/jaredsburrows/gradle-license-plugin",
            "licenses": [
                {
                    "license": "The Apache Software License, Version 2.0",
                    "license_url": "http://www.apache.org/licenses/LICENSE-2.0.txt"
                }
             ]
        }
        """
        data.append(json.loads(gradleLicensePlugin))

        licenseName = ""
        licenseUrl = ""
        for entry in data:
            projectName = entry["project"]
            projectUrl = entry["url"]
            for license in entry["licenses"]:
                licenseName = license["license"]
                licenseUrl = license["license_url"]

            licenseFile.write("Mapbox GL uses portions of %s.  \n" % projectName +
                              ("URL: [%s](%s)  \n" % (projectUrl, projectUrl) if projectUrl is not None else "") +
                              "License: [%s](%s)" % (licenseName, licenseUrl) +
                              "\n\n===========================================================================\n\n")
