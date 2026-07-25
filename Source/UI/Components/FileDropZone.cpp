#include "FileDropZone.h"
#include "../LookAndFeel/AcidMoogLookAndFeel.h"

FileDropZone::FileDropZone() = default;

bool FileDropZone::isSupportedAudioFile (const juce::String& path)
{
    return path.endsWithIgnoreCase (".wav")
        || path.endsWithIgnoreCase (".aif")
        || path.endsWithIgnoreCase (".aiff")
        || path.endsWithIgnoreCase (".flac")
        || path.endsWithIgnoreCase (".mp3")
        || path.endsWithIgnoreCase (".ogg");
}

void FileDropZone::setStatusText (const juce::String& text)
{
    status = text;
    repaint();
}

bool FileDropZone::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
        if (isSupportedAudioFile (f))
            return true;

    return false;
}

void FileDropZone::fileDragEnter (const juce::StringArray&, int, int)
{
    dragOver = true;
    repaint();
}

void FileDropZone::fileDragExit (const juce::StringArray&)
{
    dragOver = false;
    repaint();
}

void FileDropZone::filesDropped (const juce::StringArray& files, int, int)
{
    dragOver = false;
    repaint();

    for (const auto& f : files)
    {
        if (isSupportedAudioFile (f) && onFileChosen != nullptr)
        {
            onFileChosen (juce::File (f));
            return;
        }
    }
}

void FileDropZone::mouseDown (const juce::MouseEvent&)
{
    chooser = std::make_unique<juce::FileChooser> ("Carica un sample",
                                                   juce::File{},
                                                   "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");

    const auto flags = juce::FileBrowserComponent::openMode
                     | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();

        if (file.existsAsFile() && onFileChosen != nullptr)
            onFileChosen (file);
    });
}

void FileDropZone::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    g.setColour (juce::Colours::black.withAlpha (dragOver ? 0.45f : 0.65f));
    g.fillRoundedRectangle (bounds, 4.0f);

    // Bordo tratteggiato, che si illumina durante il drag.
    juce::Path border;
    border.addRoundedRectangle (bounds, 4.0f);

    const float dashes[] = { 4.0f, 3.0f };
    juce::PathStrokeType stroke (dragOver ? 1.8f : 1.2f);

    juce::Path dashed;
    stroke.createDashedStroke (dashed, border, dashes, 2);

    g.setColour (dragOver ? AcidMoogLookAndFeel::ledAmber
                          : AcidMoogLookAndFeel::engraving.withAlpha (0.45f));
    g.fillPath (dashed);

    g.setFont (juce::Font (juce::FontOptions { 10.0f }).boldened());
    g.setColour (dragOver ? AcidMoogLookAndFeel::ledAmber
                          : AcidMoogLookAndFeel::engraving.withAlpha (0.8f));
    g.drawFittedText (status, bounds.toNearestInt().reduced (6, 2),
                      juce::Justification::centred, 2);
}
