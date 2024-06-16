/**
 * @file lxml.h
 *
 * @version 2.0
 *
 * @author Calum Judd Anderson
 *
 * @brief XML Parser Library
 */
#ifndef LITTLE_XML_H
#define LITTLE_XML_H
/******************Include Start*******************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************Include End********************/

/*******************Define Start*******************/

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

#define BUFF_HANDLE_SIZE 102
#define LEX_BUFF_SIZE (2*BUFF_HANDLE_SIZE)

/********************Define End********************/

/********************Enum Start********************/

enum XMLReadType {
    READ_XML_UNSUPPORTED = -1,
    READ_XML_FILE,
    READ_XML_BUFF
};

enum TagType {
    TAG_UNSUPPORTED = -1,
    TAG_START,
    TAG_INLINE
};

/*********************Enum End*********************/

/************Struct-Declaration Start**************/

struct XMLReadFileHandle {
    FILE *fp;
    size_t fpInputSize, offset;

    int (*open)(struct XMLReadFileHandle*, char*);
    int (*close)(struct XMLReadFileHandle*);
    int (*read)(struct XMLReadFileHandle*, char *, size_t);
};

struct XMLReadBuffHandle {
    char *buffInput;
    size_t buffInputSize, offset;

    int (*open)(struct XMLReadBuffHandle*, char*);
    int (*close)(struct XMLReadBuffHandle*);
    int (*read)(struct XMLReadBuffHandle*, char*, size_t);
};

struct XMLLoadContext {
    enum XMLReadType type;
    char buff[BUFF_HANDLE_SIZE+1], *handle;
    size_t buffSize, *inputSize, *offset;

    struct XMLReadFileHandle readFile;
    struct XMLReadBuffHandle readBuff;

    int (*read)(struct XMLLoadContext*);
    int(*open)(struct XMLLoadContext*);
    int (*close)(struct XMLLoadContext*);
};

struct XMLAttribute {
    char *key, *value;

    void (*free)(struct XMLAttribute*);
};

struct XMLAttributeList {
    size_t size, heapSize;
    struct XMLAttribute **attribute;

    int (*add)(struct XMLAttributeList*, struct XMLAttribute);
    void (*free)(struct XMLAttributeList*);

    char* (*getAttributeValue)(struct XMLAttributeList*, char*);
    struct XMLAttribute* (*getAttribute)(struct XMLAttributeList*, char*);
};

struct XMLNodeList {
    size_t size, heapSize;
    struct XMLNode **data;

    int (*add)(struct XMLNodeList*, struct XMLNode*, struct XMLNode*);
    void (*free)(struct XMLNodeList*);
};

struct XMLNode {
    char *tag, *inner_text;

    struct XMLNode *parent;
    struct XMLAttributeList attributes;
    struct XMLNodeList children;

    int (*add)(struct XMLNode*, struct XMLNode*);
    void (*free)(struct XMLNode*);

    char* (*getAttributeValue)(struct XMLNode*, char*);
    struct XMLAttribute* (*getAttribute)(struct XMLNode*, char*);
};

struct XMLDocument {
    struct XMLNode *root;
    char *version, *encoding;
    int success;

    void (*free)(struct XMLDocument*);
};

/*************Struct-Declaration End***************/

/*****************Prototype Start******************/

/* XML Read File Handle Prototype Start */

struct XMLReadFileHandle XMLReadFileHandle_init();
static int xmlReadFileHandle_open(struct XMLReadFileHandle *self, char *path);
static int xmlReadFileHandle_close(struct XMLReadFileHandle *self);
static int xmlReadFileHandle_read(struct XMLReadFileHandle *self, char *buff, size_t buffSize);

/* XML Read File Handle Prototype End */

/* XML Read Buff Handle Prototype Start */

struct XMLReadBuffHandle XMLReadBuffHandle_init();
static int xmlReadBuffHandle_open(struct XMLReadBuffHandle *self, char *buffInput);
static int xmlReadBuffHandle_close(struct XMLReadBuffHandle *self);
static int xmlReadBuffHandle_read(struct XMLReadBuffHandle *self, char *buff, size_t buffSize);

/* XML Read Buff Handle Prototype End */

/* XML Load Context Prototype Start */

struct XMLLoadContext XMLLoadContext_init(enum XMLReadType type, char *handle);
static int XMLLoadContext_open(struct XMLLoadContext *self);
static int XMLLoadContext_close(struct XMLLoadContext *self);
static int XMLLoadContext_read(struct XMLLoadContext *self);

/* XML Load Context Prototype End */

/* XML Attribute Functions Prototype Start */

struct XMLAttribute* XMLAttribute_init(char *key, char *value);

static void XMLAttribute_free(struct XMLAttribute *attr);

/* XML Attribute Functions Prototype End */

/* XML Attribute List Functions Prototype Start */

struct XMLAttributeList  XMLAttributeList_init();

static int XMLAttributeList_add(struct XMLAttributeList *self, struct XMLAttribute attr);
static void XMLAttributeList_free(struct XMLAttributeList *self);

static struct XMLAttribute* XMLAttributeList_getAttribute(struct XMLAttributeList *self, char *key);
static char* XMLAttributeList_getAttributeValue(struct XMLAttributeList *self, char *key);

/* XML Attribute List Functions Prototype End */

/* XML Node Functions Prototype Start */

struct XMLNode* XMLNode_init();

static int XMLNode_add(struct XMLNode *self, struct XMLNode *node);
static void XMLNode_free(struct XMLNode *self);
static struct XMLAttribute* XMLNode_getAttribute(struct XMLNode *node, char *key);
static char* XMLNode_getAttributeValue(struct XMLNode *node, char *key);

/* XML Node Functions Prototype End */

/* XML Node List Functions Prototype Start */

struct XMLNodeList XMLNodeList_init();

static int XMLNodeList_add(struct XMLNodeList *self, struct XMLNode *parent, struct XMLNode *node);
static void XMLNodeList_free(struct XMLNodeList *self);

/* XML Node List Functions Prototype End */

/* XML Document Functions Prototype Start */

struct XMLDocument XMLDocument_load(struct XMLLoadContext ctx);
int XMLDocument_write(struct XMLDocument doc, const char *path, int indent);

static void XMLDocument_free(struct XMLDocument *doc);

/* XML Document Functions Prototype End */

static char* lxmlStrdup(const char *str);
static char* lxmlReadXmlContentsIntoMemory(FILE *fp);

static enum TagType lxmlParseAttrs(char *buf, size_t *i, char *lex, size_t *lexi, struct XMLNode *curr_node);

static int lxmlEndsWith(const char *haystack, const char *needle);
static int lxmlParseEndOfNode(const char *buf, size_t *bufOffset, char *lex, size_t lexBufSize, size_t *lexi);

static void node_out(FILE *file, struct XMLNode *node, int indent, int times);

/******************Prototype End*******************/

/*******************Public Start*******************/

/**
 * @brief Initialises an 'XMLReadFileHandle'
 *
 * @return handle - A newly constructed 'XMLReadFileHandle'
 */
struct XMLReadFileHandle XMLReadFileHandle_init() {
    struct XMLReadFileHandle handle = { 0, 0, 0, xmlReadFileHandle_open, xmlReadFileHandle_close, xmlReadFileHandle_read };
    return handle;
} /* End of XMLReadFileHandle_init */

/**
 * @brief Initialises an 'XMLReadBuffHandle'
 *
 * @return handle - A newly constructed 'XMLReadBuffHandle'
 */
struct XMLReadBuffHandle XMLReadBuffHandle_init() {
    struct XMLReadBuffHandle handle = { 0, 0, 0, xmlReadBuffHandle_open, xmlReadBuffHandle_close, xmlReadBuffHandle_read };
    return handle;
} /* End of XMLReadBuffHandle_init */

struct XMLLoadContext XMLLoadContext_init(enum XMLReadType type, char *handle) {
    struct XMLLoadContext ctx = { READ_XML_UNSUPPORTED, { 0 }, 0, BUFF_HANDLE_SIZE, 0, 0, { 0 }, { 0 }, 0, 0, 0 };

    ctx.type = (READ_XML_FILE == type || READ_XML_BUFF == type) ? type : READ_XML_UNSUPPORTED;
    ctx.handle = handle;

    ctx.readFile = XMLReadFileHandle_init();
    ctx.readBuff = XMLReadBuffHandle_init();

    ctx.open = XMLLoadContext_open;
    ctx.close = XMLLoadContext_close;
    ctx.read = XMLLoadContext_read;

    return ctx;
} /* End of XMLLoadContext_init */

/**
 * @brief Initialises an 'XMLAttribute'
 *        If 'key'|'value' are supplied then a deep-copy of these values will be made
 *        **Note:** Both 'key' and 'value' if these wish to be set
 *
 * @param key - Optional XML key attribute to set
 * @param value - Optional XML value attribute to set
 * @return attr - A new heap allocated 'XMLAttribute'
 */
struct XMLAttribute* XMLAttribute_init(char *key, char *value) {
    struct XMLAttribute *attr = calloc(1, sizeof(struct XMLAttribute));

    if (NULL != attr) {
        attr->free = XMLAttribute_free;

        if (NULL != key && NULL != value) {
            size_t keylen = strlen(key),
                   valuelen = strlen(value);

            if (0 != keylen && 0 != valuelen) {
                attr->key = lxmlStrdup(key);
                attr->value = lxmlStrdup(value);

                if (NULL == attr->key || NULL == attr->value) {
                    free(attr->key);
                    free(attr->value);
                    attr->key = attr->value = NULL;

                    free(attr);
                    attr = NULL;
                }
            }
        }
    }

    return attr;
} /* End of XMLAttribute_init */

/**
 * @brief Initialises an 'XMLAttributeList'
 *
 * @return list - A newly constructed 'XMLAttributeList'
 */
struct XMLAttributeList XMLAttributeList_init() {
    struct XMLAttributeList list = { 0, 0, 0, XMLAttributeList_add, XMLAttributeList_free, XMLAttributeList_getAttributeValue, XMLAttributeList_getAttribute };
    return list;
} /* End of XMLAttributeList_init */

/**
 * @brief Initialises an 'XMLNodeList'
 *
 * @return list - A newly constructed 'XMLNodeList'
 */
struct XMLNodeList XMLNodeList_init() {
    struct XMLNodeList list = { 0, 0, 0, XMLNodeList_add, XMLNodeList_free };
    return list;
} /* End of XMLNodeList_init */

/**
 * @brief Initialises an 'XMLNode'
 *
 * @return node - A 'calloc' 'XMLNode' that has been initialised
 */
struct XMLNode* XMLNode_init() {
    struct XMLNode *node = calloc(1, sizeof(struct XMLNode));

    if (NULL != node) {
        node->attributes = XMLAttributeList_init();
        node->children = XMLNodeList_init();

        node->add = XMLNode_add;
        node->free = XMLNode_free;

        node->getAttribute = XMLNode_getAttribute;
        node->getAttributeValue = XMLNode_getAttributeValue;
    }

    return node;
} /* End of XMLNode_init */

/**
 * @brief Free's a given 'XMLNode'
 *
 * @param node - The 'XMLNode' to free
 */
void XMLNode_free(struct XMLNode *node) {
    if (NULL != node) {
        if (node->tag) {
            free(node->tag);
            node->tag = NULL;
        }

        if (node->inner_text) {
            free(node->inner_text);
            node->inner_text = NULL;
        }

        node->attributes.free(&node->attributes);
        node->children.free(&node->children);
        memset(node, '\0', sizeof(struct XMLNode));
    }
} /* End of XMLNode_free */

/**
 * @brief Serialises the XML document into an 'XMLDocument'
 *
 * @return doc - The 'XMLDocument' that the XML has been serilaised in to
 */
struct XMLDocument XMLDocument_load(struct XMLLoadContext ctx) {
    struct XMLDocument doc = { 0, 0, 0, 0, XMLDocument_free };

    char lex[LEX_BUFF_SIZE+1] = { 0 };
    size_t i = 0, lexi = 0;

    struct XMLNode *curr_node = NULL, *tmp = NULL;

    int success = ctx.open(&ctx);

    success &= ctx.read(&ctx);
    doc.root = XMLNode_init(NULL);
    curr_node = doc.root;

    while (TRUE == success) {

        if (i == ctx.buffSize) {
            success = ctx.read(&ctx);
            i = 0;

            if (FALSE == success)
                break;
        } else if (i == ctx.buffSize || lexi == LEX_BUFF_SIZE) {
            success = FALSE;
            break;
        }

        /* Last Round & NULL Terminator */
        if (*ctx.offset == *ctx.inputSize && '\0' == ctx.buff[i])
            break;

        if ('<' == ctx.buff[i]) {
            lex[lexi] = '\0';

            /* Inner text */
            if (lexi > 0) {
                if (NULL == curr_node) {
                    fprintf(stderr, "Text outside of document\n");
                    break;
                }

                if (NULL == curr_node->inner_text)
                    curr_node->inner_text = lxmlStrdup(lex);

                lexi = 0;
            }

            /* End of node */
            if ('/' == ctx.buff[i + 1]) {
                i += 2;
                
                success = lxmlParseEndOfNode(ctx.buff, &i, lex, BUFF_HANDLE_SIZE, &lexi);

                if (NULL == curr_node) {
                    fprintf(stderr, "Already at the root\n");
                    break;
                }

                if (0 != strcmp(NULL == curr_node->tag ? "" : curr_node->tag, lex)) {
                    fprintf(stderr, "Mismatched tags (%s != %s)\n", curr_node->tag, lex);
                    break;
                }

                curr_node = curr_node->parent;
                i++;
                continue;
            }

            /* Special nodes */
            if ('!' == ctx.buff[i + 1]) {
                while (' ' != ctx.buff[i]  && '>' != ctx.buff[i])
                    lex[lexi++] = ctx.buff[i++];
                lex[lexi] = '\0';

                /* Comments */
                if (0 == strcmp(lex, "<!--")) {
                    lex[lexi] = '\0';
                    while (FALSE == lxmlEndsWith(lex, "-->")) {
                        lex[lexi++] = ctx.buff[i++];
                        lex[lexi] = '\0';
                    }
                    continue;
                }
            }

            /* Declaration tags */
            if ('?' == ctx.buff[i + 1]) {
                while (' ' != ctx.buff[i] && '>' != ctx.buff[i])
                    lex[lexi++] = ctx.buff[i++];
                lex[lexi] = '\0';

                /* This is the XML declaration */
                if (0 == strcmp(lex, "<?xml")) {
                    struct XMLNode *desc = XMLNode_init(NULL);
                    lexi = 0;

                    lxmlParseAttrs(ctx.buff, &i, lex, &lexi, desc);

                    doc.version = desc->getAttributeValue(desc, "version");
                    doc.encoding = desc->getAttributeValue(desc, "encoding");

                    desc->free(desc);
                    free(desc);
                    desc = NULL;
                    continue;
                }
            }

            /* Set current node */
            tmp = XMLNode_init();

            if (NULL != tmp) {
                enum TagType type = TAG_UNSUPPORTED;
                ++i;
                /* Start tag */
                type = lxmlParseAttrs(ctx.buff, &i, lex, &lexi, tmp);
                curr_node->add(curr_node, tmp);

                if (TAG_INLINE != type) {
                    /* Set tag name if none */
                    lex[lexi] = '\0';

                    if (NULL == curr_node->tag)
                        curr_node->tag = lxmlStrdup(lex);

                    /* Reset lexer */
                    lexi = 0;
                    curr_node = tmp;
                }
                
                ++i;
                continue;
            } else {
                success = FALSE;
                break;
            }
        } else {
            lex[lexi++] = ctx.buff[i++];
        }
    }

    if (FALSE == success)
        doc.free(&doc);

    doc.success = success;

    return doc;
} /* End of XMLDocument_load */

int XMLDocument_write(struct XMLDocument doc, const char *path, int indent) {
    FILE *file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return FALSE;
    }

    fprintf(
        file, "<?xml version=\"%s\" encoding=\"%s\" ?>\n",
        (doc.version) ? doc.version : "1.0",
        (doc.encoding) ? doc.encoding : "UTF-8"
    );
    node_out(file, doc.root, indent, 0);
    fclose(file);

    return TRUE;
} /* End of XMLDocument_write */

/********************Public End********************/

/******************Private Start*******************/

/**
 * @brief Closes the file handle pointed to by 'self'
 *
 * @param self - The XMLReadFileHandle
 * @return success - A flag indicating the status of the subroutine
 */
static int xmlReadFileHandle_close(struct XMLReadFileHandle *self) {
    int success = FALSE;

    if (NULL != self) {
        if (NULL != self->fp) {
            int closeStatus = fclose(self->fp);

            if (0 == closeStatus)
                success = TRUE;
            self->fp = NULL;
        } else
            success = TRUE;
    }

    return success;
} /* End of xmlReadFileHandle_close */

/**
 * @brief Opens a file handle given 'path' and assigns it to 'self'
 *
 * @param self - The XMLReadFileHandle
 * @param path - The location to open
 * @return success - A flag indicating the status of the subroutine
 */
static int xmlReadFileHandle_open(struct XMLReadFileHandle *self, char *path) {
    int success = FALSE;

    if (NULL != self && NULL != path) {
        self->fp = fopen(path, "rb");

        if (NULL != self->fp)
            success = TRUE;
    }

    return success;
} /* End of xmlReadFileHandle_open */

/**
 * @brief Reads from the file handle pointed to by 'self' and places the result into 'buff'
 *
 * @param self - The XMLReadFileHandle
 * @return success - A flag indicating the status of the subroutine
 */
static int xmlReadFileHandle_read(struct XMLReadFileHandle *self, char *buff, size_t buffSize) {
    int success = FALSE;

    if (NULL != self && NULL != self->fp) {
        size_t bytesRead = fread(buff, sizeof(char), buffSize, self->fp);
        success = (BUFF_HANDLE_SIZE == bytesRead) ? TRUE : (0 != feof(self->fp)) ? TRUE : FALSE;
    }

    return success;
} /* End of xmlReadFileHandle_read */

/**
 * @brief Closes the buffer handle pointed to by 'self'
 *
 * @param self - The XMLReadBuffHandle
 * @return success - A flag indicating the status of the subroutine
 */
static int xmlReadBuffHandle_close(struct XMLReadBuffHandle *self) {
    int success = FALSE;

    if (NULL != self) {
        self->buffInput = NULL;
        success = TRUE;
    }

    return success;
} /* End of xmlReadBuffHandle_close */

/**
 * @brief Opens a file handle given 'path' and assigns it to 'self'
 *
 * @param self - The XMLReadBuffHandle
 * @param path - The location to open
 * @return success - A flag indicating the status of the subroutine
 */
static int xmlReadBuffHandle_open(struct XMLReadBuffHandle *self, char *buffInput) {
    int success = FALSE;

    if (NULL != self && NULL != buffInput) {
        self->buffInput = buffInput;
        self->buffInputSize = strlen(buffInput);
        self->offset = 0;
        success = TRUE;
    }

    return success;
} /* End of xmlReadBuffHandle_open */

/**
 * @brief Reads from the file handle pointed to by 'self' and places the result into 'buff'
 *
 * @param self - The XMLReadBuffHandle
 * @return success - A flag indicating the status of the subroutine
 */
static int xmlReadBuffHandle_read(struct XMLReadBuffHandle *self, char *buff, size_t buffSize) {
    int success = FALSE;

    if (NULL != self && NULL != self->buffInput && NULL != buff && buffSize > 1) {
        size_t leftToRead = self->buffInputSize - self->offset;
        size_t readBytes = (leftToRead < buffSize) ? leftToRead : buffSize-1;
        memset(buff, '\0', buffSize);
        printf("Left to read: %lu %lu %lu\n", leftToRead, buffSize, readBytes);
        /* TODO: Offsets */
        memcpy(buff, self->buffInput, readBytes);
        self->offset += readBytes;
        success = TRUE;
    }

    return success;
} /* End of xmlReadBuffHandle_read */

/**
 * @brief Opens the 'handle' using the appropriate method as described by 'self->type'
 *
 * @param self - A reference to the 'XMLLoadContext' to locate the appropriate routine from
 * @param handle - The handle to 'open'
 * @return success - A flag indicating the status of the subroutine
 */
static int XMLLoadContext_open(struct XMLLoadContext *self) {
    int success = FALSE;

    if (NULL != self) {
        switch (self->type) {
            case(READ_XML_FILE):
            success = self->readFile.open(&self->readFile, self->handle);
            self->offset = &self->readFile.offset;
            self->inputSize = &self->readFile.fpInputSize;
            break;

            case(READ_XML_BUFF):
            success = self->readBuff.open(&self->readBuff, self->handle);
            self->offset = &self->readBuff.offset;
            self->inputSize = &self->readBuff.buffInputSize;
            break;

            default:
            break;
        }
    }

    return success;
} /* End of XMLLoadContext_open */

/**
 * @brief Closes the 'handle' using the appropriate method as described by 'self->type'
 *
 * @param self - A reference to the 'XMLLoadContext' to locate the appropriate routine from
 * @return success - A flag indicating the status of the subroutine
 */
static int XMLLoadContext_close(struct XMLLoadContext *self) {
    int success = FALSE;

    if (NULL != self) {
        switch (self->type) {
            case(READ_XML_FILE):
            success = self->readFile.close(&self->readFile);
            break;

            case(READ_XML_BUFF):
            success = self->readBuff.close(&self->readBuff);
            break;

            default:
            break;
        }
    }

    return success;
} /* End of XMLLoadContext_close */

/**
 * @brief Reads from the 'handle' using the appropriate method as described by 'self->type'
 *
 * @param self - A reference to the 'XMLLoadContext' to locate the appropriate routine from
 * @return success - A flag indicating the status of the subroutine
 */
static int XMLLoadContext_read(struct XMLLoadContext *self) {
    int success = FALSE;

    if (NULL != self) {
        switch (self->type) {
            case(READ_XML_FILE):
            success = self->readFile.read(&self->readFile, self->buff, BUFF_HANDLE_SIZE);
            break;

            case(READ_XML_BUFF):
            success = self->readBuff.read(&self->readBuff, self->buff, BUFF_HANDLE_SIZE);
            break;

            default:
            break;
        }
    }

    return success;
} /* End of XMLLoadContext_read */

/**
 * @brief Frees the data on a given 'XMLAttribute'
 *
 * @param self - A reference to the 'XMLAttribute' to free
 */
static void XMLAttribute_free(struct XMLAttribute *self) {
    if (NULL != self) {
        free(self->key);
        free(self->value);
        self->key = self->value = NULL;
    }
} /* End of XMLAttribute_free */

/**
 * @brief Adds a given 'XMLAttribute' to 'self'
 *
 * @param self - A pointer to the 'XMLAttributeList' to add the 'XMLAttribute to
 * @param attr - The 'XMLAttribute' to add to the list
 * @return success - A Flag indicating the status of the subroutine
 */
static int XMLAttributeList_add(struct XMLAttributeList *self, struct XMLAttribute attr) {
    int success = FALSE;

    if (NULL != self) {
        if (0 == self->size && NULL == self->attribute) {
            self->attribute = calloc(1, sizeof(struct XMLAttribute*));

            if (NULL != self->attribute)
                self->heapSize = 1;
        }

        while (self->size >= self->heapSize) {
            self->heapSize *= 2; /* FIXME: Handle 'realloc' Out of Memory Errors */
            self->attribute = realloc(self->attribute, sizeof(struct XMLAttribute*) * self->heapSize);
            memset(self->attribute + self->size, '\0', (sizeof(struct XMLAttribute*) * (self->heapSize - self->size)));
        }

        if (NULL != self->attribute) {
            self->attribute[self->size] = XMLAttribute_init(attr.key, attr.value);

            if (NULL != self->attribute[self->size]) {
                ++self->size;
                success = TRUE;
            }
        }
    }

    return success;
} /* End of XMLAttributeList_add */

/**
 * @brief Frees the given 'XMLAttributeList'
 *
 * @param self - The 'XMLAttributeList' to free
 */
static void XMLAttributeList_free(struct XMLAttributeList *self) {
    if (NULL != self) {
        size_t i = 0;
        for (;i<self->size;++i)
            self->attribute[i]->free(self->attribute[i]);

        for (i=0;i<self->heapSize;++i) {
            free(self->attribute[i]);
            self->attribute[i] = NULL;
        }

        free(self->attribute);
        self->attribute = NULL;

        memset(self, '\0', sizeof(struct XMLAttributeList));
    }
} /* End of XMLAttributeList_free */

/**
 * @brief Obtains the associated 'XMLAttribute' given the 'key'
 *
 * @param self - A reference to the 'XMLAttributeList' to search through
 * @param key - The value to search for
 * @return retAttr - The 'XMLAttribute' containing the associated 'key'
 */
static struct XMLAttribute* XMLAttributeList_getAttribute(struct XMLAttributeList *self, char *key) {
    struct XMLAttribute *retAttr = NULL;

    if (NULL != self && NULL != key) {
        size_t i = 0;
        for (; i < self->size; ++i) {
            struct XMLAttribute *attr = self->attribute[i];

            if (NULL != attr) {
                if (0 == strcmp(attr->key, key)) {
                    retAttr = attr;
                    break;
                }
            }
        }
    }

    return retAttr;
} /* End of XMLNode_getAttribute */

/**
 * @brief Obtains the associated 'value' given the 'key' from the 'XMLAttributeList'
 *
 * @param self - A reference to the 'XMLAttributeList' to search from
 * @param key - A string containing the value to search for
 * @return attrVal - The associated attribute value from the 'XMLAttributeList'
 */
static char* XMLAttributeList_getAttributeValue(struct XMLAttributeList *self, char *key) {
    char *attrVal = NULL;

    if (NULL != self && NULL != key) {
        size_t i = 0;
        for (; i < self->size; ++i) {
            struct XMLAttribute *attr = self->attribute[i];

            if (NULL != attr) {
                if (0 == strcmp(attr->key, key)) {
                    attrVal = lxmlStrdup(attr->value);
                    break;
                }
            }
        }
    }

    return attrVal;
}/* End of XMLAttributeList_getAttributeValue */

/**
 * @brief Convenience function to add an 'XMLNode' to the 'children'
 *        **Note:** See 'XMLNodeList_add' for implementation
 *
 * @param self - A reference to the struct you wish to add the node to
 * @param node - The 'XMLNode' to add to 'self'
 * @return success - A flag indicating the status of the subroutine
 */
static int XMLNode_add(struct XMLNode *self, struct XMLNode *node) {
    return (NULL != self && NULL != self->children.add && NULL != node) ? self->children.add(&self->children, self, node) : FALSE;
} /* End of XMLNode_add */

/**
 * @brief Obtains the associated 'XMLAttribute' given the 'key'
 *        **Note:** See 'XMLAttributeList_getAttribute' for implementation
 *
 * @param self - A reference to the 'XMLNode' to search through
 * @param key - The value to search for
 * @return retAttr - The 'XMLAttribute' containing the associated 'key'
 */
static struct XMLAttribute* XMLNode_getAttribute(struct XMLNode *self, char *key) {
    return (NULL != self && NULL != self->attributes.getAttribute && NULL != key) ? self->attributes.getAttribute(&self->attributes, key) : NULL;
} /* End of XMLNode_getAttribute */

/**
 * @brief Obtains the associated 'value' given the 'key' from the 'XMLNode'
 *        **Note:** See 'XMLAttributeList_getAttributeValue' for implementation
 *
 * @param self - A reference to the 'XMLNode' to search from
 * @param key - A string containing the value to search for
 * @return attrVal - The associated attribute value from the 'XMLNode'
 */
static char* XMLNode_getAttributeValue(struct XMLNode *self, char *key) {
    return (NULL != self && NULL != self->attributes.getAttributeValue && NULL != key) ? self->attributes.getAttributeValue(&self->attributes, key) : NULL;
} /* End of XMLNode_getAttributeValue */

/**
 * @brief Adds a 'XMLNode' to the 'XMLNodeList'
 *
 * @param self - A reference to the struct you wish to add the node to
 * @param parent - A reference to the parent
 * @param node - The 'XMLNode' to add to 'self'
 * @return success - A flag indicating the status of the subroutine
 */
static int XMLNodeList_add(struct XMLNodeList *self, struct XMLNode *parent, struct XMLNode *node) {
    int success = FALSE;

    if (NULL != self) {
        if (0 == self->size && NULL == self->data) {
            self->data = calloc(1, sizeof(struct XMLNode*));

            if (NULL != self->data)
                self->heapSize = 1;
        }

        while (self->size >= self->heapSize) {
            self->heapSize *= 2;
            self->data = realloc(self->data, sizeof(struct XMLNode*) * self->heapSize);
            memset(self->data + self->size, '\0', (sizeof(struct XMLNode*) * (self->heapSize - self->size)));
        }

        if (NULL != self->data) {
            node->parent = parent;
            self->data[self->size++] = node;
            success = TRUE;
        }
    }

    return success;
} /* End of XMLNodeList_add */

/**
 * @brief Frees the given 'XMLNodeList'
 *
 * @param self - A reference to the struct you wish to free
 */
static void XMLNodeList_free(struct XMLNodeList *self) {
    if (NULL != self)     {
        size_t i = 0;
        for (;i<self->size;++i)
            self->data[i]->free(self->data[i]);

        for (i=0;i<self->heapSize;++i) {
            free(self->data[i]);
            self->data[i] = NULL;
        }

        free(self->data);
        self->data = NULL;

        memset(self, '\0', sizeof(struct XMLNodeList));
    }
} /* End of XMLNodeList_free */

/**
 * @brief Frees the given 'XMLDocument'
 *
 * @param self - A reference to the struct you wish to free
 */
static void XMLDocument_free(struct XMLDocument *self) {
    if (NULL != self) {
        XMLNode_free(self->root);
        free(self->root);
        self->root = NULL;

        free(self->version);
        free(self->encoding);

        self->version = self->encoding = NULL;
        self->success = FALSE;
    }
} /* End of XMLDocument_free */

/**
 * @brief Searches the 'haystack' to see if it ends with the 'needle'
 *
 * @param haystack - The string to search through
 * @param needle - The item to search for
 * @return success - A flag indicating the status of the subroutine
 */
static int lxmlEndsWith(const char *haystack, const char *needle) {
    int success = FALSE;

    if (NULL != haystack && NULL != needle) {
        size_t h_len = strlen(haystack),
               n_len = strlen(needle);

        if (h_len >= n_len) {
            size_t i = 0;
            for (; i < n_len; ++i)
                if (haystack[h_len - n_len + i] != needle[i])
                    break;

            if (i == n_len)
                success = TRUE;
        }
    }

    return success;
} /* End of lxmlEndsWith */

static enum TagType lxmlParseAttrs(char *buf, size_t *i, char *lex, size_t *lexi, struct XMLNode *curr_node) {
    enum TagType type = TAG_START;
    struct XMLAttribute curr_attr = { 0, 0, XMLAttribute_free };

    while ('>' != buf[*i]) {
        lex[(*lexi)++] = buf[(*i)++];

        /* Tag name */
        if ((' ' == buf[*i] || '>' == buf[*i+1]) && NULL == curr_node->tag) {
            if ('>' == buf[*i+1])
                lex[(*lexi)++] = buf[*i];

            lex[*lexi] = '\0';
            /* FIXME: Can be NULL */
            curr_node->tag = lxmlStrdup(lex);
            *lexi = 0;
            (*i)++;
            continue;
        }

        /* Usually ignore spaces */
        if (' ' == lex[*lexi-1]) {
            (*lexi)--;
        }

        /* Attribute key */
        if ('=' == buf[*i]) {
            lex[*lexi] = '\0';
            curr_attr.key = lxmlStrdup(lex);
            *lexi = 0;
            continue;
        }

        /* Attribute value */
        if ('"' == buf[*i]) {
            if (NULL == curr_attr.key) {
                fprintf(stderr, "Value has no key\n");
                type = TAG_START;
                break;
            }

            *lexi = 0;
            (*i)++;

            while ('"' != buf[*i])
                lex[(*lexi)++] = buf[(*i)++];
            lex[*lexi] = '\0';
            curr_attr.value = lxmlStrdup(lex);

            curr_node->attributes.add(&curr_node->attributes, curr_attr);

            curr_attr.free(&curr_attr);
            *lexi = 0;
            (*i)++;
            continue;
        }

        /* Inline node */
        if ('/' == buf[*i - 1] && '>' == buf[*i]) {
            lex[*lexi] = '\0';
            if (NULL == curr_node->tag) /* FIXME: Can be NULL */
                curr_node->tag = lxmlStrdup(lex);
            (*i)++;
            type = TAG_INLINE;
            break;
        }
    }

    curr_attr.free(&curr_attr);

    return type;
} /* End of lxmlParseAttrs */

/**
 * @brief Reads the entire file, pointed to by 'fp' into memory
 *        and returns it as a 'calloc' piece of memory 'buf'
 *
 * @param fp - The file pointer to read
 * @return buf - The buffer containing all of the files contents
 */
static char* lxmlReadXmlContentsIntoMemory(FILE *fp) {
    char *buf = NULL;

    if (NULL != fp) {
        long fileSize = 0;
        size_t bytesRead = 0;

        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        buf = calloc(fileSize+1, sizeof(char));

        if (NULL != buf) {
            bytesRead = fread(buf, sizeof(char), fileSize, fp);

            if (fileSize > 0 && bytesRead != (size_t) fileSize) {
                free(buf);
                buf = NULL;
            }
        }
    }

    return buf;
} /* End of lxmlReadXmlContentsIntoMemory */

static void node_out(FILE *file, struct XMLNode *node, int indent, int times) {
    size_t i = 0, j = 0;

    for (; i < node->children.size; ++i) {
        struct XMLNode *child = node->children.data[i];

        if (times > 0)
            fprintf(file, "%*s", indent * times, " ");

        fprintf(file, "<%s", child->tag);
        for (j = 0; j < child->attributes.size; ++j) {
            struct XMLAttribute *attr = child->attributes.attribute[j];

            if (NULL != attr && NULL != attr->value && 0 != strcmp(attr->value, ""))
                fprintf(file, " %s=\"%s\"", attr->key, attr->value);
        }

        if (0 == child->children.size && NULL == child->inner_text)
            fprintf(file, " />\n");
        else {
            fprintf(file, ">");
            if (0 == child->children.size)
                fprintf(file, "%s</%s>\n", child->inner_text, child->tag);
            else {
                fprintf(file, "\n");
                node_out(file, child, indent, times + 1);
                if (times > 0)
                    fprintf(file, "%*s", indent * times, " ");
                fprintf(file, "</%s>\n", child->tag);
            }
        }
    }
} /* End of node_out */

/**
 * @brief Clones the string located at 'str'
 *
 * @param str - The string to duplicate
 * @return strdup - The cloned string
 */
static char* lxmlStrdup(const char *str) {
    char *strdup = NULL;

    if (NULL != str) {
        size_t strlength = strlen(str);

        if (0 != strlength) {
            strdup = calloc(strlength+1, sizeof(char));

            if (NULL != strdup)
                strcpy(strdup, str);
        }
    }

    return strdup;
} /* End of lxmlStrdup */

/**
 * @brief Parses the input buffer for the end of node
 *
 * @param buf - The buffer to parse
 * @param bufOffset - The offet to begin parsing from
 * @param lex - The buffer to place the lexime into
 * @param lexi - The offset for the lexime buffer
 *
 * @return success - A Flag indicating the status of the subroutine
 */
static int lxmlParseEndOfNode(const char *buf, size_t *bufOffset, char *lex, size_t lexBufSize, size_t *lexi) {
    int success = FALSE;

    if (NULL != buf && NULL != bufOffset && NULL != lex && NULL != lexi) {
        success = TRUE;

        while ('>' != buf[*bufOffset]) {
            if ('\0' != buf[*bufOffset] || *lexi >= lexBufSize)
                lex[(*lexi)++] = buf[(*bufOffset)++];
            else {
                success = FALSE;
                break;
            }
        }

        lex[*lexi] = '\0';
    }

    return success;
} /* End of lxmlParseEndOfNode*/

/*******************Private End********************/
#endif /* LITTLE_XML_H */
