/*
 * debugXML.c : This is a set of routines used for debugging the tree
 *              produced by the XML parser.
 *
 * Daniel Veillard <Daniel.Veillard@w3.org>
 */

#include <stdio.h>
#include "tree.h"
#include "parser.h"
#include "entities.h"
#include "debugXML.h"

#define IS_BLANK(c)							\
  (((c) == '\n') || ((c) == '\r') || ((c) == '\t') || ((c) == ' '))

void GXxmlDebugDumpString(FILE *output, const CHAR *str) {
    int i;
    for (i = 0;i < 40;i++)
        if (str[i] == 0) return;
	else if (IS_BLANK(str[i])) fputc(' ', output);
	else fputc(str[i], output);
    fprintf(output, "...");
}

void GXxmlDebugDumpNamespace(FILE *output, GXxmlNsPtr ns, int depth) {
    int i;
    char shift[100];

    for (i = 0;((i < depth) && (i < 25));i++)
        shift[2 * i] = shift[2 * i + 1] = ' ';
    shift[2 * i] = shift[2 * i + 1] = 0;

    fprintf(output, shift);
    if (ns->type == XML_GLOBAL_NAMESPACE)
        fprintf(output, "old ");
    fprintf(output, "namespace %s href=", ns->prefix);
    GXxmlDebugDumpString(output, ns->href);
    fprintf(output, "\n");
}

void GXxmlDebugDumpNamespaceList(FILE *output, GXxmlNsPtr ns, int depth) {
    while (ns != NULL) {
        GXxmlDebugDumpNamespace(output, ns, depth);
	ns = ns->next;
    }
}

void GXxmlDebugDumpEntity(FILE *output, GXxmlEntityPtr ent, int depth) {
    int i;
    char shift[100];

    for (i = 0;((i < depth) && (i < 25));i++)
        shift[2 * i] = shift[2 * i + 1] = ' ';
    shift[2 * i] = shift[2 * i + 1] = 0;

    fprintf(output, shift);
    switch (ent->type) {
        case XML_INTERNAL_GENERAL_ENTITY:
	    fprintf(output, "INTERNAL_GENERAL_ENTITY ");
	    break;
        case XML_EXTERNAL_GENERAL_PARSED_ENTITY:
	    fprintf(output, "EXTERNAL_GENERAL_PARSED_ENTITY ");
	    break;
        case XML_EXTERNAL_GENERAL_UNPARSED_ENTITY:
	    fprintf(output, "EXTERNAL_GENERAL_UNPARSED_ENTITY ");
	    break;
        case XML_INTERNAL_PARAMETER_ENTITY:
	    fprintf(output, "INTERNAL_PARAMETER_ENTITY ");
	    break;
        case XML_EXTERNAL_PARAMETER_ENTITY:
	    fprintf(output, "EXTERNAL_PARAMETER_ENTITY ");
	    break;
	default:
	    fprintf(output, "ENTITY_%d ! ", ent->type);
    }
    fprintf(output, "%s\n", ent->name);
    if (ent->ExternalID) {
        fprintf(output, shift);
        fprintf(output, "ExternalID=%s\n", ent->ExternalID);
    }
    if (ent->SystemID) {
        fprintf(output, shift);
        fprintf(output, "SystemID=%s\n", ent->SystemID);
    }
    if (ent->content) {
        fprintf(output, shift);
	fprintf(output, "content=");
	GXxmlDebugDumpString(output, ent->content);
	fprintf(output, "\n");
    }
}

void GXxmlDebugDumpAttr(FILE *output, GXxmlAttrPtr attr, int depth) {
    int i;
    char shift[100];

    for (i = 0;((i < depth) && (i < 25));i++)
        shift[2 * i] = shift[2 * i + 1] = ' ';
    shift[2 * i] = shift[2 * i + 1] = 0;

    fprintf(output, shift);
    fprintf(output, "ATTRIBUTE %s\n", attr->name);
    if (attr->val != NULL) 
        GXxmlDebugDumpNodeList(output, attr->val, depth + 1);
}

void GXxmlDebugDumpAttrList(FILE *output, GXxmlAttrPtr attr, int depth) {
    while (attr != NULL) {
        GXxmlDebugDumpAttr(output, attr, depth);
	attr = attr->next;
    }
}

void GXxmlDebugDumpNode(FILE *output, GXxmlNodePtr node, int depth) {
    int i;
    char shift[100];

    for (i = 0;((i < depth) && (i < 25));i++)
        shift[2 * i] = shift[2 * i + 1] = ' ';
    shift[2 * i] = shift[2 * i + 1] = 0;

    fprintf(output, shift);
    switch (node->type) {
	case XML_ELEMENT_NODE:
	    fprintf(output, "ELEMENT ");
	    if (node->ns != NULL)
	        fprintf(output, "%s:%s\n", node->ns->prefix, node->name);
	    else
	        fprintf(output, "%s\n", node->name);
	    break;
	case XML_ATTRIBUTE_NODE:
	    fprintf(output, "Error, ATTRIBUTE found here\n");
	    break;
	case XML_TEXT_NODE:
	    fprintf(output, "TEXT\n");
	    break;
	case XML_CDATA_SECTION_NODE:
	    fprintf(output, "CDATA_SECTION\n");
	    break;
	case XML_ENTITY_REF_NODE:
	    fprintf(output, "ENTITY_REF\n");
	    break;
	case XML_ENTITY_NODE:
	    fprintf(output, "ENTITY\n");
	    break;
	case XML_PI_NODE:
	    fprintf(output, "PI\n");
	    break;
	case XML_COMMENT_NODE:
	    fprintf(output, "COMMENT\n");
	    break;
	case XML_DOCUMENT_NODE:
	    fprintf(output, "Error, DOCUMENT found here\n");
	    break;
	case XML_DOCUMENT_TYPE_NODE:
	    fprintf(output, "DOCUMENT_TYPE\n");
	    break;
	case XML_DOCUMENT_FRAG_NODE:
	    fprintf(output, "DOCUMENT_FRAG\n");
	    break;
	case XML_NOTATION_NODE:
	    fprintf(output, "NOTATION\n");
	    break;
	default:
	    fprintf(output, "NODE_%d\n", node->type);
    }
    if (node->doc == NULL) {
        fprintf(output, shift);
	fprintf(output, "doc == NULL !!!\n");
    }
    if (node->nsDef != NULL) 
        GXxmlDebugDumpNamespaceList(output, node->nsDef, depth + 1);
    if (node->properties != NULL)
	GXxmlDebugDumpAttrList(output, node->properties, depth + 1);
    if (node->type != XML_ENTITY_REF_NODE) {
	if (node->content != NULL) {
	    fprintf(output, shift);
	    fprintf(output, "content=");
	    GXxmlDebugDumpString(output, node->content);
	    fprintf(output, "\n");
	}
    } else {
        GXxmlEntityPtr ent;
	ent = GXxmlGetDocEntity(node->doc, node->name);
	if (ent != NULL)
	    GXxmlDebugDumpEntity(output, ent, depth + 1);
    }
    if (node->childs != NULL)
	GXxmlDebugDumpNodeList(output, node->childs, depth + 1);
}

void GXxmlDebugDumpNodeList(FILE *output, GXxmlNodePtr node, int depth) {
    while (node != NULL) {
        GXxmlDebugDumpNode(output, node, depth);
	node = node->next;
    }
}


void GXxmlDebugDumpDocument(FILE *output, GXxmlDocPtr doc) {
    if (output == NULL) output = stdout;
    if (doc == NULL) {
        fprintf(output, "DOCUMENT == NULL !\n");
	return;
    }

    switch (doc->type) {
	case XML_ELEMENT_NODE:
	    fprintf(output, "Error, ELEMENT found here ");
	    break;
	case XML_ATTRIBUTE_NODE:
	    fprintf(output, "Error, ATTRIBUTE found here\n");
	    break;
	case XML_TEXT_NODE:
	    fprintf(output, "Error, TEXT\n");
	    break;
	case XML_CDATA_SECTION_NODE:
	    fprintf(output, "Error, CDATA_SECTION\n");
	    break;
	case XML_ENTITY_REF_NODE:
	    fprintf(output, "Error, ENTITY_REF\n");
	    break;
	case XML_ENTITY_NODE:
	    fprintf(output, "Error, ENTITY\n");
	    break;
	case XML_PI_NODE:
	    fprintf(output, "Error, PI\n");
	    break;
	case XML_COMMENT_NODE:
	    fprintf(output, "Error, COMMENT\n");
	    break;
	case XML_DOCUMENT_NODE:
	    fprintf(output, "DOCUMENT\n");
	    break;
	case XML_DOCUMENT_TYPE_NODE:
	    fprintf(output, "Error, DOCUMENT_TYPE\n");
	    break;
	case XML_DOCUMENT_FRAG_NODE:
	    fprintf(output, "Error, DOCUMENT_FRAG\n");
	    break;
	case XML_NOTATION_NODE:
	    fprintf(output, "Error, NOTATION\n");
	    break;
	default:
	    fprintf(output, "NODE_%d\n", doc->type);
    }
    if (doc->name != NULL) {
	fprintf(output, "name=");
        GXxmlDebugDumpString(output, (CHAR *)doc->name);
	fprintf(output, "\n");
    }
    if (doc->version != NULL) {
	fprintf(output, "version=");
        GXxmlDebugDumpString(output, doc->version);
	fprintf(output, "\n");
    }
    if (doc->encoding != NULL) {
	fprintf(output, "encoding=");
        GXxmlDebugDumpString(output, doc->encoding);
	fprintf(output, "\n");
    }
    if (doc->standalone)
        fprintf(output, "standalone=true\n");
    if (doc->oldNs != NULL) 
        GXxmlDebugDumpNamespaceList(output, doc->oldNs, 0);
    if (doc->root != NULL)
        GXxmlDebugDumpNodeList(output, doc->root, 1);
}
