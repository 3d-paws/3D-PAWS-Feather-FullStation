# LoRaWAN Station Information (INFO)
[←Top](../README.md)<BR>

Station Information is generated, stored on the SD card and transmitted with every boot and then every 6 hours from boot.

INFO messages are handle differently between WiFi and LoRaWAN.

### LoRaWAN INFO
When using LoRaWAN option, observations and INFO messages are sent via LoRaWAN. To a LoRaWAN gateway tied to a Cloud Service provider. Then forwarded by the gateway to the Cloud Service provider. For example The Things Network (TTN). The provider receives the messages and then applies a webhook to forward the message to it's logging destination.

#### TTN Web Hook
At TTN under Under Application->Yor Application->Webhooks A webhook is defined to forward messages to a logging destination. The webhook is set with the following parameters. See [Add a Webhook to TTN](AddWebhookToTTN.md).
```
Webhook ID: lwmsg-to-website
Webhook format: JSON
Base URL: https://some.domain.com
Additional headers: x-api-key, YOUR_API_KEY_1234
Enabled event type: Uplink message, /ttn/relay.php
```
#### Website Message Processing
Here is example PHP code that runs on the website receiving LoRaWAN messages from the provider. If it is a INFO message it will log it to the INFO log file. If it is a observation it will log and forward the observation to a CHORDS site.
```php
# cat relay.php
<?php
  //
  // relay.php - Receive LoRaWAN messages from the provider.
  //             If it is a INFO message it will log it to the INFO log file.
  //             If it is a observation it will log and forward the observation to a CHORDS site
  //             Use file to lookup up lora_device_addr to obtain forwarding information to Chords
  //
  $top = '/usr/local/www/some.domain.com/ttn';
  $infolog = $top . '/info.log';
  $relaylog = $top . '/relay.log';
  $relaydb = $top . '/relay.db';

  // SEE API keygen at https://api-key.me/index
  if ( (!isset($_SERVER['HTTP_X_API_KEY'])) ||
       ($_SERVER['HTTP_X_API_KEY'] != 'YOUR_API_KEY_1234')) {
    exit;
  }
  if (! $entityBody = file_get_contents('php://input')) {
    exit;
  }

  $json = json_decode($entityBody);

  if (!isset($json->uplink_message->frm_payload)) {
    exit;
  }

  if (! $relay = file_get_contents($relaydb)) {
    exit;
  }

  // https://www.thethingsnetwork.org/docs/lorawan/rssi-and-snr/
  $payload          = $json->uplink_message->frm_payload;
  $rssi             = $json->uplink_message->rx_metadata[0]->rssi;
  $snr              = $json->uplink_message->rx_metadata[0]->snr;
  $gateway_eui      = $json->uplink_message->rx_metadata[0]->gateway_ids->eui;
  $lora_device_addr = $json->end_device_ids->dev_eui;
  $payload_decoded  = base64_decode($payload);

  // Is this a INFO "MT":"INFO" if so save in INFO file
  if (str_contains($payload_decoded, '"MT":"INFO"')) {
   $out = sprintf(
    "{\"tm\":\"%s\",\"gweui\":\"%s\",\"deveui\":\"%s\",\"rssi\":%s,\"snr\":%s,\"info\":\"%s\"}\n",
     gmdate('Y-m-d h:i:s \G\M\T', $_SERVER['REQUEST_TIME']),
     $gateway_eui,
     $lora_device_addr,
     $rssi,
     $snr,
     $payload_decoded);

    file_put_contents($infolog, $out, FILE_APPEND);
  }
  else {
    // Forward to Chords via database table lookup

    foreach(preg_split("/((\r?\n)|(\r\n?))/", $relay) as $line){
      $line = str_replace(' ', '', $line);

      list ($relay_device_id, $relay_device_url) = preg_split('/\,/', $line);
      if ($lora_device_addr == $relay_device_id) {
        if ($relay_device_url) {
          // Relay observation to Chords
          $url = sprintf("%s&%s&rssi=%s&snr=%s", $relay_device_url, $payload_decoded, $rssi, $snr);
          $result = file_get_contents($url);

          // Log the observation locally
          $out = sprintf("%s,%s,%s,%s,%s\n",
            gmdate('Y-m-d h:i:s \G\M\T', $_SERVER['REQUEST_TIME']),
            $gateway_eui,
            $relay_device_id,
            $url,
            $result);
          file_put_contents($relaylog, $out, FILE_APPEND);
        }
        else {
          $out = sprintf("%s,%s,NO PAYLOAD,%s\n",
            gmdate('Y-m-d h:i:s \G\M\T', $_SERVER['REQUEST_TIME']),
            $relay_device_id, $entityBody);
          file_put_contents($relaylog, $out, FILE_APPEND);
        }
        break;
      }
    }
  }
?>
```

Here is the format of the relay.db file that is read by the above program. The relay.db file is used to map a LoRaWAN device ID to a Chords site and instrument_id.
```
70B3D522D0FFFBCB,http://3d.some_chords_site.com/measurements/url_create?key=CHORDSAPIKEY&instrument_id=1
```

The INFO and RELAY logfiles on the webserver host will need permission set so the webserver can write to the file. For example, notice the group bits are read-write and the group is apache. The web server runs with group apache.
```
-rw-rw-r-- 1 root apache 0 Feb 10 12:17 /usr/local/www/some.domain.com/ttn/info.log
-rw-rw-r-- 1 root apache 0 Feb 10 12:17 /usr/local/www/some.domain.com/ttn/relay.log
-rw-r----- 1 root apache 0 Feb  5 14:00 /usr/local/www/some.domain.com/ttn/relay.db
```

#### LoRaWAN INFO message. 
Notice the INFO message is 3 seperate messages. There is a LoRa transmission message length limit. We stay under 222 bytes per message. We also leave 30 seconds between each transmission for LoRa far use.
```json
{
  "tm": "2026-02-10 05:29:33 GMT",
  "gweui": "B827EBFFFED358D4",
  "deveui": "70B3D57ED0075295",
  "rssi": -51,
  "snr": 7.5,
  "info": {
    "MT": "INFO",
    "at": "2026-02-10T17:29:48",
    "devid": "330eff6367815b7d93bfbcec",
    "board": "AFM0LoRa",
    "ver": "FSLW-260209",
    "bv": 17.11,
    "hth": 1,
    "elev": 0,
    "obsi": "1m",
    "t2nt": "4294907s",
    "drbt": "22m",
    "n2s": "NF"
  }
}
{
  "tm": "2026-02-10 05:30:08 GMT",
  "gweui": "B827EBFFFED358D4",
  "deveui": "70B3D57ED0075295",
  "rssi": -54,
  "snr": 10,
  "info": {
    "MT": "INFO",
    "LW": "EU868,OTAA,7,14",
    "devs": "sd"
  }
}
{
  "tm": "2026-02-10 05:30:33 GMT",
  "gweui": "B827EBFFFED358D4",
  "deveui": "70B3D57ED0075295",
  "rssi": -54,
  "snr": 10.2,
  "info": {
    "MT": "INFO",
    "sensors": "MCP1,SHT1,SI,HI,WBT,WBGT WO/GLOBE,RG1(A3),5MDIST(A4,0),VBV(A5)"
  }
}
```

