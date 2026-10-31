// The page is always dark (the light/dark switch was removed). This runs before
// paint so there is never a white flash on first load.
export const THEME_INIT = `(function(){try{document.documentElement.setAttribute('data-theme','dark');}catch(e){}})();`;
