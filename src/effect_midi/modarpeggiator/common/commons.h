#ifndef _H_COMMONS_
#define _H_COMMONS_

#include <stdint.h>

/**
   MIDI event.
 */
struct MidiEvent {
   /**
      Size of internal data.
    */
    static const uint32_t kDataSize = 4;

   /**
      Time offset in frames.
    */
    uint32_t frame;

   /**
      Number of bytes used.
    */
    uint32_t size;

   /**
      MIDI data.@n
      If size > kDataSize, dataExt is used (otherwise null).
    */
    uint8_t        data[kDataSize];
    const uint8_t* dataExt;
};

#endif