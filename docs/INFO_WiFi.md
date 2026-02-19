# WiFi Station Information (INFO)
[←Top](../README.md)<BR>

Station Information is generated, stored on the SD card and transmitted with every boot and then every 6 hours from boot.

INFO messages are handle differently between WiFi and LoRaWAN.

### WiFi INFO
CHORDS can not accept INFO messages.  An alternative site is used to log the messages.  Configuration for this site is setup in the CONFIG.TXT file. Here are those configuration settings.

```
# Information web server, Not Chords
info_server=some.domain.com
info_server_port=443
info_urlpath=/info/log.php
# Will be added to the HTML Header as X-API-Key
info_apikey=1234
```
Here is example PHP code that runs on the website receiving INFO messages and appending them in a log file.
```php
<?php
//
// log.php - log validated infomessages to info log file
//

// Log file path
$logFile = '/usr/local/www/some.domain.com/info/info.log';

// Expected API key
$expectedApiKey = 'YOUR_API_KEY_1234';

// Check for the HTTP_X_API_KEY header
if (!isset($_SERVER['HTTP_X_API_KEY']) || $_SERVER['HTTP_X_API_KEY'] !== $expectedApiKey) {
    // If the API key is missing or incorrect, return a 403 Forbidden error
    http_response_code(403);
    echo "Error: Forbidden. Invalid API key.";
    exit;
}

// Check if there is POST data
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Get the POST data
    $postData = file_get_contents("php://input");

    // Check if data is successfully retrieved
    if ($postData) {
        $logEntry = "[" . date("Y-m-d H:i:s") . "] " . $postData . "\n";

        // Try to write the log entry to the file
        if (file_put_contents($logFile, $logEntry, FILE_APPEND | LOCK_EX) === false) {
            // If there was an error writing to the file, return a 500 error
            http_response_code(500);
            echo "Error: Unable to write to log file.";
        } else {
            // Success
            echo "Log entry saved.";
        }
    } else {
        // If there was no data in the POST request, return a 400 error
        http_response_code(400);
        echo "Error: No data received.";
    }
} else {
    // If not a POST request, return a 405 Method Not Allowed error
    http_response_code(405);
    echo "Error: Only POST requests are allowed.";
}
?>
```

The INFO logfile on the webserver host will need permission set so the webserver can write to the file. For example, notice the group bits are read-write and the group is apache. The web server runs with group apache.
```
-rw-rw-r-- 1 root apache 0 Feb 10 12:17 /usr/local/www/some.domain.com/info/info.log
```
#### WiFi INFO message
```JSON
{
  "MT": "INFO",
  "at": "2026-02-13T00:37:11",
  "devid": "73eee600304d98c9fc7fc5f4",
  "board": "AFM0WiFi",
  "ver": "FSLW-260211",
  "bv": 17.35,
  "hth": 1,
  "elev": 0,
  "ls": "3d.chordsrt.com",
  "lsp": 80,
  "lsurl": "/measurements/url_create",
  "lsapi": "21DE6A8A",
  "lsid": 53,
  "ntp": "pool.ntp.org",
  "ssid": "-",
  "obsi": "1m",
  "t2nt": "4294907s",
  "drbt": "22m",
  "n2s": "NF",
  "devs": "sd,gps",
  "wifi": {
    "fw": "19.5.2",
    "ssid": "-",
    "auth": "WPA/2",
    "ip": "10.237.128.215",
    "rssi": -58
  },
  "lat": 41.072108,
  "lon": -102.034435,
  "alt": 1551.6,
  "sat": 13,
  "sensors": "MCP1,SHT1,SI,AS5600,WS(A2),HI,WBT,WBGT WO/GLOBE,RG1(A3),5MDIST(A4,0),VBV(A5)"
}
```