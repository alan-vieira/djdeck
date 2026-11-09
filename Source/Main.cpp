/*
  ==============================================================================
    Main.cpp - Ponto de entrada da aplicação DJ
  ==============================================================================
*/

#include <juce_core/juce_core.h>
#include "MainComponent.h"

struct ProjectInfo
{
    static const char* const projectName;
    static const char* const companyName;
    static const char* const versionString;
    static const int         versionNumber;
};

inline const char* const ProjectInfo::projectName = "djdeck";
inline const char* const ProjectInfo::companyName = "DJ App";
inline const char* const ProjectInfo::versionString = "0.1.0";
inline const int ProjectInfo::versionNumber = 1;

//==============================================================================
class DJApplication : public juce::JUCEApplication
{
public:
    DJApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
    }

    //==============================================================================
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel()
                                 .findColour(juce::ResizableWindow::backgroundColourId),
                             DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
#else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
#endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
START_JUCE_APPLICATION (DJApplication)
