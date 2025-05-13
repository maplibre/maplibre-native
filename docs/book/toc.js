// Populate the sidebar
//
// This is a script, and not included directly in the page, to control the total size of the book.
// The TOC contains an entry for each page, so if each page includes a copy of the TOC,
// the total size of the page becomes O(n**2).
class MDBookSidebarScrollbox extends HTMLElement {
    constructor() {
        super();
    }
    connectedCallback() {
        this.innerHTML = '<ol class="chapter"><li class="chapter-item expanded affix "><a href="introduction.html">Introduction</a></li><li class="chapter-item expanded "><a href="platforms/index.html"><strong aria-hidden="true">1.</strong> Platforms</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="platforms/android/index.html"><strong aria-hidden="true">1.1.</strong> Android</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="platforms/android/android-tests.html"><strong aria-hidden="true">1.1.1.</strong> Tests</a></li><li class="chapter-item expanded "><a href="platforms/android/android-documentation.html"><strong aria-hidden="true">1.1.2.</strong> Documentation</a></li><li class="chapter-item expanded "><a href="platforms/android/benchmark.html"><strong aria-hidden="true">1.1.3.</strong> Benchmark</a></li><li class="chapter-item expanded "><a href="platforms/android/release.html"><strong aria-hidden="true">1.1.4.</strong> Release</a></li></ol></li><li class="chapter-item expanded "><a href="platforms/ios/index.html"><strong aria-hidden="true">1.2.</strong> iOS</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="platforms/ios/ios-tests.html"><strong aria-hidden="true">1.2.1.</strong> Tests</a></li><li class="chapter-item expanded "><a href="platforms/ios/ios-documentation.html"><strong aria-hidden="true">1.2.2.</strong> Documentation</a></li><li class="chapter-item expanded "><a href="platforms/ios/release.html"><strong aria-hidden="true">1.2.3.</strong> Release</a></li><li class="chapter-item expanded "><a href="platforms/ios/dev-apps.html"><strong aria-hidden="true">1.2.4.</strong> Development Apps</a></li></ol></li><li class="chapter-item expanded "><a href="platforms/macos/index.html"><strong aria-hidden="true">1.3.</strong> macOS</a></li><li class="chapter-item expanded "><a href="platforms/linux/index.html"><strong aria-hidden="true">1.4.</strong> Linux</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="platforms/linux/using-docker.html"><strong aria-hidden="true">1.4.1.</strong> Using Docker</a></li></ol></li></ol></li><li class="chapter-item expanded "><a href="release-policy.html"><strong aria-hidden="true">2.</strong> Release Policy</a></li><li class="chapter-item expanded "><a href="render-tests.html"><strong aria-hidden="true">3.</strong> Render Tests</a></li><li class="chapter-item expanded "><a href="design/index.html"><strong aria-hidden="true">4.</strong> Design</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="design/ten-thousand-foot-view.html"><strong aria-hidden="true">4.1.</strong> Ten Thousand Foot View</a></li><li class="chapter-item expanded "><a href="design/coordinate-system.html"><strong aria-hidden="true">4.2.</strong> Coordinate System</a></li><li class="chapter-item expanded "><a href="design/expressions.html"><strong aria-hidden="true">4.3.</strong> Expressions</a></li><li class="chapter-item expanded "><a href="design/archictural-problems-and-recommendations.html"><strong aria-hidden="true">4.4.</strong> Architectural Problems and Recommendations</a></li><li class="chapter-item expanded "><a href="design/android-map-rendering-data-flow.html"><strong aria-hidden="true">4.5.</strong> Android Map Rendering Data Flow</a></li><li class="chapter-item expanded "><a href="design/geometry-tile-worker.html"><strong aria-hidden="true">4.6.</strong> Geometry Tile Worker</a></li></ol></li><li class="chapter-item expanded "><a href="profiling/index.html"><strong aria-hidden="true">5.</strong> Profiling applications that use MapLibre Native</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="profiling/tracy-profiling.html"><strong aria-hidden="true">5.1.</strong> Tracy profiling</a></li></ol></li><li class="chapter-item expanded "><a href="rust.html"><strong aria-hidden="true">6.</strong> Rust</a></li></ol>';
        // Set the current, active page, and reveal it if it's hidden
        let current_page = document.location.href.toString().split("#")[0].split("?")[0];
        if (current_page.endsWith("/")) {
            current_page += "index.html";
        }
        var links = Array.prototype.slice.call(this.querySelectorAll("a"));
        var l = links.length;
        for (var i = 0; i < l; ++i) {
            var link = links[i];
            var href = link.getAttribute("href");
            if (href && !href.startsWith("#") && !/^(?:[a-z+]+:)?\/\//.test(href)) {
                link.href = path_to_root + href;
            }
            // The "index" page is supposed to alias the first chapter in the book.
            if (link.href === current_page || (i === 0 && path_to_root === "" && current_page.endsWith("/index.html"))) {
                link.classList.add("active");
                var parent = link.parentElement;
                if (parent && parent.classList.contains("chapter-item")) {
                    parent.classList.add("expanded");
                }
                while (parent) {
                    if (parent.tagName === "LI" && parent.previousElementSibling) {
                        if (parent.previousElementSibling.classList.contains("chapter-item")) {
                            parent.previousElementSibling.classList.add("expanded");
                        }
                    }
                    parent = parent.parentElement;
                }
            }
        }
        // Track and set sidebar scroll position
        this.addEventListener('click', function(e) {
            if (e.target.tagName === 'A') {
                sessionStorage.setItem('sidebar-scroll', this.scrollTop);
            }
        }, { passive: true });
        var sidebarScrollTop = sessionStorage.getItem('sidebar-scroll');
        sessionStorage.removeItem('sidebar-scroll');
        if (sidebarScrollTop) {
            // preserve sidebar scroll position when navigating via links within sidebar
            this.scrollTop = sidebarScrollTop;
        } else {
            // scroll sidebar to current active section when navigating via "next/previous chapter" buttons
            var activeSection = document.querySelector('#sidebar .active');
            if (activeSection) {
                activeSection.scrollIntoView({ block: 'center' });
            }
        }
        // Toggle buttons
        var sidebarAnchorToggles = document.querySelectorAll('#sidebar a.toggle');
        function toggleSection(ev) {
            ev.currentTarget.parentElement.classList.toggle('expanded');
        }
        Array.from(sidebarAnchorToggles).forEach(function (el) {
            el.addEventListener('click', toggleSection);
        });
    }
}
window.customElements.define("mdbook-sidebar-scrollbox", MDBookSidebarScrollbox);
