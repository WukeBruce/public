/*
 * entities.c : implementation for the XML entities handking
 *
 * See Copyright for the status of this software.
 *
 * $Id: entities.c,v 1.10 1998/11/27 06:39:46 veillard Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entities.h"

/*
 * The XML predefined entities.
 */

struct GXxmlPredefinedEntityValue {
    const char *name;
    const char *value;
};
struct GXxmlPredefinedEntityValue GXxmlPredefinedEntityValues[] = {
    { "lt", "<" },
    { "gt", ">" },
    { "apos", "'" },
    { "quot", "\"" },
    { "amp", "&" }
};

GXxmlEntitiesTablePtr GXxmlPredefinedEntities = NULL;

/*
 * A buffer used for converting entities to their equivalent and back.
 *
 * TODO: remove this, this helps performances but forbid reentrancy in a 
 *       stupid way.
 */
static int buffer_size = 0;
static CHAR *buffer = NULL;

void GXgrowBuffer(void) {
    buffer_size *= 2;
    buffer = (CHAR *) GxCore_Realloc(buffer, buffer_size * sizeof(CHAR));
    if (buffer == NULL) {
	perror("GxCore_Realloc failed");
	exit(1);
    }
}

/*
 * GXxmlFreeEntity : clean-up an entity record.
 */
void GXxmlFreeEntity(GXxmlEntityPtr entity) {
    if (entity == NULL) return;

    if (entity->name != NULL)
	GxCore_Free((char *) entity->name);
    if (entity->ExternalID != NULL)
        GxCore_Free((char *) entity->ExternalID);
    if (entity->SystemID != NULL)
        GxCore_Free((char *) entity->SystemID);
    if (entity->content != NULL)
        GxCore_Free((char *) entity->content);
    memset(entity, -1, sizeof(GXxmlEntity));
}

/*
 * GXxmlAddEntity : register a new entity for an entities table.
 *
 * TODO !!! We should check here that the combination of type
 *          ExternalID and SystemID is valid.
 */
static void
GXxmlAddEntity(GXxmlEntitiesTablePtr table, const CHAR *name, int type,
              const CHAR *ExternalID, const CHAR *SystemID, CHAR *content) {
    int i;
    GXxmlEntityPtr cur;
    int len;

    for (i = 0;i < table->nb_entities;i++) {
        cur = &table->table[i];
	if (!GXxmlStrcmp(cur->name, name)) {
	    /*
	     * The entity is already defined in this Dtd, the spec says to NOT
	     * override it ... Is it worth a Warning ??? !!!
	     */
	    return;
	}
    }
    if (table->nb_entities >= table->max_entities) {
        /*
	 * need more elements.
	 */
	table->max_entities *= 2;
	table->table = (GXxmlEntityPtr) 
	    GxCore_Realloc(table->table, table->max_entities * sizeof(GXxmlEntity));
	if (table->table) {
	    perror("GxCore_Realloc failed");
	    exit(1);
	}
    }
    cur = &table->table[table->nb_entities];
    cur->name = GXxmlStrdup(name);
    for (len = 0;name[0] != 0;name++)len++;
    cur->len = len;
    cur->type = type;
    if (ExternalID != NULL)
	cur->ExternalID = GXxmlStrdup(ExternalID);
    else
        cur->ExternalID = NULL;
    if (SystemID != NULL)
	cur->SystemID = GXxmlStrdup(SystemID);
    else
        cur->SystemID = NULL;
    if (content != NULL)
	cur->content = GXxmlStrdup(content);
    else
        cur->content = NULL;
    table->nb_entities++;
}

/**
 * GXxmlInitializePredefinedEntities:
 *
 * Set up the predefined entities.
 */
void GXxmlInitializePredefinedEntities(void) {
    int i;
    CHAR name[50];
    CHAR value[50];
    const char *in;
    CHAR *out;

    if (GXxmlPredefinedEntities != NULL) return;

    GXxmlPredefinedEntities = GXxmlCreateEntitiesTable();
    for (i = 0;i < sizeof(GXxmlPredefinedEntityValues) / 
                   sizeof(GXxmlPredefinedEntityValues[0]);i++) {
        in = GXxmlPredefinedEntityValues[i].name;
	out = &name[0];
	for (;(*out++ = (CHAR) *in);)in++;
        in = GXxmlPredefinedEntityValues[i].value;
	out = &value[0];
	for (;(*out++ = (CHAR) *in);)in++;
        GXxmlAddEntity(GXxmlPredefinedEntities, (const CHAR *) &name[0],
	             XML_INTERNAL_PREDEFINED_ENTITY, NULL, NULL,
		     &value[0]);
    }
}

/**
 * GXxmlGetPredefinedEntity:
 * @name:  the entity name
 *
 * Check whether this name is an predefined entity.
 *
 * return values: NULL if not, othervise the entity
 */
GXxmlEntityPtr
GXxmlGetPredefinedEntity(const CHAR *name) {
    int i;
    GXxmlEntityPtr cur;

    if (GXxmlPredefinedEntities == NULL)
        GXxmlInitializePredefinedEntities();
    for (i = 0;i < GXxmlPredefinedEntities->nb_entities;i++) {
	cur = &GXxmlPredefinedEntities->table[i];
	if (!GXxmlStrcmp(cur->name, name)) return(cur);
    }
    return(NULL);
}

/**
 * GXxmlAddDtdEntity:
 * @doc:  the document
 * @name:  the entity name
 * @type:  the entity type XML_xxx_yyy_ENTITY
 * @ExternalID:  the entity external ID if available
 * @SystemID:  the entity system ID if available
 * @content:  the entity content
 *
 * Register a new entity for this document DTD.
 */
void
GXxmlAddDtdEntity(GXxmlDocPtr doc, const CHAR *name, int type,
              const CHAR *ExternalID, const CHAR *SystemID, CHAR *content) {
    GXxmlEntitiesTablePtr table;

    if (doc->dtd == NULL) {
        fprintf(stderr, "GXxmlAddDtdEntity: document without Dtd !\n");
	return;
    }
    table = (GXxmlEntitiesTablePtr) doc->dtd->entities;
    if (table == NULL) {
        table = GXxmlCreateEntitiesTable();
	doc->dtd->entities = table;
    }
    GXxmlAddEntity(table, name, type, ExternalID, SystemID, content);
}

/**
 * GXxmlAddDocEntity:
 * @doc:  the document
 * @name:  the entity name
 * @type:  the entity type XML_xxx_yyy_ENTITY
 * @ExternalID:  the entity external ID if available
 * @SystemID:  the entity system ID if available
 * @content:  the entity content
 *
 * Register a new entity for this document.
 */
void
GXxmlAddDocEntity(GXxmlDocPtr doc, const CHAR *name, int type,
              const CHAR *ExternalID, const CHAR *SystemID, CHAR *content) {
    GXxmlEntitiesTablePtr table;

    table = (GXxmlEntitiesTablePtr) doc->entities;
    if (table == NULL) {
        table = GXxmlCreateEntitiesTable();
	doc->entities = table;
    }
    GXxmlAddEntity(doc->entities, name, type, ExternalID, SystemID, content);
}

/**
 * GXxmlGetDtdEntity:
 * @doc:  the document referencing the entity
 * @name:  the entity name
 *
 * Do an entity lookup in the Dtd entity hash table and
 * returns the corresponding entity, if found.
 * 
 * return values: A pointer to the entity structure or NULL if not found.
 */
GXxmlEntityPtr
GXxmlGetDtdEntity(GXxmlDocPtr doc, const CHAR *name) {
    int i;
    GXxmlEntityPtr cur;
    GXxmlEntitiesTablePtr table;

    if ((doc->dtd != NULL) && (doc->dtd->entities != NULL)) {
	table = (GXxmlEntitiesTablePtr) doc->dtd->entities;
	for (i = 0;i < table->nb_entities;i++) {
	    cur = &table->table[i];
	    if (!GXxmlStrcmp(cur->name, name)) return(cur);
	}
    }
    return(NULL);
}

/**
 * GXxmlGetDocEntity:
 * @doc:  the document referencing the entity
 * @name:  the entity name
 *
 * Do an entity lookup in the document entity hash table and
 * returns the corrsponding entity, otherwise a lookup is done
 * in the predefined entities too.
 * 
 * return values: A pointer to the entity structure or NULL if not found.
 */
GXxmlEntityPtr
GXxmlGetDocEntity(GXxmlDocPtr doc, const CHAR *name) {
    int i;
    GXxmlEntityPtr cur;
    GXxmlEntitiesTablePtr table;

    if (doc->entities != NULL) {
	table = (GXxmlEntitiesTablePtr) doc->entities;
	for (i = 0;i < table->nb_entities;i++) {
	    cur = &table->table[i];
	    if (!GXxmlStrcmp(cur->name, name)) return(cur);
	}
    }
    if (GXxmlPredefinedEntities == NULL)
        GXxmlInitializePredefinedEntities();
    table = GXxmlPredefinedEntities;
    for (i = 0;i < table->nb_entities;i++) {
	cur = &table->table[i];
	if (!GXxmlStrcmp(cur->name, name)) return(cur);
    }

    return(NULL);
}

/*
 * [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD]
 *                  | [#x10000-#x10FFFF]
 * any Unicode character, excluding the surrogate blocks, FFFE, and FFFF.
 */
#define IS_CHAR(c)							\
    (((c) == 0x09) || ((c) == 0x0a) || ((c) == 0x0d) ||			\
     (((c) >= 0x20) && ((c) != 0xFFFE) && ((c) != 0xFFFF)))

/**
 * GXxmlEncodeEntities:
 * @doc:  the document containing the string
 * @input:  A string to convert to XML.
 *
 * Do a global encoding of a string, replacing the predefined entities
 * and non ASCII values with their entities and CharRef counterparts.
 *
 * TODO !!!! Once moved to UTF-8 internal encoding, the encoding of non-ascii
 *           get erroneous.
 *
 * TODO This routine is not reentrant and this will be changed, the interface
 *      should not be modified though.
 *
 * return values: A newly allocated string with the substitution done.
 */

static int GXxmlIsTextUTF8(unsigned char* str)
{
    int nBytes=0;
    unsigned char chr = 0;
    int bAllAscii = 1;
    int i = 0;

    for(i=0; i<strlen((char *)str); ++i)
    {
        chr= *(str+i);
        if( (chr&0x80) != 0 )
            bAllAscii= 0;
        if(nBytes==0)
        {
            if(chr>=0x80)
            {
                if(chr>=0xFC&&chr<=0xFD)
                    nBytes=6;
                else if(chr>=0xF8)
                    nBytes=5;
                else if(chr>=0xF0)
                    nBytes=4;
                else if(chr>=0xE0)
                    nBytes=3;
                else if(chr>=0xC0)
                    nBytes=2;
                else
                    return 0;
                nBytes--;
            }
        }
        else
        {
            if( (chr&0xC0) != 0x80 )
                return 0;
            nBytes--;
        }
    }
    if( nBytes > 0 )
        return 0;
    if( bAllAscii )
        return 0;

    return 1;
}

CHAR *
GXxmlEncodeEntities(GXxmlDocPtr doc, const CHAR *input) {
    const CHAR *cur = input;
    CHAR *out = buffer;
    int is_utf8_text = 0;

    if (input == NULL) return(NULL);
    if (buffer == NULL) {
        buffer_size = 1000;
        buffer = (CHAR *) GxCore_Malloc(buffer_size * sizeof(CHAR));
	if (buffer == NULL) {
	    perror("GxCore_Malloc failed");
            exit(1);
	}
	out = buffer;
    }

    is_utf8_text = GXxmlIsTextUTF8((unsigned char *)input);
    while (*cur != '\0') {
        if (out - buffer > buffer_size - 100) {
	    int index = out - buffer;

	    GXgrowBuffer();
	    out = &buffer[index];
	}

	/*
	 * By default one have to encode at least '<', '>', '"' and '&' !
	 */
	if (*cur == '<') {
	    *out++ = '&';
	    *out++ = 'l';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '>') {
	    *out++ = '&';
	    *out++ = 'g';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '&') {
	    *out++ = '&';
	    *out++ = 'a';
	    *out++ = 'm';
	    *out++ = 'p';
	    *out++ = ';';
	} else if (*cur == '"') {
	    *out++ = '&';
	    *out++ = 'q';
	    *out++ = 'u';
	    *out++ = 'o';
	    *out++ = 't';
	    *out++ = ';';
	} else if (*cur == '\'') {
	    *out++ = '&';
	    *out++ = 'a';
	    *out++ = 'p';
	    *out++ = 'o';
	    *out++ = 's';
	    *out++ = ';';
	} else if (((*cur >= 0x20) && (*cur < 0x80)) ||
	    (*cur == '\n') || (*cur == '\r') || (*cur == '\t')) {
	    /*
	     * default case, just copy !
	     */
	    *out++ = *cur;
	} else if(is_utf8_text)
	{
	    *out++ = *cur;
#ifndef USE_UTF_8
	} else if ((sizeof(CHAR) == 1) && (*cur >= 0x80)) {
	    char buf[10], *ptr;
#ifdef HAVE_SNPRINTF
	    snprintf(buf, 9, "&#%d;", *cur);
#else
	    sprintf(buf, "&#%d;", *cur);
#endif
            ptr = buf;
	    while (*ptr != 0) *out++ = *ptr++;
#endif
	} else if (IS_CHAR(*cur)) {
	    char buf[10], *ptr;

#ifdef HAVE_SNPRINTF
	    snprintf(buf, 9, "&#%d;", *cur);
#else
	    sprintf(buf, "&#%d;", *cur);
#endif
            ptr = buf;
	    while (*ptr != 0) *out++ = *ptr++;
	}
#if 0
	else {
	    /*
	     * default case, this is not a valid char !
	     * Skip it...
	     */
	    fprintf(stderr, "GXxmlEncodeEntities: invalid char %d\n", (int) *cur);
	}
#endif
	cur++;
    }
    *out++ = 0;
    return(buffer);
}

/**
 * GXxmlCreateEntitiesTable:
 *
 * create and initialize an empty entities hash table.
 *
 * return values: the GXxmlEntitiesTablePtr just created or NULL in case of error.
 */
GXxmlEntitiesTablePtr
GXxmlCreateEntitiesTable(void) {
    GXxmlEntitiesTablePtr ret;

    ret = (GXxmlEntitiesTablePtr) 
         GxCore_Malloc(sizeof(GXxmlEntitiesTable));
    if (ret == NULL) {
        fprintf(stderr, "GXxmlCreateEntitiesTable : GxCore_Malloc(%d) failed\n",
	        sizeof(GXxmlEntitiesTable));
        return(NULL);
    }
    ret->max_entities = XML_MIN_ENTITIES_TABLE;
    ret->nb_entities = 0;
    ret->table = (GXxmlEntityPtr ) 
         GxCore_Malloc(ret->max_entities * sizeof(GXxmlEntity));
    if (ret == NULL) {
        fprintf(stderr, "GXxmlCreateEntitiesTable : GxCore_Malloc(%d) failed\n",
	        ret->max_entities * sizeof(GXxmlEntity));
	GxCore_Free(ret);
        return(NULL);
    }
    return(ret);
}

/**
 * GXxmlFreeEntitiesTable:
 * @table:  An entity table
 *
 * Deallocate the memory used by an entities hash table.
 */
void
GXxmlFreeEntitiesTable(GXxmlEntitiesTablePtr table) {
    int i;

    if (table == NULL) return;

    for (i = 0;i < table->nb_entities;i++) {
        GXxmlFreeEntity(&table->table[i]);
    }
    GxCore_Free(table->table);
    GxCore_Free(table);
}

/**
 * GXxmlCopyEntitiesTable:
 * @table:  An entity table
 *
 * Build a copy of an entity table.
 * 
 * return values: the new GXxmlEntitiesTablePtr or NULL in case of error.
 */
GXxmlEntitiesTablePtr
GXxmlCopyEntitiesTable(GXxmlEntitiesTablePtr table) {
    GXxmlEntitiesTablePtr ret;
    GXxmlEntityPtr cur, ent;
    int i;

    ret = (GXxmlEntitiesTablePtr) GxCore_Malloc(sizeof(GXxmlEntitiesTable));
    if (ret == NULL) {
        fprintf(stderr, "GXxmlCopyEntitiesTable: out of memory !\n");
	return(NULL);
    }
    ret->table = (GXxmlEntityPtr) GxCore_Malloc(table->max_entities *
                                         sizeof(GXxmlEntity));
    if (ret->table == NULL) {
        fprintf(stderr, "GXxmlCopyEntitiesTable: out of memory !\n");
	GxCore_Free(ret);
	return(NULL);
    }
    ret->max_entities = table->max_entities;
    ret->nb_entities = table->nb_entities;
    for (i = 0;i < ret->nb_entities;i++) {
	cur = &ret->table[i];
	ent = &table->table[i];
	cur->len = ent->len;
	cur->type = ent->type;
	if (ent->name != NULL)
	    cur->name = GXxmlStrdup(ent->name);
	else
	    cur->name = NULL;
	if (ent->ExternalID != NULL)
	    cur->ExternalID = GXxmlStrdup(ent->ExternalID);
	else
	    cur->ExternalID = NULL;
	if (ent->SystemID != NULL)
	    cur->SystemID = GXxmlStrdup(ent->SystemID);
	else
	    cur->SystemID = NULL;
	if (ent->content != NULL)
	    cur->content = GXxmlStrdup(ent->content);
	else
	    cur->content = NULL;
    }
    return(ret);
}

/**
 * GXxmlDumpEntitiesTable:
 * @table:  An entity table
 *
 * This will dump the content of the entity table as an XML DTD definition
 *
 * NOTE: TODO an extra parameter allowing a reentant implementation will
 *       be added.
 */
void
GXxmlDumpEntitiesTable(GXxmlEntitiesTablePtr table) {
    int i;
    GXxmlEntityPtr cur;

    if (table == NULL) return;

    for (i = 0;i < table->nb_entities;i++) {
        cur = &table->table[i];
        switch (cur->type) {
	    case XML_INTERNAL_GENERAL_ENTITY:
	        GXxmlBufferWriteChar("<!ENTITY ");
		GXxmlBufferWriteCHAR(cur->name);
		GXxmlBufferWriteChar(" \"");
		GXxmlBufferWriteCHAR(cur->content);
		GXxmlBufferWriteChar("\">\n");
	        break;
	    case XML_EXTERNAL_GENERAL_PARSED_ENTITY:
	        GXxmlBufferWriteChar("<!ENTITY ");
		GXxmlBufferWriteCHAR(cur->name);
		if (cur->ExternalID != NULL) {
		     GXxmlBufferWriteChar(" PUBLIC \"");
		     GXxmlBufferWriteCHAR(cur->ExternalID);
		     GXxmlBufferWriteChar("\" \"");
		     GXxmlBufferWriteCHAR(cur->SystemID);
		     GXxmlBufferWriteChar("\"");
		} else {
		     GXxmlBufferWriteChar(" SYSTEM \"");
		     GXxmlBufferWriteCHAR(cur->SystemID);
		     GXxmlBufferWriteChar("\"");
		}
		GXxmlBufferWriteChar(">\n");
	        break;
	    case XML_EXTERNAL_GENERAL_UNPARSED_ENTITY:
	        GXxmlBufferWriteChar("<!ENTITY ");
		GXxmlBufferWriteCHAR(cur->name);
		if (cur->ExternalID != NULL) {
		     GXxmlBufferWriteChar(" PUBLIC \"");
		     GXxmlBufferWriteCHAR(cur->ExternalID);
		     GXxmlBufferWriteChar("\" \"");
		     GXxmlBufferWriteCHAR(cur->SystemID);
		     GXxmlBufferWriteChar("\"");
		} else {
		     GXxmlBufferWriteChar(" SYSTEM \"");
		     GXxmlBufferWriteCHAR(cur->SystemID);
		     GXxmlBufferWriteChar("\"");
		}
		if (cur->content != NULL) { /* Should be true ! */
		    GXxmlBufferWriteChar(" NDATA ");
		    GXxmlBufferWriteCHAR(cur->content);
		}
		GXxmlBufferWriteChar(">\n");
	        break;
	    case XML_INTERNAL_PARAMETER_ENTITY:
	        GXxmlBufferWriteChar("<!ENTITY % ");
		GXxmlBufferWriteCHAR(cur->name);
		GXxmlBufferWriteChar(" \"");
		GXxmlBufferWriteCHAR(cur->content);
		GXxmlBufferWriteChar("\">\n");
	        break;
	    case XML_EXTERNAL_PARAMETER_ENTITY:
	        GXxmlBufferWriteChar("<!ENTITY % ");
		GXxmlBufferWriteCHAR(cur->name);
		if (cur->ExternalID != NULL) {
		     GXxmlBufferWriteChar(" PUBLIC \"");
		     GXxmlBufferWriteCHAR(cur->ExternalID);
		     GXxmlBufferWriteChar("\" \"");
		     GXxmlBufferWriteCHAR(cur->SystemID);
		     GXxmlBufferWriteChar("\"");
		} else {
		     GXxmlBufferWriteChar(" SYSTEM \"");
		     GXxmlBufferWriteCHAR(cur->SystemID);
		     GXxmlBufferWriteChar("\"");
		}
		GXxmlBufferWriteChar(">\n");
	        break;
	    default:
	        fprintf(stderr,
		    "GXxmlDumpEntitiesTable: internal: unknown type %d\n",
		        cur->type);
	}
    }
}
