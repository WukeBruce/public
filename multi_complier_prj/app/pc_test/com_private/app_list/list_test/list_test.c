
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "list.h"
#include "debug.h"

/**
class struct

*/

#define ARRAY_SIZE_COM(n)	(sizeof(n)/sizeof(n[0]))
#define OFFSETOF_COM(type,member)	((size_t)&(((type *)0)->member))

#define container_of(ptr, type, member) ({          \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - OFFSETOF_COM(type,member) );})


typedef struct _Foo {
	int data_;
	int data_1;
	char data_2;
	int data_3;
    struct gxlist_head link;
} Foo;

static GX_LIST_HEAD(test_link);

int list_test(void) {

    Foo *p_foo;
	Foo *pos;

	int i=0;

    for (i=0; i<5; i++)
    {
         p_foo = (Foo*)malloc(sizeof(Foo));
		 COM_INFO("p_foo addr = %p \n",p_foo);

         p_foo->data_ = (i+1)*10;
		 gxlist_add_tail(&(p_foo->link), &test_link);
    }

	COM_INFO("sizeof(unsigned long) = %d\n",sizeof(unsigned long));
	COM_INFO("Foo offset(data_2) = %d\n",OFFSETOF_COM(Foo,data_2));
	COM_INFO("p_foo addr = %p \n",container_of(&(p_foo->data_2), Foo, data_2));

    gxlist_for_each_entry(pos, &test_link, link) {
        COM_INFO("%d\n", pos->data_);
    }

    return 0;
}

