/*
 * parser.c : an XML 1.0 non-verifying parser
 *
 * See Copyright for the status of this software.
 *
 * $Id: parser.c,v 1.13 1998/11/16 01:04:26 veillard Exp $
 */

#ifdef WIN32
#define HAVE_FCNTL_H
#include <io.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h> /* for memset() only */
#include <stdlib.h>
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

#include "tree.h"
#include "parser.h"
#include "entities.h"


/************************************************************************
 *									*
 * 		Parser stacks related functions and macros		*
 *									*
 ************************************************************************/
/*
 * Generic function for accessing stacks in the Parser Context
 */

#define PUSH_AND_POP(type, name)					\
int GX##name##Push(GXxmlParserCtxtPtr ctxt, type value) {			\
    if (ctxt->name##Nr >= ctxt->name##Max) {				\
	ctxt->name##Max *= 2;						\
        ctxt->name##Tab = (void *) GxCore_Realloc(ctxt->name##Tab,		\
	             ctxt->name##Max * sizeof(ctxt->name##Tab[0]));	\
        if (ctxt->name##Tab == NULL) {					\
	    fprintf(stderr, "GxCore_Realloc failed !\n");			\
	    exit(1);							\
	}								\
    }									\
    ctxt->name##Tab[ctxt->name##Nr] = value;				\
    ctxt->name = value;							\
    return(ctxt->name##Nr++);						\
}									\
type GX##name##Pop(GXxmlParserCtxtPtr ctxt) {					\
    if (ctxt->name##Nr <= 0) return(0);					\
    ctxt->name##Nr--;							\
    if (ctxt->name##Nr > 0)						\
	ctxt->name = ctxt->name##Tab[ctxt->name##Nr - 1];		\
    else								\
        ctxt->name = NULL;						\
    return(ctxt->name);							\
}									\

PUSH_AND_POP(GXxmlParserInputPtr, input)
PUSH_AND_POP(GXxmlNodePtr, node)

/*
 * Macros for accessing the content. Those should be used only by the parser,
 * and not exported.
 *
 * Dirty macros, i.e. one need to make assumption on the context to use them
 *
 *   CUR_PTR return the current pointer to the CHAR to be parsed.
 *   CUR     returns the current CHAR value, i.e. a 8 bit value if compiled
 *           in ISO-Latin or UTF-8, and the current 16 bit value if compiled
 *           in UNICODE mode. This should be used internally by the parser
 *           only to compare to ASCII values otherwise it would break when
 *           running with UTF-8 encoding.
 *   NXT(n)  returns the n'th next CHAR. Same as CUR is should be used only
 *           to compare on ASCII based substring.
 *   SKIP(n) Skip n CHAR, and must also be used only to skip ASCII defined
 *           strings within the parser.
 *
 * Clean macros, not dependent of an ASCII context.
 *
 *   CURRENT Returns the current char value, with the full decoding of
 *           UTF-8 if we are using this mode. It returns an int.
 *   NEXT    Skip to the next character, this does the proper decoding
 *           in UTF-8 mode. It also pop-up unfinished entities on the fly.
 *           It returns the pointer to the current CHAR.
 */

#define CUR (*ctxt->input->cur)
#define SKIP(val) ctxt->input->cur += (val)
#define NXT(val) ctxt->input->cur[(val)]
#define CUR_PTR ctxt->input->cur

#define SKIP_BLANKS 							\
    while (IS_BLANK(*(ctxt->input->cur))) NEXT

#ifndef USE_UTF_8
#define CURRENT (*ctxt->input->cur)
#define NEXT ((*ctxt->input->cur) ?					\
                (((*(ctxt->input->cur) == '\n') ?			\
		    (ctxt->input->line++, ctxt->input->col = 1) :	\
		    (ctxt->input->col++)), ctxt->input->cur++) :	\
		(GXxmlPopInput(ctxt), ctxt->input->cur))
#else
#endif


/**
 * xmlPopInput:
 * @ctxt:  an XML parser context
 *
 * xmlPopInput: the current input pointed by ctxt->input came to an end
 *          pop it and return the next char.
 *
 * TODO A deallocation of the popped Input structure is needed
 * return values: the current CHAR in the parser context
 */
CHAR
GXxmlPopInput(GXxmlParserCtxtPtr ctxt) {
    if (ctxt->inputNr == 1) return(0); /* End of main Input */
    GXinputPop(ctxt);
    return(CUR);
}

/**
 * xmlPushInput:
 * @ctxt:  an XML parser context
 * @input:  an XML parser input fragment (entity, XML fragment ...).
 *
 * xmlPushInput: switch to a new input stream which is stacked on top
 *               of the previous one(s).
 */
void
GXxmlPushInput(GXxmlParserCtxtPtr ctxt, GXxmlParserInputPtr input) {
    if (input == NULL) return;
    GXinputPush(ctxt, input);
}

/**
 * xmlNewEntityInputStream:
 * @ctxt:  an XML parser context
 * @entity:  an Entity pointer
 *
 * Create a new input stream based on a memory buffer.
 * return vakues: the new input stream
 */
GXxmlParserInputPtr
GXxmlNewEntityInputStream(GXxmlParserCtxtPtr ctxt, GXxmlEntityPtr entity) {
    GXxmlParserInputPtr input;

    if (entity == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt,
	      "internal: xmlNewEntityInputStream entity = NULL\n");
	return(NULL);
    }
    if (entity->content == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt,
	      "internal: xmlNewEntityInputStream entity->input = NULL\n");
	return(NULL);
    }
    input = (GXxmlParserInputPtr) GxCore_Malloc(sizeof(GXxmlParserInput));
    if (input == NULL) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "GxCore_Malloc: couldn't allocate a new input stream\n");
	return(NULL);
    }
    input->filename = (const char *)entity->SystemID; /* TODO !!! char <- CHAR */
    input->base = entity->content;
    input->cur = entity->content;
    input->line = 1;
    input->col = 1;
    return(input);
}

/*
 * A few macros needed to help building the parser.
 */

#ifdef UNICODE
/************************************************************************
 *									*
 * UNICODE version of the macros.      					*
 *									*
 ************************************************************************/
/*
 * [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD]
 *                  | [#x10000-#x10FFFF]
 * any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 */
#define IS_CHAR(c)							\
    (((c) == 0x09) || ((c) == 0x0a) || ((c) == 0x0d) ||			\
     (((c) >= 0x20) && ((c) != 0xFFFE) && ((c) != 0xFFFF)))

/*
 * [3] S ::= (#x20 | #x9 | #xD | #xA)+
 */
#define IS_BLANK(c) (((c) == 0x20) || ((c) == 0x09) || ((c) == 0xa) ||	\
                     ((c) == 0x0D))

/*
 * [85] BaseChar ::= ... long list see REC ...
 *
 * VI is your friend !
 * :1,$ s/\[#x\([0-9A-Z]*\)-#x\([0-9A-Z]*\)\]/     (((c) >= 0x\1) \&\& ((c) <= 0x\2)) ||/
 * and 
 * :1,$ s/#x\([0-9A-Z]*\)/     ((c) == 0x\1) ||/
 */
#define IS_BASECHAR(c)							\
     ((((c) >= 0x0041) && ((c) <= 0x005A)) ||				\
      (((c) >= 0x0061) && ((c) <= 0x007A)) ||				\
      (((c) >= 0x00C0) && ((c) <= 0x00D6)) ||				\
      (((c) >= 0x00D8) && ((c) <= 0x00F6)) ||				\
      (((c) >= 0x00F8) && ((c) <= 0x00FF)) ||				\
      (((c) >= 0x0100) && ((c) <= 0x0131)) ||				\
      (((c) >= 0x0134) && ((c) <= 0x013E)) ||				\
      (((c) >= 0x0141) && ((c) <= 0x0148)) ||				\
      (((c) >= 0x014A) && ((c) <= 0x017E)) ||				\
      (((c) >= 0x0180) && ((c) <= 0x01C3)) ||				\
      (((c) >= 0x01CD) && ((c) <= 0x01F0)) ||				\
      (((c) >= 0x01F4) && ((c) <= 0x01F5)) ||				\
      (((c) >= 0x01FA) && ((c) <= 0x0217)) ||				\
      (((c) >= 0x0250) && ((c) <= 0x02A8)) ||				\
      (((c) >= 0x02BB) && ((c) <= 0x02C1)) ||				\
      ((c) == 0x0386) ||						\
      (((c) >= 0x0388) && ((c) <= 0x038A)) ||				\
      ((c) == 0x038C) ||						\
      (((c) >= 0x038E) && ((c) <= 0x03A1)) ||				\
      (((c) >= 0x03A3) && ((c) <= 0x03CE)) ||				\
      (((c) >= 0x03D0) && ((c) <= 0x03D6)) ||				\
      ((c) == 0x03DA) ||						\
      ((c) == 0x03DC) ||						\
      ((c) == 0x03DE) ||						\
      ((c) == 0x03E0) ||						\
      (((c) >= 0x03E2) && ((c) <= 0x03F3)) ||				\
      (((c) >= 0x0401) && ((c) <= 0x040C)) ||				\
      (((c) >= 0x040E) && ((c) <= 0x044F)) ||				\
      (((c) >= 0x0451) && ((c) <= 0x045C)) ||				\
      (((c) >= 0x045E) && ((c) <= 0x0481)) ||				\
      (((c) >= 0x0490) && ((c) <= 0x04C4)) ||				\
      (((c) >= 0x04C7) && ((c) <= 0x04C8)) ||				\
      (((c) >= 0x04CB) && ((c) <= 0x04CC)) ||				\
      (((c) >= 0x04D0) && ((c) <= 0x04EB)) ||				\
      (((c) >= 0x04EE) && ((c) <= 0x04F5)) ||				\
      (((c) >= 0x04F8) && ((c) <= 0x04F9)) ||				\
      (((c) >= 0x0531) && ((c) <= 0x0556)) ||				\
      ((c) == 0x0559) ||						\
      (((c) >= 0x0561) && ((c) <= 0x0586)) ||				\
      (((c) >= 0x05D0) && ((c) <= 0x05EA)) ||				\
      (((c) >= 0x05F0) && ((c) <= 0x05F2)) ||				\
      (((c) >= 0x0621) && ((c) <= 0x063A)) ||				\
      (((c) >= 0x0641) && ((c) <= 0x064A)) ||				\
      (((c) >= 0x0671) && ((c) <= 0x06B7)) ||				\
      (((c) >= 0x06BA) && ((c) <= 0x06BE)) ||				\
      (((c) >= 0x06C0) && ((c) <= 0x06CE)) ||				\
      (((c) >= 0x06D0) && ((c) <= 0x06D3)) ||				\
      ((c) == 0x06D5) ||						\
      (((c) >= 0x06E5) && ((c) <= 0x06E6)) ||				\
      (((c) >= 0x0905) && ((c) <= 0x0939)) ||				\
      ((c) == 0x093D) ||						\
      (((c) >= 0x0958) && ((c) <= 0x0961)) ||				\
      (((c) >= 0x0985) && ((c) <= 0x098C)) ||				\
      (((c) >= 0x098F) && ((c) <= 0x0990)) ||				\
      (((c) >= 0x0993) && ((c) <= 0x09A8)) ||				\
      (((c) >= 0x09AA) && ((c) <= 0x09B0)) ||				\
      ((c) == 0x09B2) ||						\
      (((c) >= 0x09B6) && ((c) <= 0x09B9)) ||				\
      (((c) >= 0x09DC) && ((c) <= 0x09DD)) ||				\
      (((c) >= 0x09DF) && ((c) <= 0x09E1)) ||				\
      (((c) >= 0x09F0) && ((c) <= 0x09F1)) ||				\
      (((c) >= 0x0A05) && ((c) <= 0x0A0A)) ||				\
      (((c) >= 0x0A0F) && ((c) <= 0x0A10)) ||				\
      (((c) >= 0x0A13) && ((c) <= 0x0A28)) ||				\
      (((c) >= 0x0A2A) && ((c) <= 0x0A30)) ||				\
      (((c) >= 0x0A32) && ((c) <= 0x0A33)) ||				\
      (((c) >= 0x0A35) && ((c) <= 0x0A36)) ||				\
      (((c) >= 0x0A38) && ((c) <= 0x0A39)) ||				\
      (((c) >= 0x0A59) && ((c) <= 0x0A5C)) ||				\
      ((c) == 0x0A5E) ||						\
      (((c) >= 0x0A72) && ((c) <= 0x0A74)) ||				\
      (((c) >= 0x0A85) && ((c) <= 0x0A8B)) ||				\
      ((c) == 0x0A8D) ||						\
      (((c) >= 0x0A8F) && ((c) <= 0x0A91)) ||				\
      (((c) >= 0x0A93) && ((c) <= 0x0AA8)) ||				\
      (((c) >= 0x0AAA) && ((c) <= 0x0AB0)) ||				\
      (((c) >= 0x0AB2) && ((c) <= 0x0AB3)) ||				\
      (((c) >= 0x0AB5) && ((c) <= 0x0AB9)) ||				\
      ((c) == 0x0ABD) ||						\
      ((c) == 0x0AE0) ||						\
      (((c) >= 0x0B05) && ((c) <= 0x0B0C)) ||				\
      (((c) >= 0x0B0F) && ((c) <= 0x0B10)) ||				\
      (((c) >= 0x0B13) && ((c) <= 0x0B28)) ||				\
      (((c) >= 0x0B2A) && ((c) <= 0x0B30)) ||				\
      (((c) >= 0x0B32) && ((c) <= 0x0B33)) ||				\
      (((c) >= 0x0B36) && ((c) <= 0x0B39)) ||				\
      ((c) == 0x0B3D) ||						\
      (((c) >= 0x0B5C) && ((c) <= 0x0B5D)) ||				\
      (((c) >= 0x0B5F) && ((c) <= 0x0B61)) ||				\
      (((c) >= 0x0B85) && ((c) <= 0x0B8A)) ||				\
      (((c) >= 0x0B8E) && ((c) <= 0x0B90)) ||				\
      (((c) >= 0x0B92) && ((c) <= 0x0B95)) ||				\
      (((c) >= 0x0B99) && ((c) <= 0x0B9A)) ||				\
      ((c) == 0x0B9C) ||						\
      (((c) >= 0x0B9E) && ((c) <= 0x0B9F)) ||				\
      (((c) >= 0x0BA3) && ((c) <= 0x0BA4)) ||				\
      (((c) >= 0x0BA8) && ((c) <= 0x0BAA)) ||				\
      (((c) >= 0x0BAE) && ((c) <= 0x0BB5)) ||				\
      (((c) >= 0x0BB7) && ((c) <= 0x0BB9)) ||				\
      (((c) >= 0x0C05) && ((c) <= 0x0C0C)) ||				\
      (((c) >= 0x0C0E) && ((c) <= 0x0C10)) ||				\
      (((c) >= 0x0C12) && ((c) <= 0x0C28)) ||				\
      (((c) >= 0x0C2A) && ((c) <= 0x0C33)) ||				\
      (((c) >= 0x0C35) && ((c) <= 0x0C39)) ||				\
      (((c) >= 0x0C60) && ((c) <= 0x0C61)) ||				\
      (((c) >= 0x0C85) && ((c) <= 0x0C8C)) ||				\
      (((c) >= 0x0C8E) && ((c) <= 0x0C90)) ||				\
      (((c) >= 0x0C92) && ((c) <= 0x0CA8)) ||				\
      (((c) >= 0x0CAA) && ((c) <= 0x0CB3)) ||				\
      (((c) >= 0x0CB5) && ((c) <= 0x0CB9)) ||				\
      ((c) == 0x0CDE) ||						\
      (((c) >= 0x0CE0) && ((c) <= 0x0CE1)) ||				\
      (((c) >= 0x0D05) && ((c) <= 0x0D0C)) ||				\
      (((c) >= 0x0D0E) && ((c) <= 0x0D10)) ||				\
      (((c) >= 0x0D12) && ((c) <= 0x0D28)) ||				\
      (((c) >= 0x0D2A) && ((c) <= 0x0D39)) ||				\
      (((c) >= 0x0D60) && ((c) <= 0x0D61)) ||				\
      (((c) >= 0x0E01) && ((c) <= 0x0E2E)) ||				\
      ((c) == 0x0E30) ||						\
      (((c) >= 0x0E32) && ((c) <= 0x0E33)) ||				\
      (((c) >= 0x0E40) && ((c) <= 0x0E45)) ||				\
      (((c) >= 0x0E81) && ((c) <= 0x0E82)) ||				\
      ((c) == 0x0E84) ||						\
      (((c) >= 0x0E87) && ((c) <= 0x0E88)) ||				\
      ((c) == 0x0E8A) ||						\
      ((c) == 0x0E8D) ||						\
      (((c) >= 0x0E94) && ((c) <= 0x0E97)) ||				\
      (((c) >= 0x0E99) && ((c) <= 0x0E9F)) ||				\
      (((c) >= 0x0EA1) && ((c) <= 0x0EA3)) ||				\
      ((c) == 0x0EA5) ||						\
      ((c) == 0x0EA7) ||						\
      (((c) >= 0x0EAA) && ((c) <= 0x0EAB)) ||				\
      (((c) >= 0x0EAD) && ((c) <= 0x0EAE)) ||				\
      ((c) == 0x0EB0) ||						\
      (((c) >= 0x0EB2) && ((c) <= 0x0EB3)) ||				\
      ((c) == 0x0EBD) ||						\
      (((c) >= 0x0EC0) && ((c) <= 0x0EC4)) ||				\
      (((c) >= 0x0F40) && ((c) <= 0x0F47)) ||				\
      (((c) >= 0x0F49) && ((c) <= 0x0F69)) ||				\
      (((c) >= 0x10A0) && ((c) <= 0x10C5)) ||				\
      (((c) >= 0x10D0) && ((c) <= 0x10F6)) ||				\
      ((c) == 0x1100) ||						\
      (((c) >= 0x1102) && ((c) <= 0x1103)) ||				\
      (((c) >= 0x1105) && ((c) <= 0x1107)) ||				\
      ((c) == 0x1109) ||						\
      (((c) >= 0x110B) && ((c) <= 0x110C)) ||				\
      (((c) >= 0x110E) && ((c) <= 0x1112)) ||				\
      ((c) == 0x113C) ||						\
      ((c) == 0x113E) ||						\
      ((c) == 0x1140) ||						\
      ((c) == 0x114C) ||						\
      ((c) == 0x114E) ||						\
      ((c) == 0x1150) ||						\
      (((c) >= 0x1154) && ((c) <= 0x1155)) ||				\
      ((c) == 0x1159) ||						\
      (((c) >= 0x115F) && ((c) <= 0x1161)) ||				\
      ((c) == 0x1163) ||						\
      ((c) == 0x1165) ||						\
      ((c) == 0x1167) ||						\
      ((c) == 0x1169) ||						\
      (((c) >= 0x116D) && ((c) <= 0x116E)) ||				\
      (((c) >= 0x1172) && ((c) <= 0x1173)) ||				\
      ((c) == 0x1175) ||						\
      ((c) == 0x119E) ||						\
      ((c) == 0x11A8) ||						\
      ((c) == 0x11AB) ||						\
      (((c) >= 0x11AE) && ((c) <= 0x11AF)) ||				\
      (((c) >= 0x11B7) && ((c) <= 0x11B8)) ||				\
      ((c) == 0x11BA) ||						\
      (((c) >= 0x11BC) && ((c) <= 0x11C2)) ||				\
      ((c) == 0x11EB) ||						\
      ((c) == 0x11F0) ||						\
      ((c) == 0x11F9) ||						\
      (((c) >= 0x1E00) && ((c) <= 0x1E9B)) ||				\
      (((c) >= 0x1EA0) && ((c) <= 0x1EF9)) ||				\
      (((c) >= 0x1F00) && ((c) <= 0x1F15)) ||				\
      (((c) >= 0x1F18) && ((c) <= 0x1F1D)) ||				\
      (((c) >= 0x1F20) && ((c) <= 0x1F45)) ||				\
      (((c) >= 0x1F48) && ((c) <= 0x1F4D)) ||				\
      (((c) >= 0x1F50) && ((c) <= 0x1F57)) ||				\
      ((c) == 0x1F59) ||						\
      ((c) == 0x1F5B) ||						\
      ((c) == 0x1F5D) ||						\
      (((c) >= 0x1F5F) && ((c) <= 0x1F7D)) ||				\
      (((c) >= 0x1F80) && ((c) <= 0x1FB4)) ||				\
      (((c) >= 0x1FB6) && ((c) <= 0x1FBC)) ||				\
      ((c) == 0x1FBE) ||						\
      (((c) >= 0x1FC2) && ((c) <= 0x1FC4)) ||				\
      (((c) >= 0x1FC6) && ((c) <= 0x1FCC)) ||				\
      (((c) >= 0x1FD0) && ((c) <= 0x1FD3)) ||				\
      (((c) >= 0x1FD6) && ((c) <= 0x1FDB)) ||				\
      (((c) >= 0x1FE0) && ((c) <= 0x1FEC)) ||				\
      (((c) >= 0x1FF2) && ((c) <= 0x1FF4)) ||				\
      (((c) >= 0x1FF6) && ((c) <= 0x1FFC)) ||				\
      ((c) == 0x2126) ||						\
      (((c) >= 0x212A) && ((c) <= 0x212B)) ||				\
      ((c) == 0x212E) ||						\
      (((c) >= 0x2180) && ((c) <= 0x2182)) ||				\
      (((c) >= 0x3041) && ((c) <= 0x3094)) ||				\
      (((c) >= 0x30A1) && ((c) <= 0x30FA)) ||				\
      (((c) >= 0x3105) && ((c) <= 0x312C)) ||				\
      (((c) >= 0xAC00) && ((c) <= 0xD7A3)))

/*
 * [88] Digit ::= ... long list see REC ...
 */
#define IS_DIGIT(c) 							\
     ((((c) >= 0x0030) && ((c) <= 0x0039)) ||				\
      (((c) >= 0x0660) && ((c) <= 0x0669)) ||				\
      (((c) >= 0x06F0) && ((c) <= 0x06F9)) ||				\
      (((c) >= 0x0966) && ((c) <= 0x096F)) ||				\
      (((c) >= 0x09E6) && ((c) <= 0x09EF)) ||				\
      (((c) >= 0x0A66) && ((c) <= 0x0A6F)) ||				\
      (((c) >= 0x0AE6) && ((c) <= 0x0AEF)) ||				\
      (((c) >= 0x0B66) && ((c) <= 0x0B6F)) ||				\
      (((c) >= 0x0BE7) && ((c) <= 0x0BEF)) ||				\
      (((c) >= 0x0C66) && ((c) <= 0x0C6F)) ||				\
      (((c) >= 0x0CE6) && ((c) <= 0x0CEF)) ||				\
      (((c) >= 0x0D66) && ((c) <= 0x0D6F)) ||				\
      (((c) >= 0x0E50) && ((c) <= 0x0E59)) ||				\
      (((c) >= 0x0ED0) && ((c) <= 0x0ED9)) ||				\
      (((c) >= 0x0F20) && ((c) <= 0x0F29)))

/*
 * [87] CombiningChar ::= ... long list see REC ...
 */
#define IS_COMBINING(c) 						\
     ((((c) >= 0x0300) && ((c) <= 0x0345)) ||				\
      (((c) >= 0x0360) && ((c) <= 0x0361)) ||				\
      (((c) >= 0x0483) && ((c) <= 0x0486)) ||				\
      (((c) >= 0x0591) && ((c) <= 0x05A1)) ||				\
      (((c) >= 0x05A3) && ((c) <= 0x05B9)) ||				\
      (((c) >= 0x05BB) && ((c) <= 0x05BD)) ||				\
      ((c) == 0x05BF) ||						\
      (((c) >= 0x05C1) && ((c) <= 0x05C2)) ||				\
      ((c) == 0x05C4) ||						\
      (((c) >= 0x064B) && ((c) <= 0x0652)) ||				\
      ((c) == 0x0670) ||						\
      (((c) >= 0x06D6) && ((c) <= 0x06DC)) ||				\
      (((c) >= 0x06DD) && ((c) <= 0x06DF)) ||				\
      (((c) >= 0x06E0) && ((c) <= 0x06E4)) ||				\
      (((c) >= 0x06E7) && ((c) <= 0x06E8)) ||				\
      (((c) >= 0x06EA) && ((c) <= 0x06ED)) ||				\
      (((c) >= 0x0901) && ((c) <= 0x0903)) ||				\
      ((c) == 0x093C) ||						\
      (((c) >= 0x093E) && ((c) <= 0x094C)) ||				\
      ((c) == 0x094D) ||						\
      (((c) >= 0x0951) && ((c) <= 0x0954)) ||				\
      (((c) >= 0x0962) && ((c) <= 0x0963)) ||				\
      (((c) >= 0x0981) && ((c) <= 0x0983)) ||				\
      ((c) == 0x09BC) ||						\
      ((c) == 0x09BE) ||						\
      ((c) == 0x09BF) ||						\
      (((c) >= 0x09C0) && ((c) <= 0x09C4)) ||				\
      (((c) >= 0x09C7) && ((c) <= 0x09C8)) ||				\
      (((c) >= 0x09CB) && ((c) <= 0x09CD)) ||				\
      ((c) == 0x09D7) ||						\
      (((c) >= 0x09E2) && ((c) <= 0x09E3)) ||				\
      ((c) == 0x0A02) ||						\
      ((c) == 0x0A3C) ||						\
      ((c) == 0x0A3E) ||						\
      ((c) == 0x0A3F) ||						\
      (((c) >= 0x0A40) && ((c) <= 0x0A42)) ||				\
      (((c) >= 0x0A47) && ((c) <= 0x0A48)) ||				\
      (((c) >= 0x0A4B) && ((c) <= 0x0A4D)) ||				\
      (((c) >= 0x0A70) && ((c) <= 0x0A71)) ||				\
      (((c) >= 0x0A81) && ((c) <= 0x0A83)) ||				\
      ((c) == 0x0ABC) ||						\
      (((c) >= 0x0ABE) && ((c) <= 0x0AC5)) ||				\
      (((c) >= 0x0AC7) && ((c) <= 0x0AC9)) ||				\
      (((c) >= 0x0ACB) && ((c) <= 0x0ACD)) ||				\
      (((c) >= 0x0B01) && ((c) <= 0x0B03)) ||				\
      ((c) == 0x0B3C) ||						\
      (((c) >= 0x0B3E) && ((c) <= 0x0B43)) ||				\
      (((c) >= 0x0B47) && ((c) <= 0x0B48)) ||				\
      (((c) >= 0x0B4B) && ((c) <= 0x0B4D)) ||				\
      (((c) >= 0x0B56) && ((c) <= 0x0B57)) ||				\
      (((c) >= 0x0B82) && ((c) <= 0x0B83)) ||				\
      (((c) >= 0x0BBE) && ((c) <= 0x0BC2)) ||				\
      (((c) >= 0x0BC6) && ((c) <= 0x0BC8)) ||				\
      (((c) >= 0x0BCA) && ((c) <= 0x0BCD)) ||				\
      ((c) == 0x0BD7) ||						\
      (((c) >= 0x0C01) && ((c) <= 0x0C03)) ||				\
      (((c) >= 0x0C3E) && ((c) <= 0x0C44)) ||				\
      (((c) >= 0x0C46) && ((c) <= 0x0C48)) ||				\
      (((c) >= 0x0C4A) && ((c) <= 0x0C4D)) ||				\
      (((c) >= 0x0C55) && ((c) <= 0x0C56)) ||				\
      (((c) >= 0x0C82) && ((c) <= 0x0C83)) ||				\
      (((c) >= 0x0CBE) && ((c) <= 0x0CC4)) ||				\
      (((c) >= 0x0CC6) && ((c) <= 0x0CC8)) ||				\
      (((c) >= 0x0CCA) && ((c) <= 0x0CCD)) ||				\
      (((c) >= 0x0CD5) && ((c) <= 0x0CD6)) ||				\
      (((c) >= 0x0D02) && ((c) <= 0x0D03)) ||				\
      (((c) >= 0x0D3E) && ((c) <= 0x0D43)) ||				\
      (((c) >= 0x0D46) && ((c) <= 0x0D48)) ||				\
      (((c) >= 0x0D4A) && ((c) <= 0x0D4D)) ||				\
      ((c) == 0x0D57) ||						\
      ((c) == 0x0E31) ||						\
      (((c) >= 0x0E34) && ((c) <= 0x0E3A)) ||				\
      (((c) >= 0x0E47) && ((c) <= 0x0E4E)) ||				\
      ((c) == 0x0EB1) ||						\
      (((c) >= 0x0EB4) && ((c) <= 0x0EB9)) ||				\
      (((c) >= 0x0EBB) && ((c) <= 0x0EBC)) ||				\
      (((c) >= 0x0EC8) && ((c) <= 0x0ECD)) ||				\
      (((c) >= 0x0F18) && ((c) <= 0x0F19)) ||				\
      ((c) == 0x0F35) ||						\
      ((c) == 0x0F37) ||						\
      ((c) == 0x0F39) ||						\
      ((c) == 0x0F3E) ||						\
      ((c) == 0x0F3F) ||						\
      (((c) >= 0x0F71) && ((c) <= 0x0F84)) ||				\
      (((c) >= 0x0F86) && ((c) <= 0x0F8B)) ||				\
      (((c) >= 0x0F90) && ((c) <= 0x0F95)) ||				\
      ((c) == 0x0F97) ||						\
      (((c) >= 0x0F99) && ((c) <= 0x0FAD)) ||				\
      (((c) >= 0x0FB1) && ((c) <= 0x0FB7)) ||				\
      ((c) == 0x0FB9) ||						\
      (((c) >= 0x20D0) && ((c) <= 0x20DC)) ||				\
      ((c) == 0x20E1) ||						\
      (((c) >= 0x302A) && ((c) <= 0x302F)) ||				\
      ((c) == 0x3099) ||						\
      ((c) == 0x309A))

/*
 * [89] Extender ::= #x00B7 | #x02D0 | #x02D1 | #x0387 | #x0640 |
 *                   #x0E46 | #x0EC6 | #x3005 | [#x3031-#x3035] |
 *                   [#x309D-#x309E] | [#x30FC-#x30FE]
 */
#define IS_EXTENDER(c)							\
    (((c) == 0xb7) || ((c) == 0x2d0) || ((c) == 0x2d1) ||		\
     ((c) == 0x387) || ((c) == 0x640) || ((c) == 0xe46) ||		\
     ((c) == 0xec6) || ((c) == 0x3005)					\
     (((c) >= 0x3031) && ((c) <= 0x3035)) ||				\
     (((c) >= 0x309b) && ((c) <= 0x309e)) ||				\
     (((c) >= 0x30fc) && ((c) <= 0x30fe)))

/*
 * [86] Ideographic ::= [#x4E00-#x9FA5] | #x3007 | [#x3021-#x3029]
 */
#define IS_IDEOGRAPHIC(c)						\
    ((((c) >= 0x4e00) && ((c) <= 0x9fa5)) ||				\
     (((c) >= 0xf900) && ((c) <= 0xfa2d)) ||				\
     (((c) >= 0x3021) && ((c) <= 0x3029)) ||				\
      ((c) == 0x3007))

/*
 * [84] Letter ::= BaseChar | Ideographic 
 */
#define IS_LETTER(c) (IS_BASECHAR(c) || IS_IDEOGRAPHIC(c))

#else
#ifndef USE_UTF_8
/************************************************************************
 *									*
 * 8bits / ISO-Latin version of the macros.				*
 *									*
 ************************************************************************/
/*
 * [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD]
 *                  | [#x10000-#x10FFFF]
 * any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 */
#define IS_CHAR(c)							\
    (((c) == 0x09) || ((c) == 0x0a) || ((c) == 0x0d) || ((c) >= 0x20) ||\
     ((c) == 0xa))

/*
 * [85] BaseChar ::= ... long list see REC ...
 */
#define IS_BASECHAR(c)							\
    ((((c) >= 0x41) && ((c) <= 0x5a)) ||				\
     (((c) >= 0x61) && ((c) <= 0x7a)) ||				\
     (((c) >= 0xaa) && ((c) <= 0x5b)) ||				\
     (((c) >= 0xc0) && ((c) <= 0xd6)) ||				\
     (((c) >= 0xd8) && ((c) <= 0xf6)) ||				\
     (((c) >= 0xf8) && ((c) <= 0xff)) ||				\
      ((c) == 0xba))

/*
 * [88] Digit ::= ... long list see REC ...
 */
#define IS_DIGIT(c) (((c) >= 0x30) && ((c) <= 0x39))

/*
 * [84] Letter ::= BaseChar | Ideographic 
 */
#define IS_LETTER(c) IS_BASECHAR(c)


/*
 * [87] CombiningChar ::= ... long list see REC ...
 */
#define IS_COMBINING(c) 0

/*
 * [89] Extender ::= #x00B7 | #x02D0 | #x02D1 | #x0387 | #x0640 |
 *                   #x0E46 | #x0EC6 | #x3005 | [#x3031-#x3035] |
 *                   [#x309D-#x309E] | [#x30FC-#x30FE]
 */
#define IS_EXTENDER(c) ((c) == 0xb7)

#else /* USE_UTF_8 */
/************************************************************************
 *									*
 * 8bits / UTF-8 version of the macros.					*
 *									*
 ************************************************************************/

TODO !!!
#endif /* USE_UTF_8 */
#endif /* !UNICODE */

/*
 * Blank chars.
 *
 * [3] S ::= (#x20 | #x9 | #xD | #xA)+
 */
#define IS_BLANK(c) (((c) == 0x20) || ((c) == 0x09) || ((c) == 0xa) ||	\
                     ((c) == 0x0D))

/*
 * [13] PubidChar ::= #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]
 */
#define IS_PUBIDCHAR(c)							\
    (((c) == 0x20) || ((c) == 0x0D) || ((c) == 0x0A) ||			\
     (((c) >= 'a') && ((c) <= 'z')) ||					\
     (((c) >= 'A') && ((c) <= 'Z')) ||					\
     (((c) >= '0') && ((c) <= '9')) ||					\
     ((c) == '-') || ((c) == '\'') || ((c) == '(') || ((c) == ')') ||	\
     ((c) == '+') || ((c) == ',') || ((c) == '.') || ((c) == '/') ||	\
     ((c) == ':') || ((c) == '=') || ((c) == '?') || ((c) == ';') ||	\
     ((c) == '!') || ((c) == '*') || ((c) == '#') || ((c) == '@') ||	\
     ((c) == '$') || ((c) == '_') || ((c) == '%'))

#define SKIP_EOL(p) 							\
    if (*(p) == 0x13) { p++ ; if (*(p) == 0x10) p++; }			\
    if (*(p) == 0x10) { p++ ; if (*(p) == 0x13) p++; }

#define MOVETO_ENDTAG(p)						\
    while (IS_CHAR(*p) && (*(p) != '>')) (p)++

#define MOVETO_STARTTAG(p)						\
    while (IS_CHAR(*p) && (*(p) != '<')) (p)++

/************************************************************************
 *									*
 *		Commodity functions to handle CHARs			*
 *									*
 ************************************************************************/

/**
 * xmlStrndup:
 * @cur:  the input CHAR *
 * @len:  the len of @cur
 *
 * a strndup for array of CHAR's
 * return values: a new CHAR * or NULL
 */

CHAR *
GXxmlStrndup(const CHAR *cur, int len) {
    CHAR *ret = GxCore_Malloc((len + 1) * sizeof(CHAR));

    if (ret == NULL) {
        fprintf(stderr, "GxCore_Malloc of %d byte failed\n",
	        (len + 1) * sizeof(CHAR));
        return(NULL);
    }

    memcpy(ret, cur, len * sizeof(CHAR));
    ret[len] = 0;
    return(ret);
}

/**
 * xmlStrdup:
 * @cur:  the input CHAR *
 *
 * a GxCore_Strdup for array of CHAR's
 * return values: a new CHAR * or NULL
 */

CHAR *
GXxmlStrdup(const CHAR *cur) {
    const CHAR *p = cur;

    while (IS_CHAR(*p)) p++;
    return(GXxmlStrndup(cur, p - cur));
}

/**
 * xmlCharStrndup:
 * @cur:  the input char *
 * @len:  the len of @cur
 *
 * a strndup for char's to CHAR's
 * return values: a new CHAR * or NULL
 */

CHAR *
GXxmlCharStrndup(const char *cur, int len) {
    int i;
    CHAR *ret = GxCore_Malloc((len + 1) * sizeof(CHAR));

    if (ret == NULL) {
        fprintf(stderr, "GxCore_Malloc of %d byte failed\n",
	        (len + 1) * sizeof(CHAR));
        return(NULL);
    }
    for (i = 0;i < len;i++)
        ret[i] = (CHAR) cur[i];
    ret[len] = 0;
    return(ret);
}

/**
 * xmlCharStrdup:
 * @cur:  the input char *
 * @len:  the len of @cur
 *
 * a GxCore_Strdup for char's to CHAR's
 * return values: a new CHAR * or NULL
 */

CHAR *
GXxmlCharStrdup(const char *cur) {
    const char *p = cur;

    while (*p != '\0') p++;
    return(GXxmlCharStrndup(cur, p - cur));
}

/**
 * xmlStrcmp:
 * @str1:  the first CHAR *
 * @str2:  the second CHAR *
 *
 * a strcmp for CHAR's
 * return values: the integer result of the comparison
 */

int
GXxmlStrcmp(const CHAR *str1, const CHAR *str2) {
    register int tmp;

    do {
        tmp = *str1++ - *str2++;
	if (tmp != 0) return(tmp);
    } while ((*str1 != 0) && (*str2 != 0));
    return (*str1 - *str2);
}

/**
 * xmlStrncmp:
 * @str1:  the first CHAR *
 * @str2:  the second CHAR *
 * @len:  the max comparison length
 *
 * a strncmp for CHAR's
 * return values: the integer result of the comparison
 */

int
GXxmlStrncmp(const CHAR *str1, const CHAR *str2, int len) {
    register int tmp;

    if (len <= 0) return(0);
    do {
        tmp = *str1++ - *str2++;
	if (tmp != 0) return(tmp);
	len--;
        if (len <= 0) return(0);
    } while ((*str1 != 0) && (*str2 != 0));
    return (*str1 - *str2);
}

/**
 * xmlStrchr:
 * @str:  the CHAR * array
 * @val:  the CHAR to search
 *
 * a strchr for CHAR's
 * return values: the CHAR * for the first occurence or NULL.
 */

CHAR *
GXxmlStrchr(const CHAR *str, CHAR val) {
    while (*str != 0) {
        if (*str == val) return((CHAR *) str);
	str++;
    }
    return(NULL);
}

/**
 * xmlStrlen:
 * @str:  the CHAR * array
 *
 * lenght of a CHAR's string
 * return values: the number of CHAR contained in the ARRAY.
 */

int
GXxmlStrlen(const CHAR *str) {
    int len = 0;

    if (str == NULL) return(0);
    while (*str != 0) {
	str++;
	len++;
    }
    return(len);
}

/**
 * xmlStrncat:
 * @first:  the original CHAR * array
 * @add:  the CHAR * array added
 * @len:  the length of @add
 *
 * a strncat for array of CHAR's
 * return values: a new CHAR * containing the concatenated string.
 */

CHAR *
GXxmlStrncat(CHAR *cur, const CHAR *add, int len) {
    int size;
    CHAR *ret;

    if ((add == NULL) || (len == 0))
        return(cur);
    if (cur == NULL)
        return(GXxmlStrndup(add, len));

    size = GXxmlStrlen(cur);
    ret = GxCore_Realloc(cur, (size + len + 1) * sizeof(CHAR));
    if (ret == NULL) {
        fprintf(stderr, "xmlStrncat: GxCore_Realloc of %d byte failed\n",
	        (size + len + 1) * sizeof(CHAR));
        return(cur);
    }
    memcpy(&ret[size], add, len * sizeof(CHAR));
    ret[size + len] = 0;
    return(ret);
}

/**
 * xmlStrcat:
 * @first:  the original CHAR * array
 * @add:  the CHAR * array added
 *
 * a strcat for array of CHAR's
 * return values: a new CHAR * containing the concatenated string.
 */

CHAR *
GXxmlStrcat(CHAR *cur, const CHAR *add) {
    const CHAR *p = add;

    if (add == NULL) return(cur);
    if (cur == NULL) 
        return(GXxmlStrdup(add));

    while (IS_CHAR(*p)) p++;
    return(GXxmlStrncat(cur, add, p - add));
}

/************************************************************************
 *									*
 *		Commodity functions, cleanup needed ?			*
 *									*
 ************************************************************************/

/**
 * areBlanks:
 * @ctxt:  an XML parser context
 * @str:  a CHAR *
 * @len:  the size of @str
 *
 * Is this a sequence of blank chars that one can ignore ?
 *
 * TODO: to be corrected accodingly to DTD information if available
 * return values: 1 if ignorable 0 otherwise.
 */

static int areBlanks(GXxmlParserCtxtPtr ctxt, const CHAR *str, int len) {
    int i;
    GXxmlNodePtr lastChild;

    for (i = 0;i < len;i++)
        if (!(IS_BLANK(str[i]))) return(0);

    if (CUR != '<') return(0);
    lastChild = GXxmlGetLastChild(ctxt->node);
    if (lastChild == NULL) {
        if (ctxt->node->content != NULL) return(0);
    } else if (GXxmlNodeIsText(lastChild))
        return(0);
    return(1);
}

/**
 * xmlHandleEntity:
 * @ctxt:  an XML parser context
 * @entity:  an XML entity pointer.
 *
 * Default handling of defined entities, when should we define a new input
 * stream ? When do we just handle that as a set of chars ?
 * TODO: we should call the SAX handler here and have it resolve the issue
 */

void
GXxmlHandleEntity(GXxmlParserCtxtPtr ctxt, GXxmlEntityPtr entity) {
    int len;
    GXxmlParserInputPtr input;

    if (entity->content == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "xmlHandleEntity %s: content == NULL\n",
	               entity->name);
        return;
    }
    len = GXxmlStrlen(entity->content);
    if (len <= 2) goto handle_as_char;

    /*
     * Redefine its content as an input stream.
     */
    input = GXxmlNewEntityInputStream(ctxt, entity);
    GXxmlPushInput(ctxt, input);
    return;

handle_as_char:
    /*
     * Just handle the content as a set of chars.
     */
    if (ctxt->sax != NULL)
	ctxt->sax->characters(ctxt, entity->content, 0, len);

}

/*
 * Forward definition for recusive behaviour.
 */
GXxmlNodePtr GXxmlParseElement(GXxmlParserCtxtPtr ctxt);
CHAR *GXxmlParsePEReference(GXxmlParserCtxtPtr ctxt);
CHAR *GXxmlParseReference(GXxmlParserCtxtPtr ctxt);

/************************************************************************
 *									*
 *		Extra stuff for namespace support			*
 *	Relates to http://www.w3.org/TR/WD-xml-names			*
 *									*
 ************************************************************************/

/**
 * xmlNamespaceParseNCName:
 * @ctxt:  an XML parser context
 *
 * parse an XML namespace name.
 *
 * [NS 3] NCName ::= (Letter | '_') (NCNameChar)*
 *
 * [NS 4] NCNameChar ::= Letter | Digit | '.' | '-' | '_' |
 *                       CombiningChar | Extender
 * return values: the namespace name or NULL
 */

CHAR *
GXxmlNamespaceParseNCName(GXxmlParserCtxtPtr ctxt) {
    const CHAR *q;
    CHAR *ret = NULL;

    if (!IS_LETTER(CUR) && (CUR != '_')) return(NULL);
    q = NEXT;

    while ((IS_LETTER(CUR)) || (IS_DIGIT(CUR)) ||
           (CUR == '.') || (CUR == '-') ||
	   (CUR == '_') ||
	   (IS_COMBINING(CUR)) ||
	   (IS_EXTENDER(CUR)))
	NEXT;
    
    ret = GXxmlStrndup(q, CUR_PTR - q);

    return(ret);
}

/**
 * xmlNamespaceParseQName:
 * @ctxt:  an XML parser context
 * @prefix:  a CHAR ** 
 *
 * parse an XML qualified name
 *
 * [NS 5] QName ::= (Prefix ':')? LocalPart
 *
 * [NS 6] Prefix ::= NCName
 *
 * [NS 7] LocalPart ::= NCName
 * return values: the function returns the local part, and prefix is updated
 *   to get the Prefix if any.
 */

CHAR *
GXxmlNamespaceParseQName(GXxmlParserCtxtPtr ctxt, CHAR **prefix) {
    CHAR *ret = NULL;

    *prefix = NULL;
    ret = GXxmlNamespaceParseNCName(ctxt);
    if (CUR == ':') {
        *prefix = ret;
	NEXT;
	ret = GXxmlNamespaceParseNCName(ctxt);
    }

    return(ret);
}

/**
 * xmlNamespaceParseNSDef:
 * @ctxt:  an XML parser context
 *
 * parse a namespace prefix declaration
 *
 * [NS 1] NSDef ::= PrefixDef Eq SystemLiteral
 *
 * [NS 2] PrefixDef ::= 'xmlns' (':' NCName)?
 * return values: the namespace name
 */

CHAR *
GXxmlNamespaceParseNSDef(GXxmlParserCtxtPtr ctxt) {
    CHAR *name = NULL;

    if ((CUR == 'x') && (NXT(1) == 'm') &&
        (NXT(2) == 'l') && (NXT(3) == 'n') &&
	(NXT(4) == 's')) {
	SKIP(5);
	if (CUR == ':') {
	    NEXT;
	    name = GXxmlNamespaceParseNCName(ctxt);
	}
    }
    return(name);
}

/**
 * xmlParseQuotedString:
 * @ctxt:  an XML parser context
 *
 * [OLD] Parse and return a string between quotes or doublequotes
 * return values: the string parser or NULL.
 */
CHAR *
GXxmlParseQuotedString(GXxmlParserCtxtPtr ctxt) {
    CHAR *ret = NULL;
    const CHAR *q;

    if (CUR == '"') {
        NEXT;
	q = CUR_PTR;
	while (IS_CHAR(CUR) && (CUR != '"')) NEXT;
	if (CUR != '"') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "String not closed \"%.50s\"\n", q);
        } else {
            ret = GXxmlStrndup(q, CUR_PTR - q);
	    NEXT;
	}
    } else if (CUR == '\''){
        NEXT;
	q = CUR_PTR;
	while (IS_CHAR(CUR) && (CUR != '\'')) NEXT;
	if (CUR != '\'') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "String not closed \"%.50s\"\n", q);
        } else {
            ret = GXxmlStrndup(q, CUR_PTR - q);
	    NEXT;
	}
    }
    return(ret);
}

/**
 * xmlParseNamespace:
 * @ctxt:  an XML parser context
 *
 * [OLD] xmlParseNamespace: parse specific PI '<?namespace ...' constructs.
 *
 * This is what the older xml-name Working Draft specified, a bunch of
 * other stuff may still rely on it, so support is still here as
 * if ot was declared on the root of the Tree:-(
 */

void
GXxmlParseNamespace(GXxmlParserCtxtPtr ctxt) {
    CHAR *href = NULL;
    CHAR *prefix = NULL;
    int garbage = 0;

    /*
     * We just skipped "namespace" or "xml:namespace"
     */
    SKIP_BLANKS;

    while (IS_CHAR(CUR) && (CUR != '>')) {
	/*
	 * We can have "ns" or "prefix" attributes
	 * Old encoding as 'href' or 'AS' attributes is still supported
	 */
	if ((CUR == 'n') && (NXT(1) == 's')) {
	    garbage = 0;
	    SKIP(2);
	    SKIP_BLANKS;

	    if (CUR != '=') continue;
	    NEXT;
	    SKIP_BLANKS;

	    href = GXxmlParseQuotedString(ctxt);
	    SKIP_BLANKS;
	} else if ((CUR == 'h') && (NXT(1) == 'r') &&
	    (NXT(2) == 'e') && (NXT(3) == 'f')) {
	    garbage = 0;
	    SKIP(4);
	    SKIP_BLANKS;

	    if (CUR != '=') continue;
	    NEXT;
	    SKIP_BLANKS;

	    href = GXxmlParseQuotedString(ctxt);
	    SKIP_BLANKS;
	} else if ((CUR == 'p') && (NXT(1) == 'r') &&
	           (NXT(2) == 'e') && (NXT(3) == 'f') &&
	           (NXT(4) == 'i') && (NXT(5) == 'x')) {
	    garbage = 0;
	    SKIP(6);
	    SKIP_BLANKS;

	    if (CUR != '=') continue;
	    NEXT;
	    SKIP_BLANKS;

	    prefix = GXxmlParseQuotedString(ctxt);
	    SKIP_BLANKS;
	} else if ((CUR == 'A') && (NXT(1) == 'S')) {
	    garbage = 0;
	    SKIP(2);
	    SKIP_BLANKS;

	    if (CUR != '=') continue;
	    NEXT;
	    SKIP_BLANKS;

	    prefix = GXxmlParseQuotedString(ctxt);
	    SKIP_BLANKS;
	} else if ((CUR == '?') && (NXT(1) == '>')) {
	    garbage = 0;
	    CUR_PTR ++;
	} else {
            /*
	     * Found garbage when parsing the namespace
	     */
	    if (!garbage)
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "xmlParseNamespace found garbage\n");
            NEXT;
        }
    }

    MOVETO_ENDTAG(CUR_PTR);
    NEXT;

    /*
     * Register the DTD.
     */
    if (href != NULL)
        GXxmlNewGlobalNs(ctxt->doc, href, prefix);

    if (prefix != NULL) GxCore_Free(prefix);
    if (href != NULL) GxCore_Free(href);
}

/************************************************************************
 *									*
 *			The parser itself				*
 *	Relates to http://www.w3.org/TR/REC-xml				*
 *									*
 ************************************************************************/

/**
 * xmlParseName:
 * @ctxt:  an XML parser context
 *
 * parse an XML name.
 *
 * [4] NameChar ::= Letter | Digit | '.' | '-' | '_' | ':' |
 *                  CombiningChar | Extender
 *
 * [5] Name ::= (Letter | '_' | ':') (NameChar)*
 *
 * [6] Names ::= Name (S Name)*
 * return values: the Name parsed or NULL
 */

CHAR *
GXxmlParseName(GXxmlParserCtxtPtr ctxt) {
    const CHAR *q;
    CHAR *ret = NULL;

    if (!IS_LETTER(CUR) && (CUR != '_') &&
        (CUR != ':')) return(NULL);
    q = NEXT;

    while ((IS_LETTER(CUR)) || (IS_DIGIT(CUR)) ||
           (CUR == '.') || (CUR == '-') ||
	   (CUR == '_') || (CUR == ':') || 
	   (IS_COMBINING(CUR)) ||
	   (IS_EXTENDER(CUR)))
	NEXT;
    
    ret = GXxmlStrndup(q, CUR_PTR - q);

    return(ret);
}

/**
 * GXxmlParseNmtoken:
 * @ctxt:  an XML parser context
 * 
 * parse an XML Nmtoken.
 *
 * [7] Nmtoken ::= (NameChar)+
 *
 * [8] Nmtokens ::= Nmtoken (S Nmtoken)*
 * return values: the Nmtoken parsed or NULL
 */

CHAR *
GXxmlParseNmtoken(GXxmlParserCtxtPtr ctxt) {
    const CHAR *q;
    CHAR *ret = NULL;

    q = NEXT;

    while ((IS_LETTER(CUR)) || (IS_DIGIT(CUR)) ||
           (CUR == '.') || (CUR == '-') ||
	   (CUR == '_') || (CUR == ':') || 
	   (IS_COMBINING(CUR)) ||
	   (IS_EXTENDER(CUR)))
	NEXT;
    
    ret = GXxmlStrndup(q, CUR_PTR - q);

    return(ret);
}

/**
 * xmlParseEntityValue:
 * @ctxt:  an XML parser context
 *
 * parse a value for ENTITY decl.
 *
 * [9] EntityValue ::= '"' ([^%&"] | PEReference | Reference)* '"' |
 *	               "'" ([^%&'] | PEReference | Reference)* "'"
 * return values: the EntityValue parsed or NULL
 */

CHAR *
GXxmlParseEntityValue(GXxmlParserCtxtPtr ctxt) {
    CHAR *ret = NULL, *cur;
    const CHAR *q;

    if (CUR == '"') {
        NEXT;

	q = CUR_PTR;
	while ((IS_CHAR(CUR)) && (CUR != '"')) {
	    if (CUR == '%') {
	        ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        cur = GXxmlParsePEReference(ctxt);
		ret = GXxmlStrcat(ret, cur);
		q = CUR_PTR;
	    } else if (CUR == '&') {
	        ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        cur = GXxmlParseReference(ctxt);
		if (cur != NULL) {
		    CHAR buf[2];
		    buf[0] = '&';
		    buf[1] = 0;
		    ret = GXxmlStrncat(ret, buf, 1);
		    ret = GXxmlStrcat(ret, cur);
		    buf[0] = ';';
		    buf[1] = 0;
		    ret = GXxmlStrncat(ret, buf, 1);
		}
		q = CUR_PTR;
	    } else 
	        NEXT;
	}
	if (!IS_CHAR(CUR)) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished EntityValue\n");
	} else {
	    ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	    NEXT;
        }
    } else if (CUR == '\'') {
        NEXT;
	q = CUR_PTR;
	while ((IS_CHAR(CUR)) && (CUR != '\'')) {
	    if (CUR == '%') {
	        ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        cur = GXxmlParsePEReference(ctxt);
		ret = GXxmlStrcat(ret, cur);
		q = CUR_PTR;
	    } else if (CUR == '&') {
	        ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        cur = GXxmlParseReference(ctxt);
		if (cur != NULL) {
		    CHAR buf[2];
		    buf[0] = '&';
		    buf[1] = 0;
		    ret = GXxmlStrncat(ret, buf, 1);
		    ret = GXxmlStrcat(ret, cur);
		    buf[0] = ';';
		    buf[1] = 0;
		    ret = GXxmlStrncat(ret, buf, 1);
		}
		q = CUR_PTR;
	    } else 
	        NEXT;
	}
	if (!IS_CHAR(CUR)) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished EntityValue\n");
	} else {
	    ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	    NEXT;
        }
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "GXxmlParseEntityValue \" or ' expected\n");
    }
    
    return(ret);
}

/**
 * GXxmlParseAttValue:
 * @ctxt:  an XML parser context
 *
 * parse a value for an attribute
 *
 * [10] AttValue ::= '"' ([^<&"] | Reference)* '"' |
 *                   "'" ([^<&'] | Reference)* "'"
 * return values: the AttValue parsed or NULL.
 */

CHAR *
GXxmlParseAttValue(GXxmlParserCtxtPtr ctxt) {
    CHAR *ret = NULL, *cur;
    const CHAR *q;

    if (CUR == '"') {
        NEXT;

	q = CUR_PTR;
	while ((IS_CHAR(CUR)) && (CUR != '"')) {
	    if (CUR == '&') {
	        ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        cur = GXxmlParseReference(ctxt);
		if (cur != NULL) {
		    /*
		     * Special case for '&amp;', we don't want to
		     * resolve it here since it will break later
		     * when searching entities in the string.
		     */
		    if ((cur[0] == '&') && (cur[1] == 0)) {
		        CHAR buf[6] = { '&', 'a', 'm', 'p', ';', 0 };
		        ret = GXxmlStrncat(ret, buf, 5);
		    } else
		        ret = GXxmlStrcat(ret, cur);
		    GxCore_Free(cur);
		}
		q = CUR_PTR;
	    } else 
	        NEXT;
	    /*
	     * Pop out finished entity references.
	     */
	    while ((CUR == 0) && (ctxt->inputNr > 1)) {
		if (CUR_PTR != q)
		    ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        GXxmlPopInput(ctxt);
		q = CUR_PTR;
	    }
	}
	if (!IS_CHAR(CUR)) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished AttValue\n");
	} else {
	    ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	    NEXT;
        }
    } else if (CUR == '\'') {
        NEXT;
	q = CUR_PTR;
	while ((IS_CHAR(CUR)) && (CUR != '\'')) {
	    if (CUR == '&') {
	        ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        cur = GXxmlParseReference(ctxt);
		if (cur != NULL) {
		    /*
		     * Special case for '&amp;', we don't want to
		     * resolve it here since it will break later
		     * when searching entities in the string.
		     */
		    if ((cur[0] == '&') && (cur[1] == 0)) {
		        CHAR buf[6] = { '&', 'a', 'm', 'p', ';', 0 };
		        ret = GXxmlStrncat(ret, buf, 5);
		    } else
		        ret = GXxmlStrcat(ret, cur);
		    GxCore_Free(cur);
		}
		q = CUR_PTR;
	    } else 
	        NEXT;
	    /*
	     * Pop out finished entity references.
	     */
	    while ((CUR == 0) && (ctxt->inputNr > 1)) {
		if (CUR_PTR != q)
		    ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	        GXxmlPopInput(ctxt);
		q = CUR_PTR;
	    }
	}
	if (!IS_CHAR(CUR)) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished AttValue\n");
	} else {
	    ret = GXxmlStrncat(ret, q, CUR_PTR - q);
	    NEXT;
        }
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "AttValue: \" or ' expected\n");
    }
    
    return(ret);
}

/**
 * GXxmlParseSystemLiteral:
 * @ctxt:  an XML parser context
 * 
 * parse an XML Literal
 *
 * [11] SystemLiteral ::= ('"' [^"]* '"') | ("'" [^']* "'")
 * return values: the SystemLiteral parsed or NULL
 */

CHAR *
GXxmlParseSystemLiteral(GXxmlParserCtxtPtr ctxt) {
    const CHAR *q;
    CHAR *ret = NULL;

    if (CUR == '"') {
        NEXT;
	q = CUR_PTR;
	while ((IS_CHAR(CUR)) && (CUR != '"'))
	    NEXT;
	if (!IS_CHAR(CUR)) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished SystemLiteral\n");
	} else {
	    ret = GXxmlStrndup(q, CUR_PTR - q);
	    NEXT;
        }
    } else if (CUR == '\'') {
        NEXT;
	q = CUR_PTR;
	while ((IS_CHAR(CUR)) && (CUR != '\''))
	    NEXT;
	if (!IS_CHAR(CUR)) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished SystemLiteral\n");
	} else {
	    ret = GXxmlStrndup(q, CUR_PTR - q);
	    NEXT;
        }
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "SystemLiteral \" or ' expected\n");
    }
    
    return(ret);
}

/**
 * GXxmlParsePubidLiteral:
 * @ctxt:  an XML parser context
 *
 * parse an XML public literal
 * return values: the PubidLiteral parsed or NULL.
 */

CHAR *
GXxmlParsePubidLiteral(GXxmlParserCtxtPtr ctxt) {
    const CHAR *q;
    CHAR *ret = NULL;
    /*
     * Name ::= (Letter | '_') (NameChar)*
     */
    if (CUR == '"') {
        NEXT;
	q = CUR_PTR;
	while (IS_PUBIDCHAR(CUR)) NEXT;
	if (CUR != '"') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished PubidLiteral\n");
	} else {
	    ret = GXxmlStrndup(q, CUR_PTR - q);
	    NEXT;
	}
    } else if (CUR == '\'') {
        NEXT;
	q = CUR_PTR;
	while ((IS_LETTER(CUR)) && (CUR != '\''))
	    NEXT;
	if (!IS_LETTER(CUR)) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Unfinished PubidLiteral\n");
	} else {
	    ret = GXxmlStrndup(q, CUR_PTR - q);
	    NEXT;
	}
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "SystemLiteral \" or ' expected\n");
    }
    
    return(ret);
}

/**
 * GXxmlParseCharData:
 * @ctxt:  an XML parser context
 * @cdata:  int indicating whether we are within a CDATA section
 *
 * parse a CharData section.
 * if we are within a CDATA section ']]>' marks an end of section.
 *
 * [14] CharData ::= [^<&]* - ([^<&]* ']]>' [^<&]*)
 * return values: 
 */

void
GXxmlParseCharData(GXxmlParserCtxtPtr ctxt, int cdata) {
    const CHAR *q;

    q = CUR_PTR;
    while ((IS_CHAR(CUR)) && (CUR != '<') &&
           (CUR != '&')) {
        NEXT;
	if ((cdata) && (CUR == ']') && (NXT(1) == ']') &&
	    (NXT(2) == '>')) break;
    }
    if (q == CUR_PTR) return;

    /*
     * Ok the segment [q CUR_PTR] is to be consumed as chars.
     */
    if (ctxt->sax != NULL) {
	if (areBlanks(ctxt, q, CUR_PTR - q))
	    ctxt->sax->ignorableWhitespace(ctxt, q, 0, CUR_PTR - q);
	else
	    ctxt->sax->characters(ctxt, q, 0, CUR_PTR - q);
    }
}

/**
 * GXxmlParseExternalID:
 * @ctxt:  an XML parser context
 * @publicID:  a CHAR** receiving PubidLiteral
 *
 * Parse an External ID
 *
 * [75] ExternalID ::= 'SYSTEM' S SystemLiteral
 *                   | 'PUBLIC' S PubidLiteral S SystemLiteral
 * return values: the function returns SystemLiteral and in the second
 *                case publicID receives PubidLiteral
 */

CHAR *
GXxmlParseExternalID(GXxmlParserCtxtPtr ctxt, CHAR **publicID) {
    CHAR *URI = NULL;

    if ((CUR == 'S') && (NXT(1) == 'Y') &&
         (NXT(2) == 'S') && (NXT(3) == 'T') &&
	 (NXT(4) == 'E') && (NXT(5) == 'M')) {
        SKIP(6);
        SKIP_BLANKS;
	URI = GXxmlParseSystemLiteral(ctxt);
	if (URI == NULL)
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt,
	          "GXxmlParseExternalID: SYSTEM, no URI\n");
    } else if ((CUR == 'P') && (NXT(1) == 'U') &&
	       (NXT(2) == 'B') && (NXT(3) == 'L') &&
	       (NXT(4) == 'I') && (NXT(5) == 'C')) {
        SKIP(6);
        SKIP_BLANKS;
	*publicID = GXxmlParsePubidLiteral(ctxt);
	if (*publicID == NULL)
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, 
	          "GXxmlParseExternalID: PUBLIC, no Public Identifier\n");
        SKIP_BLANKS;
	URI = GXxmlParseSystemLiteral(ctxt);
	if (URI == NULL)
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, 
	           "GXxmlParseExternalID: PUBLIC, no URI\n");
    }
    return(URI);
}

/**
 * GXxmlParseComment:
 * @create:  should we create a node
 *
 * Skip an XML (SGML) comment <!-- .... -->
 *  This may or may not create a node (depending on the context)
 *  The spec says that "For compatibility, the string "--" (double-hyphen)
 *  must not occur within comments. "
 *
 * [15] Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
 *
 * TODO: this should call a SAX function which will handle (or not) the
 *       creation of the comment !
 * return values: 
 */
GXxmlNodePtr GXxmlParseComment(GXxmlParserCtxtPtr ctxt, int create) {
    GXxmlNodePtr ret = NULL;
    const CHAR *q, *start;
    const CHAR *r;
    CHAR *val;

    /*
     * Check that there is a comment right here.
     */
    if ((CUR != '<') || (NXT(1) != '!') ||
        (NXT(2) != '-') || (NXT(3) != '-')) return(NULL);

    SKIP(4);
    start = q = CUR_PTR;
    NEXT;
    r = CUR_PTR;
    NEXT;
    while (IS_CHAR(CUR) &&
           ((CUR == ':') || (CUR != '>') ||
	    (*r != '-') || (*q != '-'))) {
	if ((*r == '-') && (*q == '-'))
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt,
	       "Comment must not contain '--' (double-hyphen)`\n");
        NEXT;r++;q++;
    }
    if (!IS_CHAR(CUR)) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "Comment not terminated \n<!--%.50s\n", start);
    } else {
        NEXT;
	if (create) {
	    val = GXxmlStrndup(start, q - start);
	    ret = GXxmlNewDocComment(ctxt->doc, val);
	    GxCore_Free(val);
	}
    }
    return(ret);
}

/**
 * GXxmlParsePITarget:
 * @ctxt:  an XML parser context
 * 
 * parse the name of a PI
 *
 * [17] PITarget ::= Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))
 * return values: the PITarget name or NULL
 */

CHAR *
GXxmlParsePITarget(GXxmlParserCtxtPtr ctxt) {
    CHAR *name;

    name = GXxmlParseName(ctxt);
    if ((name != NULL) && (name[3] == 0) &&
        ((name[0] == 'x') || (name[0] == 'X')) &&
        ((name[1] == 'm') || (name[1] == 'M')) &&
        ((name[2] == 'l') || (name[2] == 'L'))) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "GXxmlParsePItarget: invalid name prefix 'GXxml'\n");
	return(NULL);
    }
    return(name);
}

/**
 * GXxmlParsePI:
 * @ctxt:  an XML parser context
 * 
 * parse an XML Processing Instruction.
 *
 * [16] PI ::= '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
 * return values: the PI name or NULL
 */

void
GXxmlParsePI(GXxmlParserCtxtPtr ctxt) {
    CHAR *target;

    if ((CUR == '<') && (NXT(1) == '?')) {
	/*
	 * this is a Processing Instruction.
	 */
	SKIP(2);

	/*
	 * Parse the target name and check for special support like
	 * namespace.
	 *
	 * TODO : PI handling should be dynamically redefinable using an
	 *        API. Only namespace should be in the code IMHO ...
	 */
        target = GXxmlParsePITarget(ctxt);
	if (target != NULL) {
	    /*
	     * Support for the old Processing Instruction related to namespace.
	     */
	    if ((target[0] == 'n') && (target[1] == 'a') &&
		(target[2] == 'm') && (target[3] == 'e') &&
		(target[4] == 's') && (target[5] == 'p') &&
		(target[6] == 'a') && (target[7] == 'c') &&
		(target[8] == 'e')) {
		GXxmlParseNamespace(ctxt);
	    } else if ((target[0] == 'x') && (target[1] == 'm') &&
		       (target[2] == 'l') && (target[3] == ':') &&
		       (target[4] == 'n') && (target[5] == 'a') &&
		       (target[6] == 'm') && (target[7] == 'e') &&
		       (target[8] == 's') && (target[9] == 'p') &&
		       (target[10] == 'a') && (target[11] == 'c') &&
		       (target[12] == 'e')) {
		GXxmlParseNamespace(ctxt);
	    } else {
	        const CHAR *q = CUR_PTR;

		while (IS_CHAR(CUR) &&
		       ((CUR != '?') || (NXT(1) != '>')))
		    NEXT;
		if (!IS_CHAR(CUR)) {
		    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		        ctxt->sax->error(ctxt, "GXxmlParsePI: PI %s never end ...\n",
		                   target);
		} else {
		    CHAR *data;

		    data = GXxmlStrndup(CUR_PTR, CUR_PTR - q);
		    SKIP(2);

		    /*
		     * SAX: PI detected.
		     */
		    if (ctxt->sax) 
			ctxt->sax->processingInstruction(ctxt, target, data);
		    /*
		     * Unknown PI, ignore it !
		     */
		    else 
			GXxmlParserWarning(ctxt,
		           "GXxmlParsePI : skipping unknown PI %s\n",
				         target);
	            GxCore_Free(data);
                }
	    }
	    GxCore_Free(target);
	} else {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParsePI : no target name\n");
	    /********* Should we try to complete parsing the PI ???
	    while (IS_CHAR(CUR) &&
		   (CUR != '?') && (CUR != '>'))
		NEXT;
	    if (!IS_CHAR(CUR)) {
		fprintf(stderr, "GXxmlParsePI: PI %s never end ...\n",
			target);
	    }
	     ********************************************************/
	}
    }
}

/**
 * GXxmlParseNotationDecl:
 * @ctxt:  an XML parser context
 *
 * parse a notation declaration
 *
 * [82] NotationDecl ::= '<!NOTATION' S Name S (ExternalID |  PublicID) S? '>'
 *
 * [83] PublicID ::= 'PUBLIC' S PubidLiteral
 *
 * NOTE: Actually [75] and [83] interract badly since [75] can generate
 *       'PUBLIC' S PubidLiteral S SystemLiteral
 *
 * Hence there is actually 3 choices:
 *     'PUBLIC' S PubidLiteral
 *     'PUBLIC' S PubidLiteral S SystemLiteral
 * and 'SYSTEM' S SystemLiteral
 *
 * TODO: no handling of the values parsed !
 */

void
GXxmlParseNotationDecl(GXxmlParserCtxtPtr ctxt) {
    CHAR *name;
    
    if ((CUR == '<') && (NXT(1) == '!') &&
        (NXT(2) == 'N') && (NXT(3) == 'O') &&
        (NXT(4) == 'T') && (NXT(5) == 'A') &&
        (NXT(6) == 'T') && (NXT(7) == 'I') &&
        (NXT(8) == 'O') && (NXT(9) == 'N') &&
        (IS_BLANK(NXT(10)))) {
	SKIP(10);
        SKIP_BLANKS;

        name = GXxmlParseName(ctxt);
	if (name == NULL) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt,
	        "GXxmlParseAttributeListDecl: no name for Element\n");
	    return;
	}
	SKIP_BLANKS;
	/*
	 * TODO !!!
	 */
	while ((IS_CHAR(CUR)) && (CUR != '>'))
	    NEXT;
	GxCore_Free(name);
    }
}

/**
 * GXxmlParseEntityDecl:
 * @ctxt:  an XML parser context
 *
 * parse <!ENTITY declarations
 *
 * [70] EntityDecl ::= GEDecl | PEDecl
 *
 * [71] GEDecl ::= '<!ENTITY' S Name S EntityDef S? '>'
 *
 * [72] PEDecl ::= '<!ENTITY' S '%' S Name S PEDef S? '>'
 *
 * [73] EntityDef ::= EntityValue | (ExternalID NDataDecl?)
 *
 * [74] PEDef ::= EntityValue | ExternalID
 *
 * [76] NDataDecl ::= S 'NDATA' S Name
 */

void
GXxmlParseEntityDecl(GXxmlParserCtxtPtr ctxt) {
    CHAR *name = NULL;
    CHAR *value = NULL;
    CHAR *URI = NULL, *literal = NULL;
    CHAR *ndata = NULL;
    int isParameter = 0;
    
    if ((CUR == '<') && (NXT(1) == '!') &&
        (NXT(2) == 'E') && (NXT(3) == 'N') &&
        (NXT(4) == 'T') && (NXT(5) == 'I') &&
        (NXT(6) == 'T') && (NXT(7) == 'Y') &&
        (IS_BLANK(NXT(8)))) {
	SKIP(8);
        SKIP_BLANKS;

	if (CUR == '%') {
	    NEXT;
	    SKIP_BLANKS;
	    isParameter = 1;
	}

        name = GXxmlParseName(ctxt);
	if (name == NULL) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParseEntityDecl: no name\n");
            return;
	}
        SKIP_BLANKS;

	/*
	 * TODO handle the various case of definitions...
	 */
	if (isParameter) {
	    if ((CUR == '"') || (CUR == '\''))
	        value = GXxmlParseEntityValue(ctxt);
		if (value) {
		    GXxmlAddDocEntity(ctxt->doc, name,
		                    XML_INTERNAL_PARAMETER_ENTITY,
				    NULL, NULL, value);
		}
	    else {
	        URI = GXxmlParseExternalID(ctxt, &literal);
		if (URI) {
		    GXxmlAddDocEntity(ctxt->doc, name,
		                    XML_EXTERNAL_PARAMETER_ENTITY,
				    literal, URI, NULL);
		}
	    }
	} else {
	    if ((CUR == '"') || (CUR == '\'')) {
	        value = GXxmlParseEntityValue(ctxt);
		GXxmlAddDocEntity(ctxt->doc, name,
				XML_INTERNAL_GENERAL_ENTITY,
				NULL, NULL, value);
	    } else {
	        URI = GXxmlParseExternalID(ctxt, &literal);
		SKIP_BLANKS;
		if ((CUR == 'N') && (NXT(1) == 'D') &&
		    (NXT(2) == 'A') && (NXT(3) == 'T') &&
		    (NXT(4) == 'A')) {
		    SKIP(5);
		    SKIP_BLANKS;
		    ndata = GXxmlParseName(ctxt);
		    GXxmlAddDocEntity(ctxt->doc, name,
				    XML_EXTERNAL_GENERAL_UNPARSED_ENTITY,
				    literal, URI, ndata);
		} else {
		    GXxmlAddDocEntity(ctxt->doc, name,
				    XML_EXTERNAL_GENERAL_PARSED_ENTITY,
				    literal, URI, NULL);
		}
	    }
	}
	SKIP_BLANKS;
	if (CUR != '>') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, 
	            "GXxmlParseEntityDecl: entity %s not terminated\n", name);
	} else
	    NEXT;
	if (name != NULL) GxCore_Free(name);
	if (value != NULL) GxCore_Free(value);
	if (URI != NULL) GxCore_Free(URI);
	if (literal != NULL) GxCore_Free(literal);
	if (ndata != NULL) GxCore_Free(ndata);
    }
}

/**
 * GXxmlParseEnumeratedType:
 * @ctxt:  an XML parser context
 * @name:  ???
 * @:  
 *
 * parse and Enumerated attribute type.
 *
 * [57] EnumeratedType ::= NotationType | Enumeration
 *
 * [58] NotationType ::= 'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'
 *
 * [59] Enumeration ::= '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')'
 *
 * TODO: not implemented !!!
 */

void
GXxmlParseEnumeratedType(GXxmlParserCtxtPtr ctxt, CHAR *name) {
    /*
     * TODO !!!
     */
    while ((IS_CHAR(CUR)) && (CUR != '>'))
        NEXT;
}

/**
 * GXxmlParseAttributeType:
 * @ctxt:  an XML parser context
 * @name:  ???
 *
 * : parse the Attribute list def for an element
 *
 * [54] AttType ::= StringType | TokenizedType | EnumeratedType
 *
 * [55] StringType ::= 'CDATA'
 *
 * [56] TokenizedType ::= 'ID' | 'IDREF' | 'IDREFS' | 'ENTITY' |
 *                        'ENTITIES' | 'NMTOKEN' | 'NMTOKENS'
 *
 * TODO: not implemented !!!
 */
void
GXxmlParseAttributeType(GXxmlParserCtxtPtr ctxt, CHAR *name) {
    /* TODO !!! */
    if ((CUR == 'C') && (NXT(1) == 'D') &&
        (NXT(2) == 'A') && (NXT(3) == 'T') &&
        (NXT(4) == 'A')) {
	SKIP(5);
     } else if ((CUR == 'I') && (NXT(1) == 'D')) {
        SKIP(2);
     } else if ((CUR == 'I') && (NXT(1) == 'D') &&
        (NXT(2) == 'R') && (NXT(3) == 'E') &&
        (NXT(4) == 'F')) {
	SKIP(5);
     } else if ((CUR == 'I') && (NXT(1) == 'D') &&
        (NXT(2) == 'R') && (NXT(3) == 'E') &&
        (NXT(4) == 'F') && (NXT(5) == 'S')) {
	SKIP(6);
     } else if ((CUR == 'E') && (NXT(1) == 'N') &&
        (NXT(2) == 'T') && (NXT(3) == 'I') &&
        (NXT(4) == 'T') && (NXT(5) == 'Y')) {
	SKIP(6);
     } else if ((CUR == 'E') && (NXT(1) == 'N') &&
        (NXT(2) == 'T') && (NXT(3) == 'I') &&
        (NXT(4) == 'T') && (NXT(5) == 'I') &&
        (NXT(6) == 'E') && (NXT(7) == 'S')) {
	SKIP(8);
     } else if ((CUR == 'N') && (NXT(1) == 'M') &&
        (NXT(2) == 'T') && (NXT(3) == 'O') &&
        (NXT(4) == 'K') && (NXT(5) == 'E') &&
        (NXT(6) == 'N')) {
	SKIP(7);
     } else if ((CUR == 'N') && (NXT(1) == 'M') &&
        (NXT(2) == 'T') && (NXT(3) == 'O') &&
        (NXT(4) == 'K') && (NXT(5) == 'E') &&
        (NXT(6) == 'N') && (NXT(7) == 'S')) {
     } else {
        GXxmlParseEnumeratedType(ctxt, name);
     }
}

/**
 * GXxmlParseAttributeListDecl:
 * @ctxt:  an XML parser context
 *
 * : parse the Attribute list def for an element
 *
 * [52] AttlistDecl ::= '<!ATTLIST' S Name AttDef* S? '>'
 *
 * [53] AttDef ::= S Name S AttType S DefaultDecl
 *
 * TODO: not implemented !!!
 */
void
GXxmlParseAttributeListDecl(GXxmlParserCtxtPtr ctxt) {
    CHAR *name;

    /* TODO !!! */
    if ((CUR == '<') && (NXT(1) == '!') &&
        (NXT(2) == 'A') && (NXT(3) == 'T') &&
        (NXT(4) == 'T') && (NXT(5) == 'L') &&
        (NXT(6) == 'I') && (NXT(7) == 'S') &&
        (NXT(8) == 'T') && (IS_BLANK(NXT(9)))) {
	SKIP(9);
        SKIP_BLANKS;
        name = GXxmlParseName(ctxt);
	if (name == NULL) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, 
	        "GXxmlParseAttributeListDecl: no name for Element\n");
	    return;
	}
	SKIP_BLANKS;
	while (CUR != '>') {
	    const CHAR *check = CUR_PTR;

	    GXxmlParseAttributeType(ctxt, name);
	    SKIP_BLANKS;
	    if (check == CUR_PTR) {
	        if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, 
		    "GXxmlParseAttributeListDecl: detected error\n");
		break;
	    }
	}
	if (CUR == '>')
	    NEXT;

	GxCore_Free(name);
    }
}

/**
 * GXxmlParseElementContentDecl:
 * @ctxt:  an XML parser context
 * @name:  ???
 *
 * parse the declaration for an Element content
 * either Mixed or Children, the cases EMPTY and ANY being handled
 * 
 * [47] children ::= (choice | seq) ('?' | '*' | '+')?
 *
 * [48] cp ::= (Name | choice | seq) ('?' | '*' | '+')?
 *
 * [49] choice ::= '(' S? cp ( S? '|' S? cp )* S? ')'
 *
 * [50] seq ::= '(' S? cp ( S? ',' S? cp )* S? ')'
 *
 * or
 *
 * [51] Mixed ::= '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*' |
 *                '(' S? '#PCDATA' S? ')'
 *
 * TODO: not implemented !!!
 */

void
GXxmlParseElementContentDecl(GXxmlParserCtxtPtr ctxt, CHAR *name) {
    /*
     * TODO This has to be parsed correctly, currently we just skip until
     *      we reach the first '>'.
     * !!!
     */
    while ((IS_CHAR(CUR)) && (CUR != '>'))
        NEXT;
}

/**
 * GXxmlParseElementDecl:
 * @ctxt:  an XML parser context
 *
 * parse an Element declaration.
 *
 * [45] elementdecl ::= '<!ELEMENT' S Name S contentspec S? '>'
 *
 * [46] contentspec ::= 'EMPTY' | 'ANY' | Mixed | children
 *
 * TODO There is a check [ VC: Unique Element Type Declaration ]
 */
void
GXxmlParseElementDecl(GXxmlParserCtxtPtr ctxt) {
    CHAR *name;

    if ((CUR == '<') && (NXT(1) == '!') &&
        (NXT(2) == 'E') && (NXT(3) == 'L') &&
        (NXT(4) == 'E') && (NXT(5) == 'M') &&
        (NXT(6) == 'E') && (NXT(7) == 'N') &&
        (NXT(8) == 'T') && (IS_BLANK(NXT(9)))) {
	SKIP(9);
        SKIP_BLANKS;
        name = GXxmlParseName(ctxt);
	if (name == NULL) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParseElementDecl: no name for Element\n");
	    return;
	}
        SKIP_BLANKS;
	if ((CUR == 'E') && (NXT(1) == 'M') &&
	    (NXT(2) == 'P') && (NXT(3) == 'T') &&
	    (NXT(4) == 'Y')) {
	    SKIP(5);
	    /*
	     * Element must always be empty.
	     */
	} else if ((CUR == 'A') && (NXT(1) == 'N') &&
	           (NXT(2) == 'Y')) {
	    SKIP(3);
	    /*
	     * Element is a generic container.
	     */
	} else {
	    GXxmlParseElementContentDecl(ctxt, name);
	}
	SKIP_BLANKS;
	if (CUR != '>') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, 
	          "GXxmlParseElementDecl: expected '>' at the end\n");
	} else
	    NEXT;
    }
}

/**
 * GXxmlParseMarkupDecl:
 * @ctxt:  an XML parser context
 * 
 * parse Markup declarations
 *
 * [29] markupdecl ::= elementdecl | AttlistDecl | EntityDecl |
 *                     NotationDecl | PI | Comment
 *
 * TODO There is a check [ VC: Proper Declaration/PE Nesting ]
 */
void
GXxmlParseMarkupDecl(GXxmlParserCtxtPtr ctxt) {
    GXxmlParseElementDecl(ctxt);
    GXxmlParseAttributeListDecl(ctxt);
    GXxmlParseEntityDecl(ctxt);
    GXxmlParseNotationDecl(ctxt);
    GXxmlParsePI(ctxt);
    GXxmlParseComment(ctxt, 0);
}

/**
 * GXxmlParseCharRef:
 * @ctxt:  an XML parser context
 *
 * parse Reference declarations
 *
 * [66] CharRef ::= '&#' [0-9]+ ';' |
 *                  '&#x' [0-9a-fA-F]+ ';'
 * return values: the value parsed
 */
CHAR *
GXxmlParseCharRef(GXxmlParserCtxtPtr ctxt) {
    unsigned int val = 0;
    CHAR buf[4];

    if ((CUR == '&') && (NXT(1) == '#') &&
        (NXT(2) == 'x')) {
	SKIP(3);
	while (CUR != ';') {
	    if ((CUR >= '0') && (CUR <= '9')) 
	        val = val * 16 + (CUR - '0');
	    else if ((CUR >= 'a') && (CUR <= 'f'))
	        val = val * 16 + (CUR - 'a') + 10;
	    else if ((CUR >= 'A') && (CUR <= 'F'))
	        val = val * 16 + (CUR - 'A') + 10;
	    else {
	        if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, 
		         "GXxmlParseCharRef: invalid hexa value\n");
		val = 0;
		break;
	    }
	    NEXT;
	}
	if (CUR == ';')
	    NEXT;
    } else if  ((CUR == '&') && (NXT(1) == '#')) {
	SKIP(2);
	while (CUR != ';') {
	    if ((CUR >= '0') && (CUR <= '9')) 
	        val = val * 10 + (CUR - '0');
	    else {
	        if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, 
		         "GXxmlParseCharRef: invalid decimal value\n");
		val = 0;
		break;
	    }
	    NEXT;
	}
	if (CUR == ';')
	    NEXT;
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "GXxmlParseCharRef: invalid value\n");
    }
    /*
     * Check the value IS_CHAR ...
     */
    if (IS_CHAR(val)) {
		if(val>0xFFFF)
		{
			buf[0] = 0xE0 | ((val >> 12) & 0x0F);
			buf[1] = 0x80 | ((val >> 6)  & 0x3F);
			buf[2] = 0x80 | (val & 0x3F);
			buf[3] = 0;
		}
		else if(val>0xFF)
		{
			buf[0] = 0xC0 | ((val >> 6) & 0x1F);
			buf[1] = 0x80 | (val & 0x3F);
			buf[2] = 0;
		}
		else
		{
			buf[0] = (char)(val & 0x7F);
			buf[1] = 0;
		}
		return(GXxmlStrndup(buf, strlen((char *)buf)));
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "GXxmlParseCharRef: invalid CHAR value %d\n", val);
    }
    return(NULL);
}

/**
 * GXxmlParseEntityRef:
 * @ctxt:  an XML parser context
 *
 * parse ENTITY references declarations
 *
 * [68] EntityRef ::= '&' Name ';'
 * return values: the entity ref string or NULL if directly as input stream.
 */
CHAR *
GXxmlParseEntityRef(GXxmlParserCtxtPtr ctxt) {
    CHAR *ret = NULL;
    const CHAR *q;
    CHAR *name;
    GXxmlParserInputPtr input = NULL;

    q = CUR_PTR;
    if (CUR == '&') {
        NEXT;
        name = GXxmlParseName(ctxt);
	if (name == NULL) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParseEntityRef: no name\n");
	} else {
	    if (CUR == ';') {
	        NEXT;
		/*
		 * We parsed the entity reference correctly, call SAX
		 * interface for the proper behaviour:
		 *   - get a new input stream
		 *   - or keep the reference inline
		 */
		if (ctxt->sax)
		    input = ctxt->sax->resolveEntity(ctxt, NULL, name);
		if (input != NULL)
		    GXxmlPushInput(ctxt, input);
		else {
		    ret = GXxmlStrndup(q, CUR_PTR - q);
		}
	    } else {
		char cst[2] = { '&', 0 };

		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "GXxmlParseEntityRef: expecting ';'\n");
		ret = GXxmlStrndup((CHAR*)cst, 1);
		ret = GXxmlStrcat(ret, name);
	    }
	    GxCore_Free(name);
	}
    }
    return(ret);
}

/**
 * GXxmlParseReference:
 * @ctxt:  an XML parser context
 * 
 * parse Reference declarations
 *
 * [67] Reference ::= EntityRef | CharRef
 * return values: the entity string or NULL if handled directly by pushing
 *      the entity value as the input.
 */
CHAR *
GXxmlParseReference(GXxmlParserCtxtPtr ctxt) {
    if ((CUR == '&') && (NXT(1) == '#')) {
        return(GXxmlParseCharRef(ctxt));
    } else if (CUR == '&') {
        return(GXxmlParseEntityRef(ctxt));
    }
    return(NULL);
}

/**
 * GXxmlParsePEReference:
 * @ctxt:  an XML parser context
 *
 * parse PEReference declarations
 *
 * [69] PEReference ::= '%' Name ';'
 * return values: the entity content or NULL if handled directly.
 */
CHAR *
GXxmlParsePEReference(GXxmlParserCtxtPtr ctxt) {
    CHAR *ret = NULL;
    CHAR *name;
    GXxmlEntityPtr entity;
    GXxmlParserInputPtr input;

    if (CUR == '%') {
        NEXT;
        name = GXxmlParseName(ctxt);
	if (name == NULL) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParsePEReference: no name\n");
	} else {
	    if (CUR == ';') {
	        NEXT;
		entity = GXxmlGetDtdEntity(ctxt->doc, name);
		if (entity == NULL) {
		    if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
		        ctxt->sax->warning(ctxt,
		         "GXxmlParsePEReference: %%%s; not found\n");
		} else {
		    input = GXxmlNewEntityInputStream(ctxt, entity);
		    GXxmlPushInput(ctxt, input);
		}
	    } else {
		char cst[2] = { '%', 0 };

		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "GXxmlParsePEReference: expecting ';'\n");
		ret = GXxmlStrndup((CHAR *)cst, 1);
		ret = GXxmlStrcat(ret, name);
	    }
	    GxCore_Free(name);
	}
    }
    return(ret);
}

/**
 * GXxmlParseDocTypeDecl :
 * @ctxt:  an XML parser context
 *
 * parse a DOCTYPE declaration
 *
 * [28] doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S? 
 *                      ('[' (markupdecl | PEReference | S)* ']' S?)? '>'
 */

void
GXxmlParseDocTypeDecl(GXxmlParserCtxtPtr ctxt) {
    CHAR *name;
    CHAR *ExternalID = NULL;
    CHAR *URI = NULL;

    /*
     * We know that '<!DOCTYPE' has been detected.
     */
    SKIP(9);

    SKIP_BLANKS;

    /*
     * Parse the DOCTYPE name.
     */
    name = GXxmlParseName(ctxt);
    if (name == NULL) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "GXxmlParseDocTypeDecl : no DOCTYPE name !\n");
    }

    SKIP_BLANKS;

    /*
     * Check for SystemID and ExternalID
     */
    URI = GXxmlParseExternalID(ctxt, &ExternalID);
    SKIP_BLANKS;

    GXxmlNewDtd(ctxt->doc, name, ExternalID, URI);

    /*
     * Is there any DTD definition ?
     */
    if (CUR == '[') {
        NEXT;
	/*
	 * Parse the succession of Markup declarations and 
	 * PEReferences.
	 * Subsequence (markupdecl | PEReference | S)*
	 */
	while (CUR != ']') {
	    const CHAR *check = CUR_PTR;

	    SKIP_BLANKS;
	    GXxmlParseMarkupDecl(ctxt);
	    GXxmlParsePEReference(ctxt);

	    if (CUR_PTR == check) {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, 
		 "GXxmlParseDocTypeDecl: error detected in Markup declaration\n");
		break;
	    }
	}
	if (CUR == ']') NEXT;
    }

    /*
     * We should be at the end of the DOCTYPE declaration.
     */
    if (CUR != '>') {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "DOCTYPE unproperly terminated\n");
        /* We shouldn't try to resynchronize ... */
    }
    NEXT;

    /*
     * Cleanup, since we don't use all those identifiers
     * TODO : the DOCTYPE if available should be stored !
     */
    if (URI != NULL) GxCore_Free(URI);
    if (ExternalID != NULL) GxCore_Free(ExternalID);
    if (name != NULL) GxCore_Free(name);
}

/**
 * GXxmlParseAttribute:
 * @ctxt:  an XML parser context
 * @node:  the node carrying the attribute
 *
 * parse an attribute
 *
 * [41] Attribute ::= Name Eq AttValue
 *
 * [25] Eq ::= S? '=' S?
 *
 * With namespace:
 *
 * [NS 11] Attribute ::= QName Eq AttValue
 *
 * Also the case QName == GXxmlns:??? is handled independently as a namespace
 * definition.
 */

GXxmlAttrPtr GXxmlParseAttribute(GXxmlParserCtxtPtr ctxt, GXxmlNodePtr node) {
    CHAR *name;
    CHAR *ns;
    CHAR *value = NULL;
    GXxmlAttrPtr ret;

    name = GXxmlNamespaceParseQName(ctxt, &ns);
    if (name == NULL) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "error parsing attribute name\n");
        return(NULL);
    }

    /*
     * read the value
     */
    SKIP_BLANKS;
    if (CUR == '=') {
        NEXT;
	SKIP_BLANKS;
	value = GXxmlParseAttValue(ctxt);
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "Specification mandate value for attribute %s\n",
	               name);
    }

    /*
     * Check whether it's a namespace definition
     */
    if ((ns == NULL) &&
        (name[0] == 'x') && (name[1] == 'm') && (name[2] == 'l') &&
        (name[3] == 'n') && (name[4] == 's') && (name[5] == 0)) {
	/* a default namespace definition */
	GXxmlNewNs(node, value, NULL);
	if (name != NULL) 
	    GxCore_Free(name);
	if (value != NULL)
	    GxCore_Free(value);
	return(NULL);
    }
    if ((ns != NULL) && (ns[0] == 'x') && (ns[1] == 'm') && (ns[2] == 'l') &&
        (ns[3] == 'n') && (ns[4] == 's') && (ns[5] == 0)) {
	/* a standard namespace definition */
	GXxmlNewNs(node, value, name);
	GxCore_Free(ns);
	if (name != NULL) 
	    GxCore_Free(name);
	if (value != NULL)
	    GxCore_Free(value);
	return(NULL);
    }

    ret = GXxmlNewProp(ctxt->node, name, NULL);
    if (ret != NULL)
        ret->val = GXxmlStringGetNodeList(ctxt->doc, value);

    if (ns != NULL)
      GxCore_Free(ns);
    if (value != NULL)
	GxCore_Free(value);
    GxCore_Free(name);
    return(ret);
}

/**
 * GXxmlParseStartTag:
 * @ctxt:  an XML parser context
 * 
 * parse a start of tag either for rule element or
 * EmptyElement. In both case we don't parse the tag closing chars.
 *
 * [40] STag ::= '<' Name (S Attribute)* S? '>'
 *
 * [44] EmptyElemTag ::= '<' Name (S Attribute)* S? '/>'
 *
 * With namespace:
 *
 * [NS 8] STag ::= '<' QName (S Attribute)* S? '>'
 *
 * [NS 10] EmptyElement ::= '<' QName (S Attribute)* S? '/>'
 *
 * return values: the XML new node or NULL.
 */

GXxmlNodePtr GXxmlParseStartTag(GXxmlParserCtxtPtr ctxt) {
    CHAR *namespace, *name;
    GXxmlNsPtr ns = NULL;
    GXxmlNodePtr ret = NULL;
    GXxmlNodePtr parent = ctxt->node;

    if (CUR != '<') return(NULL);
    NEXT;

    name = GXxmlNamespaceParseQName(ctxt, &namespace);
    if (name == NULL) return(NULL);

    /*
     * Note : the namespace resolution is deferred until the end of the
     *        attributes parsing, since local namespace can be defined as
     *        an attribute at this level.
     */
    ret = GXxmlNewDocNode(ctxt->doc, ns, name, NULL);
    if (ret == NULL) {
	if (namespace != NULL)
	    GxCore_Free(namespace);
	GxCore_Free(name);
        return(NULL);
    }

    /*
     * We are parsing a new node.
     */
    GXnodePush(ctxt, ret);

    /*
     * Now parse the attributes, it ends up with the ending
     *
     * (S Attribute)* S?
     */
    SKIP_BLANKS;
    while ((IS_CHAR(CUR)) &&
           (CUR != '>') && 
	   ((CUR != '/') || (NXT(1) != '>'))) {
	const CHAR *q = CUR_PTR;

	GXxmlParseAttribute(ctxt, ret);
	SKIP_BLANKS;

        if (q == CUR_PTR) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, 
	         "GXxmlParseStartTag: problem parsing attributes\n");
	    break;
	}
    }

    /*
     * Search the namespace
     */
    ns = GXxmlSearchNs(ctxt->doc, ret, namespace);
    if (ns == NULL) /* ret still doesn't have a parent yet ! */
	ns = GXxmlSearchNs(ctxt->doc, parent, namespace);
    GXxmlSetNs(ret, ns);
    if (namespace != NULL)
	GxCore_Free(namespace);

    /*
     * SAX: Start of Element !
     */
    if (ctxt->sax != NULL)
        ctxt->sax->startElement(ctxt, name);
    GxCore_Free(name);

    /*
     * Link the child element
     */
    if (ctxt->nodeNr < 2) return(ret);
    parent = ctxt->nodeTab[ctxt->nodeNr - 2];
    if (parent != NULL)
	GXxmlAddChild(parent, ctxt->node);

    return(ret);
}

/**
 * GXxmlParseEndTag:
 * @ctxt:  an XML parser context
 * @nsPtr:  the current node namespace definition
 * @tagPtr:  CHAR** receive the tag value
 *
 * parse an end of tag
 *
 * [42] ETag ::= '</' Name S? '>'
 *
 * With namespace
 *
 * [9] ETag ::= '</' QName S? '>'
 *    
 * return values: tagPtr receive the tag name just read
 */

void
GXxmlParseEndTag(GXxmlParserCtxtPtr ctxt, GXxmlNsPtr *nsPtr, CHAR **tagPtr) {
    CHAR *namespace, *name;
    GXxmlNsPtr ns = NULL;

    *nsPtr = NULL;
    *tagPtr = NULL;

    if ((CUR != '<') || (NXT(1) != '/')) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "GXxmlParseEndTag: '</' not found\n");
	return;
    }
    SKIP(2);

    name = GXxmlNamespaceParseQName(ctxt, &namespace);

    /*
     * Search the namespace
     */
    ns = GXxmlSearchNs(ctxt->doc, ctxt->node, namespace);
    if (namespace != NULL)
	GxCore_Free(namespace);

    *nsPtr = ns;
    *tagPtr = name;

    /*
     * We should definitely be at the ending "S? '>'" part
     */
    SKIP_BLANKS;
    if ((!IS_CHAR(CUR)) || (CUR != '>')) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "End tag : expected '>'\n");
    } else
	NEXT;

    return;
}

/**
 * GXxmlParseCDSect:
 * @ctxt:  an XML parser context
 * 
 * Parse escaped pure raw content.
 *
 * [18] CDSect ::= CDStart CData CDEnd
 *
 * [19] CDStart ::= '<![CDATA['
 *
 * [20] Data ::= (Char* - (Char* ']]>' Char*))
 *
 * [21] CDEnd ::= ']]>'
 */
void
GXxmlParseCDSect(GXxmlParserCtxtPtr ctxt) {
    const CHAR *r, *s, *base;

    if ((CUR == '<') && (NXT(1) == '!') &&
	(NXT(2) == '[') && (NXT(3) == 'C') &&
	(NXT(4) == 'D') && (NXT(5) == 'A') &&
	(NXT(6) == 'T') && (NXT(7) == 'A') &&
	(NXT(8) == '[')) {
	SKIP(9);
    } else
        return;
    base = CUR_PTR;
    if (!IS_CHAR(CUR)) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "CData section not finished\n%.50s\n", base);
        return;
    }
    r = NEXT;
    if (!IS_CHAR(CUR)) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "CData section not finished\n%.50s\n", base);
        return;
    }
    s = NEXT;
    while (IS_CHAR(CUR) &&
           ((*r != ']') || (*s != ']') || (CUR != '>'))) {
        r++;s++;NEXT;
    }
    if (!IS_CHAR(CUR)) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "CData section not finished\n%.50s\n", base);
        return;
    }

    /*
     * Ok the segment [base CUR_PTR] is to be consumed as chars.
     */
    ctxt->node->type = XML_CDATA_SECTION_NODE;
    SKIP(1);
    if (ctxt->sax != NULL) {
	if (areBlanks(ctxt, base, CUR_PTR - base))
	    ctxt->sax->ignorableWhitespace(ctxt, base, 0, CUR_PTR - base);
	else
	    ctxt->sax->characters(ctxt, base, 0, r - base/*CUR_PTR - base*/);
    }
}

/**
 * GXxmlParseContent:
 * @ctxt:  an XML parser context
 *
 * Parse a content:
 *
 * [43] content ::= (element | CharData | Reference | CDSect | PI | Comment)*
 */

void
GXxmlParseContent(GXxmlParserCtxtPtr ctxt) {
    GXxmlNodePtr ret = NULL;

    while ((CUR != '<') || (NXT(1) != '/')) {
	const CHAR *test = CUR_PTR;
        ret = NULL;

	/*
	 * First case : a Processing Instruction.
	 */
	if ((CUR == '<') && (NXT(1) == '?')) {
	    GXxmlParsePI(ctxt);
	}
	/*
	 * Second case : a CDSection
	 */
	else if ((CUR == '<') && (NXT(1) == '!') &&
	    (NXT(2) == '[') && (NXT(3) == 'C') &&
	    (NXT(4) == 'D') && (NXT(5) == 'A') &&
	    (NXT(6) == 'T') && (NXT(7) == 'A') &&
	    (NXT(8) == '[')) {
	    GXxmlParseCDSect(ctxt);
	}
	/*
	 * Third case :  a comment
	 */
	else if ((CUR == '<') && (NXT(1) == '!') &&
		 (NXT(2) == '-') && (NXT(3) == '-')) {
	    ret = GXxmlParseComment(ctxt, 1);
	}
	/*
	 * Fourth case :  a sub-element.
	 */
	else if (CUR == '<') {
	    ret = GXxmlParseElement(ctxt);
	}
	/*
	 * Fifth case : a reference. If if has not been resolved,
	 *    parsing returns it's Name, create the node 
	 */
	else if (CUR == '&') {
	    CHAR *val = GXxmlParseReference(ctxt);
	    if (val != NULL) {
	        if (val[0] != '&') {
		    /*
		     * inline predefined entity.
		     */
                    if (ctxt->sax != NULL)
			ctxt->sax->characters(ctxt, val, 0, GXxmlStrlen(val));
		} else {
		    /*
		     * user defined entity, create a node.
		     */
		    ret = GXxmlNewReference(ctxt->doc, val);
		    GXxmlAddChild(ctxt->node, ret);
		}
		GxCore_Free(val);
	    }
	}
	/*
	 * Last case, text. Note that References are handled directly.
	 */
	else {
	    GXxmlParseCharData(ctxt, 0);
	}

	/*
	 * Pop-up of finished entities.
	 */
	while ((CUR == 0) && (ctxt->inputNr > 1)) GXxmlPopInput(ctxt);

	if (test == CUR_PTR) {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "detected an error in element content\n");
            break;
	}
    }
}

/**
 * GXxmlParseElement:
 * @ctxt:  an XML parser context
 *
 * parse an XML element, this is highly recursive
 *
 * [39] element ::= EmptyElemTag | STag content ETag
 *
 * [41] Attribute ::= Name Eq AttValue
 * return values: the XML new node or NULL
 */


GXxmlNodePtr GXxmlParseElement(GXxmlParserCtxtPtr ctxt) {
    GXxmlNodePtr ret;
    const CHAR *openTag = CUR_PTR;
    GXxmlParserNodeInfo node_info;
    CHAR *endTag;
    GXxmlNsPtr endNs;

    /* Capture start position */
    node_info.begin_pos = CUR_PTR - ctxt->input->base;
    node_info.begin_line = ctxt->input->line;

    ret = GXxmlParseStartTag(ctxt);
    if (ret == NULL) {
        return(NULL);
    }

    /*
     * Check for an Empty Element.
     */
    if ((CUR == '/') && (NXT(1) == '>')) {
        SKIP(2);
	if (ctxt->sax != NULL)
	    ctxt->sax->endElement(ctxt, ret->name);

	/*
	 * end of parsing of this node.
	 */
	GXnodePop(ctxt);

	return(ret);
    }
    if (CUR == '>') NEXT;
    else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "Couldn't find end of Start Tag\n%.30s\n",
	                     openTag);

	/*
	 * end of parsing of this node.
	 */
	GXnodePop(ctxt);

	return(NULL);
    }

    /*
     * Parse the content of the element:
     */
    GXxmlParseContent(ctxt);
    if (!IS_CHAR(CUR)) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt,
	         "Premature end of data in tag %.30s\n", openTag);

	/*
	 * end of parsing of this node.
	 */
	GXnodePop(ctxt);

	return(NULL);
    }

    /*
     * parse the end of tag: '</' should be here.
     */
    GXxmlParseEndTag(ctxt, &endNs, &endTag);

    /*
     * Check that the Name in the ETag is the same as in the STag.
     */
    if (endNs != ret->ns) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, 
	    "Start and End tags don't use the same namespace\n%.30s\n%.30s\n",
	               openTag, endTag);
    }
    if (endTag == NULL ) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "The End tag has no name\n%.30s\n", openTag);
    } else if (GXxmlStrcmp(ret->name, endTag)) {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, 
	    "Start and End tags don't use the same name\n%.30s\n%.30s\n",
	               openTag, endTag);
    }
    /*
     * SAX: End of Tag
     */
    else if (ctxt->sax != NULL)
        ctxt->sax->endElement(ctxt, endTag);

    if (endTag != NULL)
	GxCore_Free(endTag);

    /* Capture end position and add node */
    if ( ret != NULL && ctxt->record_info ) {
      node_info.end_pos = CUR_PTR - ctxt->input->base;
      node_info.end_line = ctxt->input->line;
      node_info.node = ret;
      GXxmlParserAddNodeInfo(ctxt, &node_info);
    }

    /*
     * end of parsing of this node.
     */
    GXnodePop(ctxt);

    return(ret);
}

/**
 * GXxmlParseVersionNum:
 * @ctxt:  an XML parser context
 *
 * parse the XML version value.
 *
 * [26] VersionNum ::= ([a-zA-Z0-9_.:] | '-')+
 * return values: the string giving the XML version number, or NULL
 */
CHAR *
GXxmlParseVersionNum(GXxmlParserCtxtPtr ctxt) {
    const CHAR *q = CUR_PTR;
    CHAR *ret;

    while (IS_CHAR(CUR) &&
           (((CUR >= 'a') && (CUR <= 'z')) ||
            ((CUR >= 'A') && (CUR <= 'Z')) ||
            ((CUR >= '0') && (CUR <= '9')) ||
            (CUR == '_') || (CUR == '.') ||
	    (CUR == ':') || (CUR == '-'))) NEXT;
    ret = GXxmlStrndup(q, CUR_PTR - q);
    return(ret);
}

/**
 * GXxmlParseVersionInfo:
 * @ctxt:  an XML parser context
 * 
 * parse the XML version.
 *
 * [24] VersionInfo ::= S 'version' Eq (' VersionNum ' | " VersionNum ")
 * 
 * [25] Eq ::= S? '=' S?
 *
 * return values: the version string, e.g. "1.0"
 */

CHAR *
GXxmlParseVersionInfo(GXxmlParserCtxtPtr ctxt) {
    CHAR *version = NULL;
    const CHAR *q;

    if ((CUR == 'v') && (NXT(1) == 'e') &&
        (NXT(2) == 'r') && (NXT(3) == 's') &&
	(NXT(4) == 'i') && (NXT(5) == 'o') &&
	(NXT(6) == 'n')) {
	SKIP(7);
	SKIP_BLANKS;
	if (CUR != '=') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParseVersionInfo : expected '='\n");
	    return(NULL);
        }
	NEXT;
	SKIP_BLANKS;
	if (CUR == '"') {
	    NEXT;
	    q = CUR_PTR;
	    version = GXxmlParseVersionNum(ctxt);
	    if (CUR != '"') {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "String not closed\n%.50s\n", q);
	    } else
	        NEXT;
	} else if (CUR == '\''){
	    NEXT;
	    q = CUR_PTR;
	    version = GXxmlParseVersionNum(ctxt);
	    if (CUR != '\'') {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "String not closed\n%.50s\n", q);
	    } else
	        NEXT;
	} else {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParseVersionInfo : expected ' or \"\n");
	}
    }
    return(version);
}

/**
 * GXxmlParseEncName:
 * @ctxt:  an XML parser context
 *
 * parse the XML encoding name
 *
 * [81] EncName ::= [A-Za-z] ([A-Za-z0-9._] | '-')*
 *
 * return values: the encoding name value or NULL
 */
CHAR *
GXxmlParseEncName(GXxmlParserCtxtPtr ctxt) {
    const CHAR *q = CUR_PTR;
    CHAR *ret = NULL;

    if (((CUR >= 'a') && (CUR <= 'z')) ||
        ((CUR >= 'A') && (CUR <= 'Z'))) {
	NEXT;
	while (IS_CHAR(CUR) &&
	       (((CUR >= 'a') && (CUR <= 'z')) ||
		((CUR >= 'A') && (CUR <= 'Z')) ||
		((CUR >= '0') && (CUR <= '9')) ||
		(CUR == '-'))) NEXT;
	ret = GXxmlStrndup(q, CUR_PTR - q);
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "Invalid XML encoding name\n");
    }
    return(ret);
}

/**
 * GXxmlParseEncodingDecl:
 * @ctxt:  an XML parser context
 * 
 * parse the XML encoding declaration
 *
 * [80] EncodingDecl ::= S 'encoding' Eq ('"' EncName '"' |  "'" EncName "'")
 *
 * TODO: this should setup the conversion filters.
 *
 * return values: the encoding value or NULL
 */

CHAR *
GXxmlParseEncodingDecl(GXxmlParserCtxtPtr ctxt) {
    CHAR *encoding = NULL;
    const CHAR *q;

    SKIP_BLANKS;
    if ((CUR == 'e') && (NXT(1) == 'n') &&
        (NXT(2) == 'c') && (NXT(3) == 'o') &&
	(NXT(4) == 'd') && (NXT(5) == 'i') &&
	(NXT(6) == 'n') && (NXT(7) == 'g')) {
	SKIP(8);
	SKIP_BLANKS;
	if (CUR != '=') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParseEncodingDecl : expected '='\n");
	    return(NULL);
        }
	NEXT;
	SKIP_BLANKS;
	if (CUR == '"') {
	    NEXT;
	    q = CUR_PTR;
	    encoding = GXxmlParseEncName(ctxt);
	    if (CUR != '"') {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "String not closed\n%.50s\n", q);
	    } else
	        NEXT;
	} else if (CUR == '\''){
	    NEXT;
	    q = CUR_PTR;
	    encoding = GXxmlParseEncName(ctxt);
	    if (CUR != '\'') {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "String not closed\n%.50s\n", q);
	    } else
	        NEXT;
	} else if (CUR == '"'){
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "GXxmlParseEncodingDecl : expected ' or \"\n");
	}
    }
    return(encoding);
}

/**
 * GXxmlParseSDDecl:
 * @ctxt:  an XML parser context
 *
 * parse the XML standalone declaration
 *
 * [32] SDDecl ::= S 'standalone' Eq
 *                 (("'" ('yes' | 'no') "'") | ('"' ('yes' | 'no')'"')) 
 * return values: 1 if standalone, 0 otherwise
 */

int
GXxmlParseSDDecl(GXxmlParserCtxtPtr ctxt) {
    int standalone = -1;

    SKIP_BLANKS;
    if ((CUR == 's') && (NXT(1) == 't') &&
        (NXT(2) == 'a') && (NXT(3) == 'n') &&
	(NXT(4) == 'd') && (NXT(5) == 'a') &&
	(NXT(6) == 'l') && (NXT(7) == 'o') &&
	(NXT(8) == 'n') && (NXT(9) == 'e')) {
	SKIP(10);
	if (CUR != '=') {
	    if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "XML standalone declaration : expected '='\n");
	    return(standalone);
        }
	NEXT;
	SKIP_BLANKS;
        if (CUR == '\''){
	    NEXT;
	    if ((CUR == 'n') && (NXT(1) == 'o')) {
	        standalone = 0;
                SKIP(2);
	    } else if ((CUR == 'y') && (NXT(1) == 'e') &&
	               (NXT(2) == 's')) {
	        standalone = 1;
		SKIP(3);
            } else {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "standalone accepts only 'yes' or 'no'\n");
	    }
	    if (CUR != '\'') {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "String not closed\n");
	    } else
	        NEXT;
	} else if (CUR == '"'){
	    NEXT;
	    if ((CUR == 'n') && (NXT(1) == 'o')) {
	        standalone = 0;
		SKIP(2);
	    } else if ((CUR == 'y') && (NXT(1) == 'e') &&
	               (NXT(2) == 's')) {
	        standalone = 1;
                SKIP(3);
            } else {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "standalone accepts only 'yes' or 'no'\n");
	    }
	    if (CUR != '"') {
		if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
		    ctxt->sax->error(ctxt, "String not closed\n");
	    } else
	        NEXT;
	} else {
            if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	        ctxt->sax->error(ctxt, "Standalone value not found\n");
        }
    }
    return(standalone);
}

/**
 * GXxmlParseXMLDecl:
 * @ctxt:  an XML parser context
 * 
 * parse an XML declaration header
 *
 * [23] XMLDecl ::= '<?GXxml' VersionInfo EncodingDecl? SDDecl? S? '?>'
 */

void
GXxmlParseXMLDecl(GXxmlParserCtxtPtr ctxt) {
    CHAR *version;

    /*
     * We know that '<?GXxml' is here.
     */
    SKIP(5);

    SKIP_BLANKS;

    /*
     * We should have the VersionInfo here.
     */
    version = GXxmlParseVersionInfo(ctxt);
    if (version == NULL)
	version = GXxmlCharStrdup(XML_DEFAULT_VERSION);
    ctxt->doc = GXxmlNewDoc(version);
    GxCore_Free(version);

    /*
     * We may have the encoding declaration
     */
    ctxt->doc->encoding = GXxmlParseEncodingDecl(ctxt);

    /*
     * We may have the standalone status.
     */
    ctxt->doc->standalone = GXxmlParseSDDecl(ctxt);

    SKIP_BLANKS;
    if ((CUR == '?') && (NXT(1) == '>')) {
        SKIP(2);
    } else if (CUR == '>') {
        /* Deprecated old WD ... */
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "XML declaration must end-up with '?>'\n");
	NEXT;
    } else {
	if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "parsing XML declaration: '?>' expected\n");
	MOVETO_ENDTAG(CUR_PTR);
	NEXT;
    }
}

/**
 * GXxmlParseMisc:
 * @ctxt:  an XML parser context
 * 
 * parse an XML Misc* optionnal field.
 *
 * [27] Misc ::= Comment | PI |  S
 */

void
GXxmlParseMisc(GXxmlParserCtxtPtr ctxt) {
    while (((CUR == '<') && (NXT(1) == '?')) ||
           ((CUR == '<') && (NXT(1) == '!') &&
	    (NXT(2) == '-') && (NXT(3) == '-')) ||
           IS_BLANK(CUR)) {
        if ((CUR == '<') && (NXT(1) == '?')) {
	    GXxmlParsePI(ctxt);
	} else if (IS_BLANK(CUR)) {
	    NEXT;
	} else
	    GXxmlParseComment(ctxt, 0);
    }
}

/**
 * GXxmlParseDocument :
 * @ctxt:  an XML parser context
 * 
 * parse an XML document (and build a tree if using the standard SAX
 * interface).
 *
 * [1] document ::= prolog element Misc*
 *
 * [22] prolog ::= XMLDecl? Misc* (doctypedecl Misc*)?
 *
 * return values: 0, -1 in case of error. the parser context is augmented
 *                as a result of the parsing.
 */

int
GXxmlParseDocument(GXxmlParserCtxtPtr ctxt) {
    GXxmlDefaultSAXHandlerInit();

    /*
     * SAX: beginning of the document processing.
     */
    if (ctxt->sax) 
        ctxt->sax->setDocumentLocator(ctxt, &GXxmlDefaultSAXLocator);
    if (ctxt->sax)
        ctxt->sax->startDocument(ctxt);

    /*
     * We should check for encoding here and plug-in some
     * conversion code TODO !!!!
     */

    /*
     * Wipe out everything which is before the first '<'
     */
    SKIP_BLANKS;

    /*
     * Check for the XMLDecl in the Prolog.
     */
    if ((CUR == '<') && (NXT(1) == '?') &&
        (NXT(2) == 'x') && (NXT(3) == 'm') &&
	(NXT(4) == 'l')) {
	GXxmlParseXMLDecl(ctxt);
	/* SKIP_EOL(cur); */
	SKIP_BLANKS;
    } else if ((CUR == '<') && (NXT(1) == '?') &&
        (NXT(2) == 'X') && (NXT(3) == 'M') &&
	(NXT(4) == 'L')) {
	/*
	 * The first drafts were using <?XML and the final W3C REC
	 * now use <?GXxml ...
	 */
	GXxmlParseXMLDecl(ctxt);
	/* SKIP_EOL(cur); */
	SKIP_BLANKS;
    } else {
	CHAR *version;

	version = GXxmlCharStrdup(XML_DEFAULT_VERSION);
	ctxt->doc = GXxmlNewDoc(version);
	GxCore_Free(version);
    }

    /*
     * The Misc part of the Prolog
     */
    GXxmlParseMisc(ctxt);

    /*
     * Then possibly doc type declaration(s) and more Misc
     * (doctypedecl Misc*)?
     */
    if ((CUR == '<') && (NXT(1) == '!') &&
	(NXT(2) == 'D') && (NXT(3) == 'O') &&
	(NXT(4) == 'C') && (NXT(5) == 'T') &&
	(NXT(6) == 'Y') && (NXT(7) == 'P') &&
	(NXT(8) == 'E')) {
	GXxmlParseDocTypeDecl(ctxt);
	GXxmlParseMisc(ctxt);
    }

    /*
     * Time to start parsing the tree itself
     */
    ctxt->doc->root = GXxmlParseElement(ctxt);

    /*
     * The Misc part at the end
     */
    GXxmlParseMisc(ctxt);

    /*
     * SAX: end of the document processing.
     */
    if (ctxt->sax) 
        ctxt->sax->endDocument(ctxt);
    return(0);
}

/**
 * GXxmlSAXParseDoc :
 * @sax:  the SAX handler block
 * @cur:  a pointer to an array of CHAR
 *
 * parse an XML in-memory document and build a tree.
 * It use the given SAX function block to handle the parsing callback.
 * If sax is NULL, fallback to the default DOM tree building routines.
 * 
 * return values: the resulting document tree
 */

GXxmlDocPtr GXxmlSAXParseDoc(GXxmlSAXHandlerPtr sax, CHAR *cur) {
    GXxmlDocPtr ret;
    GXxmlParserCtxtPtr ctxt;
    GXxmlParserInputPtr input;

    if (cur == NULL) return(NULL);

    ctxt = (GXxmlParserCtxtPtr) GxCore_Malloc(sizeof(GXxmlParserCtxt));
    if (ctxt == NULL) {
        perror("GxCore_Malloc");
	return(NULL);
    }
    GXxmlInitParserCtxt(ctxt);
    if (sax != NULL) ctxt->sax = sax;
    input = (GXxmlParserInputPtr) GxCore_Malloc(sizeof(GXxmlParserInput));
    if (input == NULL) {
        perror("GxCore_Malloc");
	GxCore_Free(ctxt);
	return(NULL);
    }

    input->filename = NULL;
    input->line = 1;
    input->col = 1;
    input->base = cur;
    input->cur = cur;

    GXinputPush(ctxt, input);


    GXxmlParseDocument(ctxt);
    ret = ctxt->doc;
    GxCore_Free(ctxt->nodeTab);
    GxCore_Free(ctxt->inputTab);
    if (input->filename != NULL)
	GxCore_Free((char *)input->filename);
    GxCore_Free(input);
    GxCore_Free(ctxt);
    
    return(ret);
}

/**
 * GXxmlParseDoc :
 * @cur:  a pointer to an array of CHAR
 *
 * parse an XML in-memory document and build a tree.
 * 
 * return values: the resulting document tree
 */

GXxmlDocPtr GXxmlParseDoc(CHAR *cur) {
    return(GXxmlSAXParseDoc(NULL, cur));
}

/**
 * GXxmlSAXParseFile :
 * @sax:  the SAX handler block
 * @filename:  the filename
 *
 * parse an XML file and build a tree. Automatic support for ZLIB/Compress
 * compressed document is provided by default if found at compile-time.
 * It use the given SAX function block to handle the parsing callback.
 * If sax is NULL, fallback to the default DOM tree building routines.
 *
 * return values: the resulting document tree
 */

GXxmlDocPtr GXxmlSAXParseFile(GXxmlSAXHandlerPtr sax, const char *filename) {
    GXxmlDocPtr ret;
#ifdef HAVE_ZLIB_H
    gzFile input;
#else
    int input;
#endif
    int res;
    int len;
    struct stat buf;
    char *buffer;
    GXxmlParserCtxtPtr ctxt;
    GXxmlParserInputPtr inputStream;

    res = stat(filename, &buf);
    if (res < 0) return(NULL);

#ifdef HAVE_ZLIB_H
    len = (buf.st_size * 8) + 1000;
retry_bigger:
    buffer = GxCore_Malloc(len);
#else
    len = buf.st_size + 100;
    buffer = GxCore_Malloc(len);
#endif
    if (buffer == NULL) {
	perror("GxCore_Malloc");
        return(NULL);
    }

    memset(buffer, 0, len);
#ifdef HAVE_ZLIB_H
    input = gzopen (filename, "r");
    if (input == NULL) {
        fprintf (stderr, "Cannot read file %s :\n", filename);
	perror ("gzopen failed");
	return(NULL);
    }
#else
    input = open (filename, O_RDONLY);
    if (input < 0) {
        fprintf (stderr, "Cannot read file %s :\n", filename);
	perror ("open failed");
	return(NULL);
    }
#endif
#ifdef HAVE_ZLIB_H
    res = gzread(input, buffer, len);
#else
    res = read(input, buffer, buf.st_size);
#endif
    if (res < 0) {
        fprintf (stderr, "Cannot read file %s :\n", filename);
#ifdef HAVE_ZLIB_H
	perror ("gzread failed");
#else
	perror ("read failed");
#endif
	return(NULL);
    }
#ifdef HAVE_ZLIB_H
    gzclose(input);
    if (res >= len) {
        GxCore_Free(buffer);
	len *= 2;
	goto retry_bigger;
    }
    buf.st_size = res;
#else
    close(input);
#endif

    buffer[buf.st_size] = '\0';

    ctxt = (GXxmlParserCtxtPtr) GxCore_Malloc(sizeof(GXxmlParserCtxt));
    if (ctxt == NULL) {
        perror("GxCore_Malloc");
	return(NULL);
    }
    GXxmlInitParserCtxt(ctxt);
    if (sax != NULL) ctxt->sax = sax;
    inputStream = (GXxmlParserInputPtr) GxCore_Malloc(sizeof(GXxmlParserInput));
    if (inputStream == NULL) {
        perror("GxCore_Malloc");
	GxCore_Free(ctxt);
	return(NULL);
    }

    inputStream->filename = GxCore_Strdup(filename);
    inputStream->line = 1;
    inputStream->col = 1;

    /*
     * TODO : plug some encoding conversion routines here. !!!
     */
    inputStream->base = (CHAR *)buffer;
    inputStream->cur = (CHAR *)buffer;

    GXinputPush(ctxt, inputStream);

    GXxmlParseDocument(ctxt);

    ret = ctxt->doc;
    GxCore_Free(buffer);
    GxCore_Free(ctxt->nodeTab);
    GxCore_Free(ctxt->inputTab);
    if (inputStream->filename != NULL)
	GxCore_Free((char *)inputStream->filename);
    GxCore_Free(inputStream);
    GxCore_Free(ctxt);
    
    return(ret);
}

/**
 * GXxmlParseFile :
 * @filename:  the filename
 *
 * parse an XML file and build a tree. Automatic support for ZLIB/Compress
 * compressed document is provided by default if found at compile-time.
 *
 * return values: the resulting document tree
 */

GXxmlDocPtr GXxmlParseFile(const char *filename) {
    return(GXxmlSAXParseFile(NULL, filename));
}

/**
 * GXxmlSAXParseMemory :
 * @sax:  the SAX handler block
 * @cur:  an pointer to a char array
 * @size:  the siwe of the array
 *
 * parse an XML in-memory block and use the given SAX function block
 * to handle the parsing callback. If sax is NULL, fallback to the default
 * DOM tree building routines.
 * 
 * TODO : plug some encoding conversion routines here. !!!
 *
 * return values: the resulting document tree
 */

GXxmlDocPtr GXxmlSAXParseMemory(GXxmlSAXHandlerPtr sax, char *buffer, int size) {
    GXxmlDocPtr ret;
    GXxmlParserCtxtPtr ctxt;
    GXxmlParserInputPtr input;

    buffer[size - 1] = '\0';

    ctxt = (GXxmlParserCtxtPtr) GxCore_Malloc(sizeof(GXxmlParserCtxt));
    if (ctxt == NULL) {
        perror("GxCore_Malloc");
	return(NULL);
    }
    GXxmlInitParserCtxt(ctxt);
    if (sax != NULL) ctxt->sax = sax;
    input = (GXxmlParserInputPtr) GxCore_Malloc(sizeof(GXxmlParserInput));
    if (input == NULL) {
        perror("GxCore_Malloc");
        GxCore_Free(ctxt->nodeTab);
	GxCore_Free(ctxt->inputTab);
	GxCore_Free(ctxt);
	return(NULL);
    }

    input->filename = NULL;
    input->line = 1;
    input->col = 1;

    /*
     * TODO : plug some encoding conversion routines here. !!!
     */
    input->base = (CHAR *)buffer;
    input->cur = (CHAR *)buffer;

    GXinputPush(ctxt, input);

    GXxmlParseDocument(ctxt);

    ret = ctxt->doc;
    GxCore_Free(ctxt->nodeTab);
    GxCore_Free(ctxt->inputTab);
    if (input->filename != NULL)
	GxCore_Free((char *)input->filename);
    GxCore_Free(input);
    GxCore_Free(ctxt);
    
    return(ret);
}

/**
 * GXxmlParseMemory :
 * @cur:  an pointer to a char array
 * @size:  the size of the array
 *
 * parse an XML in-memory block and build a tree.
 * 
 * return values: the resulting document tree
 */

GXxmlDocPtr GXxmlParseMemory(char *buffer, int size) {
   return(GXxmlSAXParseMemory(NULL, buffer, size));
}

/**
 * GXxmlInitParserCtxt:
 * @ctxt:  an XML parser context
 *
 * Initialize a parser context
 */

void
GXxmlInitParserCtxt(GXxmlParserCtxtPtr ctxt)
{
  /* Allocate the Input stack */
  ctxt->inputTab = (GXxmlParserInputPtr *) GxCore_Malloc(5 * sizeof(GXxmlParserInputPtr));
  ctxt->inputNr = 0;
  ctxt->inputMax = 5;
  ctxt->input = NULL;

  /* Allocate the Node stack */
  ctxt->nodeTab = (GXxmlNodePtr *) GxCore_Malloc(10 * sizeof(GXxmlNodePtr));
  ctxt->nodeNr = 0;
  ctxt->nodeMax = 10;
  ctxt->node = NULL;

  ctxt->sax = &GXxmlDefaultSAXHandler;
  ctxt->doc = NULL;
  ctxt->record_info = 0;
  GXxmlInitNodeInfoSeq(&ctxt->node_seq);
}

/**
 * GXxmlClearParserCtxt:
 * @ctxt:  an XML parser context
 *
 * Clear (release owned resources) and reinitialize a parser context
 */

void
GXxmlClearParserCtxt(GXxmlParserCtxtPtr ctxt)
{
  GXxmlClearNodeInfoSeq(&ctxt->node_seq);
  GXxmlInitParserCtxt(ctxt);
}


/**
 * GXxmlSetupParserForBuffer:
 * @ctxt:  an XML parser context
 * @buffer:  a CHAR * buffer
 * @filename:  a file name
 *
 * Setup the parser context to parse a new buffer; Clears any prior
 * contents from the parser context. The buffer parameter must not be
 * NULL, but the filename parameter can be
 */

void
GXxmlSetupParserForBuffer(GXxmlParserCtxtPtr ctxt, const CHAR* buffer,
                             const char* filename)
{
  GXxmlParserInputPtr input;

  input = (GXxmlParserInputPtr) GxCore_Malloc(sizeof(GXxmlParserInput));
  if (input == NULL) {
      perror("GxCore_Malloc");
      GxCore_Free(ctxt);
      exit(1);
  }

  GXxmlClearParserCtxt(ctxt);
  if (input->filename != NULL)
      input->filename = GxCore_Strdup(filename);
  else
      input->filename = NULL;
  input->line = 1;
  input->col = 1;
  input->base = buffer;
  input->cur = buffer;

  GXinputPush(ctxt, input);
}


/**
 * GXxmlParserFindNodeInfo:
 * @ctxt:  an XML parser context
 * @node:  an XML node within the tree
 *
 * Find the parser node info struct for a given node
 * 
 * return values: an GXxmlParserNodeInfo block pointer or NULL
 */
const GXxmlParserNodeInfo* GXxmlParserFindNodeInfo(const GXxmlParserCtxt* ctx,
						   const GXxmlNode* node)
{
  unsigned long pos;

  /* Find position where node should be at */
  pos = GXxmlParserFindNodeInfoIndex(&ctx->node_seq, node);
  if ( ctx->node_seq.buffer[pos].node == node )
    return &ctx->node_seq.buffer[pos];
  else
    return NULL;
}


/**
 * GXxmlInitNodeInfoSeq :
 * @seq:  a node info sequence pointer
 *
 * -- Initialize (set to initial state) node info sequence
 */
void
GXxmlInitNodeInfoSeq(GXxmlParserNodeInfoSeqPtr seq)
{
  seq->length = 0;
  seq->maximum = 0;
  seq->buffer = NULL;
}

/**
 * GXxmlClearNodeInfoSeq :
 * @seq:  a node info sequence pointer
 *
 * -- Clear (release memory and reinitialize) node
 *   info sequence
 */
void
GXxmlClearNodeInfoSeq(GXxmlParserNodeInfoSeqPtr seq)
{
  if ( seq->buffer != NULL )
    GxCore_Free(seq->buffer);
  GXxmlInitNodeInfoSeq(seq);
}


/**
 * GXxmlParserFindNodeInfoIndex:
 * @seq:  a node info sequence pointer
 * @node:  an XML node pointer
 *
 * 
 * GXxmlParserFindNodeInfoIndex : Find the index that the info record for
 *   the given node is or should be at in a sorted sequence
 * return values: a long indicating the position of the record
 */
unsigned long GXxmlParserFindNodeInfoIndex(const GXxmlParserNodeInfoSeq* seq,
                                         const GXxmlNode* node)
{
  unsigned long upper, lower, middle;
  int found = 0;

  /* Do a binary search for the key */
  lower = 1;
  upper = seq->length;
  middle = 0;
  while ( lower <= upper && !found) {
    middle = lower + (upper - lower) / 2;
    if ( node == seq->buffer[middle - 1].node )
      found = 1;
    else if ( node < seq->buffer[middle - 1].node )
      upper = middle - 1;
    else
      lower = middle + 1;
  }

  /* Return position */
  if ( middle == 0 || seq->buffer[middle - 1].node < node )
    return middle;
  else 
    return middle - 1;
}


/**
 * GXxmlParserAddNodeInfo:
 * @ctxt:  an XML parser context
 * @seq:  a node info sequence pointer
 *
 * Insert node info record into the sorted sequence
 */
void
GXxmlParserAddNodeInfo(GXxmlParserCtxtPtr ctxt, 
                     const GXxmlParserNodeInfo* info)
{
  unsigned long pos;
  static unsigned int block_size = 5;

  /* Find pos and check to see if node is already in the sequence */
  pos = GXxmlParserFindNodeInfoIndex(&ctxt->node_seq, info->node);
  if ( pos < ctxt->node_seq.length
       && ctxt->node_seq.buffer[pos].node == info->node ) {
    ctxt->node_seq.buffer[pos] = *info;
  }

  /* Otherwise, we need to add new node to buffer */
  else {
    /* Expand buffer by 5 if needed */
    if ( ctxt->node_seq.length + 1 > ctxt->node_seq.maximum ) {
      GXxmlParserNodeInfo* tmp_buffer;
      unsigned int byte_size = (sizeof(*ctxt->node_seq.buffer)
                                *(ctxt->node_seq.maximum + block_size));

      if ( ctxt->node_seq.buffer == NULL )
        tmp_buffer = (GXxmlParserNodeInfo*)GxCore_Malloc(byte_size);
      else
        tmp_buffer = (GXxmlParserNodeInfo*)GxCore_Realloc(ctxt->node_seq.buffer, byte_size);

      if ( tmp_buffer == NULL ) {
        if ((ctxt->sax != NULL) && (ctxt->sax->error != NULL))
	    ctxt->sax->error(ctxt, "Out of memory\n");
        return;
      }
      ctxt->node_seq.buffer = tmp_buffer;
      ctxt->node_seq.maximum += block_size;
    }

    /* If position is not at end, move elements out of the way */
    if ( pos != ctxt->node_seq.length ) {
      unsigned long i;

      for ( i = ctxt->node_seq.length; i > pos; i-- )
        ctxt->node_seq.buffer[i] = ctxt->node_seq.buffer[i - 1];
    }
  
    /* Copy element and increase length */
    ctxt->node_seq.buffer[pos] = *info;
    ctxt->node_seq.length++;
  }   
}
