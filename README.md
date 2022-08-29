# A ArduinoJson Study
- Using ArduinoJson to read Reddit API
- Base on the corresponding "Case study" in the book "Mastering ArduinoJson"

## Improvements:
- Only using Arduino Core's `client` class --> More configuration flexible and less memory usage in compare to the `httpClient` class
- Minimize heap usage (no String)
- Clean code 