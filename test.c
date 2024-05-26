#include "lxml.h"

int main()
{
    XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml")) {
        char fields_buff[1024] = { 0 };
        XMLNode* str = XMLNode_child(doc.root, 0);

        XMLNodeList* fields = XMLNode_children(str, "field");
        for (int i = 0; i < fields->size; i++) {
            XMLNode* field = XMLNodeList_at(fields, i);
            XMLAttribute* type = XMLNode_attr(field, "type");
            snprintf(fields_buff, 1024, "%s %s\n", type->key, type->value);
        }

        free(fields->data);
        fields->data = NULL;

        free(fields);
        fields = NULL;

        XMLDocument_write(&doc, "out.xml", 4);
        XMLDocument_free(&doc);
    }

    return 0;
}
