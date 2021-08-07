/**
 * Xtase - fgalliat @Jun 2020
 * 
 * Menu routines
 * 
 */

#include "../globals.h"

#include "../xtbase.h"

#if not HAS_MENU

int XTBaseApp::menu(int itmn, char** itmv, char* title) {
    return -1;
}

#else

void _menu_cls() {
    console.cls();
    _drawFrontBtns("Up", "Down", "Select");
}

void _menu_dispItems(int itmn, char** itmv, int selectedItem) {
    int topItm = 3;
    int nbDisp = min(itmn, 10);
    for(int i = 0; i < nbDisp; i++) {

        console.setCursor( 1, topItm+i );
        if ( i == selectedItem ) {
            console.write('>');
        } else {
            console.write(' ');
        }

        console.write( ' ' );
        console.write( ('a'+i) );
        console.print( ". " );
        console.print( itmv[i] );

    }
}

extern char __waitKey();

int XTBaseApp::menu(int itmn, char** itmv, char* title) {
    #if not INTERACTIVE_MODE
        return 0;
    #endif

    int selectedItem = 0;

    int xTOff = 0;
    if ( title != NULL ) {
        int t_len = strlen(title);
        t_len = 3+ t_len + 3;
        xTOff = ( console.getWidth() - t_len ) / 2;
    }

    while(true) {
        _menu_cls();

        if ( title != NULL ) {
            console.setCursor(xTOff, 1);
            console.print("-( ");
            console.print(title);
            console.print(" )-");
        }

        _menu_dispItems( itmn, itmv, selectedItem );

        bool aBtnWasPressed = false;
        while( ! aBtnWasPressed ) {
            pollButtons();

            char ch = __waitKey();
            if ( ch != 0x00 ) {
                if ( ch >= 'a' && ch <= ( 'a'+(itmn-1) ) ) {
                    selectedItem = ch - 'a';
                    return selectedItem;
                }
                ch = __waitKey();
            }

            if ( isBtnApressed() ) {
                aBtnWasPressed = true;
                waitAllBtnsReleased();
                if ( selectedItem <= 0 ) { selectedItem = itmn-1; }
                else { selectedItem--; }
            }
            else if ( isBtnBpressed() ) {
                aBtnWasPressed = true;
                waitAllBtnsReleased();
                if ( selectedItem >= itmn-1 ) { selectedItem = 0; }
                else { selectedItem++; }
            }
            else if ( isBtnCpressed() ) {
                aBtnWasPressed = true;
                waitAllBtnsReleased();
                return selectedItem;
            }

            delay(50);
            yield();
        }
    }

    return -1;
}

#endif