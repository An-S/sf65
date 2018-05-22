#include "sf65.h"

char *sf65_EvaluateExpression ( sf65ParsingData_t *ParserData, sf65Options_t *CMDOpt, char *p1, char *p2 ) {
    int allocation = ParserData -> linesize;

    switch ( ParserData -> current_expr.exprType ) {
    case SF65_COMMENT:
        //Indicate that comment includes rest of line
        p2 = ( char* ) ( ParserData -> linebuf + allocation  - 1 );

        // Get x position for output of comment
        sf65_SetOutputXPositionInLine (
            ParserData,
            getCommentSpacing ( ParserData -> linebuf, p1, ParserData )
        );

        /*
         * If comment is aligned with mnemonic anyway, then align
         * also with indentation respected.
         * Indent by level times nesting space/tab width
         */
        if (
            sf65_GetOutputXPositionInLine ( ParserData ) ==
            CMDOptions -> start_mnemonic
        ) {
            sf65_IncOutputXPositionByNestingLevel (
                ParserData,
                CMDOptions -> nesting_space
            );
        }
        break;

    case SF65_MACRONAME:
        sf65_SetOutputXPositionInLine ( ParserData, CMDOptions -> start_mnemonic );
        sf65_SetPaddingSpaceFlag ( ParserData );
        break;
    case SF65_MNEMONIC:
        sf65_PlaceMnemonicInLine ( p1, p2, CMDOptions, ParserData );
        switch ( ParserData -> prev -> current_expr.exprType ) {
        case SF65_LABEL:
            sf65_SetPaddingSpaceFlag ( ParserData );
            break;
        default:
            break;
        }
        break;

    case SF65_DIRECTIVE:
        sf65_PlaceDirectiveInLine ( p1, p2, CMDOptions, ParserData );
        conditionallyAddPaddingLineAfterSection ( CMDOptions, ParserData );
        sf65_SetPaddingSpaceFlag ( ParserData );

        break;

    case SF65_OPERAND:
        sf65_PlaceOperandInLine ( p1, p2, CMDOptions, ParserData );

        // If operand does not start with a variable or label character or a number
        // directly attach operand to mnemonic. f.e. "lda #$ 00" is not desired but
        // "lda #$00". However, "lda label1" or "sta 1" is ok.
        if ( !isExpressionCharacter ( *p1 ) ) sf65_ClearPaddingSpaceFlag ( ParserData );

        break;

    case SF65_IDENTIFIER:
        switch ( ParserData -> prev -> current_expr.exprType ) {
        case SF65_ASSIGNMENT:
            sf65_SetOutputXPositionInLine ( ParserData, 0 );
            sf65_SetPaddingSpaceFlag ( ParserData );
            break;
        case SF65_MNEMONIC:
            sf65_SetOutputXPositionInLine ( ParserData, CMDOptions->start_operand );
            break;
        default:
            sf65_SetOutputXPositionInLine ( ParserData, 0 ); // Do not put identifier at special position
            break;
        }

        break;

    case SF65_LABEL:
        // Tell that separating space after label is needed
        sf65_SetPaddingSpaceFlag ( ParserData );

        // Leave label at start of line as default
        sf65_SetOutputXPositionInLine ( ParserData, 0 );

        /*
         * Code to detect oversized labels.
         */

        // Check, if p2 already at end of line
        // Then additional cr is not needed
        if ( allocation > p2 - p1 ) {
            // Test if user requested linebreak after oversized labels on cmdline
            if ( CMDOptions -> oversized_labels_own_line ) {
                // Label is considered oversized if is is longer than
                // configured x pos for mnemonics
                if ( p2 - p1 >= CMDOptions -> start_mnemonic ) {
                    sf65_SetLinefeedType ( ParserData, SF65_INSTANT_ADD_LF );
                }
            }
            if ( CMDOptions -> labels_own_line ) {
                sf65_SetLinefeedType ( ParserData, SF65_INSTANT_ADD_LF );
            }
        }

        // Separately position labels which are not at
        // beginning of line and terminated by colon
        if ( !ParserData -> beginning_of_line ) {
            // Place label in same column as directives
            // The sense of this to indent nested labels with code
            // The behaviour can be overritten by removing the colon after the label
            // and placing it at the start of the line
            sf65_SetOutputXPositionInLine ( ParserData, CMDOptions -> start_mnemonic );
            ParserData -> flags = LEVEL_MINUS;
        } else {
            ParserData -> flags = DONT_RELOCATE;
        }
        break;

    case SF65_EMPTYLINE:
        sf65_ResetLinefeedFlag ( ParserData, SF65_ADD_LF );

        break;

    case SF65_ASSIGNMENT:
        sf65_SetPaddingSpaceFlag ( ParserData );
        sf65_SetOutputXPositionInLine ( ParserData, CMDOptions -> start_mnemonic );
        break;

    default:
        // Detect separator for comma separated list of values
        switch ( ParserData -> prev -> current_expr.exprType ) {
        case SF65_COMMASEP:
            // Align comma separated list of values
            sf65_SetOutputXPositionInLine (
                ParserData,
                sf65_align (
                    sf65_GetCurrentColumnCounter ( ParserData ),
                    CMDOptions -> nesting_space
                )
            );
            ParserData -> flags = DONT_RELOCATE;
            break;

        case SF65_MNEMONIC:
        case SF65_LABEL:
        case SF65_ASSIGNMENT:
            sf65_SetPaddingSpaceFlag ( ParserData );
            break;

        default:
            // No comma separated list of values.
            if ( isExpressionCharacter ( *p1 ) &&
                    isExpressionCharacter ( ParserData -> prev -> current_expr.rightmostChar ) ) {
                sf65_SetPaddingSpaceFlag ( ParserData );
            }
            // Standard output
            sf65_SetOutputXPositionInLine ( ParserData, 0 );
            break;
        }

        break;
    }
    return p2;
}
