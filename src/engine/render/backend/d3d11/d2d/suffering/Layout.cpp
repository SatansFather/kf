/*
static UINT const fontResourceIDs[] = {  IDR_FONT_PERICLES, IDR_FONT_KOOTENAY };

// Create a custom font collection comprising our two font resources. We could have done this
// in the constructor rather than every time. However, if you set break points on the loader 
// callbacks you'll find they're only called the first time the font collection is created. 
// Thereafter the font collection data is cached so recreating it is quite fast.
hr = fontContext_.Initialize();
if (FAILED(hr))
return hr;

IDWriteFontCollection* fontCollection = NULL;
hr = fontContext_.CreateFontCollection(
	fontResourceIDs,
	sizeof(fontResourceIDs),
	&fontCollection
);
if (FAILED(hr))
return hr;*/