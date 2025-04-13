Pod::Spec.new do |s|
    version = "#{ENV['VERSION']}"

    s.name = 'MapLibre'
    s.version = version
    s.license = { :type => 'BSD', :file => "LICENSE.md" }
    s.homepage = 'https://maplibre.org/'
    s.authors = { 'MapLibre' => '' }
    s.summary = 'Open source vector map solution for iOS with full styling capabilities.'
    s.platform = :ios
    s.source = {
        :http => "https://github.com/maplibre/maplibre-native/releases/download/ios-v#{version.to_s}/MapLibre.dynamic.xcframework.zip",
        :type => "zip"
    }
    s.social_media_url  = 'https://mastodon.social/@maplibre'
    s.ios.deployment_target = '12.0'
    s.ios.vendored_frameworks = "MapLibre.xcframework"
end
