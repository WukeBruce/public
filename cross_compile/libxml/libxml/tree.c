/*
 * tree.c : implemetation of access function for an XML tree.
 *
 * See Copyright for the status of this software.
 *
 * $Id: tree.c,v 1.19 1998/12/06 18:08:28 veillard Exp $
 *
 * TODO Cleanup the Dump mechanism.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h> /* for memset() only ! */

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

#include "tree.h"
#include "entities.h"

static CHAR GXxmlStringText[] = { 't', 'e', 'x', 't', 0 };
int GXoldXMLWDcompatibility = 0;
int GXxmlIndentTreeOutput = 1;

static int GXxmlCompressMode = 0;

/************************************************************************
 *									*
 *		Allocation and deallocation of basic structures		*
 *									*
 ************************************************************************/
 
/**
 * GXxmlUpgradeOldNs:
 * @doc:  a document pointer
 * 
 * Upgrade old style Namespaces (PI) and move them to the root of the document.
 */
void
GXxmlUpgradeOldNs(GXxmlDocPtr doc) {
    GXxmlNsPtr cur;

    if ((doc == NULL) || (doc->oldNs == NULL)) return;
    if (doc->root == NULL) {
        fprintf(stderr, "GXxmlUpgradeOldNs: failed no root !\n");
	return;
    }

    cur = doc->oldNs;
    while (cur->next != NULL) {
	cur->type = XML_LOCAL_NAMESPACE;
        cur = cur->next;
    }
    cur->type = XML_LOCAL_NAMESPACE;
    cur->next = doc->root->nsDef;
    doc->root->nsDef = doc->oldNs;
    doc->oldNs = NULL;
}

/**
 * GXxmlNewNs:
 * @node:  the element carrying the namespace
 * @href:  the URI associated
 * @prefix:  the prefix for the namespace
 *
 * Creation of a new Namespace.
 * return values: returns a new namespace pointer
 */
GXxmlNsPtr
GXxmlNewNs(GXxmlNodePtr node, const CHAR *href, const CHAR *prefix) {
    GXxmlNsPtr cur;

    if (href == NULL) {
        fprintf(stderr, "GXxmlNewNs: href == NULL !\n");
	return(NULL);
    }

    /*
     * Allocate a new DTD and fill the fields.
     */
    cur = (GXxmlNsPtr) GxCore_Malloc(sizeof(GXxmlNs));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewNs : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_LOCAL_NAMESPACE;
    if (href != NULL)
	cur->href = GXxmlStrdup(href); 
    else
        cur->href = NULL;
    if (prefix != NULL)
	cur->prefix = GXxmlStrdup(prefix); 
    else
        cur->prefix = NULL;

    /*
     * Add it at the end to preserve parsing order ...
     */
    cur->next = NULL;
    if (node != NULL) {
	if (node->nsDef == NULL) {
	    node->nsDef = cur;
	} else {
	    GXxmlNsPtr prev = node->nsDef;

	    while (prev->next != NULL) prev = prev->next;
	    prev->next = cur;
	}
    }

    return(cur);
}

/**
 * GXxmlNewGlobalNs:
 * @doc:  the document carrying the namespace
 * @href:  the URI associated
 * @prefix:  the prefix for the namespace
 *
 * Creation of a Namespace, the old way using PI and without scoping, to AVOID.
 * return values: returns a new namespace pointer
 */
GXxmlNsPtr
GXxmlNewGlobalNs(GXxmlDocPtr doc, const CHAR *href, const CHAR *prefix) {
    GXxmlNsPtr cur;

    /*
     * Allocate a new DTD and fill the fields.
     */
    cur = (GXxmlNsPtr) GxCore_Malloc(sizeof(GXxmlNs));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewGlobalNs : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_GLOBAL_NAMESPACE;
    if (href != NULL)
	cur->href = GXxmlStrdup(href); 
    else
        cur->href = NULL;
    if (prefix != NULL)
	cur->prefix = GXxmlStrdup(prefix); 
    else
        cur->prefix = NULL;

    /*
     * Add it at the end to preserve parsing order ...
     */
    cur->next = NULL;
    if (doc != NULL) {
	if (doc->oldNs == NULL) {
	    doc->oldNs = cur;
	} else {
	    GXxmlNsPtr prev = doc->oldNs;

	    while (prev->next != NULL) prev = prev->next;
	    prev->next = cur;
	}
    }

    return(cur);
}

/**
 * GXxmlSetNs:
 * @node:  a node in the document
 * @ns:  a namespace pointer
 *
 * Associate a namespace to a node, a posteriori.
 */
void
GXxmlSetNs(GXxmlNodePtr node, GXxmlNsPtr ns) {
    if (node == NULL) {
        fprintf(stderr, "GXxmlSetNs: node == NULL\n");
	return;
    }
    node->ns = ns;
}

/**
 * GXxmlFreeNs:
 * @cur:  the namespace pointer
 *
 * GxCore_Free up the structures associated to a namespace
 */
void
GXxmlFreeNs(GXxmlNsPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreeNs : ns == NULL\n");
	return;
    }
    if (cur->href != NULL) GxCore_Free((char *) cur->href);
    if (cur->prefix != NULL) GxCore_Free((char *) cur->prefix);
    memset(cur, -1, sizeof(GXxmlNs));
    GxCore_Free(cur);
}

/**
 * GXxmlFreeNsList:
 * @cur:  the first namespace pointer
 *
 * GxCore_Free up all the structures associated to the chained namespaces.
 */
void
GXxmlFreeNsList(GXxmlNsPtr cur) {
    GXxmlNsPtr next;
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreeNsList : ns == NULL\n");
	return;
    }
    while (cur != NULL) {
        next = cur->next;
        GXxmlFreeNs(cur);
	cur = next;
    }
}

/**
 * GXxmlNewDtd:
 * @doc:  the document pointer
 * @name:  the DTD name
 * @ExternalID:  the external ID
 * @SystemID:  the system ID
 *
 * Creation of a new DTD.
 * return values: a pointer to the new DTD structure
 */
GXxmlDtdPtr
GXxmlNewDtd(GXxmlDocPtr doc, const CHAR *name,
                    const CHAR *ExternalID, const CHAR *SystemID) {
    GXxmlDtdPtr cur;

    if ((doc != NULL) && (doc->dtd != NULL)) {
        fprintf(stderr, "GXxmlNewDtd(%s): document %s already have a DTD %s\n",
	/* !!! */ (char *) name, doc->name, /* !!! */ (char *)doc->dtd->name);
    }

    /*
     * Allocate a new DTD and fill the fields.
     */
    cur = (GXxmlDtdPtr) GxCore_Malloc(sizeof(GXxmlDtd));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewDtd : GxCore_Malloc failed\n");
	return(NULL);
    }

    if (name != NULL)
	cur->name = GXxmlStrdup(name); 
    else
        cur->name = NULL;
    if (ExternalID != NULL)
	cur->ExternalID = GXxmlStrdup(ExternalID); 
    else
        cur->ExternalID = NULL;
    if (SystemID != NULL)
	cur->SystemID = GXxmlStrdup(SystemID); 
    else
        cur->SystemID = NULL;
    cur->elements = NULL;
    cur->entities = NULL;
    if (doc != NULL)
	doc->dtd = cur;

    return(cur);
}

/**
 * GXxmlFreeDtd:
 * @cur:  the DTD structure to GxCore_Free up
 *
 * GxCore_Free a DTD structure.
 */
void
GXxmlFreeDtd(GXxmlDtdPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreeDtd : DTD == NULL\n");
	return;
    }
    if (cur->name != NULL) GxCore_Free((char *) cur->name);
    if (cur->SystemID != NULL) GxCore_Free((char *) cur->SystemID);
    if (cur->ExternalID != NULL) GxCore_Free((char *) cur->ExternalID);
    if (cur->elements != NULL)
        fprintf(stderr, "GXxmlFreeDtd: cur->elements != NULL !!! \n");
    if (cur->entities != NULL)
        GXxmlFreeEntitiesTable((GXxmlEntitiesTablePtr) cur->entities);
    memset(cur, -1, sizeof(GXxmlDtd));
    GxCore_Free(cur);
}

/**
 * GXxmlNewDoc:
 * @version:  CHAR string giving the version of XML "1.0"
 *
 * Create a new document
 */
GXxmlDocPtr
GXxmlNewDoc(const CHAR *version) {
    GXxmlDocPtr cur;

    if (version == NULL) {
        fprintf(stderr, "GXxmlNewDoc : version == NULL\n");
	return(NULL);
    }

    /*
     * Allocate a new document and fill the fields.
     */
    cur = (GXxmlDocPtr) GxCore_Malloc(sizeof(GXxmlDoc));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewDoc : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_DOCUMENT_NODE;
    cur->version = GXxmlStrdup(version); 
    cur->name = NULL;
    cur->root = NULL; 
    cur->dtd = NULL;
    cur->oldNs = NULL;
    cur->encoding = NULL;
    cur->entities = NULL;
    cur->standalone = -1;
    cur->compression = GXxmlCompressMode;
#ifndef WITHOUT_CORBA
    cur->_private = NULL;
    cur->vepv = NULL;
#endif
    return(cur);
}

/**
 * GXxmlFreeDoc:
 * @cur:  pointer to the document
 * @:  
 *
 * GxCore_Free up all the structures used by a document, tree included.
 */
void
GXxmlFreeDoc(GXxmlDocPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreeDoc : document == NULL\n");
	return;
    }
    GxCore_Free((char *) cur->version);
    if (cur->name != NULL) GxCore_Free((char *) cur->name);
    if (cur->encoding != NULL) GxCore_Free((char *) cur->encoding);
    if (cur->root != NULL) GXxmlFreeNode(cur->root);
    if (cur->dtd != NULL) GXxmlFreeDtd(cur->dtd);
    if (cur->oldNs != NULL) GXxmlFreeNsList(cur->oldNs);
    if (cur->entities != NULL)
        GXxmlFreeEntitiesTable((GXxmlEntitiesTablePtr) cur->entities);
    memset(cur, -1, sizeof(GXxmlDoc));
    GxCore_Free(cur);
}

/**
 * GXxmlStringLenGetNodeList:
 * @doc:  the document
 * @value:  the value of the text
 * @int len:  the length of the string value
 *
 * Parse the value string and build the node list associated. Should
 * produce a flat tree with only TEXTs and ENTITY_REFs.
 * return values: a pointer to the first child
 */
GXxmlNodePtr
GXxmlStringLenGetNodeList(GXxmlDocPtr doc, const CHAR *value, int len) {
    GXxmlNodePtr ret = NULL, last = NULL;
    GXxmlNodePtr node;
    CHAR *val;
    const CHAR *cur = value;
    const CHAR *q;
    GXxmlEntityPtr ent;

    if (value == NULL) return(NULL);

    q = cur;
    while ((*cur != 0) && (cur - value < len)) {
	if (*cur == '&') {
	    /*
	     * Save the current text.
	     */
            if (cur != q) {
		if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
		    GXxmlNodeAddContentLen(last, q, cur - q);
		} else {
		    node = GXxmlNewDocTextLen(doc, q, cur - q);
		    if (node == NULL) return(ret);
		    if (last == NULL)
			last = ret = node;
		    else {
			last->next = node;
			node->prev = last;
			last = node;
		    }
		}
	    }
	    /*
	     * Read the entity string
	     */
	    cur++;
	    q = cur;
	    while ((*cur != 0) && (cur - value < len) && (*cur != ';')) cur++;
	    if ((*cur == 0) || (cur - value >= len)) {
	        fprintf(stderr,
		        "GXxmlStringGetNodeList: unterminated entity %30s\n", q);
	        return(ret);
	    }
            if (cur != q) {
		/*
		 * Predefined entities don't generate nodes
		 */
		val = GXxmlStrndup(q, cur - q);
		ent = GXxmlGetDocEntity(doc, val);
		if ((ent != NULL) &&
		    (ent->type == XML_INTERNAL_PREDEFINED_ENTITY)) {
		    if (last == NULL) {
		        node = GXxmlNewDocText(doc, ent->content);
			last = ret = node;
		    } else
		        GXxmlNodeAddContent(last, ent->content);
		        
		} else {
		    /*
		     * Create a new REFERENCE_REF node
		     */
		    node = GXxmlNewReference(doc, val);
		    if (node == NULL) {
			if (val != NULL) GxCore_Free(val);
		        return(ret);
		    }
		    if (last == NULL)
			last = ret = node;
		    else {
			last->next = node;
			node->prev = last;
			last = node;
		    }
		}
		GxCore_Free(val);
	    }
	    cur++;
	    q = cur;
	} else 
	    cur++;
    }
    if (cur != q) {
        /*
	 * Handle the last piece of text.
	 */
	if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
	    GXxmlNodeAddContentLen(last, q, cur - q);
	} else {
	    node = GXxmlNewDocTextLen(doc, q, cur - q);
	    if (node == NULL) return(ret);
	    if (last == NULL)
		last = ret = node;
	    else {
		last->next = node;
		node->prev = last;
		last = node;
	    }
	}
    }
    return(ret);
}

/**
 * GXxmlStringGetNodeList:
 * @doc:  the document
 * @value:  the value of the attribute
 *
 * Parse the value string and build the node list associated. Should
 * produce a flat tree with only TEXTs and ENTITY_REFs.
 * return values: a pointer to the first child
 */
GXxmlNodePtr
GXxmlStringGetNodeList(GXxmlDocPtr doc, const CHAR *value) {
    GXxmlNodePtr ret = NULL, last = NULL;
    GXxmlNodePtr node;
    CHAR *val;
    const CHAR *cur = value;
    const CHAR *q;
    GXxmlEntityPtr ent;

    if (value == NULL) return(NULL);

    q = cur;
    while (*cur != 0) {
	if (*cur == '&') {
	    /*
	     * Save the current text.
	     */
            if (cur != q) {
		if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
		    GXxmlNodeAddContentLen(last, q, cur - q);
		} else {
		    node = GXxmlNewDocTextLen(doc, q, cur - q);
		    if (node == NULL) return(ret);
		    if (last == NULL)
			last = ret = node;
		    else {
			last->next = node;
			node->prev = last;
			last = node;
		    }
		}
	    }
	    /*
	     * Read the entity string
	     */
	    cur++;
	    q = cur;
	    while ((*cur != 0) && (*cur != ';')) cur++;
	    if (*cur == 0) {
	        fprintf(stderr,
		        "GXxmlStringGetNodeList: unterminated entity %30s\n", q);
	        return(ret);
	    }
            if (cur != q) {
		/*
		 * Predefined entities don't generate nodes
		 */
		val = GXxmlStrndup(q, cur - q);
		ent = GXxmlGetDocEntity(doc, val);
		if ((ent != NULL) &&
		    (ent->type == XML_INTERNAL_PREDEFINED_ENTITY)) {
		    if (last == NULL) {
		        node = GXxmlNewDocText(doc, ent->content);
			last = ret = node;
		    } else
		        GXxmlNodeAddContent(last, ent->content);
		        
		} else {
		    /*
		     * Create a new REFERENCE_REF node
		     */
		    node = GXxmlNewReference(doc, val);
		    if (node == NULL) {
			if (val != NULL) GxCore_Free(val);
		        return(ret);
		    }
		    if (last == NULL)
			last = ret = node;
		    else {
			last->next = node;
			node->prev = last;
			last = node;
		    }
		}
		GxCore_Free(val);
	    }
	    cur++;
	    q = cur;
	} else 
	    cur++;
    }
    if (cur != q) {
        /*
	 * Handle the last piece of text.
	 */
	if ((last != NULL) && (last->type == XML_TEXT_NODE)) {
	    GXxmlNodeAddContentLen(last, q, cur - q);
	} else {
	    node = GXxmlNewDocTextLen(doc, q, cur - q);
	    if (node == NULL) return(ret);
	    if (last == NULL)
		last = ret = node;
	    else {
		last->next = node;
		node->prev = last;
		last = node;
	    }
	}
    }
    return(ret);
}

/**
 * GXxmlNodeListGetString:
 * @doc:  the document
 * @list:  a Node list
 * @inLine:  should we replace entity contents or show their external form
 *
 * Returns the string equivalent to the text contained in the Node list
 * made of TEXTs and ENTITY_REFs
 * return values: a pointer to the string copy, the calller must GxCore_Free it.
 */
CHAR *
GXxmlNodeListGetString(GXxmlDocPtr doc, GXxmlNodePtr list, int inLine) {
    GXxmlNodePtr node = list;
    CHAR *ret = NULL;
    GXxmlEntityPtr ent;

    if (list == NULL) return(NULL);

    while (node != NULL) {
        if (node->type == XML_TEXT_NODE) {
	    if (inLine)
		ret = GXxmlStrcat(ret, node->content);
	    else
		ret = GXxmlStrcat(ret, GXxmlEncodeEntities(doc, node->content));
	} else if (node->type == XML_ENTITY_REF_NODE) {
	    if (inLine) {
		ent = GXxmlGetDocEntity(doc, node->name);
		if (ent != NULL)
		    ret = GXxmlStrcat(ret, ent->content);
		else
		    ret = GXxmlStrcat(ret, node->content);
            } else {
	        CHAR buf[2];
		buf[0] = '&'; buf[1] = 0;
		ret = GXxmlStrncat(ret, buf, 1);
		ret = GXxmlStrcat(ret, node->name);
		buf[0] = ';'; buf[1] = 0;
		ret = GXxmlStrncat(ret, buf, 1);
	    }
	}
#if 0
	else {
	    fprintf(stderr, "GXxmlGetNodeListString : invalide node type %d\n",
	            node->type);
	}
#endif
	node = node->next;
    }
    return(ret);
}

/**
 * GXxmlNewProp:
 * @node:  the holding node
 * @name:  the name of the attribute
 * @value:  the value of the attribute
 *
 * Create a new property carried by a node.
 * return values: a pointer to the attribute
 */
GXxmlAttrPtr
GXxmlNewProp(GXxmlNodePtr node, const CHAR *name, const CHAR *value) {
    GXxmlAttrPtr cur;

    if (name == NULL) {
        fprintf(stderr, "GXxmlNewProp : name == NULL\n");
	return(NULL);
    }

    /*
     * Allocate a new property and fill the fields.
     */
    cur = (GXxmlAttrPtr) GxCore_Malloc(sizeof(GXxmlAttr));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewProp : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_ATTRIBUTE_NODE;
    cur->node = node; 
    cur->name = GXxmlStrdup(name);
    if (value != NULL)
	cur->val = GXxmlStringGetNodeList(node->doc, value);
    else 
	cur->val = NULL;
#ifndef WITHOUT_CORBA
    cur->_private = NULL;
    cur->vepv = NULL;
#endif

    /*
     * Add it at the end to preserve parsing order ...
     */
    cur->next = NULL;
    if (node != NULL) {
	if (node->properties == NULL) {
	    node->properties = cur;
	} else {
	    GXxmlAttrPtr prev = node->properties;

	    while (prev->next != NULL) prev = prev->next;
	    prev->next = cur;
	}
    }
    return(cur);
}

/**
 * GXxmlNewDocProp:
 * @doc:  the document
 * @name:  the name of the attribute
 * @value:  the value of the attribute
 *
 * Create a new property carried by a document.
 * return values: a pointer to the attribute
 */
GXxmlAttrPtr
GXxmlNewDocProp(GXxmlDocPtr doc, const CHAR *name, const CHAR *value) {
    GXxmlAttrPtr cur;

    if (name == NULL) {
        fprintf(stderr, "GXxmlNewProp : name == NULL\n");
	return(NULL);
    }

    /*
     * Allocate a new property and fill the fields.
     */
    cur = (GXxmlAttrPtr) GxCore_Malloc(sizeof(GXxmlAttr));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewProp : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_ATTRIBUTE_NODE;
    cur->node = NULL; 
    cur->name = GXxmlStrdup(name);
    if (value != NULL)
	cur->val = GXxmlStringGetNodeList(doc, value);
    else 
	cur->val = NULL;
#ifndef WITHOUT_CORBA
    cur->_private = NULL;
    cur->vepv = NULL;
#endif

    cur->next = NULL;
    return(cur);
}

/**
 * GXxmlFreePropList:
 * @cur:  the first property in the list
 *
 * GxCore_Free a property and all its siblings, all the childs are freed too.
 */
void
GXxmlFreePropList(GXxmlAttrPtr cur) {
    GXxmlAttrPtr next;
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreePropList : property == NULL\n");
	return;
    }
    while (cur != NULL) {
        next = cur->next;
        GXxmlFreeProp(cur);
	cur = next;
    }
}

/**
 * GXxmlFreeProp:
 * @cur:  the first property in the list
 *
 * GxCore_Free one property, all the childs are freed too.
 */
void
GXxmlFreeProp(GXxmlAttrPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreeProp : property == NULL\n");
	return;
    }
    if (cur->name != NULL) GxCore_Free((char *) cur->name);
    if (cur->val != NULL) GXxmlFreeNodeList(cur->val);
    memset(cur, -1, sizeof(GXxmlAttr));
    GxCore_Free(cur);
}

/**
 * GXxmlNewNode:
 * @ns:  namespace if any
 * @name:  the node name
 * @content:  the text content if any
 *
 * Creation of a new node element. @ns and @content are optionnal (NULL).
 * If content is non NULL, a child list containing the TEXTs and
 * ENTITY_REFs node will be created.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewNode(GXxmlNsPtr ns, const CHAR *name) {
    GXxmlNodePtr cur;

    if (name == NULL) {
        fprintf(stderr, "GXxmlNewNode : name == NULL\n");
	return(NULL);
    }

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (GXxmlNodePtr) GxCore_Malloc(sizeof(GXxmlNode));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewNode : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_ELEMENT_NODE;
    cur->doc = NULL;
    cur->parent = NULL; 
    cur->next = NULL;
    cur->prev = NULL;
    cur->childs = NULL;
    cur->properties = NULL;
    cur->name = GXxmlStrdup(name);
    cur->ns = ns;
    cur->nsDef = NULL;
    cur->content = NULL;
#ifndef WITHOUT_CORBA
    cur->_private = NULL;
    cur->vepv = NULL;
#endif
    return(cur);
}

/**
 * GXxmlNewDocNode:
 * @doc:  the document
 * @ns:  namespace if any
 * @name:  the node name
 * @content:  the text content if any
 *
 * Creation of a new node element within a document. @ns and @content
 * are optionnal (NULL).
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewDocNode(GXxmlDocPtr doc, GXxmlNsPtr ns,
                         const CHAR *name, CHAR *content) {
    GXxmlNodePtr cur;

    cur = GXxmlNewNode(ns, name);
    if (cur != NULL) {
        cur->doc = doc;
	if (content != NULL)
	    cur->childs = GXxmlStringGetNodeList(doc, content);
    }
    return(cur);
}


/**
 * GXxmlNewText:
 * @content:  the text content
 *
 * Creation of a new text node.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewText(const CHAR *content) {
    GXxmlNodePtr cur;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (GXxmlNodePtr) GxCore_Malloc(sizeof(GXxmlNode));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewText : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_TEXT_NODE;
    cur->doc = NULL;
    cur->parent = NULL; 
    cur->next = NULL; 
    cur->prev = NULL; 
    cur->childs = NULL; 
    cur->properties = NULL; 
    cur->type = XML_TEXT_NODE;
    cur->name = GXxmlStrdup(GXxmlStringText);
    cur->ns = NULL;
    cur->nsDef = NULL;
    if (content != NULL)
	cur->content = GXxmlStrdup(content);
    else 
	cur->content = NULL;
    return(cur);
}

/**
 * GXxmlNewReference:
 * @doc: the document
 * @name:  the reference name, or the reference string with & and ;
 *
 * Creation of a new reference node.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewReference(GXxmlDocPtr doc, const CHAR *name) {
    GXxmlNodePtr cur;
    GXxmlEntityPtr ent;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (GXxmlNodePtr) GxCore_Malloc(sizeof(GXxmlNode));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewText : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_ENTITY_REF_NODE;
    cur->doc = doc;
    cur->parent = NULL; 
    cur->next = NULL; 
    cur->prev = NULL; 
    cur->childs = NULL; 
    cur->properties = NULL; 
    if (name[0] == '&') {
        int len;
        name++;
	len = GXxmlStrlen(name);
	if (name[len - 1] == ';')
	    cur->name = GXxmlStrndup(name, len - 1);
	else
	    cur->name = GXxmlStrndup(name, len);
    } else
	cur->name = GXxmlStrdup(name);
    cur->ns = NULL;
    cur->nsDef = NULL;

    ent = GXxmlGetDocEntity(doc, cur->name);
    if (ent != NULL)
	cur->content = ent->content;
    else
        cur->content = NULL;
    return(cur);
}

/**
 * GXxmlNewDocText:
 * @doc: the document
 * @content:  the text content
 *
 * Creation of a new text node within a document.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewDocText(GXxmlDocPtr doc, const CHAR *content) {
    GXxmlNodePtr cur;

    cur = GXxmlNewText(content);
    if (cur != NULL) cur->doc = doc;
    return(cur);
}

/**
 * GXxmlNewTextLen:
 * @content:  the text content
 * @len:  the text len.
 *
 * Creation of a new text node with an extra parameter for the content's lenght
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewTextLen(const CHAR *content, int len) {
    GXxmlNodePtr cur;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (GXxmlNodePtr) GxCore_Malloc(sizeof(GXxmlNode));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewText : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_TEXT_NODE;
    cur->doc = NULL; 
    cur->parent = NULL; 
    cur->prev = NULL; 
    cur->next = NULL; 
    cur->childs = NULL; 
    cur->properties = NULL; 
    cur->type = XML_TEXT_NODE;
    cur->name = GXxmlStrdup(GXxmlStringText);
    cur->ns = NULL;
    cur->nsDef = NULL;
    if (content != NULL)
	cur->content = GXxmlStrndup(content, len);
    else 
	cur->content = NULL;
    return(cur);
}

/**
 * GXxmlNewDocTextLen:
 * @doc: the document
 * @content:  the text content
 * @len:  the text len.
 *
 * Creation of a new text node with an extra content lenght parameter. The
 * text node pertain to a given document.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewDocTextLen(GXxmlDocPtr doc, const CHAR *content, int len) {
    GXxmlNodePtr cur;

    cur = GXxmlNewTextLen(content, len);
    if (cur != NULL) cur->doc = doc;
    return(cur);
}

/**
 * GXxmlNewComment:
 * @content:  the comment content
 *
 * Creation of a new node containing a comment.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewComment(CHAR *content) {
    GXxmlNodePtr cur;

    /*
     * Allocate a new node and fill the fields.
     */
    cur = (GXxmlNodePtr) GxCore_Malloc(sizeof(GXxmlNode));
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNewComment : GxCore_Malloc failed\n");
	return(NULL);
    }

    cur->type = XML_COMMENT_NODE;
    cur->doc = NULL; 
    cur->parent = NULL; 
    cur->prev = NULL; 
    cur->next = NULL; 
    cur->childs = NULL; 
    cur->properties = NULL; 
    cur->type = XML_COMMENT_NODE;
    cur->name = GXxmlStrdup(GXxmlStringText);
    cur->ns = NULL;
    cur->nsDef = NULL;
    if (content != NULL)
	cur->content = GXxmlStrdup(content);
    else 
	cur->content = NULL;
    return(cur);
}

/**
 * GXxmlNewComment:
 * @doc:  the document
 * @content:  the comment content
 *
 * Creation of a new node containing a commentwithin a document.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewDocComment(GXxmlDocPtr doc, CHAR *content) {
    GXxmlNodePtr cur;

    cur = GXxmlNewComment(content);
    if (cur != NULL) cur->doc = doc;
    return(cur);
}

/**
 * GXxmlNewChild:
 * @parent:  the parent node
 * @ns:  a namespace if any
 * @name:  the name of the child
 * @content:  the content of the child if any.
 *
 * 
 * Creation of a new child element, added at the end of @parent childs list.
 * @ns and @content parameters are optionnal (NULL). If content is non NULL,
 * a child list containing the TEXTs and ENTITY_REFs node will be created.
 * return values: a pointer to the new node object.
 */
GXxmlNodePtr
GXxmlNewChild(GXxmlNodePtr parent, GXxmlNsPtr ns,
                       const CHAR *name, CHAR *content) {
    GXxmlNodePtr cur, prev;

    if (parent == NULL) {
        fprintf(stderr, "GXxmlNewChild : parent == NULL\n");
	return(NULL);
    }

    if (name == NULL) {
        fprintf(stderr, "GXxmlNewChild : name == NULL\n");
	return(NULL);
    }

    /*
     * Allocate a new node
     */
    if (ns == NULL)
	cur = GXxmlNewDocNode(parent->doc, parent->ns, name, content);
    else
	cur = GXxmlNewDocNode(parent->doc, ns, name, content);
    if (cur == NULL) return(NULL);

    /*
     * add the new element at the end of the childs list.
     */
    cur->type = XML_ELEMENT_NODE;
    cur->parent = parent;
    cur->doc = parent->doc;
    if (parent->childs == NULL) {
        parent->childs = cur;
    } else {
        prev = parent->childs;
	while (prev->next != NULL) prev = prev->next;
	prev->next = cur;
	cur->prev = prev;
    }

    return(cur);
}

/**
 * GXxmlAddChild:
 * @parent:  the parent node
 * @cur:  the child node
 *
 * Add a new child element, to @parent, at the end of the child list.
 * return values: the child or NULL in case of error.
 */
GXxmlNodePtr
GXxmlAddChild(GXxmlNodePtr parent, GXxmlNodePtr cur) {
    GXxmlNodePtr prev;

    if (parent == NULL) {
        fprintf(stderr, "GXxmladdChild : parent == NULL\n");
	return(NULL);
    }

    if (cur == NULL) {
        fprintf(stderr, "GXxmladdChild : child == NULL\n");
	return(NULL);
    }

    if ((cur->doc != NULL) && (parent->doc != NULL) &&
        (cur->doc != parent->doc)) {
	fprintf(stderr, "Elements moved to a different document\n");
    }

    /*
     * add the new element at the end of the childs list.
     */
    cur->parent = parent;
    cur->doc = parent->doc; /* the parent may not be linked to a doc ! */
    /*
     * Handle the case where parent->content != NULL, in that case it will
     * create a intermediate TEXT node.
     */
    if (parent->content != NULL) {
        GXxmlNodePtr text;
	
	text = GXxmlNewDocText(parent->doc, parent->content);
	if (text != NULL) {
	    text->next = parent->childs;
	    if (text->next != NULL)
		text->next->prev = text;
	    parent->childs = text;
	    GxCore_Free(parent->content);
	    parent->content = NULL;
	}
    }
    if (parent->childs == NULL) {
        parent->childs = cur;
    } else {
        prev = parent->childs;
	while (prev->next != NULL) prev = prev->next;
	prev->next = cur;
	cur->prev = prev;
    }

    return(cur);
}

/**
 * GXxmlGetLastChild:
 * @parent:  the parent node
 *
 * Search the last child of a node.
 * return values: the last child or NULL if none.
 */
GXxmlNodePtr
GXxmlGetLastChild(GXxmlNodePtr parent) {
    GXxmlNodePtr last;

    if (parent == NULL) {
        fprintf(stderr, "GXxmlGetLastChild : parent == NULL\n");
	return(NULL);
    }

    /*
     * add the new element at the end of the childs list.
     */
    if (parent->childs == NULL) {
        return(NULL);
    } else {
        last = parent->childs;
	while (last->next != NULL) last = last->next;
    }
    return(last);
}

/**
 * GXxmlFreeNodeList:
 * @cur:  the first node in the list
 *
 * GxCore_Free a node and all its siblings, this is a recursive behaviour, all
 * the childs are freed too.
 */
void
GXxmlFreeNodeList(GXxmlNodePtr cur) {
    GXxmlNodePtr next;
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreeNodeList : node == NULL\n");
	return;
    }
    while (cur != NULL) {
        next = cur->next;
        GXxmlFreeNode(cur);
	cur = next;
    }
}

/**
 * GXxmlFreeNode:
 * @cur:  the node
 *
 * GxCore_Free a node, this is a recursive behaviour, all the childs are freed too.
 */
void
GXxmlFreeNode(GXxmlNodePtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlFreeNode : node == NULL\n");
	return;
    }
    cur->doc = NULL;
    cur->parent = NULL;
    cur->next = NULL;
    cur->prev = NULL;
    if (cur->childs != NULL) GXxmlFreeNodeList(cur->childs);
    if (cur->properties != NULL) GXxmlFreePropList(cur->properties);
    if (cur->type != XML_ENTITY_REF_NODE)
	if (cur->content != NULL) GxCore_Free(cur->content);
    if (cur->name != NULL) GxCore_Free((char *) cur->name);
    if (cur->nsDef != NULL) GXxmlFreeNsList(cur->nsDef);
    memset(cur, -1, sizeof(GXxmlNode));
    GxCore_Free(cur);
}

/**
 * GXxmlUnlinkNode:
 * @cur:  the node
 *
 * Unlink a node from it's current context, the node is not freed
 */
void
GXxmlUnlinkNode(GXxmlNodePtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlUnlinkNode : node == NULL\n");
	return;
    }
    if ((cur->parent != NULL) && (cur->parent->childs == cur))
        cur->parent->childs = cur->next;
    if (cur->next != NULL)
        cur->next->prev = cur->prev;
    if (cur->prev != NULL)
        cur->prev->next = cur->next;
    cur->next = cur->prev = NULL;
    cur->parent = NULL;
}

/************************************************************************
 *									*
 *		Copy operations						*
 *									*
 ************************************************************************/
 
/**
 * GXxmlCopyNamespace:
 * @cur:  the namespace
 *
 * Do a copy of the namespace.
 *
 * Returns: a new GXxmlNsPtr, or NULL in case of error.
 */
GXxmlNsPtr
GXxmlCopyNamespace(GXxmlNsPtr cur) {
    GXxmlNsPtr ret;

    if (cur == NULL) return(NULL);
    switch (cur->type) {
        case XML_GLOBAL_NAMESPACE:
	    ret = GXxmlNewGlobalNs(NULL, cur->href, cur->prefix);
	    break;
	case XML_LOCAL_NAMESPACE:
	    ret = GXxmlNewNs(NULL, cur->href, cur->prefix);
	    break;
	default:
	    fprintf(stderr, "GXxmlCopyNamespace: unknown type %d\n", cur->type);
	    return(NULL);
    }
    return(ret);
}

/**
 * GXxmlCopyNamespaceList:
 * @cur:  the first namespace
 *
 * Do a copy of an namespace list.
 *
 * Returns: a new GXxmlNsPtr, or NULL in case of error.
 */
GXxmlNsPtr
GXxmlCopyNamespaceList(GXxmlNsPtr cur) {
    GXxmlNsPtr ret = NULL;
    GXxmlNsPtr p = NULL,q;

    while (cur != NULL) {
        q = GXxmlCopyNamespace(cur);
	if (p == NULL) {
	    ret = p = q;
	} else {
	    p->next = q;
	    p = q;
	}
	cur = cur->next;
    }
    return(ret);
}

/**
 * GXxmlCopyProp:
 * @cur:  the attribute
 *
 * Do a copy of the attribute.
 *
 * Returns: a new GXxmlAttrPtr, or NULL in case of error.
 */
GXxmlAttrPtr
GXxmlCopyProp(GXxmlAttrPtr cur) {
    GXxmlAttrPtr ret;

    if (cur == NULL) return(NULL);
    if (cur->val != NULL)
	ret = GXxmlNewDocProp(cur->val->doc, cur->name, NULL);
    else
	ret = GXxmlNewDocProp(NULL, cur->name, NULL);
    if (ret == NULL) return(NULL);
    if (cur->val != NULL)
	ret->val = GXxmlCopyNodeList(cur->val);
    return(ret);
}

/**
 * GXxmlCopyPropList:
 * @cur:  the first attribute
 *
 * Do a copy of an attribute list.
 *
 * Returns: a new GXxmlAttrPtr, or NULL in case of error.
 */
GXxmlAttrPtr
GXxmlCopyPropList(GXxmlAttrPtr cur) {
    GXxmlAttrPtr ret = NULL;
    GXxmlAttrPtr p = NULL,q;

    while (cur != NULL) {
        q = GXxmlCopyProp(cur);
	if (p == NULL) {
	    ret = p = q;
	} else {
	    p->next = q;
	    p = q;
	}
	cur = cur->next;
    }
    return(ret);
}

/*
 * NOTE about the CopyNode operations !
 *
 * They are splitted into external and internal parts for one
 * tricky reason: namespaces. Doing a direct copy of a node
 * say RPM:Copyright without changing the namespace pointer to
 * something else can produce stale links. One way to do it is
 * to keep a reference counter but this doesn't work as soon
 * as one move the element or the subtree out of the scope of
 * the existing namespace. The actual solution seems to add
 * a copy of the namespace at the top of the copied tree if
 * not available in the subtree.
 * Hence two functions, the public front-end call the inner ones
 */

static GXxmlNodePtr
GXxmlStaticCopyNodeList(GXxmlNodePtr node, GXxmlDocPtr doc, GXxmlNodePtr parent);

static GXxmlNodePtr
GXxmlStaticCopyNode(GXxmlNodePtr node, GXxmlDocPtr doc, GXxmlNodePtr parent,
                  int recursive) {
    GXxmlNodePtr ret;

    if (node == NULL) return(NULL);
    /*
     * Allocate a new node and fill the fields.
     */
    ret = (GXxmlNodePtr) GxCore_Malloc(sizeof(GXxmlNode));
    if (ret == NULL) {
        fprintf(stderr, "GXxmlStaticCopyNode : GxCore_Malloc failed\n");
	return(NULL);
    }

    ret->type = node->type;
    ret->doc = doc;
    ret->parent = parent; 
    ret->next = NULL;
    ret->prev = NULL;
    ret->childs = NULL;
    ret->properties = NULL;
    if (node->name != NULL)
	ret->name = GXxmlStrdup(node->name);
    else
        ret->name = NULL;
    ret->ns = NULL;
    ret->nsDef = NULL;
    if ((node->content != NULL) && (node->type != XML_ENTITY_REF_NODE))
	ret->content = GXxmlStrdup(node->content);
    else
	ret->content = NULL;
#ifndef WITHOUT_CORBA
    ret->_private = NULL;
    ret->vepv = NULL;
#endif
    if (parent != NULL)
        GXxmlAddChild(parent, ret);
    
    if (!recursive) return(ret);
    if (node->properties != NULL)
        ret->properties = GXxmlCopyPropList(node->properties);
    if (node->nsDef != NULL)
        ret->nsDef = GXxmlCopyNamespaceList(node->nsDef);

    if (node->ns != NULL) {
        GXxmlNsPtr ns;

	ns = GXxmlSearchNs(doc, ret, node->ns->prefix);
	if (ns == NULL) {
	    /*
	     * Humm, we are copying an element whose namespace is defined
	     * out of the new tree scope. Search it in the original tree
	     * and add it at the top of the new tree
	     */
	    ns = GXxmlSearchNs(node->doc, node, node->ns->prefix);
	    if (ns != NULL) {
	        GXxmlNodePtr root = ret;

		while (root->parent != NULL) root = root->parent;
		GXxmlNewNs(root, ns->href, ns->prefix);
	    }
	} else {
	    /*
	     * reference the existing namespace definition in our own tree.
	     */
	    ret->ns = ns;
	}
    }
    if (node->childs != NULL)
        ret->childs = GXxmlStaticCopyNodeList(node->childs, doc, ret);
    return(ret);
}

static GXxmlNodePtr
GXxmlStaticCopyNodeList(GXxmlNodePtr node, GXxmlDocPtr doc, GXxmlNodePtr parent) {
    GXxmlNodePtr ret = NULL;
    GXxmlNodePtr p = NULL,q;

    while (node != NULL) {
        q = GXxmlStaticCopyNode(node, doc, parent, 1);
	if (parent == NULL) {
	    if (ret == NULL) ret = q;
	} else {
	    if (ret == NULL) {
		q->prev = NULL;
		ret = p = q;
	    } else {
		p->next = q;
		q->prev = p;
		p = q;
	    }
	}
	node = node->next;
    }
    return(ret);
}

/**
 * GXxmlCopyNode:
 * @node:  the node
 * @recursive:  if 1 do a recursive copy.
 *
 * Do a copy of the node.
 *
 * Returns: a new GXxmlNodePtr, or NULL in case of error.
 */
GXxmlNodePtr
GXxmlCopyNode(GXxmlNodePtr node, int recursive) {
    GXxmlNodePtr ret;

    ret = GXxmlStaticCopyNode(node, NULL, NULL, recursive);
    return(ret);
}

/**
 * GXxmlCopyNodeList:
 * @node:  the first node in the list.
 *
 * Do a recursive copy of the node list.
 *
 * Returns: a new GXxmlNodePtr, or NULL in case of error.
 */
GXxmlNodePtr GXxmlCopyNodeList(GXxmlNodePtr node) {
    GXxmlNodePtr ret = GXxmlStaticCopyNodeList(node, NULL, NULL);
    return(ret);
}

/**
 * GXxmlCopyElement:
 * @elem:  the element
 *
 * Do a copy of the element definition.
 *
 * Returns: a new GXxmlElementPtr, or NULL in case of error.
GXxmlElementPtr
GXxmlCopyElement(GXxmlElementPtr elem) {
    GXxmlElementPtr ret;

    if (elem == NULL) return(NULL);
    ret = GXxmlNewDocElement(elem->doc, elem->ns, elem->name, elem->content);
    if (ret == NULL) return(NULL);
    if (!recursive) return(ret);
    if (elem->properties != NULL)
        ret->properties = GXxmlCopyPropList(elem->properties);
    
    if (elem->nsDef != NULL)
        ret->nsDef = GXxmlCopyNamespaceList(elem->nsDef);
    if (elem->childs != NULL)
        ret->childs = GXxmlCopyElementList(elem->childs);
    return(ret);
}
 */

/**
 * GXxmlCopyDtd:
 * @dtd:  the dtd
 *
 * Do a copy of the dtd.
 *
 * Returns: a new GXxmlDtdPtr, or NULL in case of error.
 */
GXxmlDtdPtr
GXxmlCopyDtd(GXxmlDtdPtr dtd) {
    GXxmlDtdPtr ret;

    if (dtd == NULL) return(NULL);
    ret = GXxmlNewDtd(NULL, dtd->name, dtd->ExternalID, dtd->SystemID);
    if (ret == NULL) return(NULL);
    if (dtd->entities != NULL)
        ret->entities = (void *) GXxmlCopyEntitiesTable(
	                    (GXxmlEntitiesTablePtr) dtd->entities);
    /*
     * TODO: support for Element definitions.
     */
    return(ret);
}

/**
 * GXxmlCopyDoc:
 * @doc:  the document
 * @recursive:  if 1 do a recursive copy.
 *
 * Do a copy of the document info. If recursive, the content tree will
 * be copied too as well as Dtd, namespaces and entities.
 *
 * Returns: a new GXxmlDocPtr, or NULL in case of error.
 */
GXxmlDocPtr
GXxmlCopyDoc(GXxmlDocPtr doc, int recursive) {
    GXxmlDocPtr ret;

    if (doc == NULL) return(NULL);
    ret = GXxmlNewDoc(doc->version);
    if (ret == NULL) return(NULL);
    if (doc->name != NULL)
        ret->name = GxCore_Strdup(doc->name);
    if (doc->encoding != NULL)
        ret->encoding = GXxmlStrdup(doc->encoding);
    ret->compression = doc->compression;
    ret->standalone = doc->standalone;
    if (!recursive) return(ret);

    if (doc->dtd != NULL)
        ret->dtd = GXxmlCopyDtd(doc->dtd);
    if (doc->entities != NULL)
        ret->entities = (void *) GXxmlCopyEntitiesTable(
	                    (GXxmlEntitiesTablePtr) doc->entities);
    if (doc->oldNs != NULL)
        ret->oldNs = GXxmlCopyNamespaceList(doc->oldNs);
    if (doc->root != NULL)
        ret->root = GXxmlStaticCopyNodeList(doc->root, ret, NULL);
    return(ret);
}

/************************************************************************
 *									*
 *		Content access functions				*
 *									*
 ************************************************************************/
 
/**
 * GXxmlNodeGetContent:
 * @cur:  the node being read
 *
 * Read the value of a node, this can be either the text carried
 * directly by this node if it's a TEXT node or the aggregate string
 * of the values carried by this node child's (TEXT and ENTITY_REF).
 * Entity references are substitued.
 * Return value: a new CHAR * or NULL if no content is available.
 */
CHAR *
GXxmlNodeGetContent(GXxmlNodePtr cur) {
    if (cur == NULL) return(NULL);
    switch (cur->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE:
            return(GXxmlNodeListGetString(cur->doc, cur->childs, 1));
	    break;
        case XML_ATTRIBUTE_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
        case XML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
        case XML_NOTATION_NODE:
	    return(NULL);
        case XML_CDATA_SECTION_NODE:
        case XML_TEXT_NODE:
	    if (cur->content != NULL)
		return(GXxmlStrdup(cur->content));
            return(NULL);
    }
    return(NULL);
}
 
/**
 * GXxmlNodeSetContent:
 * @cur:  the node being modified
 * @content:  the new value of the content
 *
 * Replace the content of a node.
 */
void
GXxmlNodeSetContent(GXxmlNodePtr cur, const CHAR *content) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNodeSetContent : node == NULL\n");
	return;
    }
    switch (cur->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE:
	    if (cur->content != NULL) {
	        GxCore_Free(cur->content);
		cur->content = NULL;
	    }
	    if (cur->childs != NULL) GXxmlFreeNode(cur->childs);
	    cur->childs = GXxmlStringGetNodeList(cur->doc, content);
	    break;
        case XML_ATTRIBUTE_NODE:
	    break;
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
	    if (cur->content != NULL) GxCore_Free(cur->content);
	    if (cur->childs != NULL) GXxmlFreeNode(cur->childs);
	    if (content != NULL)
		cur->content = GXxmlStrdup(content);
	    else 
		cur->content = NULL;
        case XML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
	    break;
        case XML_NOTATION_NODE:
	    break;
    }
}

/**
 * GXxmlNodeSetContentLen:
 * @cur:  the node being modified
 * @content:  the new value of the content
 * @len:  the size of @content
 *
 * Replace the content of a node.
 */
void
GXxmlNodeSetContentLen(GXxmlNodePtr cur, const CHAR *content, int len) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNodeSetContentLen : node == NULL\n");
	return;
    }
    switch (cur->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE:
	    if (cur->content != NULL) {
	        GxCore_Free(cur->content);
		cur->content = NULL;
	    }
	    if (cur->childs != NULL) GXxmlFreeNode(cur->childs);
	    cur->childs = GXxmlStringLenGetNodeList(cur->doc, content, len);
	    break;
        case XML_ATTRIBUTE_NODE:
	    break;
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
	    if (cur->content != NULL) GxCore_Free(cur->content);
	    if (cur->childs != NULL) GXxmlFreeNode(cur->childs);
	    if (content != NULL)
		cur->content = GXxmlStrndup(content, len);
	    else 
		cur->content = NULL;
        case XML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
	    break;
        case XML_NOTATION_NODE:
	    if (cur->content != NULL) GxCore_Free(cur->content);
	    if (content != NULL)
		cur->content = GXxmlStrndup(content, len);
	    else 
		cur->content = NULL;
	    break;
    }
}

/**
 * GXxmlNodeAddContentLen:
 * @cur:  the node being modified
 * @content:  extra content
 * @len:  the size of @content
 * 
 * Append the extra substring to the node content.
 */
void
GXxmlNodeAddContentLen(GXxmlNodePtr cur, const CHAR *content, int len) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNodeAddContentLen : node == NULL\n");
	return;
    }
    if (len <= 0) return;
    switch (cur->type) {
        case XML_DOCUMENT_FRAG_NODE:
        case XML_ELEMENT_NODE: {
	    GXxmlNodePtr last = NULL, new;

	    if (cur->childs != NULL) {
		last = cur->childs;
		while (last->next != NULL) last = last->next;
	    } else {
	        if (cur->content != NULL) {
		    cur->childs = GXxmlStringGetNodeList(cur->doc, cur->content);
		    GxCore_Free(cur->content);
		    cur->content = NULL;
		    if (cur->childs != NULL) {
			last = cur->childs;
			while (last->next != NULL) last = last->next;
                    }
		}
	    }
	    new = GXxmlStringLenGetNodeList(cur->doc, content, len);
	    if (new != NULL) {
		GXxmlAddChild(cur, new);
	        if ((last != NULL) && (last->next == new))
		    GXxmlTextMerge(last, new);
	    }
	    break;
	}
        case XML_ATTRIBUTE_NODE:
	    break;
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_ENTITY_NODE:
        case XML_PI_NODE:
        case XML_COMMENT_NODE:
	    if (content != NULL)
		cur->content = GXxmlStrncat(cur->content, content, len);
        case XML_DOCUMENT_NODE:
        case XML_DOCUMENT_TYPE_NODE:
	    break;
        case XML_NOTATION_NODE:
	    if (content != NULL)
		cur->content = GXxmlStrncat(cur->content, content, len);
	    break;
    }
}

/**
 * GXxmlNodeAddContent:
 * @cur:  the node being modified
 * @content:  extra content
 * 
 * Append the extra substring to the node content.
 */
void
GXxmlNodeAddContent(GXxmlNodePtr cur, const CHAR *content) {
    int len;

    if (cur == NULL) {
        fprintf(stderr, "GXxmlNodeAddContent : node == NULL\n");
	return;
    }
    if (content == NULL) return;
    len = GXxmlStrlen(content);
    GXxmlNodeAddContentLen(cur, content, len);
}

/**
 * GXxmlTextMerge:
 * @first:  the first text node
 * @second:  the second text node being merged
 * 
 * Merge two text nodes into one
 * Return values: the first text node augmented
 */
GXxmlNodePtr
GXxmlTextMerge(GXxmlNodePtr first, GXxmlNodePtr second) {
    if (first == NULL) return(second);
    if (second == NULL) return(first);
    if (first->type != XML_TEXT_NODE) return(first);
    if (second->type != XML_TEXT_NODE) return(first);
    GXxmlNodeAddContent(first, second->content);
    GXxmlUnlinkNode(second);
    GXxmlFreeNode(second);
    return(first);
}

/**
 * GXxmlSearchNs:
 * @doc:  the document
 * @node:  the current node
 * @nameSpace:  the namespace string
 *
 * Search a Ns registered under a given name space for a document.
 * recurse on the parents until it finds the defined namespace
 * or return NULL otherwise.
 * @nameSpace can be NULL, this is a search for the default namespace.
 * return values: the namespace pointer or NULL.
 */
GXxmlNsPtr
GXxmlSearchNs(GXxmlDocPtr doc, GXxmlNodePtr node, const CHAR *nameSpace) {
    GXxmlNsPtr cur;

    while (node != NULL) {
	cur = node->nsDef;
	while (cur != NULL) {
	    if ((cur->prefix == NULL) && (nameSpace == NULL))
	        return(cur);
	    if ((cur->prefix != NULL) && (nameSpace != NULL) &&
	        (!GXxmlStrcmp(cur->prefix, nameSpace)))
		return(cur);
	    cur = cur->next;
	}
	node = node->parent;
    }
    if (doc != NULL) {
        cur = doc->oldNs;
	while (cur != NULL) {
	    if ((cur->prefix != NULL) && (nameSpace != NULL) &&
	        (!GXxmlStrcmp(cur->prefix, nameSpace)))
		return(cur);
	    cur = cur->next;
	}
    }
    return(NULL);
}

/**
 * GXxmlSearchNsByHref:
 * @doc:  the document
 * @node:  the current node
 * @href:  the namespace value
 *
 * Search a Ns aliasing a given URI. Recurse on the parents until it finds
 * the defined namespace or return NULL otherwise.
 * return values: the namespace pointer or NULL.
 */
GXxmlNsPtr
GXxmlSearchNsByHref(GXxmlDocPtr doc, GXxmlNodePtr node, const CHAR *href) {
    GXxmlNsPtr cur;

    while (node != NULL) {
	cur = node->nsDef;
	while (cur != NULL) {
	    if ((cur->href != NULL) && (href != NULL) &&
	        (!GXxmlStrcmp(cur->href, href)))
		return(cur);
	    cur = cur->next;
	}
	node = node->parent;
    }
    if (doc != NULL) {
        cur = doc->oldNs;
	while (cur != NULL) {
	    if ((cur->href != NULL) && (href != NULL) &&
	        (!GXxmlStrcmp(cur->href, href)))
		return(cur);
	    cur = cur->next;
	}
    }
    return(NULL);
}

/**
 * GXxmlGetProp:
 * @node:  the node
 * @name:  the attribute name
 *
 * Search and get the value of an attribute associated to a node
 * This does the entity substitution.
 *
 * return values: the attribute value or NULL if not found.
 */
CHAR *GXxmlGetProp(GXxmlNodePtr node, const CHAR *name) {
    GXxmlAttrPtr prop = node->properties;

    while (prop != NULL) {
        if (!GXxmlStrcmp(prop->name, name)) {
	    if (prop->val != NULL)
		return GXxmlNodeListGetString(node->doc, prop->val, 1);
	    else
	        return GXxmlStrndup((CHAR*)"", 1);
	}
	prop = prop->next;
    }
    return(NULL);
}

/**
 * GXxmlSetProp:
 * @node:  the node
 * @name:  the attribute name
 * @value:  the attribute value
 *
 * Set (or reset) an attribute carried by a node.
 * return values: the attribute pointer.
 */
GXxmlAttrPtr
GXxmlSetProp(GXxmlNodePtr node, const CHAR *name, const CHAR *value) {
    GXxmlAttrPtr prop = node->properties;

    while (prop != NULL) {
        if (!GXxmlStrcmp(prop->name, name)) {
	    if (prop->val != NULL) 
	        GXxmlFreeNode(prop->val);
	    prop->val = NULL;
	    if (value != NULL)
		prop->val = GXxmlStringGetNodeList(node->doc, value);
	    return(prop);
	}
	prop = prop->next;
    }
    prop = GXxmlNewProp(node, name, value);
    return(prop);
}

/**
 * GXxmlNodeIsText:
 * @node:  the node
 * 
 * Is this node a Text node ?
 * return values: 1 yes, 0 no
 */
int
GXxmlNodeIsText(GXxmlNodePtr node) {
    if (node == NULL) return(0);

    if (node->type == XML_TEXT_NODE) return(1);
    return(0);
}

/**
 * GXxmlNodeIsText:
 * @node:  the node
 * @content:  the content
 * @len:  @content lenght
 * 
 * Concat the given string at the end of the existing node content
 */

void
GXxmlTextConcat(GXxmlNodePtr node, const CHAR *content, int len) {
    if (node == NULL) return;

    if (node->type != XML_TEXT_NODE) {
	fprintf(stderr, "GXxmlTextConcat: node is not text\n");
        return;
    }
    node->content = GXxmlStrncat(node->content, content, len);
}

/************************************************************************
 *									*
 *			Output : to a FILE or in memory			*
 *									*
 ************************************************************************/

static CHAR *buffer = NULL;
static int buffer_index = 0;
static int buffer_size = 0;

/**
 * GXxmlBufferWriteCHAR:
 * @string:  the string to add
 *
 * routine which manage and grows an output buffer. This one add
 * CHARs at the end of the array.
 */
void
GXxmlBufferWriteCHAR(const CHAR *string) {
    const CHAR *cur;

    if (buffer == NULL) {
        buffer_size = 50000;
        buffer = (CHAR *) GxCore_Malloc(buffer_size * sizeof(CHAR));
	if (buffer == NULL) {
	    fprintf(stderr, "GXxmlBufferWrite : out of memory!\n");
	    exit(1);
	}
    }
    
    if (string == NULL) return;
    for (cur = string;*cur != 0;cur++) {
        if (buffer_index  + 10 >= buffer_size) {
	    buffer_size *= 2;
	    buffer = (CHAR *) GxCore_Realloc(buffer, buffer_size * sizeof(CHAR));
	    if (buffer == NULL) {
	        fprintf(stderr, "GXxmlBufferWrite : out of memory!\n");
		exit(1);
	    }
	}
        buffer[buffer_index++] = *cur;
    }
    buffer[buffer_index] = 0;
}

/**
 * GXxmlBufferWriteChar:
 * @string:  the string to add
 *
 * routine which manage and grows an output buffer. This one add
 * C chars at the end of the array.
 */
void
GXxmlBufferWriteChar(const char *string) {
    const char *cur;

    if (buffer == NULL) {
        buffer_size = 50000;
        buffer = (CHAR *) GxCore_Malloc(buffer_size * sizeof(CHAR));
	if (buffer == NULL) {
	    fprintf(stderr, "GXxmlBufferWrite : out of memory!\n");
	    exit(1);
	}
    }
    
    if (string == NULL) return;
    for (cur = string;*cur != 0;cur++) {
        if (buffer_index  + 10 >= buffer_size) {
	    buffer_size *= 2;
	    buffer = (CHAR *) GxCore_Realloc(buffer, buffer_size * sizeof(CHAR));
	    if (buffer == NULL) {
	        fprintf(stderr, "GXxmlBufferWrite : out of memory!\n");
		exit(1);
	    }
	}
        buffer[buffer_index++] = *cur;
    }
    buffer[buffer_index] = 0;
}

/**
 * GXxmlGlobalNsDump:
 * @cur:  a namespace
 *
 * Dump a global Namespace, this is the old version based on PIs.
 */
static void
GXxmlGlobalNsDump(GXxmlNsPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlGlobalNsDump : Ns == NULL\n");
	return;
    }
    if (cur->type == XML_GLOBAL_NAMESPACE) {
	GXxmlBufferWriteChar("<?namespace");
	if (cur->href != NULL) {
	    GXxmlBufferWriteChar(" href=\"");
	    GXxmlBufferWriteCHAR(cur->href);
	    GXxmlBufferWriteChar("\"");
	}
	if (cur->prefix != NULL) {
	    GXxmlBufferWriteChar(" AS=\"");
	    GXxmlBufferWriteCHAR(cur->prefix);
	    GXxmlBufferWriteChar("\"");
	}
	GXxmlBufferWriteChar("?>\n");
    }
}

/**
 * GXxmlGlobalNsListDump:
 * @cur:  the first namespace
 *
 * Dump a list of global Namespace, this is the old version based on PIs.
 */
static void
GXxmlGlobalNsListDump(GXxmlNsPtr cur) {
    while (cur != NULL) {
        GXxmlGlobalNsDump(cur);
	cur = cur->next;
    }
}

/**
 * GXxmlNsDump:
 * @cur:  a namespace
 *
 * Dump a local Namespace definition.
 * Should be called in the context of attributes dumps.
 */
static void
GXxmlNsDump(GXxmlNsPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlNsDump : Ns == NULL\n");
	return;
    }
    if (cur->type == XML_LOCAL_NAMESPACE) {
        /* Within the context of an element attributes */
	if (cur->prefix != NULL) {
	    GXxmlBufferWriteChar(" GXxmlns:");
	    GXxmlBufferWriteCHAR(cur->prefix);
	} else
	    GXxmlBufferWriteChar(" GXxmlns");
	GXxmlBufferWriteChar("=\"");
	GXxmlBufferWriteCHAR(cur->href);
	GXxmlBufferWriteChar("\"");
    }
}

/**
 * GXxmlNsListDump:
 * @cur:  the first namespace
 *
 * Dump a list of local Namespace definitions.
 * Should be called in the context of attributes dumps.
 */
static void
GXxmlNsListDump(GXxmlNsPtr cur) {
    while (cur != NULL) {
        GXxmlNsDump(cur);
	cur = cur->next;
    }
}

/**
 * GXxmlDtdDump:
 * @doc:  the document
 * 
 * Dump the XML document DTD, if any.
 */
static void
GXxmlDtdDump(GXxmlDocPtr doc) {
    GXxmlDtdPtr cur = doc->dtd;

    if (cur == NULL) {
        fprintf(stderr, "GXxmlDtdDump : DTD == NULL\n");
	return;
    }
    GXxmlBufferWriteChar("<!DOCTYPE ");
    GXxmlBufferWriteCHAR(cur->name);
    if (cur->ExternalID != NULL) {
	GXxmlBufferWriteChar(" PUBLIC \"");
	GXxmlBufferWriteCHAR(cur->ExternalID);
	GXxmlBufferWriteChar("\" \"");
	GXxmlBufferWriteCHAR(cur->SystemID);
	GXxmlBufferWriteChar("\"");
    }  else if (cur->SystemID != NULL) {
	GXxmlBufferWriteChar(" SYSTEM \"");
	GXxmlBufferWriteCHAR(cur->SystemID);
	GXxmlBufferWriteChar("\"");
    }
    if ((cur->entities == NULL) && (doc->entities == NULL)) {
	GXxmlBufferWriteChar(">\n");
	return;
    }
    GXxmlBufferWriteChar(" [\n");
    if (cur->entities != NULL)
	GXxmlDumpEntitiesTable((GXxmlEntitiesTablePtr) cur->entities);
    if (doc->entities != NULL)
	GXxmlDumpEntitiesTable((GXxmlEntitiesTablePtr) doc->entities);
    GXxmlBufferWriteChar("]");

    /* TODO !!! a lot more things to dump ... */
    GXxmlBufferWriteChar(">\n");
}

/**
 * GXxmlAttrDump:
 * @doc:  the document
 * @cur:  the attribute pointer
 *
 * Dump an XML attribute
 */
static void
GXxmlAttrDump(GXxmlDocPtr doc, GXxmlAttrPtr cur) {
    CHAR *value;

    if (cur == NULL) {
        fprintf(stderr, "GXxmlAttrDump : property == NULL\n");
	return;
    }
    GXxmlBufferWriteChar(" ");
    GXxmlBufferWriteCHAR(cur->name);
    GXxmlBufferWriteChar("=\"");
    value = GXxmlNodeListGetString(doc, cur->val, 0);
    if (value) {
	GXxmlBufferWriteCHAR(value);
	GxCore_Free(value);
    }
    GXxmlBufferWriteChar("\"");
}

/**
 * GXxmlAttrListDump:
 * @doc:  the document
 * @cur:  the first attribute pointer
 *
 * Dump a list of XML attributes
 */
static void
GXxmlAttrListDump(GXxmlDocPtr doc, GXxmlAttrPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlAttrListDump : property == NULL\n");
	return;
    }
    while (cur != NULL) {
        GXxmlAttrDump(doc, cur);
	cur = cur->next;
    }
}


static void
GXxmlNodeDump(GXxmlDocPtr doc, GXxmlNodePtr cur, int level);
/**
 * GXxmlNodeListDump:
 * @doc:  the document
 * @cur:  the first node
 * @level: the imbrication level for indenting
 *
 * Dump an XML node list, recursive behaviour,children are printed too.
 */
static void
GXxmlNodeListDump(GXxmlDocPtr doc, GXxmlNodePtr cur, int level) {
    int needIndent = 0, i;

    if (cur == NULL) {
        fprintf(stderr, "GXxmlNodeListDump : node == NULL\n");
	return;
    }
    while (cur != NULL) {
        if ((cur->type != XML_TEXT_NODE) &&
	    (cur->type != XML_ENTITY_REF_NODE)) {
	    if (!needIndent) {
	        needIndent = 1;
		GXxmlBufferWriteChar("\n");
	    }
	}
        GXxmlNodeDump(doc, cur, level);
	cur = cur->next;
    }
    if ((GXxmlIndentTreeOutput) && (needIndent))
	for (i = 1;i < level;i++)
	    GXxmlBufferWriteChar("  ");
}

/**
 * GXxmlNodeDump:
 * @doc:  the document
 * @cur:  the current node
 * @level: the imbrication level for indenting
 *
 * Dump an XML node, recursive behaviour,children are printed too.
 */
static void
GXxmlNodeDump(GXxmlDocPtr doc, GXxmlNodePtr cur, int level) {
    int i;

    if (cur == NULL) {
        fprintf(stderr, "GXxmlNodeDump : node == NULL\n");
	return;
    }
    if (cur->type == XML_TEXT_NODE) {
	if (cur->content != NULL)
	    GXxmlBufferWriteCHAR(GXxmlEncodeEntities(doc, cur->content));
	return;
    }
    if (cur->type == XML_COMMENT_NODE) {
	if (cur->content != NULL) {
	    GXxmlBufferWriteChar("<!--");
	    GXxmlBufferWriteCHAR(cur->content);
	    GXxmlBufferWriteChar("-->");
	}
	return;
    }
    if (cur->type == XML_ENTITY_REF_NODE) {
        GXxmlBufferWriteChar("&");
	GXxmlBufferWriteCHAR(cur->name);
        GXxmlBufferWriteChar(";");
	return;
    }
    if (GXxmlIndentTreeOutput)
	for (i = 0;i < level;i++)
	    GXxmlBufferWriteChar("  ");

    GXxmlBufferWriteChar("<");
    if ((cur->ns != NULL) && (cur->ns->prefix != NULL)) {
        GXxmlBufferWriteCHAR(cur->ns->prefix);
	GXxmlBufferWriteChar(":");
    }

    GXxmlBufferWriteCHAR(cur->name);
    if (cur->nsDef)
        GXxmlNsListDump(cur->nsDef);
    if (cur->properties != NULL)
        GXxmlAttrListDump(doc, cur->properties);

    if ((cur->content == NULL) && (cur->childs == NULL)) {
        GXxmlBufferWriteChar("/>\n");
	return;
    }
    GXxmlBufferWriteChar(">");
    if (cur->content != NULL)
	GXxmlBufferWriteCHAR(GXxmlEncodeEntities(doc, cur->content));
    if (cur->childs != NULL) {
	GXxmlNodeListDump(doc, cur->childs, level + 1);
    }
    GXxmlBufferWriteChar("</");
    if ((cur->ns != NULL) && (cur->ns->prefix != NULL)) {
        GXxmlBufferWriteCHAR(cur->ns->prefix);
	GXxmlBufferWriteChar(":");
    }

    GXxmlBufferWriteCHAR(cur->name);
    GXxmlBufferWriteChar(">\n");
}

/**
 * GXxmlDocContentDump:
 * @cur:  the document
 *
 * Dump an XML document.
 */
static void
GXxmlDocContentDump(GXxmlDocPtr cur) {
    if (GXoldXMLWDcompatibility)
	GXxmlBufferWriteChar("<?XML version=\"");
    else 
	GXxmlBufferWriteChar("<?GXxml version=\"");
    GXxmlBufferWriteCHAR(cur->version);
    GXxmlBufferWriteChar("\"");
    if (cur->encoding != NULL) {
        GXxmlBufferWriteChar(" encoding=\"");
	GXxmlBufferWriteCHAR(cur->encoding);
	GXxmlBufferWriteChar("\"");
    }
    switch (cur->standalone) {
        case 0:
	    GXxmlBufferWriteChar(" standalone=\"no\"");
	    break;
        case 1:
	    GXxmlBufferWriteChar(" standalone=\"yes\"");
	    break;
    }
    GXxmlBufferWriteChar("?>\n");
    if ((cur->dtd != NULL) || (cur->entities != NULL))
        GXxmlDtdDump(cur);
    if (cur->root != NULL) {
	/* global namespace definitions, the old way */
	if (GXoldXMLWDcompatibility)
	    GXxmlGlobalNsListDump(cur->oldNs);
	else 
	    GXxmlUpgradeOldNs(cur);
        GXxmlNodeDump(cur, cur->root, 0);
    }
}

/**
 * GXxmlDocDumpMemory:
 * @cur:  the document
 * @mem:  OUT: the memory pointer
 * @size:  OUT: the memory lenght
 *
 * Dump an XML document in memory and return the CHAR * and it's size.
 * It's up to the caller to GxCore_Free the memory.
 */
void
GXxmlDocDumpMemory(GXxmlDocPtr cur, CHAR**mem, int *size) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlDocDump : document == NULL\n");
	*mem = NULL;
	*size = 0;
	return;
    }
    buffer_index = 0;
    GXxmlDocContentDump(cur);

    *mem = buffer;
    *size = buffer_index;
}

/**
 * GXxmlGetDocCompressMode:
 * @doc:  the document
 *
 * get the compression ratio for a document, ZLIB based
 * return values: 0 (uncompressed) to 9 (max compression)
 */
int
GXxmlGetDocCompressMode (GXxmlDocPtr doc) {
    if (doc == NULL) return(-1);
    return(doc->compression);
}

/**
 * GXxmlSetDocCompressMode:
 * @doc:  the document
 * @mode:  the compression ratio
 *
 * set the compression ratio for a document, ZLIB based
 * Correct values: 0 (uncompressed) to 9 (max compression)
 */
void
GXxmlSetDocCompressMode (GXxmlDocPtr doc, int mode) {
    if (doc == NULL) return;
    if (mode < 0) doc->compression = 0;
    else if (mode > 9) doc->compression = 9;
    else doc->compression = mode;
}

/**
 * GXxmlGetCompressMode:
 *
 * get the default compression mode used, ZLIB based.
 * return values: 0 (uncompressed) to 9 (max compression)
 */
int
GXxmlGetCompressMode(void) {
    return(GXxmlCompressMode);
}

/**
 * GXxmlSetCompressMode:
 * @mode:  the compression ratio
 *
 * set the default compression mode used, ZLIB based
 * Correct values: 0 (uncompressed) to 9 (max compression)
 */
void
GXxmlSetCompressMode(int mode) {
    if (mode < 0) GXxmlCompressMode = 0;
    else if (mode > 9) GXxmlCompressMode = 9;
    else GXxmlCompressMode = mode;
}

/**
 * GXxmlDocDump:
 * @f:  the FILE*
 * @cur:  the document
 *
 * Dump an XML document to an open FILE.
 */
void
GXxmlDocDump(FILE *f, GXxmlDocPtr cur) {
    if (cur == NULL) {
        fprintf(stderr, "GXxmlDocDump : document == NULL\n");
	return;
    }
    buffer_index = 0;
    GXxmlDocContentDump(cur);

    fwrite(buffer, sizeof(CHAR), buffer_index, f);
}

/**
 * GXxmlSaveFile:
 * @filename:  the filename
 * @cur:  the document
 *
 * Dump an XML document to a file. Will use compression if
 * compiled in and enabled.
 * returns: the number of file written or -1 in case of failure.
 */
int
GXxmlSaveFile(const char *filename, GXxmlDocPtr cur) {
#ifdef HAVE_ZLIB_H
    gzFile zoutput = NULL;
    char mode[15];
#endif
    FILE *output = NULL;
    int ret;

#ifdef HAVE_ZLIB_H
    if ((cur->compression > 0) && (cur->compression <= 9)) {
        sprintf(mode, "w%d", cur->compression);
	zoutput = gzopen(filename, mode);
    }
    if (zoutput == NULL) {
#endif
        output = fopen(filename, "w");
	if (output == NULL) return(-1);
#ifdef HAVE_ZLIB_H
    }
#endif

    /* 
     * save the content to a temp buffer.
     */
    buffer_index = 0;
    GXxmlDocContentDump(cur);

#ifdef HAVE_ZLIB_H
    if (zoutput != NULL) {
        ret = gzwrite(zoutput, buffer, sizeof(CHAR) * buffer_index);
	gzclose(zoutput);
	return(ret);
    }
#endif
    ret = fwrite(buffer, sizeof(CHAR), buffer_index, output);
    fclose(output);
    return(ret * sizeof(CHAR));
}

