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
#endif /* TRUE */
#ifndef FALSE
    #define FALSE 0
#endif /* FALSE */

/********************Define End********************/

/********************Enum Start********************/

/**
 * @brief Enumeration of supported XML Tag Types.
 */
enum TagType {
    TAG_UNSUPPORTED = -1,
    TAG_START,
    TAG_INLINE
};

/**
 * @brief An enumeration of XML Token types.
 */
enum XMLTokenType {
    XML_TOKEN_NONE = 0,
    XML_TOKEN_START_TAG,
    XML_TOKEN_END_TAG,
    XML_TOKEN_ATTR_KEY,
    XML_TOKEN_ATTR_VALUE,
    XML_TOKEN_TEXT,
    XML_TOKEN_EOF
};

/*********************Enum End*********************/

/************Struct-Declaration Start**************/

/**
 * @brief Struct to hold the XML Token.
 */
struct XMLToken {
    enum XMLTokenType type;
    char *raw, *value;

    void (*free)(struct XMLToken * const);
};

/**
 * @brief Struct for the XML Lexer.
 */
struct XMLLexer {
    const char *buf;
    size_t i, len;

    struct XMLToken current;
};

/**
 * @brief An XML Attribute struct containing the key and value.
 */
struct XMLAttribute {
    char *key, *value;

    void (*free)(struct XMLAttribute * const);
};

/**
 * @brief A list of XML Attribute structs.
 */
struct XMLAttributeList {
    size_t size, heapSize;
    struct XMLAttribute **attribute;

    int (*add)(struct XMLAttributeList * const , const struct XMLAttribute);
    void (*free)(struct XMLAttributeList * const);

    char* (*getAttributeValue)(const struct XMLAttributeList * const, const char * const);
    struct XMLAttribute* (*getAttribute)(const struct XMLAttributeList * const, const char * const);
};

/**
 * @brief A list of XML Node structs.
 */
struct XMLNodeList {
    size_t size, heapSize;
    struct XMLNode **data;

    int (*add)(struct XMLNodeList * const, const struct XMLNode * const, struct XMLNode * const);
    struct XMLNode* (*createAndAppend)(struct XMLNodeList * const);
    void (*free)(struct XMLNodeList * const);
};

/**
 * @brief An XML Node struct containing the details about the parsed XML entity.
 */
struct XMLNode {
    char *tag, *inner_text;

    struct XMLNode *parent;
    struct XMLAttributeList attributes;
    struct XMLNodeList children;

    int (*add)(struct XMLNode * const, struct XMLNode * const);
    struct XMLNode* (*createAndAppend)(struct XMLNode * const);
    void (*free)(struct XMLNode * const );

    char* (*getAttributeValue)(const struct XMLNode * const, const char * const);
    struct XMLAttribute* (*getAttribute)(const struct XMLNode * const, const char * const);
    struct XMLNode* (*getImmediateElementByTagName)(const struct XMLNode * const , const char * const);
};

/**
 * @brief The complete parsed XML document.
 */
struct XMLDocument {
    struct XMLNode *root;
    char *version, *encoding;
    int success;

    void (*free)(struct XMLDocument * const);
};

/*************Struct-Declaration End***************/

/*****************Prototype Start******************/

/* XML Token Functions Prototype Start */
static struct XMLToken XMLToken_init();
static void XMLToken_free(struct XMLToken *self);

/* XML Attribute Functions Prototype Start */

struct XMLAttribute* XMLAttribute_init(const char * const key, const char * const value);

static void XMLAttribute_free(struct XMLAttribute * const self);

/* XML Attribute Functions Prototype End */

/* XML Attribute List Functions Prototype Start */

struct XMLAttributeList  XMLAttributeList_init();

static int XMLAttributeList_add(struct XMLAttributeList * const self, const struct XMLAttribute attr);
static void XMLAttributeList_free(struct XMLAttributeList * const self);

static struct XMLAttribute* XMLAttributeList_getAttribute(const struct XMLAttributeList * const self, const char * const key);
static char* XMLAttributeList_getAttributeValue(const struct XMLAttributeList * const self, const char * const key);

/* XML Attribute List Functions Prototype End */

/* XML Node Functions Prototype Start */

struct XMLNode* XMLNode_init();

static int XMLNode_add(struct XMLNode * const self, struct XMLNode * const node);
static struct XMLNode* XMLNode_createAndAppend(struct XMLNode *self);
static void XMLNode_free(struct XMLNode * const self);
static struct XMLAttribute* XMLNode_getAttribute(const struct XMLNode * const self, const char * const key);
static char* XMLNode_getAttributeValue(const struct XMLNode * const self, const char * const key);
static struct XMLNode* XMLNode_getImmediateElementByTagName(const struct XMLNode * const self, const char * const tagName);
static struct XMLNode XMLNodeInitStack();

/* XML Node Functions Prototype End */

/* XML Node List Functions Prototype Start */

struct XMLNodeList XMLNodeList_init();

static int XMLNodeList_add(struct XMLNodeList * const self, const struct XMLNode * const parent, struct XMLNode * const node);
static struct XMLNode* XMLNodeList_createAndAppend(struct XMLNodeList *self);
static void XMLNodeList_free(struct XMLNodeList * const self);

/* XML Node List Functions Prototype End */

/* XML Document Functions Prototype Start */

struct XMLDocument XMLDocument_load(FILE *fp);
int XMLDocument_writeToPath(const struct XMLDocument * const doc, const char * const path, const int indent);
int XMLDocument_write(const struct XMLDocument * const doc, FILE *fp, const int indent);

static void XMLDocument_free(struct XMLDocument * const self);

/* XML Document Functions Prototype End */

static void lxmlParseAttributesFromTag(const char * const str, struct XMLNode * const node);

static char* lxmlStrdup(const char * const str);
static char* lxmlStrndup(const char * const str, const size_t strLen);
static char* lxmlReadXmlContentsIntoMemory(FILE *fp);

static int lxmlEndsWith(const char * const haystack, const char * const needle);

static void node_out(FILE *file, const struct XMLNode * const node, const char * const indentation, const int indent, const int times);

static struct XMLToken lxmlNextToken(struct XMLLexer * const lexer);

/******************Prototype End*******************/

/*******************Public Start*******************/

/**
 * @bried Free's the supplied XMLToken.
 */
static void XMLToken_free(struct XMLToken *self) {
    if (NULL != self) {
        if (NULL != self->raw)
            free(self->raw);

        if (NULL != self->value)
            free(self->value);

        *self = XMLToken_init();
    }
} /* End of XMLToken_free */

/**
 * @brief Creates an initialised XMLToken.
 *
 * @return token - A newly initialised token
 */
static struct XMLToken XMLToken_init() {
    struct XMLToken token = { XML_TOKEN_EOF, NULL, NULL, XMLToken_free };
    return token;
} /* End of XMLToken_init */

/**
 * @brief Initialises an 'XMLAttribute'
 *        If 'key'|'value' are supplied then a deep-copy of these values will be made.
 *        **Note:** Both 'key' and 'value' if these wish to be set.
 *
 * @param key   - Optional XML key attribute to set
 * @param value - Optional XML value attribute to set
 * @return attr - A new heap allocated 'XMLAttribute'
 */
struct XMLAttribute* XMLAttribute_init(const char * const key, const char * const value) {
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
 * @brief Initialises an 'XMLAttributeList'.
 */
struct XMLAttributeList XMLAttributeList_init() {
    struct XMLAttributeList list = { 0, 0, 0, XMLAttributeList_add, XMLAttributeList_free, XMLAttributeList_getAttributeValue, XMLAttributeList_getAttribute };
    return list;
} /* End of XMLAttributeList_init */

/**
 * @brief Initialises an 'XMLNodeList'.
 */
struct XMLNodeList XMLNodeList_init() {
    struct XMLNodeList list = { 0, 0, 0, XMLNodeList_add, XMLNodeList_createAndAppend, XMLNodeList_free };
    return list;
} /* End of XMLNodeList_init */

/**
 * @brief Initialises an 'XMLNode'.
 *
 * @return node - A 'calloc' 'XMLNode' that has been initialised
 */
struct XMLNode* XMLNode_init() {
    struct XMLNode *node = calloc(1, sizeof(struct XMLNode));

    if (NULL != node)
        *node = XMLNodeInitStack();

    return node;
} /* End of XMLNode_init */

/**
 * @brief Frees an 'XMLNode' struct.
 */
void XMLNode_free(struct XMLNode * const node) {
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
 * @brief Loads an XML document pointed to by 'fp'.
 *
 * @param  fp  - The 'FILE' that's been loaded
 * @return doc - The serialised XML document
 */
struct XMLDocument XMLDocument_load(FILE *fp) {
    struct XMLDocument doc = { 0, 0, 0, 0, XMLDocument_free };
    char *buf = lxmlReadXmlContentsIntoMemory(fp);

    if (NULL != buf) {
        struct XMLLexer lexer = { 0 };
        struct XMLNode *curr = NULL;
        struct XMLToken tok = XMLToken_init();

        lexer.buf = buf;
        lexer.len = strlen(buf);
        lexer.i = 0;

        doc.root = XMLNode_init();
        curr = doc.root;

        do {
            tok.free(&tok);
            tok = lxmlNextToken(&lexer);

            if (XML_TOKEN_START_TAG == tok.type) {
                struct XMLNode *child = curr->createAndAppend(curr);

                if (NULL != child) {
                    child->tag = lxmlStrdup(tok.value);

                    if (NULL != tok.raw)
                        lxmlParseAttributesFromTag(tok.raw, child);
                    curr = child;
                }
            } else if (XML_TOKEN_END_TAG == tok.type) {
                if (NULL != curr && NULL != curr->parent)
                    curr = curr->parent;
            } else if (XML_TOKEN_TEXT == tok.type) {
                if (NULL != curr && strlen(tok.value) > 0) {
                    if (NULL == curr->inner_text)
                        curr->inner_text = lxmlStrdup(tok.value);
                }
            }

        } while (XML_TOKEN_EOF != tok.type);

        free(buf);
        buf = NULL;
    }

    doc.success = (NULL != doc.root) ? TRUE : FALSE;
    return doc;
}

/**
 * @brief Writes the given 'XMLDocument' to the prescribed 'path' using the 'indent' to specify white spaces.
 *
 * @param  doc     - The document to write out
 * @param  path    - The path to write out to
 * @param  indent  - The number of white spaces to place
 * @return success - A flag indicating the status of the subroutine
 */
int XMLDocument_writeToPath(const struct XMLDocument * const doc, const char * const path, const int indent) {
    int success = FALSE;
    FILE *fp = fopen(path, "w");

    if (NULL != fp)
        success = XMLDocument_write(doc, fp, indent);
    else
        fprintf(stderr, "Could not open file '%s'\n", path);

    return success;
} /* End of XMLDocument_writeToPath */

/**
 * @brief Writes the given 'XMLDocument' to the prescribed 'path' using the 'indent' to specify white spaces.
 *
 * @param  doc     - The document to write out
 * @param  fp      - The file pointer to write to
 * @param  indent  - The number of white spaces to place
 * @return success - A flag indicating the status of the subroutine
 */
int XMLDocument_write(const struct XMLDocument * const doc, FILE *fp, const int indent) {
    if (NULL != fp) {
        fprintf(
            fp, "<?xml version=\"%s\" encoding=\"%s\" ?>\n",
            (doc->version) ? doc->version : "1.0",
            (doc->encoding) ? doc->encoding : "UTF-8"
        );
        node_out(fp, doc->root, NULL, indent, 0);
    }
    return (NULL != fp) ? TRUE : FALSE;
} /* End of XMLDocument_write */

/********************Public End********************/

/******************Private Start*******************/

/**
 * @brief Frees the data on a given 'XMLAttribute'
 *
 * @param self - A reference to the 'XMLAttribute' to free
 */
static void XMLAttribute_free(struct XMLAttribute * const self) {
    if (NULL != self) {
        free(self->key);
        free(self->value);
        self->key = self->value = NULL;
    }
} /* End of XMLAttribute_free */

/**
 * @brief Adds a given 'XMLAttribute' to 'self'.
 *
 * @param  self    - A pointer to the 'XMLAttributeList' to add the 'XMLAttribute to
 * @param  attr    - The 'XMLAttribute' to add to the list
 * @return success - A Flag indicating the status of the subroutine
 */
static int XMLAttributeList_add(struct XMLAttributeList * const self, const struct XMLAttribute attr) {
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
 * @brief Frees the given 'XMLAttributeList'.
 *
 * @param self - The 'XMLAttributeList' to free
 */
static void XMLAttributeList_free(struct XMLAttributeList * const self) {
    if (NULL != self) {
        size_t i = 0;
        for (;i < self->size; ++i)
            self->attribute[i]->free(self->attribute[i]);

        for (i = 0; i < self->heapSize; ++i) {
            free(self->attribute[i]);
            self->attribute[i] = NULL;
        }

        free(self->attribute);
        self->attribute = NULL;

        memset(self, '\0', sizeof(struct XMLAttributeList));
    }
} /* End of XMLAttributeList_free */

/**
 * @brief Obtains the associated 'XMLAttribute' given the 'key'.
 *
 * @param  self    - A reference to the 'XMLAttributeList' to search through
 * @param  key     - The value to search for
 * @return retAttr - The 'XMLAttribute' containing the associated 'key'
 */
static struct XMLAttribute* XMLAttributeList_getAttribute(const struct XMLAttributeList * const self, const char * const key) {
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
} /* End of XMLAttributeList_getAttribute */

/**
 * @brief Obtains the associated 'value' given the 'key' from the 'XMLAttributeList'.
 *
 * @param  self    - A reference to the 'XMLAttributeList' to search from
 * @param  key     - A string containing the value to search for
 * @return attrVal - The associated attribute value from the 'XMLAttributeList'
 */
static char* XMLAttributeList_getAttributeValue(const struct XMLAttributeList * const self, const char * const key) {
    char *attrVal = NULL;

    if (NULL != self && NULL != key) {
        size_t i = 0;
        for (; i < self->size; ++i) {
            struct XMLAttribute *attr = self->attribute[i];

            if (NULL != attr && NULL != attr->key) {
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
 * @brief Convenience function to add an 'XMLNode' to the 'children'.
 *        **Note:** See 'XMLNodeList_add' for implementation.
 *
 * @param  self    - A reference to the struct you wish to add the node to
 * @param  node    - The 'XMLNode' to add to 'self'
 * @return success - A flag indicating the status of the subroutine
 */
static int XMLNode_add(struct XMLNode * const self, struct XMLNode * const node) {
    return (NULL != self && NULL != self->children.add && NULL != node) ? self->children.add(&self->children, self, node) : FALSE;
} /* End of XMLNode_add */

/**
 * @brief Convenience function to add and append 'XMLNode' to the 'children'.
 *        **Note:** See 'XMLNode_add' for implementation.
 *
 * @param  self - A reference to the struct you wish to add the node to
 * @return node - The new node that's been appended to the XMLNodeList
 */
static struct XMLNode* XMLNode_createAndAppend(struct XMLNode *self) {
    struct XMLNode *node = NULL;

    if (NULL != self) {
        node = XMLNode_init();

        if (NULL != node) {
            int success = self->add(self, node);

            if (FALSE == success) {
                node->free(node);
                free(node);
                node = NULL;
            }
        }
    }

    return node;
} /* End of XMLNode_createAndAppend */

/**
 * @brief Obtains the associated 'XMLAttribute' given the 'key'.
 *        **Note:** See 'XMLAttributeList_getAttribute' for implementation.
 *
 * @param  self    - A reference to the 'XMLNode' to search through
 * @param  key     - The value to search for
 * @return retAttr - The 'XMLAttribute' containing the associated 'key'
 */
static struct XMLAttribute* XMLNode_getAttribute(const struct XMLNode * const self, const char * const key) {
    return (NULL != self && NULL != self->attributes.getAttribute && NULL != key) ? self->attributes.getAttribute(&self->attributes, key) : NULL;
} /* End of XMLNode_getAttribute */

/**
 * @brief Obtains the associated 'value' given the 'key' from the 'XMLNode'
 *        **Note:** See 'XMLAttributeList_getAttributeValue' for implementation
 *
 * @param  self    - A reference to the 'XMLNode' to search from
 * @param  key     - A string containing the value to search for
 * @return attrVal - The associated attribute value from the 'XMLNode'
 */
static char* XMLNode_getAttributeValue(const struct XMLNode * const self, const char * const key) {
    return (NULL != self && NULL != self->attributes.getAttributeValue && NULL != key) ? self->attributes.getAttributeValue(&self->attributes, key) : NULL;
} /* End of XMLNode_getAttributeValue */

/**
 * @brief Searches the immediate 'XMLNode' for the first child that matches the 'tagName'
 *        *NOTE:* This returns a pointer value to the 'XMLNode' and mustn't be free'd directly
 *
 * @param  self    - A reference to the 'XMLNode' to search from
 * @param  tagName - The name of the tag to search for
 * @return node    - The found 'XMLNode' or 'NULL' if not present
 */
static struct XMLNode* XMLNode_getImmediateElementByTagName(const struct XMLNode * const self, const char * const tagName) {
    struct XMLNode *node = NULL;

    if (NULL != self && NULL != tagName && 0 != self->children.size) {
        size_t childIndex = 0;

        for (;childIndex < self->children.size; ++childIndex) {
            if (0 == strcmp(tagName, self->children.data[childIndex]->tag)) {
                node = self->children.data[childIndex];
                break;
            }
        }
    }

    return node;
} /* End of XMLNode_getImmediateElementByTagName */

/**
 * @brief Creates a stack allocated 'XMLNode', initialises it then returns it.
 *
 * @return node - A stack allocated and initialised XMLNode
 */
static struct XMLNode XMLNodeInitStack() {
    struct XMLNode node = { NULL, NULL, NULL, { 0 }, { 0 }, XMLNode_add, XMLNode_createAndAppend, XMLNode_free, XMLNode_getAttributeValue, XMLNode_getAttribute, XMLNode_getImmediateElementByTagName };

    node.attributes = XMLAttributeList_init();
    node.children = XMLNodeList_init();

    return node;
} /* End of XMLNodeInitStack */

/**
 * @brief Adds a 'XMLNode' to the 'XMLNodeList'
 *
 * @param  self    - A reference to the struct you wish to add the node to
 * @param  parent  - A reference to the parent
 * @param  node    - The 'XMLNode' to add to 'self'
 * @return success - A flag indicating the status of the subroutine
 */
static int XMLNodeList_add(struct XMLNodeList * const self, const struct XMLNode * const parent, struct XMLNode * const node) {
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
            node->parent = (struct XMLNode*) parent;
            self->data[self->size++] = node;
            success = TRUE;
        }
    }

    return success;
} /* End of XMLNodeList_add */

/**
 * @brief Adds a 'XMLNode' to the 'XMLNodeList'
 *
 * @param  self - A reference to the struct you wish to add the node to
 * @return node - The new node that's been appended to the XMLNodeList
 */
static struct XMLNode* XMLNodeList_createAndAppend(struct XMLNodeList *self) {
    struct XMLNode *node = NULL;

    if (NULL != self) {
        node = XMLNode_init();

        if (NULL != node) {
            int success = self->add(self, NULL, node);

            if (FALSE == success) {
                node->free(node);
                free(node);
                node = NULL;
            }
        }
    }

    return node;
} /* End of XMLNodeList_createAndAppend */

/**
 * @brief Frees the given 'XMLNodeList'
 *
 * @param self - A reference to the struct you wish to free
 */
static void XMLNodeList_free(struct XMLNodeList * const self) {
    if (NULL != self)     {
        size_t i = 0;
        for (; i < self->size; ++i)
            self->data[i]->free(self->data[i]);

        for (i = 0; i < self->heapSize; ++i) {
            free(self->data[i]);
            self->data[i] = NULL;
        }

        free(self->data);
        self->data = NULL;

        *self = XMLNodeList_init();
    }
} /* End of XMLNodeList_free */

/**
 * @brief Frees the given 'XMLDocument'
 *
 * @param self - A reference to the struct you wish to free
 */
static void XMLDocument_free(struct XMLDocument * const self) {
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
 * @param  haystack - The string to search through
 * @param  needle   - The item to search for
 * @return success  - A flag indicating the status of the subroutine
 */
static int lxmlEndsWith(const char * const haystack, const char * const needle) {
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

/**
 * @brief Fetch the next token from the XML lexer
 *
 * This function returns start tags, end tags, text, and EOF tokens.
 * Start tags populate tok.value (tag name) and tok.raw (attributes as raw string).
 *
 * @param lexer - pointer to the lexer
 * @return XMLToken
 */
struct XMLToken lxmlNextToken(struct XMLLexer *lexer) {
    struct XMLToken tok = XMLToken_init();

    if (NULL == lexer || NULL == lexer->buf) {
        return tok;
    }

    size_t i = lexer->i;
    const size_t len = lexer->len;
    const char * const buf = lexer->buf;

    /* Skip leading whitespace */
    while (i < len && (' ' == buf[i] || '\t' == buf[i] || '\n' == buf[i] || '\r' == buf[i])) {
        i++;
    }

    /* EOF check */
    if (i >= len) {
        tok.type = XML_TOKEN_EOF;
        lexer->i = i;
        return tok;
    }

    /* Start of a tag? */
    if ('<' == buf[i]) {
        i++;

        /* End tag? </...> */
        int isEndTag = 0;
        if ('/' == buf[i]) {
            isEndTag = 1;
            i++;
        }

        /* Skip whitespace after '<' or '</' */
        while (i < len && (' ' == buf[i] || '\t' == buf[i] || '\n' == buf[i] || '\r' == buf[i])) {
            i++;
        }

        /* Capture tag name */
        size_t startName = i;
        while (i < len && (' ' != buf[i] && '\t' != buf[i] && '\n' != buf[i] && '\r' != buf[i] && '>' != buf[i] && '/' != buf[i])) {
            i++;
        }

        size_t nameLen = i - startName;
        if (0 < nameLen) {
            tok.value = lxmlStrndup(&buf[startName], nameLen);
        }

        /* Skip whitespace after tag name */
        while (i < len && (' ' == buf[i] || '\t' == buf[i] || '\n' == buf[i] || '\r' == buf[i])) {
            i++;
        }

        /* Capture raw attributes up to '>' or '/>' */
        size_t rawStart = i;
        while (i < len && '>' != buf[i]) {
            /* Detect self-closing tag */
            if ('/' == buf[i]) {
                i++;
                break;
            }
            i++;
        }
        size_t rawLen = i - rawStart;
        if (0 < rawLen) {
            tok.raw = lxmlStrndup(&buf[rawStart], rawLen);
        }

        /* Skip closing '>' */
        if (i < len && '>' == buf[i]) {
            i++;
        }

        /* Set token type */
        if (isEndTag) {
            tok.type = XML_TOKEN_END_TAG;
        } else {
            tok.type = XML_TOKEN_START_TAG;
        }

        /* Handle self-closing as start + immediate end if needed (optional) */
        /* You can handle this in XMLDocument_load */

        lexer->i = i;
        return tok;
    }

    /* Otherwise, parse text node until next '<' */
    size_t textStart = i;
    while (i < len && '<' != buf[i]) {
        i++;
    }
    size_t textLen = i - textStart;
    if (0 < textLen) {
        tok.type  = XML_TOKEN_TEXT;
        tok.value = lxmlStrndup(&buf[textStart], textLen);
    }

    lexer->i = i;
    return tok;
} /* End of lxmlNextToken */

/**
 * @brief Parse raw attributes from a start tag and populate node->attributes
 *
 * This expects a string containing everything after the tag name, up to '>'
 * E.g. raw = " version=\"1.0\" encoding='UTF-8' attr=\"value\" "
 *
 * @param raw  - the raw attribute string
 * @param node - the XMLNode to populate
 */
static void lxmlParseAttributesFromTag(const char * const raw, struct XMLNode * const node) {
    if (NULL != raw && NULL != node) {
        char *key = NULL, *value = NULL, quote = '\0';
        size_t i = 0, keyStart = 0, keyLen = 0, valLen = 0, valStart = 0;
        size_t len = strlen(raw);

        while (i < len) {
            /* Skip whitespace */
            while (i < len && (' ' == raw[i] || '\t' == raw[i] || '\n' == raw[i] || '\r' == raw[i])) {
                i++;
            }

            /* End of raw? */
            if (i >= len) {
                break;
            }

            /* Parse attribute key */
            keyStart = i;
            while (i < len && '=' != raw[i] && ' ' != raw[i] && '\t' != raw[i] && '\n' != raw[i] && '\r' != raw[i]) {
                i++;
            }
            keyLen = i - keyStart;
            if (0 == keyLen) {
                break; /* No more attributes */
            }

            key = lxmlStrndup(&raw[keyStart], keyLen);

            /* Skip whitespace before '=' */
            while (i < len && (' ' == raw[i] || '\t' == raw[i] || '\n' == raw[i] || '\r' == raw[i])) {
                i++;
            }

            /* Must have '=' */
            if (i >= len || '=' != raw[i]) {
                free(key);
                key = NULL;
                break;
            }
            i++; /* skip '=' */

            /* Skip whitespace after '=' */
            while (i < len && (' ' == raw[i] || '\t' == raw[i] || '\n' == raw[i] || '\r' == raw[i])) {
                i++;
            }

            /* Attribute value */
            if (i >= len) {
                free(key);
                key = NULL;
                break;
            }

            quote = 0;
            if ('"' == raw[i] || '\'' == raw[i]) {
                quote = raw[i];
                ++i;
            }

            valStart = i;
            if (0 != quote) {
                /* Value is quoted */
                while (i < len && quote == raw[i]) { i++; }
                while (i < len && quote != raw[i]) { i++; }
            } else {
                /* Unquoted value until whitespace */
                while (i < len && ' ' != raw[i] && '\t' != raw[i] && '\n' != raw[i] && '\r' != raw[i])
                    ++i;
            }

            valLen = i - valStart;
            value = lxmlStrndup(&raw[valStart], valLen);

            /* Create attribute and add to node */
            struct XMLAttribute *attr = XMLAttribute_init(key, value);
            if (NULL != attr) {
                node->attributes.add(&node->attributes, *attr);
                attr->free(attr);
                free(attr);
            }

            free(key);
            free(value);
            key = value = NULL;

            /* Skip closing quote if any */
            if (0 != quote && i < len && quote == raw[i])
                ++i;
        } /* End while */
    }
} /* End lxmlParseAttributesFromTag */

/**
 * @brief Reads the entire file, pointed to by 'fp' into memory
 *        and returns it as a 'calloc' piece of memory 'buf'
 *
 * @param  fp  - The file pointer to read
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

/**
 * @brief Writes out the 'struct XMLNode' to a 'FILE'.
 *
 * @param file        - The file to write the node out to
 * @param node        - The node to write out
 * @param indentation - The amount of indentation to use
 * @param indent      - The current level of indentation
 *
 * @param times       - The number of times to indent
 */
static void node_out(FILE *file, const struct XMLNode * const node, const char * const indentation, const int indent, const int times) {
    size_t i = 0, j = 0;
    const char * const indentationStr = (NULL != indentation) ? indentation : " ";

    for (; i < node->children.size; ++i) {
        struct XMLNode *child = node->children.data[i];

        if (times > 0)
            fprintf(file, "%*s", indent * times, indentationStr);

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
                node_out(file, child, indentationStr, indent, times + 1);
                if (times > 0)
                    fprintf(file, "%*s", indent * times, " ");
                fprintf(file, "</%s>\n", child->tag);
            }
        }
    }
} /* End of node_out */

/**
 * @brief Clones the string located at 'str'.
 *        *Note:* Merely delegates to 'lxmlStrndup' where if 'str' is 'NULL' then 'strLen' will be set to '0'.
 *
 * @param  str    - The string to duplicate
 * @return strduped - The cloned string
 */
static char* lxmlStrdup(const char * const str) {
    return lxmlStrndup(str, (NULL != str) ? strlen(str) : 0 );
} /* End of lxmlStrdup */

/**
 * @brief Clones the string located at 'str' up to'strLen'.
 *
 * @param   str     - The string to duplicate
 * @param  strLen   - The amount of bytes to copy
 * @return strduped - The cloned string
 */
static char* lxmlStrndup(const char * const str, const size_t strLen) {
    char *strduped = NULL;

    if (NULL != str) {
        strduped = calloc(strLen + 1, sizeof(char));

        if (NULL != strduped)
            memcpy(strduped, str, strLen);
    }

    return strduped;
} /* End of lxmlStrndup */

/*******************Private End********************/
#endif /* LITTLE_XML_H */
