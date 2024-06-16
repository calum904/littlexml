/**
 * @file extratests.h
 *
 * @version 1.0
 *
 * @author Calum Judd Anderson
 *
 * @brief Additional tests which are not 'C89' compliant
 */

#ifdef LXML_HAVE_FMEMOPEN
static int lxmlTestReadXmlContentsIntoMemory();
static int lxmlTestReadXmlContentsIntoMemoryPass();
static int lxmlTestReadXmlContentsIntoMemoryNullFp();

static int lxmlTestXMLDocument_load();
static int lxmlTestXmlDocumentLoadPass();

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
    struct XMLLoadContext ctx = XMLLoadContext_init(READ_XML_FILE, NULL);

    assert(NULL != fp);
    ctx.readFile.fp = fp;

    doc = XMLDocument_load(ctx);
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
