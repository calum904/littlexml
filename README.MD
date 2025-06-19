# Little XML

A fork of [jonahisadev](https://github.com/jonahisadev/littlexml)'s LittleXML.

## Example Program

An example program can be seen below:

```c
#include "lxml.h"

int main()
{
    FILE *fp = fopen("test.xml", "r");

    if (NULL != fp) {
        struct XMLDocument doc = XMLDocument_load(fp);

        if (TRUE == doc.success) {
            printf("Version %s encoding %s\n", doc.version, doc.encoding);

            printf("Root: %p %s %ld\n", doc.root, doc.root->tag, doc.root->children.size);
            if (NULL != doc.root) {
                struct XMLAttribute *attr = doc.root->getAttribute(doc.root, "name");
                char *attrVal = doc.root->getAttributeValue(doc.root, "name");

                if (NULL != attr)
                    printf("Got 'name' attribute %s %s\n", attr->key, attr->value);

                if (NULL != attrVal) {
                    printf("Got 'name' %s\n", attrVal);
                    free(attrVal);
                    attrVal = NULL;
                }
            }

            {
                FILE *out = fopen("out.xml", "w");

                if (NULL != out) {
                    XMLDocument_write(&doc, out, 4);
                    fclose(out);
                    out = NULL;
                } else
                    printf("Error opening output\n");
            }

            doc.free(&doc);
        } else
            printf("Error reading XML\n");

        fclose(fp);
        fp = NULL;
    }

    return 0;
}
```

Can be run with the supplied `test.xml`.

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<struct name="Person">
    <field name="name" type="string" />
    <field name="age" type="int" />
    <description>This defines a person</description>
</struct>
```

To run tests:

```bash
cd test
make
./testmain
```
