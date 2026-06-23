#include "FaceplateComponent.h"

FaceplateComponent::FaceplateComponent()
{
    setInterceptsMouseClicks (false, false);
}

FaceplateComponent::~FaceplateComponent() = default;

void FaceplateComponent::paint (juce::Graphics& g)
{
    drawShell         (g);
    drawNoise         (g);
    drawCornerAccents (g);
    drawScrews        (g);
    drawSeparators    (g);
    drawModuleBorders (g);
    drawModuleLabels  (g);
    drawHeader        (g);
    drawFooter        (g);
}

void FaceplateComponent::resized() {}

//==============================================================================
void FaceplateComponent::drawShell (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    // Panel background with subtle gradient (115deg)
    juce::ColourGradient shellGrad (
        juce::Colour (SR::Colour::panel).brighter (0.04f), area.getTopLeft(),
        juce::Colour (SR::Colour::panel),                  area.getBottomRight(),
        false);
    shellGrad.addColour (0.26, juce::Colour (SR::Colour::panel));
    g.setGradientFill (shellGrad);
    g.fillRect (area);

    // Outer border 1px #111316
    g.setColour (juce::Colour (0xff111316));
    g.drawRect (area, 1.0f);

    // Inner frame 1px at inset 13, aluminium 20%
    auto inner = SR::Layout::innerFrame().toFloat();
    g.setColour (juce::Colour (SR::Colour::aluminium).withAlpha (0.20f));
    g.drawRect (inner, 1.0f);
}

void FaceplateComponent::drawNoise (juce::Graphics& g)
{
    // Subtle crosshatch at opacity 0.22, soft-light simulation via overlay
    auto area = getLocalBounds().toFloat();
    g.setColour (juce::Colour (SR::Colour::aluminium).withAlpha (0.022f));
    float step = 4.0f;
    for (float x = area.getX(); x < area.getRight(); x += step)
        g.drawLine (x, area.getY(), x, area.getBottom(), 0.5f);
    for (float y = area.getY(); y < area.getBottom(); y += step)
        g.drawLine (area.getX(), y, area.getRight(), y, 0.5f);
}

void FaceplateComponent::drawCornerAccents (juce::Graphics& g)
{
    // Two diagonal cobalt lines at top-left and bottom-right (76x24px area, rotated -24deg)
    g.setColour (juce::Colour (SR::Colour::accent).withAlpha (0.72f));

    // Top-left corner mark
    {
        auto t = juce::AffineTransform::rotation (juce::degreesToRadians (-24.0f), 20.0f, 20.0f);
        juce::Path p;
        p.startNewSubPath (4.0f, 20.0f);
        p.lineTo (76.0f, 20.0f);
        p.startNewSubPath (4.0f, 28.0f);
        p.lineTo (60.0f, 28.0f);
        g.strokePath (p, juce::PathStrokeType (1.5f), t);
    }

    // Bottom-right corner mark
    {
        float brx = (float) SR::Layout::kW - 20.0f;
        float bry = (float) SR::Layout::kH - 20.0f;
        auto t = juce::AffineTransform::rotation (juce::degreesToRadians (-24.0f), brx, bry);
        juce::Path p;
        p.startNewSubPath (brx - 72.0f, bry - 8.0f);
        p.lineTo (brx - 4.0f, bry - 8.0f);
        p.startNewSubPath (brx - 56.0f, bry);
        p.lineTo (brx - 4.0f, bry);
        g.strokePath (p, juce::PathStrokeType (1.5f), t);
    }
}

void FaceplateComponent::drawScrews (juce::Graphics& g)
{
    for (auto& screw : SR::Layout::screws)
    {
        float cx = (float) screw.x;
        float cy = (float) screw.y;
        float r  = 4.0f;

        // Background
        g.setColour (juce::Colour (0xff1b1d20));
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);

        // Border
        g.setColour (juce::Colour (SR::Colour::aluminium).withAlpha (0.42f));
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);

        // Slash inside, rotated -27deg
        {
            auto t = juce::AffineTransform::rotation (juce::degreesToRadians (-27.0f), cx, cy);
            juce::Path p;
            p.startNewSubPath (cx - 2.5f, cy);
            p.lineTo (cx + 2.5f, cy);
            g.setColour (juce::Colour (SR::Colour::aluminium).withAlpha (0.7f));
            g.strokePath (p, juce::PathStrokeType (1.0f), t);
        }
    }
}

void FaceplateComponent::drawSeparators (juce::Graphics& g)
{
    g.setColour (SR::Colour::line());
    // Header separator at y=76
    g.drawLine (0.0f, 76.0f, (float) SR::Layout::kW, 76.0f, 1.0f);
    // Footer separator at y=484
    g.drawLine (0.0f, 484.0f, (float) SR::Layout::kW, 484.0f, 1.0f);
}

void FaceplateComponent::drawModuleBorders (juce::Graphics& g)
{
    auto lineCol = SR::Colour::line();
    auto bgCol   = juce::Colour (SR::Colour::panelSecondary).withAlpha (0.5f);

    // Reactor module
    auto reactor = SR::Layout::reactorModule().toFloat();
    g.setColour (bgCol);
    g.fillRect (reactor);
    g.setColour (lineCol);
    g.drawRect (reactor, 1.0f);

    // Controls module
    auto controls = SR::Layout::controlsModule().toFloat();
    g.setColour (bgCol);
    g.fillRect (controls);
    g.setColour (lineCol);
    g.drawRect (controls, 1.0f);
}

void FaceplateComponent::drawModuleLabels (juce::Graphics& g)
{
    // Reactor module: title top-left, index top-right
    auto reactor = SR::Layout::reactorModule().toFloat();
    float labelY = reactor.getY() + 5.0f;

    g.setFont (SR::Font::sans (8.0f, juce::Font::bold));
    g.setColour (juce::Colour (SR::Colour::muted));
    g.drawText ("REACTOR CORE / STEREO FIELD",
                juce::Rectangle<float> (reactor.getX() + 6.0f, labelY, 220.0f, 10.0f),
                juce::Justification::centredLeft, false);
    g.drawText ("SR-01",
                juce::Rectangle<float> (reactor.getRight() - 44.0f, labelY, 38.0f, 10.0f),
                juce::Justification::centredRight, false);

    // Controls module: title top-left, index top-right
    auto controls = SR::Layout::controlsModule().toFloat();
    g.drawText ("SPATIAL SHAPING",
                juce::Rectangle<float> (controls.getX() + 6.0f, labelY, 150.0f, 10.0f),
                juce::Justification::centredLeft, false);
    g.drawText ("DSP-A",
                juce::Rectangle<float> (controls.getRight() - 44.0f, labelY, 38.0f, 10.0f),
                juce::Justification::centredRight, false);

    // "SOURCE PROFILE" label above profile selector
    auto ps = SR::Layout::profileSelector().toFloat();
    g.setFont (SR::Font::sans (8.0f));
    g.drawText ("SOURCE PROFILE",
                juce::Rectangle<float> (ps.getX(), ps.getY() - 13.0f, ps.getWidth(), 11.0f),
                juce::Justification::centredLeft, false);
}

void FaceplateComponent::drawHeader (juce::Graphics& g)
{
    auto header = SR::Layout::header().toFloat();

    // --- Logo circle ---
    float logoCx = header.getX() + 30.0f;
    float logoCy = header.getCentreY();
    float logoR  = 15.0f;

    // Outer ring (ink border)
    g.setColour (juce::Colour (SR::Colour::ink));
    g.drawEllipse (logoCx - logoR, logoCy - logoR, logoR * 2.0f, logoR * 2.0f, 2.0f);

    // Inner partial cobalt ring (270 deg arc from 270deg to 180deg clockwise)
    juce::Path arcPath;
    arcPath.addArc (logoCx - logoR + 3.0f, logoCy - logoR + 3.0f,
                    (logoR - 3.0f) * 2.0f, (logoR - 3.0f) * 2.0f,
                    juce::MathConstants<float>::pi * 1.5f,   // 270deg
                    juce::MathConstants<float>::pi * 3.5f,   // 270deg sweep
                    true);
    g.setColour (juce::Colour (SR::Colour::accent));
    g.strokePath (arcPath, juce::PathStrokeType (2.0f));

    // Center dot
    g.setColour (juce::Colour (SR::Colour::accent));
    g.fillEllipse (logoCx - 2.5f, logoCy - 2.5f, 5.0f, 5.0f);

    // --- Brand text ---
    float textX = logoCx + logoR + 10.0f;

    g.setFont (SR::Font::sans (20.0f, juce::Font::bold));
    g.setColour (juce::Colour (SR::Colour::ink));
    g.drawText ("SPATIAL REACTOR",
                juce::Rectangle<float> (textX, logoCy - 15.0f, 260.0f, 22.0f),
                juce::Justification::centredLeft, false);

    // Subtitle
    g.setFont (SR::Font::sans (8.0f));
    g.setColour (juce::Colour (SR::Colour::muted));
    g.drawText ("STEREO IMAGING ENGINE",
                juce::Rectangle<float> (textX, logoCy + 8.0f, 200.0f, 10.0f),
                juce::Justification::centredLeft, false);
}

void FaceplateComponent::drawFooter (juce::Graphics& g)
{
    // Signature
    auto sigArea = SR::Layout::signatureArea().toFloat();
    float sigCy  = sigArea.getCentreY();

    g.setFont (SR::Font::sans (9.0f, juce::Font::bold));
    g.setColour (juce::Colour (SR::Colour::ink));
    g.drawText ("MYKYTA SHCHUR",
                juce::Rectangle<float> (sigArea.getX(), sigCy - 9.0f, sigArea.getWidth(), 11.0f),
                juce::Justification::centredRight, false);

    g.setFont (SR::Font::sans (7.0f));
    g.setColour (juce::Colour (SR::Colour::muted));
    g.drawText ("KYIV / 2026",
                juce::Rectangle<float> (sigArea.getX(), sigCy + 3.0f, sigArea.getWidth(), 9.0f),
                juce::Justification::centredRight, false);

    // Correlation label (left of meter)
    auto corrCopy = SR::Layout::corrCopyArea().toFloat();
    g.setFont (SR::Font::sans (7.0f));
    g.setColour (juce::Colour (SR::Colour::muted));
    g.drawText ("CORRELATION",
                juce::Rectangle<float> (corrCopy.getX(), corrCopy.getCentreY() + 6.0f, corrCopy.getWidth(), 9.0f),
                juce::Justification::centredLeft, false);
}
