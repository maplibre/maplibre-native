# Release

To make an Android release, do the following:

* Update `CHANGELOG.md` in a pull request
  * Add a section title containing the version which should be released, e.g. `## 9.6.0 - December 18, 2022`
  * Remove the `* Add your pull request...` entries
  * Add a new `main` section at the top of the changelog

* Once the changelog update pull request was merged, tag the commit:
  * Create a tag locally, e.g. `git tag -a android-v9.6.0 -m "Release android-v9.6.0"`
  * You need write access to push the tag, e.g. `git push --atomic origin main android-v9.6.0`

* Due to maven timeouts, the `android-release.yml` workflows currently needs a self-hosted runner
  * Go to https://github.com/maplibre/maplibre-gl-native/settings/actions/runners
  * Press "New self-hosted runner"
  * Spin up a self-hosted Linux x64 runner
  * Make sure the runner has the tags `Linux` and `self-hosted`

* Once the tag is pushed, you can run the `release-android.yml` workflow
  * Go to https://github.com/maplibre/maplibre-gl-native/actions
  * Press `android-release`
  * Press `Run workflow` from the `main` branch
