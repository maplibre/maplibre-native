#!/usr/bin/env python
import argparse
import os
import requests
import re

###
### Script input
###
parser = argparse.ArgumentParser(description='Script to upload debuggable objects to the github release. (Note: Valid GITHUB_TOKEN must exist in the environment variable).')
parser.add_argument('-d', '--dryrun', help= 'Request for the upload url without uploading to github.', action='store_true')
requiredNamed = parser.add_argument_group('required named arguments')
requiredNamed.add_argument('-f', '--file', help= 'Provide the path to the file to be uploaded.', required=True)
requiredNamed.add_argument('-p', '--project', help= 'Provide the Mapbox git project name.', required=True)
requiredNamed.add_argument('-r', '--release', help= 'Provide the release tag which the file is uploaded to.', required=True)

args = parser.parse_args()
filePath = os.path.abspath(args.file)
release = args.release
project = args.project
isLocal = args.dryrun

# request for upload url
fileName = os.path.basename(filePath)
githubRelease = requests.get(f"https://api.github.com/repos/mapbox/{project}/releases/tags/{release}")
if githubRelease.status_code == 200:
  uploadUrl = re.sub("[\{].*?[\}]", "", githubRelease.json()["upload_url"])
  print(f"Github upload Url: {uploadUrl}?name={fileName}")
  if not isLocal:
    # upload file to github release
    print("Uploading..")
    with open(filePath, 'rb') as f:
      headers = {'content-type': 'application/octet-stream', 'authorization': f"token {os.environ.get('GITHUB_TOKEN')}"}
      r = requests.post(f"{uploadUrl}?name={fileName}", headers=headers , files={fileName: f})
      print(f"Upload response: {r.text}")
else:
  print(f"### request for upload url failed: {githubRelease.text}")