/**
 * @file testmain.c
 *
 * @version 1.0
 *
 * @author Calum Judd Anderson
 *
 * @brief Unit test coverage for 'lxml.h'
 */

#include <stdio.h>
#include <assert.h>

#include "lxml.h"

/* 'fmemopen' not available in all standards */
#if defined __USE_XOPEN || defined __USE_XOPEN2K8
#define LXML_HAVE_FMEMOPEN
#endif

#define LXML_TEST_DEFAULT_ATTRS_SIZE 3
/* Ugly 'Hack' for C89 Compliance */
#define LXML_TEST_DEFAULT_ATTRS \
    struct XMLAttribute *attrs[LXML_TEST_DEFAULT_ATTRS_SIZE] = { 0 }; \
    struct XMLAttribute *attr0 = XMLAttribute_init("Hello", "World"), *attr1 = XMLAttribute_init("Foo", "Bar"), *attr2 = XMLAttribute_init("Fizz", "Buzz"); \
    attrs[0] = attr0; attrs[1] = attr1; attrs[2] = attr2;

#define TEST_BUF 1024

#define TEST_XML_TAG "<?xml"

#define TEST_HAYSTACK_1 "<description>This defines a person</description>"
#define TEST_NEEDLE_1 "</description>"
#define TEST_NEEDLE_INVALID "</Description>"
#define TEST_NEEDLE_INVALID2 "</description<"

#define TEST_XML_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"

#define TEST_XML_VALID_1 TEST_XML_HEADER \
"<struct name=\"Person\">\
    <field name=\"name\" type=\"string\" />\
    <field name=\"age\" type=\"int\" />\
    <description>This defines a person</description>\
</struct>"

#define TEST_XML_VALID_1_SIZE (sizeof(TEST_XML_VALID_1)-1)

#define TXML_TEST_XML_NODE_TREE_CHILDREN_SIZE 12
#define TEST_EXAMPLE_XML_NODE_TREE_STRING "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<top>\n  <foo />\n  <bar />\n  <baz />\n</top>\n<middle>\n  <child />\n  <child />\n  <child />\n</middle>\n<bottom>\n  <nest>\n    <nest>\n      <nest />\n    </nest>\n  </nest>\n</bottom>\n"

static struct XMLNode* tlxmlCreateTestXMLNodeTree();

static int lxmlTestPopulateAttributeListWith(struct XMLAttributeList *list, struct XMLAttribute **attrs, size_t attrsSize);
static int lxmlTestPopulateXmlAttributeListWithDefaultAttributes(struct XMLAttributeList *list);
static int lxmlTestGetDefaultAttributeValuesFromAttributeList(struct XMLAttributeList list);
static int lxmlTestGetDefaultAttributesFromAttributeList(struct XMLAttributeList list);

static void lxmlTestFreeAttrs(struct XMLAttribute **attrs, size_t attrsSize);

static int lxmlTestAttributeInit();
static int lxmlTestAttributeListInit();
static int lxmlTestAttributeListAdd();
static int lxmlTestAttributeListGetAttributeValue();

static int lxmlTestAttributeListGetAttributeValueMissing();
static int lxmlTestAttributeListGetAttribute();
static int lxmlTestAttributeListGetAttributeMissing();

static int lxmlTestParseAttributes();
static int lxmlTestParseAttributesPass();

static int lxmlTestNode();
static int lxmlTestNodeInit();
static int lxmlTestNodeAdd();
static int lxmlTestNodeGetImmediateElementByTagName();
static int lxmlTestNodeGetImmediateElementByTagNameNodeNotFound();

static int lxmlTestNodeListInit();
static int lxmlTestNodeListAdd();
static int lxmlTestNodeList();

static int lxmlTestEndsWith();
static int lxmlTestEndsWithPass();
static int lxmlTestEndsWithFail();
static int lxmlTestEndsWithFail2();

static int lxmlTestEndsWithFailHaystackLargerThanNeedle();
static int lxmlTestEndsWithFailNullHaystack();
static int lxmlTestEndsWithFailNullNeedle();

#ifdef LXML_HAVE_FMEMOPEN
static int tlxmlCompareXmlNodeListAgainstString(struct XMLNode *tree, char *str);

static int lxmlTestReadXmlContentsIntoMemory();
static int lxmlTestReadXmlContentsIntoMemoryPass();
static int lxmlTestReadXmlContentsIntoMemoryNullFp();

static int lxmlTestXMLDocument_load();
static int lxmlTestXmlDocumentLoadPass();

/**
 * @brief Compares an 'XMLNode' to a given string
 *
 * @param  tree    - The 'XMLNode' to compare
 * @param  str     - The serialised string to compare it against
 * @return success - A flag indicating the status of the subroutine
 */
static int tlxmlCompareXmlNodeListAgainstString(struct XMLNode *tree, char *str) {
    int success = FALSE;

    if (NULL != tree && NULL != str) {
        size_t strLen = strlen(str);

        unsigned char *buf = calloc(strLen + 3, sizeof(char));

        if (NULL != buf) {
            FILE *fp = fmemopen(buf, strLen + 2, "w");

            if (NULL != fp) {
                struct XMLDocument doc = { tree, "1.0", "UTF-8", TRUE, XMLDocument_free };

                success = XMLDocument_write(&doc, fp, 2);
                fclose(fp);
                fp = NULL;
               
                if (TRUE == success)
                    success = (0 == strcmp(str, (char*) buf)) ? TRUE : FALSE;

            }

            free(buf);
            buf = NULL;
        }
    }

    return success;
} /* End of tlxmlCompareXmlNodeListAgainstString */

static int lxmlTestReadXmlContentsIntoMemoryPass() {
    char *testXml = TEST_XML_VALID_1, *buf = NULL;
    FILE *fp = fmemopen(testXml, sizeof(TEST_XML_VALID_1), "r");

    assert(NULL != fp);

    buf = lxmlReadXmlContentsIntoMemory(fp);

    assert(NULL != buf);
    assert(0 == strcmp(buf, testXml));

    fclose(fp);
    fp = NULL;

    free(buf);
    buf = NULL;

    return TRUE;
} /* End of lxmlTestReadXmlContentsIntoMemoryPass */

static int lxmlTestReadXmlContentsIntoMemoryNullFp() {
    assert(NULL == lxmlReadXmlContentsIntoMemory(NULL));
    return TRUE;
} /* End of lxmlTestReadXmlContentsIntoMemoryNullFp */

static int lxmlTestReadXmlContentsIntoMemory() {
    int success = FALSE;

    success = lxmlTestReadXmlContentsIntoMemoryPass();
    success &= lxmlTestReadXmlContentsIntoMemoryNullFp();

    printf("lxmlReadXmlContentsIntoMemory: %s\n", (TRUE == success) ? "Pass" : "Fail");

    return success;
} /* End of lxmlTestReadXmlContentsIntoMemory */

static int lxmlTestXmlDocumentLoadPass() {
    int success = FALSE;
    char xmlDocument[TEST_XML_VALID_1_SIZE+1] = TEST_XML_VALID_1;
    struct XMLDocument doc = { 0 };
    FILE *fp = fmemopen(xmlDocument, TEST_XML_VALID_1_SIZE+1, "r");;

    assert(NULL != fp);

    doc = XMLDocument_load(fp);
    success = doc.success;
    assert(TRUE == success);

    doc.free(&doc);
    fclose(fp);
    fp = NULL;

    printf("lxmlTestXMLDocument_load: %s\n", (TRUE == success) ? "Pass" : "Fail");
    return success;
} /* End of lxmlTestXmlDocumentLoadPass */

static int lxmlTestXMLDocument_load() {
    int success = lxmlTestXmlDocumentLoadPass();
    return success;
} /* End of lxmlTestXMLDocument_load */

#endif /* LXML_HAVE_FMEMOPEN */

/**
 * @brief Helper function for unit tests to create an 'XMLNode' with following XML structure
 *
 * <root>
 *    <top>
 *      <foo/>
 *      <bar/>
 *      <baz/>
 *    </top>
 *    <middle>
 *       <child/>
 *       <child/>
 *       <child/>
 *    </middle>
 *    <bottom>
 *       <nest>
 *          <nest>
 *             <nest/>
 *          </nest>
 *       </nest>
 *    </bottom>
 * </root>
 *
 * @return tree - The 'XMLNode' of the above tree structure
 */
static struct XMLNode* tlxmlCreateTestXMLNodeTree() {
    struct XMLNode *tree = XMLNode_init(),
                   *tmp  = XMLNode_init(),
                   *tmp2 = XMLNode_init();
    size_t i = 0;

    assert(NULL != tree && NULL != tmp && NULL != tmp2);
    tree->add(tree, tmp);
    tmp->tag  = lxmlStrdup("top");
    tmp2->tag = lxmlStrdup("foo");

    tmp->add(tmp, tmp2);

    tmp2 = XMLNode_init();
    tmp2->tag = lxmlStrdup("bar");
    tmp->add(tmp, tmp2);

    tmp2 = XMLNode_init();
    tmp2->tag = lxmlStrdup("baz");
    tmp->add(tmp, tmp2);

    tmp = XMLNode_init();
    tree->add(tree, tmp);
    tmp->tag = lxmlStrdup("middle");

    tmp2 = XMLNode_init();
    tmp2->tag = lxmlStrdup("child");
    tmp->add(tmp, tmp2);

    tmp2 = XMLNode_init();
    tmp2->tag = lxmlStrdup("child");
    tmp->add(tmp, tmp2);

    tmp2 = XMLNode_init();
    tmp2->tag = lxmlStrdup("child");
    tmp->add(tmp, tmp2);

    tmp = XMLNode_init();
    tree->add(tree, tmp);
    tmp->tag = lxmlStrdup("bottom");

    for (i=0;i<3;++i) {
        tmp2 = XMLNode_init();
        tmp2->tag = lxmlStrdup("nest");
        tmp->add(tmp, tmp2);
        tmp = tmp2;
    }

#ifdef LXML_HAVE_FMEMOPEN
    /* Check to see that it serialised correctly */
    assert(TRUE == tlxmlCompareXmlNodeListAgainstString(tree, TEST_EXAMPLE_XML_NODE_TREE_STRING));
#endif

    return tree;
} /* End of tlxmlCreateTestXMLNodeTree */

/**
 * @brief Helper function to add an array of 'attr' pointers to 'list'
 *
 * @param list - The list to add the 'attrs' to
 * @param attrs - An array of 'attr' pointers
 * @param attrsSize - The size of the array
 * @return success - A flag indicating the status of the subroutine
 */
static int lxmlTestPopulateAttributeListWith(struct XMLAttributeList *list, struct XMLAttribute **attrs, size_t attrsSize) {
    int success = FALSE;

    if (NULL != list && NULL != attrs) {
        size_t i = 0;
       
        for (i=0;i<attrsSize;++i) {
            assert(NULL != attrs[i]);
            success = list->add(list, *attrs[i]);

            assert(TRUE == success);
        }
    }

    return success;
} /* End of lxmlTestPopulateAttributeListWith */

/**
 * @brief Helper function to populate the 'XMLAttributeList' with default test values
 *
 * @param list - The 'XMLAttributeList' to populate
 * @return success - A flag indicating the status of the subroutine
 */
static int lxmlTestPopulateXmlAttributeListWithDefaultAttributes(struct XMLAttributeList *list) {
    int success = FALSE;

    LXML_TEST_DEFAULT_ATTRS;
    success = lxmlTestPopulateAttributeListWith(list, attrs, LXML_TEST_DEFAULT_ATTRS_SIZE);
    
    lxmlTestFreeAttrs(attrs, LXML_TEST_DEFAULT_ATTRS_SIZE);

    return success;
} /* End of lxmlTestPopulateXmlAttributeListWithDefaultAttributes */

/**
 * @brief Helper function that asserts the default values can be found in 'list'
 *
 * @param list - The 'XMLAttributeList' to search through
 * @return success - A flag indicating the status of the subroutine
 */
static int lxmlTestGetDefaultAttributeValuesFromAttributeList(struct XMLAttributeList list) {
    char *tmp = list.getAttributeValue(&list, "Fizz");
    assert(NULL != tmp);
    assert(0 == strcmp("Buzz", tmp));
    free(tmp); tmp = NULL;

    tmp = list.getAttributeValue(&list, "Hello");
    assert(NULL != tmp);
    assert(0 == strcmp("World", tmp));
    free(tmp); tmp = NULL;

    tmp = list.getAttributeValue(&list, "Foo");
    assert(NULL != tmp);
    assert(0 == strcmp("Bar", tmp));
    free(tmp); tmp = NULL;

    return TRUE;
} /* End of lxmlTestGetDefaultAttributeValuesFromAttributeList */

/**
 * @brief Helper function that asserts the default attributes can be found in 'list'
 *
 * @param list - The 'XMLAttributeList' to search through
 * @return success - A flag indicating the status of the subroutine
 */
static int lxmlTestGetDefaultAttributesFromAttributeList(struct XMLAttributeList list) {
    struct XMLAttribute *tmp = list.getAttribute(&list, "Fizz");
    assert(NULL != tmp);
    assert(0 == strcmp("Fizz", tmp->key));
    assert(0 == strcmp("Buzz", tmp->value));

    tmp = list.getAttribute(&list, "Hello");
    assert(NULL != tmp);
    assert(0 == strcmp("Hello", tmp->key));
    assert(0 == strcmp("World", tmp->value));

    tmp = list.getAttribute(&list, "Foo");
    assert(NULL != tmp);
    assert(0 == strcmp("Foo", tmp->key));
    assert(0 == strcmp("Bar", tmp->value));

    return TRUE;
} /* End of lxmlTestGetDefaultAttributesFromAttributeList */

/**
 * @brief Helper function to free the 'attrs'
 *
 * @param attrs - An array of 'attr' pointers which have been 'calloc'
 * @param attrsSize - The size of the array
 */
static void lxmlTestFreeAttrs(struct XMLAttribute **attrs, size_t attrsSize) {
    if (NULL != attrs) {
        size_t i = 0;
        for (;i<attrsSize;++i) {
            if (NULL != attrs[i]) {
                attrs[i]->free(attrs[i]);
                free(attrs[i]);
                attrs[i] = NULL;
            }
        }
    }
} /* End of lxmlTestFreeAttrs */

static int lxmlTestParseAttributesPass() {
    char *testXml = TEST_XML_HEADER;
    char testLex[TEST_BUF+1] = { 0 };
    size_t i = 0, lexi = 0;
    enum TagType tagType = TAG_UNSUPPORTED;

    struct XMLNode *node = XMLNode_init();

    strcpy(testLex, testXml);
    assert(NULL != node);

    tagType = lxmlParseAttrs(testXml, &i, testLex, &lexi, node);

    assert(TAG_START == tagType);

    assert(0 == strcmp(node->tag, TEST_XML_TAG));
    assert(NULL == node->inner_text);

    assert(2 == node->attributes.size);
    assert(0 == strcmp(node->attributes.attribute[0]->key, "version"));
    assert(0 == strcmp(node->attributes.attribute[0]->value, "1.0"));
    assert(0 == strcmp(node->attributes.attribute[1]->key, "encoding"));
    assert(0 == strcmp(node->attributes.attribute[1]->value, "UTF-8"));

    XMLNode_free(node);
    free(node);
    node = NULL;

    return TRUE;
} /* End of lxmlTestParseAttributesPass */

static int lxmlTestParseAttributes() {
    int success = FALSE;

    success = lxmlTestParseAttributesPass();

    printf("lxmlTestParseAttributes: %s\n", (TRUE == success) ? "Pass" : "Fail");

    return success;
} /* End of lxmlTestParseAttributes */

static int lxmlTestEndsWithPass() {
    assert(TRUE == lxmlEndsWith(TEST_HAYSTACK_1, TEST_NEEDLE_1));
    return TRUE;
} /* End of lxmlTestEndsWithPass */

static int lxmlTestEndsWithFail() {
    assert(FALSE == lxmlEndsWith(TEST_HAYSTACK_1, TEST_NEEDLE_INVALID));
    return TRUE;
} /* End of lxmlTestEndsWithFail */

static int lxmlTestEndsWithFail2() {
    assert(FALSE == lxmlEndsWith(TEST_HAYSTACK_1, TEST_NEEDLE_INVALID2));
    return TRUE;
} /* End of lxmlTestEndsWithFail2 */

static int lxmlTestEndsWithFailHaystackLargerThanNeedle() {
    assert(FALSE == lxmlEndsWith(TEST_NEEDLE_1, TEST_HAYSTACK_1));
    return TRUE;
} /* End of lxmlTestEndsWithFailHaystackLargerThanNeedle */

static int lxmlTestEndsWithFailNullHaystack() {
    assert(FALSE == lxmlEndsWith(NULL, TEST_NEEDLE_1));
    return TRUE;
} /* End of lxmlTestEndsWithFailNullHaystack */

static int lxmlTestEndsWithFailNullNeedle() {
    assert(FALSE == lxmlEndsWith(TEST_HAYSTACK_1, NULL));
    return TRUE;
} /* End of lxmlTestEndsWithFailNullNeedle */

static int lxmlTestEndsWith() {
    int success = lxmlTestEndsWithPass();

    success &= lxmlTestEndsWithFail();
    success &= lxmlTestEndsWithFail2();
    success &= lxmlTestEndsWithFailHaystackLargerThanNeedle();
    success &= lxmlTestEndsWithFailNullHaystack();

    success &= lxmlTestEndsWithFailNullNeedle();

    printf("lxmlTestEndsWith: %s\n", (TRUE == success) ? "Pass" : "Fail");

    return success;
} /* End of lxmlTestEndsWith */

static int lxmlTestAttributeInit() {
    struct XMLAttribute *attr = XMLAttribute_init("Hello", "World");

    assert(NULL != attr);
    assert(NULL != attr->free);

    assert(0 == strcmp(attr->key, "Hello"));
    assert(0 == strcmp(attr->value, "World"));

    attr->free(attr);
    free(attr);
    attr = NULL;

    return TRUE;
} /* End of lxmlTestAttributeInit */

static int lxmlTestAttributeListInit() {
    struct XMLAttributeList list = XMLAttributeList_init();

    assert(0 == list.size);
    assert(0 == list.heapSize);

    assert(NULL != list.add);
    assert(NULL != list.free);

    return TRUE;
} /* End of lxmlTestAttributeListInit */

static int lxmlTestAttributeListAdd() {
    int success = FALSE;

    struct XMLAttributeList list = XMLAttributeList_init();
    struct XMLAttribute *attr = XMLAttribute_init("Hello", "World");

    assert(NULL != attr);

    success = list.add(&list, *attr);

    assert(TRUE == success);
    assert(1 == list.size);
    assert(1 == list.heapSize);

    assert(0 == strcmp(list.attribute[0]->key, "Hello"));
    assert(0 == strcmp(list.attribute[0]->value, "World"));

    list.free(&list);
    attr->free(attr);
    free(attr);
    attr = NULL;

    return success;
} /* End of lxmlTestAttributeListAdd */

static int lxmlTestAttributeListGetAttributeValue() {
    int success = FALSE;

    struct XMLAttributeList list = XMLAttributeList_init();

    success = lxmlTestPopulateXmlAttributeListWithDefaultAttributes(&list);
    assert(TRUE == success);

    success &= lxmlTestGetDefaultAttributeValuesFromAttributeList(list);
    assert(TRUE == success);

    list.free(&list);

    return success;
} /* End of lxmlTestAttributeListGetAttributeValue */

static int lxmlTestAttributeListGetAttributeValueMissing() {
    int success = FALSE;

    struct XMLAttributeList list = XMLAttributeList_init();
    char *tmp = NULL;
    
    success = lxmlTestPopulateXmlAttributeListWithDefaultAttributes(&list);
    assert(TRUE == success);

    tmp = list.getAttributeValue(&list, "Fish");
    assert(NULL == tmp);

    list.free(&list);

    return success;
} /* End of lxmlTestAttributeListGetAttributeValueMissing */

static int lxmlTestAttributeListGetAttribute() {
    int success = FALSE;

    struct XMLAttributeList list = XMLAttributeList_init();
    
    success = lxmlTestPopulateXmlAttributeListWithDefaultAttributes(&list);
    assert(TRUE == success);

    success &= lxmlTestGetDefaultAttributesFromAttributeList(list);
    assert(TRUE == success);

    list.free(&list);

    return success;
} /* End of lxmlTestAttributeListGetAttribute */

static int lxmlTestAttributeListGetAttributeMissing() {
    int success = FALSE;

    struct XMLAttributeList list = XMLAttributeList_init();
    struct XMLAttribute *tmp = NULL;
    
    success = lxmlTestPopulateXmlAttributeListWithDefaultAttributes(&list);
    
    assert(TRUE == success);

    tmp = list.getAttribute(&list, "Fish");
    assert(NULL == tmp);

    list.free(&list);

    return success;
} /* End of lxmlTestAttributeListGetAttributeMisisng */

static int lxmlTestAttributeList() {
    int success = lxmlTestAttributeListInit(); 

    success &= lxmlTestAttributeListAdd();
    success &= lxmlTestAttributeListGetAttributeValue();
    success &= lxmlTestAttributeListGetAttributeValueMissing();
    success &= lxmlTestAttributeListGetAttribute();

    success &= lxmlTestAttributeListGetAttributeMissing();

    printf("lxmlTestAttributeList: %s\n", (TRUE == success) ? "Pass" : "Fail");

    return success;
} /* End of lxmlTestAttributeList */

static int lxmlTestNodeInit() {
    struct XMLNode *node = XMLNode_init();

    assert(NULL != node);

    assert(0    == node->children.heapSize);
    assert(0    == node->children.size);
    assert(NULL == node->children.data);

    assert(NULL != node->children.add);
    assert(NULL != node->children.free);

    assert(0    == node->attributes.heapSize);
    assert(0    == node->attributes.size);
    assert(NULL == node->attributes.attribute);

    assert(NULL != node->attributes.add);
    assert(NULL != node->attributes.free);
    assert(NULL != node->attributes.getAttribute);
    assert(NULL != node->attributes.getAttributeValue);

    assert(NULL != node->add);
    assert(NULL != node->free);
    assert(NULL != node->getAttribute);
    assert(NULL != node->getAttributeValue);

    node->free(node);
    free(node);
    node = NULL;

    return TRUE;
} /* End of lxmlTestNodeInit */

static int lxmlTestNodeAdd() {
    struct XMLNode *node  = XMLNode_init(),
                   *child = XMLNode_init();

    assert(NULL != node && NULL != child);

    node->add(node, child);

    assert(NULL == node->parent);
    assert(node == child->parent);

    node->free(node);
    free(node);
    node = child = NULL;
    
    return TRUE;
} /* End of lxmlTestNodeAdd */

static int lxmlTestNodeGetImmediateElementByTagName() {
    struct XMLNode *tree = tlxmlCreateTestXMLNodeTree();

    assert(NULL != tree);

    {
        struct XMLNode *topNode    = tree->getImmediateElementByTagName(tree, "top"),
                       *middleNode = tree->getImmediateElementByTagName(tree, "middle"),
                       *bottomNode = tree->getImmediateElementByTagName(tree, "bottom");

        assert(NULL != topNode && NULL != middleNode && NULL != bottomNode);

        assert(0 == strcmp("top",    topNode->tag));
        assert(0 == strcmp("middle", middleNode->tag));
        assert(0 == strcmp("bottom", bottomNode->tag));
    }

    tree->free(tree);
    free(tree);
    tree = NULL;

    return TRUE;
} /* End of lxmlTestNodeGetImmediateElementByTagName */

static int lxmlTestNodeGetImmediateElementByTagNameNodeNotFound() {
    struct XMLNode *tree = tlxmlCreateTestXMLNodeTree();

    assert(NULL != tree);
    assert(NULL == tree->getImmediateElementByTagName(tree, "upper"));

    tree->free(tree);
    free(tree);
    tree = NULL;

    return TRUE;
} /* End of lxmlTestNodeGetImmediateElementByTagNameNodeNotFound */

static int lxmlTestNodeGetAttributeValues() {
    int success = FALSE;

    struct XMLNode *node = XMLNode_init();
    assert(NULL != node);

    success = lxmlTestPopulateXmlAttributeListWithDefaultAttributes(&node->attributes);
    assert(TRUE == success);

    success &= lxmlTestGetDefaultAttributeValuesFromAttributeList(node->attributes);
    assert(TRUE == success);

    node->free(node);
    free(node);
    node = NULL;

    return success;
} /* End of lxmlTestNodeGetAttributeValues */

static int lxmlTestNodeGetAttributes() {
    int success = FALSE;

    struct XMLNode *node = XMLNode_init();
    assert(NULL != node);

    success = lxmlTestPopulateXmlAttributeListWithDefaultAttributes(&node->attributes);
    assert(TRUE == success);

    success &= lxmlTestGetDefaultAttributesFromAttributeList(node->attributes);
    assert(TRUE == success);

    node->free(node);
    free(node);
    node = NULL;

    return success;
} /* End of lxmlTestNodeGetAttributes */

static int lxmlTestNode() {
    int success = lxmlTestNodeInit();

    success &= lxmlTestNodeAdd();
    success &= lxmlTestNodeGetAttributeValues();
    success &= lxmlTestNodeGetAttributes();
    success &= lxmlTestNodeGetImmediateElementByTagName();

    success &= lxmlTestNodeGetImmediateElementByTagNameNodeNotFound();

    printf("lxmlTestNode: %s\n", (TRUE == success) ? "Pass" : "Fail");

    return success;
} /* End of lxmlTestNode */

static int lxmlTestNodeListInit() {
    struct XMLNodeList list = XMLNodeList_init();

    assert(0    == list.heapSize);
    assert(0    == list.size);
    assert(NULL == list.data);

    assert(NULL != list.add);
    assert(NULL != list.free);

    return TRUE;
} /* End of lxmlTestNodeListInit */

static int lxmlTestNodeListAdd() {
    int success = FALSE;
    struct XMLNodeList list = XMLNodeList_init();
    struct XMLNode *child = XMLNode_init();

    assert(NULL != child);
    success = list.add(&list, NULL, child);
    assert(TRUE == success);

    assert(1 == list.size && 1 == list.heapSize);
    assert(list.data[0] == child);

    list.free(&list);
    child = NULL;

    return success;
} /* End of lxmlTestNodeListAdd */

static int lxmlTestNodeList() {
    int success = lxmlTestNodeListInit();
    success &= lxmlTestNodeListAdd();

    printf("lxmlTestNodeList: %s\n", (TRUE == success) ? "Pass" : "Fail");

    return success;
} /* End of lxmlTestNodeList */

int main() {
    int success = FALSE;

    success = lxmlTestAttributeInit();
    success &= lxmlTestAttributeList();

    success &= lxmlTestNode();
    success &= lxmlTestNodeList();

    success &= lxmlTestParseAttributes();
    success &= lxmlTestEndsWith();

    #ifdef LXML_HAVE_FMEMOPEN
    success &= lxmlTestReadXmlContentsIntoMemory();
    success &= lxmlTestXMLDocument_load();
    #endif

    return (TRUE == success) ? 0 : 1;
} /* End of main */
