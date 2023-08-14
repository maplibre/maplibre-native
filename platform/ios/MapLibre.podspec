Pod::Spec.new do |s|
    version = '6.0.0-pre0'

    s.name = 'MapLibre'
    s.version = version
    s.license = { :type => 'BSD', :text => '' }
    s.homepage = 'https://maplibre.org/'
    s.authors = { 'MapLibre' => '' }
    s.summary = 'Open source vector map solution for iOS with full styling capabilities.'
    s.platform = :ios
    s.source = { 
        :http => "https://github.com/maplibre/maplibre-native/releases/download/ios-v#{version.to_s}/MapLibre-#{version.to_s}.zip",
        :flatten => false
    }
    s.social_media_url  = 'https://mastodon.social/@maplibre'
    s.ios.deployment_target = '11.0'
    s.ios.vendored_frameworks = "**/MapLibre.xcframework"
end
