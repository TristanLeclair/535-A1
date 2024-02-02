# Protocol for our services

## Messages

We have to send multiple types of messages

### NOTIFICATION

Sends its own information on the network.

Format : `NOTIFICATION_code#service_name#attribute_key_1;attribute_value_1#attribute_key_2;attribute_value_2#`

### DISCOVERY

Tells every service on the network to send its NOTIFICATION message

Format : `DISCOVERY_code#`

### HEARTBEAT

Format : `HEARTBEAT_code#service_name#`

### AD

Format : `AD_code#service_name#ad_name#ad_value#`
