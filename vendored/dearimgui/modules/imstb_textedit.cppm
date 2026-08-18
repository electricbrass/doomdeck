module;

#include <imstb_textedit.h>

export module imstb_textedit;

export import imgui;

export {

    // ----- Structs -----

    using ::STB_TexteditState;
    using ::StbTexteditRow;
    using ::StbUndoRecord;
    using ::StbUndoState;
}
