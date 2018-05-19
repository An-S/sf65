#include "sf65.h"

sf65ParsingData_t sf65ParserData1;
sf65ParsingData_t sf65ParserData2;

sf65ErrCode_t sf65_GetParserDataPointers ( sf65ParsingData_t **pData1, sf65ParsingData_t **pData2 ) {
    NOT_NULL ( pData1, SF65_NULLPTR ); NOT_NULL ( pData2, SF65_NULLPTR ) {
        *pData1 = &sf65ParserData1;
        *pData2 = &sf65ParserData1;

        sf65_InitializeParser ( *pData1 );
        sf65_InitializeParser ( *pData2 );

        return SF65_NOERR;
    }
}

sf65ErrCode_t sf65_ToggleParserDataPointers ( sf65ParsingData_t **pData1, sf65ParsingData_t **pData2 ) {
    NOT_NULL ( pData1, SF65_NULLPTR ); NOT_NULL ( pData2, SF65_NULLPTR ) {
        sf65ParsingData_t **tmp = pData1;
        pData1 = pData2;
        pData2 = tmp;

        return SF65_NOERR;
    }
}
