#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

/** Area di drop per il sample: accetta drag&drop dal file system e il click per
    aprire un file chooser. */
class FileDropZone : public juce::Component,
                     public juce::FileDragAndDropTarget
{
public:
    FileDropZone();

    std::function<void (const juce::File&)> onFileChosen;

    void setStatusText (const juce::String& text);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    static bool isSupportedAudioFile (const juce::String& path);

    juce::String status { "DROP SAMPLE" };
    bool dragOver { false };
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileDropZone)
};
