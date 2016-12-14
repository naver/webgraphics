
#include "TestFillGlyphs.h"

#ifdef USE_D2D_CAIRO
#include "DrawTargetBackend.h"
#endif

BOOL TestFillGlyphs::Run(DrawTarget* aDT, void * aUserPtr)
{
    TestRunner* testRunner = static_cast<TestRunner*>(aUserPtr);
    if (!testRunner)
        return false;

    FontManager* fontManager = testRunner->GetFontManager();
    if (!fontManager)
        return false;

    RefPtr<ScaledFont> scaledFont = fontManager->CreateScaledFont("Malgun Gothic", 16);
    if (!scaledFont)
        return false;

    std::wstring testString = L"Hello?";
    Point textStartPos(20, 20);

    std::vector<mozilla::gfx::Glyph> glyphContainer;
    int inlinePos = textStartPos.x;
    for (int i = 0; i < testString.length(); i++) {
        mozilla::gfx::Glyph glyph;

        glyph.mIndex = fontManager->GetGlyph(scaledFont, testString[i]);
        glyph.mPosition = Point(inlinePos, textStartPos.y);
        glyphContainer.push_back(glyph);

        inlinePos += fontManager->GetGlyphWidth(scaledFont, glyph.mIndex);
    }

    GlyphBuffer glyphBuffer;
    glyphBuffer.mGlyphs = &glyphContainer.front();
    glyphBuffer.mNumGlyphs = glyphContainer.size();

    aDT->FillGlyphs(scaledFont, glyphBuffer, ColorPattern(Color(1, 1, 1, 1)), DrawOptions(), nullptr);

    return true;
}

#ifdef USE_D2D_CAIRO
BOOL TestFillGlyphs::Run(const std::unique_ptr<WebGraphicsContext>& aGC, void * aUserPtr)
{
    TestRunner* testRunner = static_cast<TestRunner*>(aUserPtr);
    if (!testRunner)
        return false;

    FontManager* fontManager = testRunner->GetFontManager();
    if (!fontManager)
        return false;

    cairo_scaled_font_t* scaledFont = fontManager->CreateCairoFont("Malgun Gothic", 16);

    std::string testString = "Hello?";

    cairo_glyph_t* glyphs = 0;
    int numGlyphs = 0;
    if (cairo_scaled_font_text_to_glyphs(scaledFont, 270, 20, testString.c_str(), testString.length(), &glyphs, &numGlyphs, 0, 0, 0) != CAIRO_STATUS_SUCCESS)
        return false;

    aGC->drawGlyphs(scaledFont, glyphs, numGlyphs, WebGraphicsContextState());
    cairo_glyph_free(glyphs);

    glyphs = 0, numGlyphs = 0;
    if (cairo_scaled_font_text_to_glyphs(scaledFont, 270, 40, testString.c_str(), testString.length(), &glyphs, &numGlyphs, 0, 0, 0) != CAIRO_STATUS_SUCCESS)
        return false;

    WebGraphicsContextState shadow;
    shadow.shadowBlur = 1.0f;
    shadow.shadowOffset = { 2.0f, 2.0f };
    shadow.shadowColor = { 1.0, 0.0, 0.0, 1.0 };

    aGC->drawGlyphs(scaledFont, glyphs, numGlyphs, shadow);
    cairo_glyph_free(glyphs);

    glyphs = 0, numGlyphs = 0;
    if (cairo_scaled_font_text_to_glyphs(scaledFont, 270, 60, testString.c_str(), testString.length(), &glyphs, &numGlyphs, 0, 0, 0) != CAIRO_STATUS_SUCCESS)
        return false;

    WebGraphicsContextState stroke;
    stroke.textDrawingMode = TextModeStroke;
    stroke.strokeThickness = 1.0f;
    stroke.strokeColor = { 1.0, 0.0, 0.0, 1.0 };
    stroke.strokeStyle = DottedStroke;

    aGC->drawGlyphs(scaledFont, glyphs, numGlyphs, stroke);
    cairo_glyph_free(glyphs);

    glyphs = 0, numGlyphs = 0;
    if (cairo_scaled_font_text_to_glyphs(scaledFont, 270, 80, testString.c_str(), testString.length(), &glyphs, &numGlyphs, 0, 0, 0) != CAIRO_STATUS_SUCCESS)
        return false;

    WebGraphicsContextState syntheticBold;
    syntheticBold.syntheticBoldOffset = 1.0f;

    aGC->drawGlyphs(scaledFont, glyphs, numGlyphs, syntheticBold);
    cairo_glyph_free(glyphs);

    cairo_scaled_font_destroy(scaledFont);

    return true;
}

BOOL TestFillGlyphs::Run(cairo_surface_t* aCS, void * aUserPtr)
{
    return true;
}
#endif
