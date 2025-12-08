// SPDX-License-Identifier: MIT
/**

Doxygen Awesome
https://github.com/jothepro/doxygen-awesome-css

Copyright (c) 2022 - 2025 jothepro

*/

class DoxygenAwesomeInteractiveToc {
    static topOffset = 38
    static hideMobileMenu = true
    static headers = []
    static forceGenerate = true // Force TOC generation on all pages

    static init() {
        window.addEventListener("load", () => {
            let toc = document.querySelector(".contents > .toc")
            
            // If no TOC exists and forceGenerate is enabled, create one
            if(!toc && DoxygenAwesomeInteractiveToc.forceGenerate) {
                toc = DoxygenAwesomeInteractiveToc.generateToc()
            }
            
            if(toc) {
                toc.classList.add("interactive")
                if(!DoxygenAwesomeInteractiveToc.hideMobileMenu) {
                    toc.classList.add("open")
                }
                document.querySelector(".contents > .toc > h3")?.addEventListener("click", () => {
                    if(toc.classList.contains("open")) {
                        toc.classList.remove("open")
                    } else {
                        toc.classList.add("open")
                    }
                })

                document.querySelectorAll(".contents > .toc > ul a").forEach((node) => {
                    let id = node.getAttribute("href").substring(1)
                    DoxygenAwesomeInteractiveToc.headers.push({
                        node: node,
                        headerNode: document.getElementById(id)
                    })

                    document.getElementById("doc-content")?.addEventListener("scroll",this.throttle(DoxygenAwesomeInteractiveToc.update, 100))
                })
                DoxygenAwesomeInteractiveToc.update()
            }
        })
    }


    // Function made with help from Claude; I am unable to write JS properly without assistance.
    static generateToc() {
        // Find all headings in the content
        const contents = document.querySelector(".contents")
        if(!contents) return null
        
        const headings = contents.querySelectorAll("h1, h2, h3, h4")
        if(headings.length === 0) return null
        
        // Create TOC structure
        const toc = document.createElement("div")
        toc.className = "toc"
        
        const tocTitle = document.createElement("h3")
        tocTitle.textContent = "Table of Contents"
        toc.appendChild(tocTitle)
        
        const tocList = document.createElement("ul")
        
        headings.forEach((heading, index) => {
            // Ensure heading has an ID
            if(!heading.id) {
                heading.id = `autotoc_md${index}`
            }
            
            const li = document.createElement("li")
            li.className = `level${heading.tagName.toLowerCase()}`
            
            const a = document.createElement("a")
            a.href = `#${heading.id}`
            a.textContent = heading.textContent
            
            li.appendChild(a)
            tocList.appendChild(li)
        })
        
        toc.appendChild(tocList)
        
        // Insert TOC at the beginning of contents
        contents.insertBefore(toc, contents.firstChild)
        
        return toc
    }

    static update() {
        let active = DoxygenAwesomeInteractiveToc.headers[0]?.node
        DoxygenAwesomeInteractiveToc.headers.forEach((header) => {
            let position = header.headerNode.getBoundingClientRect().top
            header.node.classList.remove("active")
            header.node.classList.remove("aboveActive")
            if(position < DoxygenAwesomeInteractiveToc.topOffset) {
                active = header.node
                active?.classList.add("aboveActive")
            }
        })
        active?.classList.add("active")
        active?.classList.remove("aboveActive")
    }

    static throttle(func, delay) {
        let lastCall = 0;
        return function (...args) {
            const now = new Date().getTime();
            if (now - lastCall < delay) {
                return;
            }
            lastCall = now;
            return setTimeout(() => {func(...args)}, delay);
        };
    }
}
