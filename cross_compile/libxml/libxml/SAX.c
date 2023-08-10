/*
 * SAX.c : Default SAX handler to build a tree.
 *
 * Daniel Veillard <Daniel.Veillard@w3.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "parser.h"
#include "entities.h"
#include "error.h"

/* #define DEBUG_SAX */

/**
 * GXgetPublicId:
 * @ctxt:  An XML parser context
 *
 * Return the public ID e.g. "-//SGMLSOURCE//DTD DEMO//EN"
 *
 * return values: a CHAR *
 */
const CHAR *
GXgetPublicId(GXxmlParserCtxtPtr ctxt)
{
    return(NULL);
}

/**
 * GXgetSystemId:
 * @ctxt:  An XML parser context
 *
 * Return the system ID, basically URI or filename e.g.
 * http://www.sgmlsource.com/dtds/memo.dtd
 *
 * return values: a CHAR *
 */
const CHAR *
GXgetSystemId(GXxmlParserCtxtPtr ctxt)
{
    return(CHAR *)(ctxt->input->filename); //zhaofzh20090916
}

/**
 * GXgetLineNumber:
 * @ctxt:  An XML parser context
 *
 * Return the line number of the current parsing point.
 *
 * return values: an int
 */
int
GXgetLineNumber(GXxmlParserCtxtPtr ctxt)
{
    return(ctxt->input->line);
}

/**
 * GXgetColumnNumber:
 * @ctxt:  An XML parser context
 *
 * Return the column number of the current parsing point.
 *
 * return values: an int
 */
int
GXgetColumnNumber(GXxmlParserCtxtPtr ctxt)
{
    return(ctxt->input->col);
}

/*
 * The default SAX Locator.
 */

GXxmlSAXLocator GXxmlDefaultSAXLocator = {
    GXgetPublicId, GXgetSystemId, GXgetLineNumber, GXgetColumnNumber
};

/**
 * GXresolveEntity:
 * @ctxt:  An XML parser context
 * @publicId: The public ID of the entity
 * @systemId: The system ID of the entity
 *
 * Special entity resolver, better left to the parser, it has
 * more context than the application layer.
 * The default behaviour is to NOT resolve the entities, in that case
 * the ENTITY_REF nodes are built in the structure (and the parameter
 * values).
 *
 * return values: the xmlParserInputPtr if inlined or NULL for DOM behaviour.
 */
GXxmlParserInputPtr
GXresolveEntity(GXxmlParserCtxtPtr ctxt, const CHAR *publicId, const CHAR *systemId)
{

#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.resolveEntity(%s, %s)\n", publicId, systemId);
#endif

    return(NULL);
}

/**
 * GXnotationDecl:
 * @ctxt:  An XML parser context
 * @name: The name of the notation
 * @publicId: The public ID of the entity
 * @systemId: The system ID of the entity
 *
 * What to do when a notation declaration has been parsed.
 * TODO Not handled currently.
 *
 * return values: 
 */
void
GXnotationDecl(GXxmlParserCtxtPtr ctxt, const CHAR *name,
	     const CHAR *publicId, const CHAR *systemId)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.notationDecl(%s, %s, %s)\n", name, publicId, systemId);
#endif
}

/**
 * GXunparsedEntityDecl:
 * @ctxt:  An XML parser context
 * @name: The name of the entity
 * @publicId: The public ID of the entity
 * @systemId: The system ID of the entity
 * @notationName: the name of the notation
 *
 * What to do when an unparsed entity declaration is parsed
 * TODO Create an Entity node.
 *
 * return values: 
 */
void
GXunparsedEntityDecl(GXxmlParserCtxtPtr ctxt, const CHAR *name,
		   const CHAR *publicId, const CHAR *systemId,
		   const CHAR *notationName)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.unparsedEntityDecl(%s, %s, %s, %s)\n",
            name, publicId, systemId, notationName);
#endif
}

/**
 * setDocumentLocator:
 * @ctxt:  An XML parser context
 * @loc: A SAX Locator
 *
 * Receive the document locator at startup, actually GXxmlDefaultSAXLocator
 * Everything is available on the context, so this is useless in our case.
 *
 * return values: 
 */
void
GXsetDocumentLocator(GXxmlParserCtxtPtr ctxt, GXxmlSAXLocatorPtr loc)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.setDocumentLocator()\n");
#endif
}

/**
 * startDocument:
 * @ctxt:  An XML parser context
 *
 * called when the document start being processed.
 *
 * return values: 
 */
void
GXstartDocument(GXxmlParserCtxtPtr ctxt)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.startDocument()\n");
#endif
}

/**
 * endDocument:
 * @ctxt:  An XML parser context
 *
 * called when the document end has been detected.
 *
 * return values: 
 */
void
GXendDocument(GXxmlParserCtxtPtr ctxt)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.endDocument()\n");
#endif
}

/**
 * startElement:
 * @ctxt:  An XML parser context
 * @name:  The element name
 *
 * called when an opening tag has been processed.
 * TODO We currently have a small pblm with the arguments ...
 *
 * return values: 
 */
void
GXstartElement(GXxmlParserCtxtPtr ctxt, const CHAR *name)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.startElement(%s)\n", name);
#endif
}

/**
 * endElement:
 * @ctxt:  An XML parser context
 * @name:  The element name
 *
 * called when the end of an element has been detected.
 *
 * return values: 
 */
void
GXendElement(GXxmlParserCtxtPtr ctxt, const CHAR *name)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.endElement(%s)\n", name);
#endif
}

/**
 * attribute:
 * @ctxt:  An XML parser context
 * @name:  The attribute name
 * @value:  The attribute value
 *
 * called when an attribute has been read by the parser.
 * The default handling is to convert the attribute into an
 * DOM subtree and past it in a new xmlAttr element added to
 * the element.
 *
 * return values: 
 */
void
GXattribute(GXxmlParserCtxtPtr ctxt, const CHAR *name, const CHAR *value)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.attribute(%s, %s)\n", name, value);
#endif
}

/**
 * characters:
 * @ctxt:  An XML parser context
 * @ch:  a CHAR string
 * @start: the first char in the string
 * @len: the number of CHAR
 *
 * receiving some chars from the parser.
 * Question: how much at a time ???
 *
 * return values: 
 */
void
GXcharacters(GXxmlParserCtxtPtr ctxt, const CHAR *ch, int start, int len)
{
    GXxmlNodePtr lastChild;

#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.characters(%.30s, %d, %d)\n", ch, start, len);
#endif
    /*
     * Handle the data if any. If there is no child
     * add it as content, otherwise if the last child is text,
     * concatenate it, else create a new node of type text.
     */

    lastChild = GXxmlGetLastChild(ctxt->node);
    if (lastChild == NULL)
	GXxmlNodeAddContentLen(ctxt->node, &ch[start], len);
    else {
	if (GXxmlNodeIsText(lastChild))
	    GXxmlTextConcat(lastChild, &ch[start], len);
	else {
	    lastChild = GXxmlNewTextLen(&ch[start], len);
	    GXxmlAddChild(ctxt->node, lastChild);
	}
    }
}

/**
 * ignorableWhitespace:
 * @ctxt:  An XML parser context
 * @ch:  a CHAR string
 * @start: the first char in the string
 * @len: the number of CHAR
 *
 * receiving some ignorable whitespaces from the parser.
 * Question: how much at a time ???
 *
 * return values: 
 */
void
GXignorableWhitespace(GXxmlParserCtxtPtr ctxt, const CHAR *ch, int start, int len)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.ignorableWhitespace(%.30s, %d, %d)\n", ch, start, len);
#endif
}

/**
 * processingInstruction:
 * @ctxt:  An XML parser context
 * @target:  the target name
 * @data: the PI data's
 * @len: the number of CHAR
 *
 * A processing instruction has been parsed.
 *
 * return values: 
 */
void
GXprocessingInstruction(GXxmlParserCtxtPtr ctxt, const CHAR *target,
                      const CHAR *data)
{
#ifdef DEBUG_SAX
    fprintf(stderr, "SAX.processingInstruction(%s, %s)\n", target, data);
#endif
}

GXxmlSAXHandler GXxmlDefaultSAXHandler = {
    GXresolveEntity,
    GXnotationDecl,
    GXunparsedEntityDecl,
    GXsetDocumentLocator,
    GXstartDocument,
    GXendDocument,
    GXstartElement,
    GXendElement,
    GXattribute,
    GXcharacters,
    GXignorableWhitespace,
    GXprocessingInstruction,
    GXxmlParserWarning,
    GXxmlParserError,
    GXxmlParserError,
};

/**
 * xmlDefaultSAXHandlerInit:
 *
 * Initialize the default SAX handler
 */
void
GXxmlDefaultSAXHandlerInit(void)
{
    GXxmlDefaultSAXHandler.resolveEntity = GXresolveEntity;
    GXxmlDefaultSAXHandler.notationDecl = GXnotationDecl;
    GXxmlDefaultSAXHandler.unparsedEntityDecl = GXunparsedEntityDecl;
    GXxmlDefaultSAXHandler.setDocumentLocator = GXsetDocumentLocator;
    GXxmlDefaultSAXHandler.startDocument = GXstartDocument;
    GXxmlDefaultSAXHandler.endDocument = GXendDocument;
    GXxmlDefaultSAXHandler.startElement = GXstartElement;
    GXxmlDefaultSAXHandler.endElement = GXendElement;
    GXxmlDefaultSAXHandler.attribute = GXattribute;
    GXxmlDefaultSAXHandler.characters = GXcharacters;
    GXxmlDefaultSAXHandler.ignorableWhitespace = GXignorableWhitespace;
    GXxmlDefaultSAXHandler.processingInstruction = GXprocessingInstruction;
    GXxmlDefaultSAXHandler.warning = GXxmlParserWarning;
    GXxmlDefaultSAXHandler.error = GXxmlParserError;
    GXxmlDefaultSAXHandler.fatalError = GXxmlParserError;
}
