#include <unistd.h>
#include "zcs/zcs.h"

int main() {
    zcs_init(ZCS_SERVICE_TYPE);
    zcs_attribute_t attribs[] = {
	    { .attr_name = "type", .value = "speaker"},
	    { .attr_name = "location", .value = "livingroom"},
	    { .attr_name = "make", .value = "yamaha"} };
    zcs_start("speaker-Z", attribs, sizeof(attribs)/sizeof(zcs_attribute_t));
    for (int i = 0; i < 1000; i++) {
        zcs_post_ad("mute", "on");
        sleep(10);
        zcs_post_ad("mute", "off");
        sleep(10);
    }
    zcs_shutdown();
    // zcs_log();
}

