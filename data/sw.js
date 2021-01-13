const staticCache = 'static-cache';
const dynamicCache = 'dynamic-cache';
const assets = [
    'index.html',
    'app.js',
    'style.css',
    'control.html',
    '/',
    'Logo.png',
    'main.js',
    '1.png',
    '2.png',
    '3.png',
    '4.png',
    '5.png',
    'fallback.html'
]

self.addEventListener('install', e => {
    e.waitUntil(
        caches.open(staticCache).then(cache => {
            cache.addAll(assets)
        })

    )

})

self.addEventListener('activate',e=>{
    console.log('sw is activated')
})

self.addEventListener('fetch', e => {
    e.respondWith(
        caches.match(e.request).then(staticRes => {
            return staticRes || fetch(e.request).then(dynamicRes => {
                return caches.open(dynamicCache).then(cache => {
                    cache.put(e.request.url, dynamicRes.clone())
                    return dynamicRes
                })
            })
        }).catch(()=>caches.match('fallback.html'))
    )
})
