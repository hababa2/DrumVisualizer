#include "Defines.hpp"

#define RTMIDI_DO_NOT_ENSURE_UNIQUE_PORTNAMES
#define RTMIDI_DO_NOT_ENABLE_WORKAROUND_UWP_WRONG_TIMESTAMPS
#include "RtMidi.h"

void callback(double deltatime, std::vector<unsigned char>* message, void* userData)
{
    U32 nBytes = message->size();
    for (U32 i = 0; i < nBytes; i++) { std::cout << "Byte " << i << " = " << (int)message->at(i) << ", "; }
    if (nBytes > 0) { std::cout << "stamp = " << deltatime << std::endl; }
}

int main()
{
	RtMidiIn* midi = new RtMidiIn();

    midi->setCallback(callback, nullptr);
    midi->openPort(0, "Midi");
    midi->ignoreTypes(false, false, false);

    std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);

    delete midi;

    return 0;
}
